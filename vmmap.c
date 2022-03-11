// Brandon Azad (@_bazad)

#include <assert.h>
#include <errno.h>
#include <mach/mach.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#if defined(__x86_64__)
#include <libproc.h>
#include <mach/mach_vm.h>
#endif

#if defined(__arm64__)
#include "libproc.h"
#include "mach_vm.h"
#endif

static void
format_display_size(char buf[5], uint64_t size) {
	const char scale[] = { 'B', 'K', 'M', 'G', 'T', 'P', 'E' };
	double display_size = size;
	unsigned scale_index = 0;
	while (display_size >= 999.5) {
		display_size /= 1024;
		scale_index++;
	}
	assert(scale_index < sizeof(scale) / sizeof(scale[0]));
	int precision = 0;
	if (display_size < 9.95 && display_size - (float)((int)display_size) > 0) {
		precision = 1;
	}
	int len = snprintf(buf, 5, "%.*f%c", precision, display_size, scale[scale_index]);
	assert(len > 0 && len < 5);
}

static void
format_memory_protection(char buf[4], int prot) {
	int len = snprintf(buf, 4, "%c%c%c",
			(prot & VM_PROT_READ    ? 'r' : '-'),
			(prot & VM_PROT_WRITE   ? 'w' : '-'),
			(prot & VM_PROT_EXECUTE ? 'x' : '-'));
	assert(len == 3);
}

static const char *
share_mode_name(unsigned char share_mode) {
	switch (share_mode) {
		case SM_COW:                    return "COW";
		case SM_PRIVATE:                return "PRV";
		case SM_EMPTY:                  return "NUL";
		case SM_SHARED:                 return "ALI";
		case SM_TRUESHARED:             return "SHR";
		case SM_PRIVATE_ALIASED:        return "P/A";
		case SM_SHARED_ALIASED:         return "S/A";
		case SM_LARGE_PAGE:             return "LPG";
		default:                        return "???";
	}
}

static bool
vmmap(task_t task, uintptr_t start, uintptr_t end, uint32_t depth) {
	int pid = -1;
	pid_for_task(task, &pid);
	for (bool first = true;; first = false) {
		mach_vm_address_t address = start;
		mach_vm_size_t size = 0;
		uint32_t depth0 = depth;
		vm_region_submap_info_data_64_t info;
		mach_msg_type_number_t count = VM_REGION_SUBMAP_INFO_COUNT_64;
		kern_return_t kr = mach_vm_region_recurse(task, &address, &size,
				&depth0, (vm_region_recurse_info_t)&info, &count);
		if (kr != KERN_SUCCESS || address > end) {
			if (first) {
				if (start == end) {
					printf("no virtual memory region contains address %p\n",
							(void *)start);
				} else {
					printf("no virtual memory region intersects %p-%p\n",
							(void *)start, (void *)end);
				}
			}
			break;
		}
		if (first) {
			printf("          START - END             [ VSIZE ] "
					"PRT/MAX SHRMOD DEPTH RESIDENT REFCNT TAG"
					"    FILE\n");
		}
		char vsize[5];
		format_display_size(vsize, size);
		char cur_prot[4];
		format_memory_protection(cur_prot, info.protection);
		char max_prot[4];
		format_memory_protection(max_prot, info.max_protection);
		// Get the file name for this region.
		char filename[4 + 4096] = {};
		memset(filename, ' ', 4);
		errno = 0;
		int ret = proc_regionfilename(pid, address, filename + 4, sizeof(filename) - 4);
		if (ret <= 0 || errno != 0) {
			filename[0] = 0;
		}
		printf("%016llx-%016llx [ %5s ] %s/%s %6s %5u %8u %6u %3u%s\n",
				address, address + size,
				vsize,
				cur_prot, max_prot,
				share_mode_name(info.share_mode),
				depth0,
				info.pages_resident,
				info.ref_count,
				info.user_tag,
				filename);
		start = address + size;
	}
	return true;
}

int
main(int argc, const char *argv[]) {
	uint32_t depth = 2048;
	char *end = NULL;
	int argidx = 1;
	// Options
	for (;;) {
		if (argidx >= argc) {
			goto arguments;
		}
		// -d <depth>
		if (strcmp(argv[argidx], "-d") == 0) {
			argidx++;
			if (argidx >= argc) {
				goto usage;
			}
			uint64_t value = strtoul(argv[argidx], &end, 0);
			if (*end != 0 || value > UINT32_MAX) {
				goto usage;
			}
			depth = value;
		} else {
			goto arguments;
		}
		argidx++;
	}
usage:
	printf("usage: vmmap [-d <depth>] <pid>\n");
	return 1;
	// Arguments
arguments:
	if (argidx >= argc) {
		goto usage;
	}
	// pid
	uint64_t value = strtoul(argv[argidx], &end, 0);
	if (*end != 0 || value > 1000000) {
		goto usage;
	}
	int pid = value;
	argidx++;
	// End of arguments
	if (argidx != argc) {
		goto usage;
	}
	// Implementation.
	mach_port_t task = MACH_PORT_NULL;
	kern_return_t kr = task_for_pid(mach_task_self(), pid, &task);
	if (kr != KERN_SUCCESS) {
		printf("task_for_pid(%d) failed: %s\n", pid, mach_error_string(kr));
		return 1;
	}
	printf("Virtal Memory Map (depth=%u) for PID %d\n", depth, pid);
	vmmap(task, 0, -1, depth);
	return 0;
}
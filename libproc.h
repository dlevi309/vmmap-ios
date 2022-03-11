/* Copyright (c) 2021 daniel
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This Source Code Form is "Incompatible With Secondary Licenses", as
 * defined by the Mozilla Public License, v. 2.0.
**/

#ifndef _LIBPROC_H_
#define _LIBPROC_H_

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/mount.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <Availability.h>

#define PROC_LISTPIDSPATH_PATH_IS_VOLUME 1
#define PROC_LISTPIDSPATH_EXCLUDE_EVTONLY 2

__BEGIN_DECLS

int proc_listpidspath(uint32_t type,
    uint32_t typeinfo,
    const char *path,
    uint32_t pathflags,
    void *buffer,
    int buffersize);

int proc_listpids(uint32_t type, uint32_t typeinfo, void *buffer, int buffersize);
int proc_listallpids(void *buffer, int buffersize);
int proc_listpgrppids(pid_t pgrpid, void *buffer, int buffersize);
int proc_listchildpids(pid_t ppid, void *buffer, int buffersize);
int proc_pidinfo(int pid, int flavor, uint64_t arg, void *buffer, int buffersize);
int proc_pidfdinfo(int pid, int fd, int flavor, void *buffer, int buffersize);
int proc_pidfileportinfo(int pid, uint32_t fileport, int flavor, void *buffer, int buffersize);
int proc_name(int pid, void *buffer, uint32_t buffersize);
int proc_regionfilename(int pid, uint64_t address, void *buffer, uint32_t buffersize);
int proc_kmsgbuf(void *buffer, uint32_t buffersize);
int proc_pidpath(int pid, void *buffer, uint32_t buffersize);
int proc_libversion(int *major, int *minor);

int proc_pid_rusage(int pid, int flavor, rusage_info_t *buffer);

#define PROC_SETPC_NONE 0
#define PROC_SETPC_THROTTLEMEM 1
#define PROC_SETPC_SUSPEND 2
#define PROC_SETPC_TERMINATE 3

int proc_setpcontrol(const int control);
int proc_setpcontrol(const int control);

int proc_track_dirty(pid_t pid, uint32_t flags);
int proc_set_dirty(pid_t pid, bool dirty);
int proc_get_dirty(pid_t pid, uint32_t *flags);
int proc_clear_dirty(pid_t pid, uint32_t flags);

int proc_terminate(pid_t pid, int *sig);

#ifdef PRIVATE
#include <sys/event.h>

int proc_list_uptrs(pid_t pid, uint64_t *buffer, uint32_t buffersize);

int proc_list_dynkqueueids(int pid, kqueue_id_t *buf, uint32_t bufsz);
int proc_piddynkqueueinfo(int pid, int flavor, kqueue_id_t kq_id, void *buffer,
    int buffersize);

#endif /* PRIVATE */

int proc_udata_info(int pid, int flavor, void *buffer, int buffersize);

__END_DECLS

#endif /*_LIBPROC_H_ */

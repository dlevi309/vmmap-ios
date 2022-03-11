CC = xcrun -sdk iphoneos gcc -arch arm64
SIGN   := ldid -Sent.xml

all:
	$(CC) vmmap.c -fobjc-arc -o vmmap
	$(SIGN) vmmap

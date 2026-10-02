LDFLAGS =
FILE=$(file)
CFLAGS=$(cflags)
CFLAGS += -g -O0 -std=gnu11 -Wall -DPWD=\"$(PWD)\" -I/usr/include/glib-2.0/glib/ -I../ -I../../../kysdk-base/src/utils -I../../../kysdk-base/src/log/
CLIBS= -lblkid -lmount -L/home/kylin/git-local-storage/kysdk/kysdk-base/src/log/lib -Wl,-rpath=/home/kylin/git-local-storage/kysdk/kysdk-base/src/log/lib -lkylog -L/home/kylin/git-local-storage/kysdk/kysdk-base/src/config/lib -Wl,-rpath=/home/kylin/git-local-storage/kysdk/kysdk-base/src/config/lib -lkyconf
CLIBS += $(clibs)

all:getdiskinfo getdisklist

getdisklist:getdisklist.c ../diskinfo.c
	gcc -o $(basename $<) $< ../diskinfo.c $(CFLAGS) $(CLIBS)

getdiskinfo:getdiskinfo.c ../diskinfo.c
	gcc -o $(basename $<) $< ../diskinfo.c $(CFLAGS) $(CLIBS)

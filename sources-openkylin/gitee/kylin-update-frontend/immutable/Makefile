CC = g++

all: plugins notify newnotify send

plugins:
	cd plugin && qmake .
	cd plugin && make -j8
notify:
	cd notification/kylin-updateresult-notify && qmake .
	cd notification/kylin-updateresult-notify && make clean
	cd notification/kylin-updateresult-notify && make -j8
	cd notification/kylin-updateresult-notify && rm -f Makefile .qmake.stash
newnotify:
	cd upgradenote/RebootAttention && qmake .
	cd upgradenote/RebootAttention && make -j8
send:
	cd NotifySend && qmake .
	cd NotifySend && make -j8

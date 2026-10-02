CC = g++

all: plugins notify

plugins:
	cd plugin && qmake .
	cd plugin && make -j8
notify:
	cd notification && cmake .
	cd notification && make -j8

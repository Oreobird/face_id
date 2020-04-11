#!/bin/sh

NCPU="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 1)"

if [ ! -e build ];then
	mkdir build
fi

cp ./src/start_stop_srv.sh ./build

if [ ! -e log ];then
	mkdir log
fi

if [ ! -e video ];then
	mkdir video
fi

cd build
cmake ..
make -j $NCPU


#!/bin/bash
qmake .
make clean
make distclean
rm -rf kylin-ipmsg.pro.user


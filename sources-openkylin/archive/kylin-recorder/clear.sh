#!/bin/bash

make clean

rm -f kylin-recorder.pro.user \
    .qmake.stash \
    Makefile \
    src/Makefile \
    src/kylin-recorder \
    debian/debhelper-build-stamp \
    debian/files \
    debian/kylin-recorder.substvars \
    debian/README.Debian \
    debian/kylin-recorder.debhelper.log

rm -rf debian/.debhelper/ \
    debian/kylin-recorder \
    ./kylin-recorder \
    ./kylin-recorder.pro.user* \
    ./*.qm

#!/bin/bash

make clean

rm -f kylin_camera.pro.user \
    .qmake.stash \
    Makefile \
    src/Makefile \
    src/kylin-camera \
    debian/debhelper-build-stamp \
    debian/files \
    debian/kylin-camera.substvars \
    debian/README.Debian \
    debian/kylin-camera.debhelper.log

rm -rf debian/.debhelper/ \
    debian/kylin-camera \
    ./kylin-camera \
    ./kylin_camera.pro.user* \
    ./*.qm

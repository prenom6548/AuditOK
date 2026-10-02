#!/bin/bash

make clean

rm -rf kylin-video.pro.user \
    debian/files \
    .qmake.stash \
    Makefile \
    debian/.debhelper/ \
    debian/debhelper-build-stamp \
    debian/kylin-video.debhelper.log \
    debian/kylin-video.substvars \
    debian/kylin-video/ \
    kylin-video

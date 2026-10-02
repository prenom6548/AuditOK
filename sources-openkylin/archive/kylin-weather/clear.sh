#!/usr/bin/env bash
# -* - coding: UTF-8 -* -

make clean

rm -r debian/.debhelper/ \
	debian/files \
	debian/kylin-weather/  \
    debian/kylin-weather.debhelper.log \
	debian/kylin-weather.substvars \
	kylin-weather \
    translations/*.qm

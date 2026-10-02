QT += testlib

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline
QMAKE_CFLAGS += --coverage -fno-inline


CONFIG += link_pkgconfig
PKGCONFIG += dbus-1 kysdk-log openssl

LIBS += -lcjson -lgcov


SOURCES +=  \
    ../../kysdk-datacollect/src/libkydatacollect.c  \
    datacollecttest.cpp

HEADERS += \
    ../../kysdk-datacollect/src/libkydatacollect.h

#file:///home/kylin/kylinos-src-fork/libkysdk-diagnosetest/
#file:///home/kylin/kylinos-src-fork/libkysdk-diagnosetest/

TEMPLATE = app
TARGET = test_customizesystembackupproxy
CONFIG += testcase

QT += core widgets testlib dbus xml

QMAKE_CXXFLAGS +=  -fno-exceptions

INCLUDEPATH += ../../.. ../../../systembackup-daemon ../../../systembackup-daemon/myprocess ../../../common

SOURCES += \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.cpp \
    main.cpp \
    test_customizesystembackupproxy.cpp \
    ../../../systembackup-daemon/customizesystembackupproxy.cpp \
    ../../../systembackup-daemon/workerfactory.cpp \
    ../../../systembackup-daemon/myprocess/calcbackupsize.cpp \
    ../../../systembackup-daemon/myprocess/mksquashfsprocess.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp \
    ../../../systembackup-daemon/mymountproxy.cpp \
    ../../../common/reflect.cpp \
    ../../../common/utils.cpp \
    ../../../common/mydusizetool.cpp \
    ../../../common/mylittleparse.cpp \
    ../../../common/mydefine.cpp

HEADERS += \
    ../../../systembackup-daemon/myprocess/mountbackupprocess.h \
    test_customizesystembackupproxy.h \
    ../../../systembackup-daemon/customizesystembackupproxy.h \
    ../../../systembackup-daemon/workerfactory.h \
    ../../../systembackup-daemon/myprocess/calcbackupsize.h \
    ../../../systembackup-daemon/myprocess/mksquashfsprocess.h \
    ../../../systembackup-daemon/parsebackuplist.h \
    ../../../systembackup-daemon/mymountproxy.h \
    ../../../common/reflect.h \
    ../../../common/utils.h \
    ../../../common/mydusizetool.h \
    ../../../common/mylittleparse.h \
    ../../../common/mydefine.h

QMAKE_CXXFLAGS += --coverage -fpermissive
QMAKE_LFLAGS += --coverage -lgcov
DEFINES += UNIT_TEST
LIBS += -lQt5Test -lQt5Core -lQt5Widgets -lQt5DBus -lQt5Xml -lgcov

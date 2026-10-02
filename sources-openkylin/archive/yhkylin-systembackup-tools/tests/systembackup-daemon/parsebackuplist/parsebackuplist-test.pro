TEMPLATE = app
TARGET = test_parsebackuplist

QT += core xml testlib dbus

QMAKE_CXXFLAGS +=  -fno-exceptions

SOURCES += \
    test_parsebackuplist.cpp \
    main.cpp \
    ../../../systembackup-daemon/parsebackuplist.cpp \
    ../../../common/mydefine.cpp

HEADERS += \
    test_parsebackuplist.h \
    ../../../systembackup-daemon/parsebackuplist.h \
    ../../../common/mydefine.h

INCLUDEPATH += \
    ../../../systembackup-daemon \
    ../../../common

LIBS += -lQt5Core -lQt5Xml -lQt5Test -lQt5DBus

# Coverage flags
QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov

TEMPLATE = app
TARGET = test_ostreeinterface

QT -= gui
QT += core testlib dbus xml

QMAKE_CXXFLAGS +=  -fno-exceptions

INCLUDEPATH += . \
    ../../.. \
    ../../../systembackup-daemon \
    ../../../systembackup-daemon/myprocess \
    ../../../common

SOURCES += main.cpp \
           ../../../common/mylittleparse.cpp \
           ../../../systembackup-daemon/parsebackuplist.cpp \
           test_ostreeinterface.cpp \
           ../../../systembackup-daemon/myprocess/OstreeInterface.cpp \
           ../../../common/utils.cpp

HEADERS += test_ostreeinterface.h \
           ../../../common/mylittleparse.h \
           ../../../systembackup-daemon/myprocess/OstreeInterface.h \
           ../../../common/utils.h \
           ../../../systembackup-daemon/parsebackuplist.h

QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage

LIBS += -lQt5Core -lQt5Test -lgcov

DEFINES += UNIT_TEST

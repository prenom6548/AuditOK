TARGET = tests

QT += core testlib dbus xml sensors concurrent

LIBS +=

PKGCONFIG += kysdk-sysinfo
TEMPLATE = app
CONFIG += c++11 console
CONFIG -= app_bundle
CONFIG += link_pkgconfig
LIBS += -lgcov  -lukui-log4qt -lukuiinputgatherclient

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage

#INCLUDEPATH += \
#          $$PWD/../DbusManager    \
#          $$PWD/../ConfigurationManager  \
#          $$PWD/../KmreAppManager
#          $$PWD/../keyboardmanager  \
#          $$PWD/../DeviceModeManager  \
#          $$PWD/../StatusManager

SOURCES += \
        $$PWD/src/main.cpp \
        ../devicerotationmanager.cpp \
        ../ConfigurationManager/configurationmanager.cpp \
        ../KmreAppManager/kmremanager.cpp \
        ../KmreAppManager/kmrenotificationwindowmanager.cpp \
        ../keyboardmanager/keyboardmanager.cpp \
        ../DeviceModeManager/devicemodemanager.cpp \
        ../StatusManager/statusmanager.cpp


HEADERS += \
        $$PWD/src/main.h    \
        ../devicerotationmanager.h \
        ../ConfigurationManager/configurationmanager.h \
        ../KmreAppManager/kmremanager.h \
        ../KmreAppManager/kmrenotificationwindowmanager.h \
        ../keyboardmanager/keyboardmanager.h \
        ../DeviceModeManager/devicemodemanager.h \
        ../StatusManager/statusmanager.h


INCLUDEPATH += $$PWD/..


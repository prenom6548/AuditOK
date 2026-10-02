QT += testlib widgets gui dbus
QT += KGlobalAccel KConfigGui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

TEMPLATE = app

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage -fno-inline -fno-access-control -fno-exceptions
QMAKE_CXXFLAGS += --coverage

SOURCES +=  \
        thememanagertest.cpp \
        ../../kysdk-thememanager/src/kthememanager.cpp \

HEADERS += \
        ../../kysdk-thememanager/src/kthememanager.h \

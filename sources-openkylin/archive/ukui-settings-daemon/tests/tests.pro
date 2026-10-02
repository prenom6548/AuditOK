QT += gui
QT += core xml widgets x11extras dbus KGlobalAccel
include($$PWD/../common/common.pri)

TARGET = tests

TEMPLATE = app
DEFINES += XRANDR_LIBRARY _FORTIFY_SOURCE=2

CONFIG += c++11 no_keywords link_pkgconfig plugin xrandr app_bundle
CONFIG += app_bundle
LIBS += -lgcov

PKGCONFIG += \
        Qt5Sensors  \
        kscreen2    \
        x11         \
        xi          \

QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage

DEFINES += QT_DEPRECATED_WARNINGS MODULE_NAME=\\\"test\\\"

SOURCES += \
        common_clib_syslog_test.cpp \
        common_eggaccelerators_test.cpp \
        common_qconftype.cpp \
        common_qgsettings.cpp \
        common_rfkillswitch.cpp \
        common_touch_calibrate.cpp \
        common_ubc_test.cpp \
        common_ukui_keygrab.cpp \
        common_xeventmonitor.cpp \
        main.cpp

INCLUDEPATH += $$PWD/..


LIBS += -lgtest -lgtest_main -lpthread

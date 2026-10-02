TEMPLATE = app
TARGET = test_QRoundProgressBar

QT += testlib widgets core gui dbus network x11extras KWindowSystem
PKGCONFIG += gsettings-qt kysdk-waylandhelper kysdk-qtwidgets kysdk-soundeffects kysdk-ukuiwindowhelper
SOURCES += \
    test_QRoundProgressBar.cpp \
    main.cpp \
    ../../../../UIControl/src/QRoundProgressBar.cpp \
    ../../../../UIControl/src/include/menumodule.cpp \
    ../../../../UIControl/src/include/theme.cpp \
    ../../../../UIControl/src/daemonipcdbus.cpp \
   ../../../../UIControl/src/include/global.cpp

HEADERS += \
    test_QRoundProgressBar.h \
    ../../../../UIControl/src/QRoundProgressBar.h \
    ../../../../UIControl/src/include/menumodule.h \
    ../../../../UIControl/src/include/theme.h \
    ../../../../UIControl/src/include/daemonipcdbus.h

INCLUDEPATH += \
    ../../../../UIControl/src \
    ../../../../UIControl/src/include

LIBS += -L../../../../UIControl/src
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

QMAKE_CXXFLAGS += --coverage
QMAKE_LFLAGS += --coverage -lgcov
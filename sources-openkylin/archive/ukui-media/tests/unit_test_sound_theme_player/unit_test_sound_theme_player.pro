QT       += core gui dbus xml testlib

TEMPLATE = app

CONFIG += c++14 link_pkgconfig no_keywords

PKGCONFIG += \
    glib-2.0 \
    gio-2.0 \
    dconf \
    gsettings-qt \
    libxml-2.0 \
    libcanberra \
    libpulse

TARGET = unit_test_sound_theme_player
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

INCLUDEPATH   += /usr/include/gtest

LIBS +=  -lgtest_main -lpthread
LIBS +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lgtest -lgcov

SOURCES +=  ../../sound-theme-player/sound_theme_player.cpp \
            main.cpp

HEADERS += ../../sound-theme-player/sound_theme_player.h

INSTALLS += target

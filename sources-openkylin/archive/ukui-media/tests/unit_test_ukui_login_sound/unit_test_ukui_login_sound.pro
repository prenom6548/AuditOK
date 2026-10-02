QT       += core gui testlib

TEMPLATE = app

CONFIG += c++17 link_pkgconfig no_keywords

PKGCONFIG += alsa

TARGET = unit_test_ukui_login_sound
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

INCLUDEPATH   += /usr/include/gtest

LIBS +=  -lgtest_main -lpthread
LIBS +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lgtest -lgcov

SOURCES +=  ../../common/user_config.cpp \
            ../../common/user_info_json.cpp \
            ../../common/sound_effect_json.cpp \
            ../../ukui-login-sound/ukui_login_sound.cpp \
            ../../ukui-login-sound/ukui_login_sound_user_config.cpp \
            main.cpp

HEADERS +=  ../../common/json.h \
            ../../common/user_config.h \
            ../../common/user_info_json.h \
            ../../common/sound_effect_json.h \
            ../../ukui-login-sound/formats.h \
            ../../ukui-login-sound/ukui_login_sound_h \
            ../../ukui-login-sound/ukui_login_sound_user_config.h

INSTALLS += target

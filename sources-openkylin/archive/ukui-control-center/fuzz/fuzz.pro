#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += core dbus KService KCoreAddons KXmlGui KGlobalAccel x11extras
QT -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent sql xml
TEMPLATE = app

TARGET = fuzzy_tests
target.source += $$TARGET
target.path = ./

QMAKE_CXXFLAGS += -fno-access-control -fno-exceptions

LIBS          += -lgtest_main -lpthread  -lpolkit-qt5-core-1
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov -ldconf -lfontconfig -lfreetype -lukuiserviceinterface

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0 \
             gio-unix-2.0 \
             kysdk-sysinfo \
             gsettings-qt \
#             freetype2 \
#             fontconfig \
             kysdk-package \
#             kysdk-ukenv \

#打桩工具
TEST_UTILS_PATH = $$PWD/../tests/kt-test-utils

# Include directories
INCLUDEPATH += ../libukcc/interface/ \
               ../libukcc/ \
                /usr/include/dconf

INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../registeredSession/plugins/accessibility/accessibilityinterface.cpp \
           ../registeredSession/plugins/keyboard/keyboardinterface.cpp \
           ../registeredSession/universalinterface.cpp \
#           ../registeredSession/plugins/autoboot/autostartinterface.cpp \
           ../registeredSession/plugins/theme/themeinterface.cpp \
           ../registeredSession/plugins/vino/vinointerface.cpp \
           ../registeredSession/plugins/wallpaper/wallpaperinterface.cpp \
           ../registeredSession/plugins/wallpaper/workerobject.cpp \
           ../registeredSession/plugins/wallpaper/xmlhandle.cpp \
           ../registeredSession/plugins/screenlock/screenlockinterface.cpp \
           ../registeredSession/plugins/screensaver/screensaverinterface.cpp \
#           ../registeredSession/plugins/area/areainterface.cpp \
#           ../registeredSession/plugins/default/defaultinterface.cpp \
#           ../registeredSession/plugins/about/aboutinterface.cpp \
#           ../registeredSession/plugins/mouse/mouseinterface.cpp \
#           ../registeredSession/plugins/datetime/datetimeinterface.cpp \
#           ../registeredSession/plugins/shortcut/shortcutinterface.cpp \
#           ../registeredSession/plugins/shortcut/getshortcutworker.cpp \
#           ../registeredSession/plugins/shortcut/type.cpp \
#           ../registeredSession/plugins/font/libfun.cpp \
#           ../registeredSession/plugins/font/external.cpp \
#           ../registeredSession/plugins/font/fontinterface.cpp \
           ../tests/unit_test_accessibility/unit_test_accessibility.cpp \
           ../tests/unit_test_keyboard/unit_test_keyboard.cpp \
#           ../tests/unit_test_autoboot/unit_test_autoboot.cpp \
           ../tests/unit_test_theme/unit_test_theme.cpp \
           ../tests/unit_test_vino/unit_test_vino.cpp \
           ../tests/unit_test_wallpaper/unit_test_wallpaper.cpp \
           ../tests/unit_test_screensaver/unit_test_screensaver.cpp \
           ../tests/unit_test_screenlock/unit_test_screenlock.cpp \
#           ../tests/unit_test_area/unit_test_area.cpp \
#           ../tests/unit_test_default/unit_test_default.cpp \
#           ../tests/unit_test_font/unit_test_font.cpp \
#           ../tests/unit_test_shortcut/unit_test_shortcut.cpp \
#           ../tests/unit_test_datetime/unit_test_datetime.cpp \
#           ../tests/unit_test_mouse/unit_test_mouse.cpp \
#           ../tests/unit_test_about/unit_test_about.cpp \
           main.cpp

HEADERS += ../registeredSession/plugins/accessibility/accessibilityinterface.h \
           ../registeredSession/plugins/keyboard/keyboardinterface.h \
           ../registeredSession/universalinterface.h \
#           ../registeredSession/plugins/autoboot/autostartinterface.h \
           ../registeredSession/plugins/theme/themeinterface.h \
           ../registeredSession/plugins/vino/vinointerface.h \
           ../registeredSession/plugins/wallpaper/wallpaperinterface.h \
           ../registeredSession/plugins/wallpaper/workerobject.h \
           ../registeredSession/plugins/wallpaper/xmlhandle.h \
           ../registeredSession/plugins/screenlock/screenlockinterface.h \
           ../registeredSession/plugins/screensaver/screensaverinterface.h \
#           ../registeredSession/plugins/area/areainterface.h \
#           ../registeredSession/plugins/default/defaultinterface.h \
#           ../registeredSession/plugins/about/aboutinterface.h \
#           ../registeredSession/plugins/font/libfun.h \
#           ../registeredSession/plugins/font/external.h \
#           ../registeredSession/plugins/font/fontinterface.h \
#           ../registeredSession/plugins/shortcut/shortcutinterface.h \
#           ../registeredSession/plugins/shortcut/getshortcutworker.h \
#           ../registeredSession/plugins/shortcut/type.h \
#           ../registeredSession/plugins/datetime/datetimeinterface.h \
#           ../registeredSession/plugins/mouse/mouseinterface.h




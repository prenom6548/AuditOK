QT += core dbus KService KCoreAddons KXmlGui KGlobalAccel x11extras KConfigCore
QT -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent sql xml


include(../env.pri)
TARGET = ukccSession
TEMPLATE = lib

CONFIG += c++11 console link_pkgconfig plugin
CONFIG -= app_bundle
PKGCONFIG += gio-2.0 \
             gio-unix-2.0 \
             kysdk-sysinfo \
             kysdk-systime \
             gsettings-qt \
             freetype2 \
             fontconfig \
             kysdk-ukenv \
             kysdk-package \

QMAKE_CXXFLAGS *= -D_FORTIFY_SOURCE=2 -O2
INCLUDEPATH += ../libukcc/interface/ \
               ../libukcc/ \
               /usr/include/dconf

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS Nile

DEFINES += PLUGIN_INSTALL_DIRS='\\"$$[QT_INSTALL_LIBS]/ukui-control-center\\"'
DEFINES += PLUGIN_INSTALL_UPDATE='\\"$$[QT_INSTALL_LIBS]/ukui-control-center/V2.0\\"'

LIBS += -L$$[QT_INSTALL_LIBS]  -lpolkit-qt5-core-1 -ldconf -lukuiserviceinterface  -lfontconfig -lfreetype

DBUS_ADAPTORS += conf/org.ukui.ukcc.session.xml
DBUS_INTERFACES += conf/org.ukui.ukcc.session.xml

inst1.files += conf/org.ukui.ukcc.session.service
inst1.path = /usr/share/dbus-1/services/
INSTALLS += inst1

DISTFILES += metadata.json

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        plugins/about/aboutinterface.cpp \
        plugins/area/areainterface.cpp \
        plugins/autoboot/autostartinterface.cpp \
        plugins/datetime/datetimeinterface.cpp \
        plugins/font/fontinterface.cpp \
        plugins/font/libfun.cpp \
        plugins/font/external.cpp \
        plugins/keyboard/keyboardinterface.cpp \
        plugins/mouse/mouseinterface.cpp \
        plugins/default/defaultinterface.cpp \
        plugins/screenlock/screenlockinterface.cpp \
        plugins/screensaver/screensaverinterface.cpp \
        plugins/shortcut/getshortcutworker.cpp \
        plugins/shortcut/shortcutinterface.cpp \
        plugins/shortcut/type.cpp \
        plugins/vino/vinointerface.cpp \
        plugins/wallpaper/wallpaperinterface.cpp \
        plugins/wallpaper/workerobject.cpp \
        plugins/wallpaper/xmlhandle.cpp \
        plugins/boot/bootinterface.cpp \
        plugins/theme/themeinterface.cpp \
        plugins/accessibility/accessibilityinterface.cpp \
        ukccsessionserver.cpp \
        ukccsessionservice.cpp \
        universalinterface.cpp \

# Default rules for deployment.
target.path = /usr/libexec/ukui-services/session
!isEmpty(target.path): INSTALLS += target


HEADERS += \
        plugins/about/aboutinterface.h \
        plugins/area/areainterface.h \
        plugins/autoboot/autostartinterface.h \
        plugins/datetime/datetimeinterface.h \
        plugins/font/fontinterface.h \
        plugins/font/libfun.h \
        plugins/font/external.h \
        plugins/keyboard/keyboardinterface.cpp \
        plugins/keyboard/keyboardinterface.h \
        plugins/mouse/mouseinterface.h \
        plugins/default/defaultinterface.h \
        plugins/screenlock/screenlockinterface.h \
        plugins/screensaver/screensaverinterface.h \
        plugins/shortcut/getshortcutworker.h \
        plugins/shortcut/shortcutinterface.h \
        plugins/shortcut/type.h \
        plugins/vino/vinointerface.h \
        plugins/wallpaper/wallpaperinterface.h \
        plugins/wallpaper/workerobject.h \
        plugins/wallpaper/xmlhandle.h \
        plugins/boot/bootinterface.h \
        plugins/theme/themeinterface.h \
        plugins/accessibility/accessibilityinterface.h \
        ukccsessionserver.h \
        ukccsessionservice.h \
        universalinterface.h \


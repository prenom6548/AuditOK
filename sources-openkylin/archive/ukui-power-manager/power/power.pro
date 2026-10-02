QT       += widgets dbus gui core
TEMPLATE = lib
CONFIG   += plugin

TARGET = $$qtLibraryTarget(power)
DESTDIR = ../..
target.path = $$[QT_INSTALL_LIBS]/ukui-control-center
INSTALLS += target

INCLUDEPATH += \
               $$PROJECT_ROOTDIR \
               /usr/include/ukcc/interface \
            /usr/include/ukcc/widgets \
            /usr/include/ukcc \
            $$PWD/../shared/include      



LIBS     += -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lupower-glib -lukcc

CONFIG +=  \
          link_pkgconfig \
          c++11

PKGCONFIG += gsettings-qt \
             gio-2.0 \
             gio-unix-2.0 \
             upower-glib \
             kysdk-qtwidgets \
             kysdk-sysinfo

FORMS +=

HEADERS += \
        power.h \
        powermacrodata.h\
        interface.h


SOURCES += \
    power.cpp\

TRANSLATIONS += \
    translations/zh_CN.ts \
    translations/bo_CN.ts \
    translations/zh_HK.ts \
    translations/ug.ts \
    translations/ky.ts \
    translations/kk.ts \
    translations/mn.ts \
    translations/ar.ts \
    translations/ms.ts \
    translations/th.ts \
    translations/vi.ts
QMAKE_EXTRA_COMPILERS += lrelease

trans.files = translations/*
trans.path = /usr/share/ukui-power-manager/power/translations

CONFIG(release, debug|release) {
    !system($$PWD/translate_generation.sh): error("Failed to generate translation")
}

json.files = org.ukui.power-manager.json
json.path = /opt/system/conf/ukui-security-support/


INSTALLS += target \
            trans


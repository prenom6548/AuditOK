QT += core gui widgets concurrent dbus

TARGET = peony-share
TEMPLATE = lib

DEFINES += PEONYSHARE_LIBRARY
DEFINES += DBUS_NAME=\\\"org.ukui.samba.share.config\\\"
DEFINES += DBUS_PATH=\\\"/org/ukui/samba/share\\\"

include(../../common.pri)

PKGCONFIG += peony kysdk-qtwidgets polkit-gobject-1
CONFIG += link_pkgconfig \
          c++11 \
	  plugin \
	  debug

SOURCES += share-page.cpp \
           advanced-share-page.cpp \
           share-emblem-provider.cpp \
           samba-config-thread.cpp \
           share-properties-page-plugin.cpp \
           samba-config-interface.cpp \
           systemd-bus-accounts.cpp


HEADERS += share-page.h \
           advanced-share-page.h \
           share-emblem-provider.h \
           samba-config-thread.h \
           share-properties-page-plugin.h \
           samba-config-interface.h \
           systemd-bus-accounts.h

TRANSLATIONS += translations/peony-share-extension_cs.ts \
                translations/peony-share-extension_de.ts \
                translations/peony-share-extension_es.ts \
                translations/peony-share-extension_fr.ts \
                translations/peony-share-extension_kk_KZ.ts \
                translations/peony-share-extension_ms.ts \
                translations/peony-share-extension_th.ts \
                translations/peony-share-extension_ug_CN.ts \
                translations/peony-share-extension_ky_KG.ts \
                translations/peony-share-extension_tr.ts \
                translations/peony-share-extension_vi.ts \
                translations/peony-share-extension_zh_CN.ts \
                translations/peony-share-extension_bo_CN.ts \
                translations/peony-share-extension_mn.ts \
                translations/peony-share-extension_zh_HK.ts

#RESOURCES += resources.qrc
CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

target.path = $$[QT_INSTALL_LIBS]/peony-extensions

INSTALLS += target

DISTFILES += \
    translations/peony-share-extension_bo_CN.ts

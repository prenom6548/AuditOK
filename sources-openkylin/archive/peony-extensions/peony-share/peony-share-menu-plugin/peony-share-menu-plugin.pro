QT  += widgets concurrent dbus

include(../../common.pri)

TEMPLATE = lib
TARGET = peony-share-menu-plugin

DEFINES += PEONYSHAREMENUPLUGIN_LIBRARY

PKGCONFIG += peony gio-2.0 udisks2 glib-2.0

CONFIG += link_pkgconfig no_keywords c++11 plugin debug

HEADERS += \
    ../app/share-emblem-provider.h \
    peony-share-menu-plugin_global.h \
    share-menu-plugin.h

SOURCES += \
    ../app/share-emblem-provider.cpp \
    share-menu-plugin.cpp

TRANSLATIONS += translations/peony-share-menu-plugin_zh_CN.ts \
                translations/peony-share-menu-plugin_bo_CN.ts \
                translations/peony-share-menu-plugin_mn.ts \
                translations/peony-share-menu-plugin_ms.ts \
                translations/peony-share-menu-plugin_th.ts \
                translations/peony-share-menu-plugin_vi.ts \
                translations/peony-share-menu-plugin_zh_HK.ts

target.path = $$[QT_INSTALL_LIBS]/peony-extensions
INSTALLS += target

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

QT += core gui widgets dbus

TARGET = peony-quick-look-plugin
TEMPLATE = lib

include(../common.pri)

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += link_pkgconfig no_keywords c++11 plugin debug

PKGCONFIG += peony ukui-previewhub

TRANSLATIONS += translations/peony-quick-look-plugin_zh_CN.ts \
                translations/peony-quick-look-plugin_de.ts \
                translations/peony-quick-look-plugin_es.ts \
                translations/peony-quick-look-plugin_fr.ts \
                translations/peony-quick-look-plugin_kk_KZ.ts \
                translations/peony-quick-look-plugin_ms.ts \
                translations/peony-quick-look-plugin_th.ts \
                translations/peony-quick-look-plugin_ug_CN.ts \
                translations/peony-quick-look-plugin_ky_KG.ts \
                translations/peony-quick-look-plugin_bo_CN.ts \
                translations/peony-quick-look-plugin_mn.ts \
                translations/peony-quick-look-plugin_tr.ts \
                translations/peony-quick-look-plugin_cs.ts \
                translations/peony-quick-look-plugin_vi.ts \
                translations/peony-quick-look-plugin_zh_HK.ts

DISTFILES += $$TRANSLATIONS

exists(/usr/include/peony-qt/quick-look-plugin-iface.h) {
    INCLUDEPATH += /usr/include/peony-qt
} else {
    error("Peony quick look plugin interface header not found. Install the matching peony development package.")
}

!packagesExist(ukui-previewhub) {
    error("ukui-previewhub development package not found. Install libukui-previewhub-dev.")
}

SOURCES += \
    quick-look-preview-factory.cpp \
    quick-look-window.cpp \
    quick-look-info-panel.cpp \
    quick-look-open-button.cpp

HEADERS += \
    quick-look-preview-factory.h \
    quick-look-window.h \
    quick-look-info-panel.h \
    quick-look-open-button.h

DISTFILES += $$PWD/common.json

commonjson.path = $$[QT_INSTALL_LIBS]/peony-extensions
commonjson.files = $$PWD/common.json
INSTALLS += commonjson

target.path = $$[QT_INSTALL_LIBS]/peony-extensions
INSTALLS += target

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

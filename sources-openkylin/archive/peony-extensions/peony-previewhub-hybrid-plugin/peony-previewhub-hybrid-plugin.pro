QT += core gui widgets dbus

TARGET = peony-previewhub-hybrid-plugin
TEMPLATE = lib

include(../common.pri)

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += link_pkgconfig no_keywords c++11 plugin debug

PKGCONFIG += peony ukui-previewhub

TRANSLATIONS += translations/peony-previewhub-hybrid-plugin_zh_CN.ts \
                translations/peony-previewhub-hybrid-plugin_de.ts \
                translations/peony-previewhub-hybrid-plugin_es.ts \
                translations/peony-previewhub-hybrid-plugin_fr.ts \
                translations/peony-previewhub-hybrid-plugin_kk_KZ.ts \
                translations/peony-previewhub-hybrid-plugin_ms.ts \
                translations/peony-previewhub-hybrid-plugin_th.ts \
                translations/peony-previewhub-hybrid-plugin_ug_CN.ts \
                translations/peony-previewhub-hybrid-plugin_ky_KG.ts \
                translations/peony-previewhub-hybrid-plugin_bo_CN.ts \
                translations/peony-previewhub-hybrid-plugin_mn.ts \
                translations/peony-previewhub-hybrid-plugin_tr.ts \
                translations/peony-previewhub-hybrid-plugin_cs.ts \
                translations/peony-previewhub-hybrid-plugin_vi.ts \
                translations/peony-previewhub-hybrid-plugin_zh_HK.ts

DISTFILES += $$TRANSLATIONS

!packagesExist(ukui-previewhub) {
    error("ukui-previewhub development package not found. Install libukui-previewhub-dev.")
}

SOURCES += \
    previewhub-hybrid-preview-factory.cpp \
    previewhub-hybrid-preview-page.cpp \
    previewhub-hybrid-hub-page.cpp

HEADERS += \
    previewhub-hybrid-preview-factory.h \
    previewhub-hybrid-preview-page.h \
    previewhub-hybrid-hub-page.h

target.path = $$[QT_INSTALL_LIBS]/peony-extensions
INSTALLS += target

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

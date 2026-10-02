QT += core gui widgets concurrent

include(../common.pri)

TARGET = peony-send-to-device
TEMPLATE = lib

DEFINES += PEONYADMIN_LIBRARY

PKGCONFIG += peony

CONFIG += link_pkgconfig no_keywords c++11 plugin debug


SOURCES += \
    send-to-device-plugin.cpp
HEADERS += \
    send-to-device-plugin.h

TRANSLATIONS += \
    translations/peony-send-to-device_cs.ts \
    translations/peony-send-to-device_de.ts \
    translations/peony-send-to-device_es.ts \
    translations/peony-send-to-device_fr.ts \
    translations/peony-send-to-device_kk_KZ.ts \
    translations/peony-send-to-device_ms.ts \
    translations/peony-send-to-device_th.ts \
    translations/peony-send-to-device_ug_CN.ts \
    translations/peony-send-to-device_ky_KG.ts \
    translations/peony-send-to-device_tr.ts \
    translations/peony-send-to-device_vi.ts \
    translations/peony-send-to-device_zh_CN.ts \
    translations/peony-send-to-device_bo_CN.ts \
    translations/peony-send-to-device_mn.ts \
    translations/peony-send-to-device_zh_HK.ts

#RESOURCES += \
#    resources.qrc

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

target.path = $$[QT_INSTALL_LIBS]/peony-extensions

INSTALLS += target

SKIP_TEST = $$(EXTENSIONS_SKIP_TEST)
SKIP_PL_COV = $$(EXTENSIONS_SKIP_PLUGIN_COVERAGE)
isEmpty(SKIP_TEST) {
    equals(SKIP_PL_COV, 0) {
        message("peony-send-to-device: coverage instrumentation enabled")
        QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
        QMAKE_CXXFLAGS += --coverage
        LIBS += -lgcov
    } else {
        message("peony-send-to-device: coverage instrumentation skipped")
    }
}

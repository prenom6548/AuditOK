QT += core gui widgets concurrent

include(../common.pri)

TARGET = peony-drive-rename
TEMPLATE = lib

DEFINES += PEONYADMIN_LIBRARY

PKGCONFIG += peony gio-2.0 udisks2 glib-2.0 kysdk-qtwidgets

CONFIG += link_pkgconfig no_keywords c++11 plugin debug


SOURCES += \
    drive-rename.cpp \
    driverenameplugin.cpp


HEADERS += \
    drive-rename.h \
    driverenameplugin.h


TRANSLATIONS += translations/peony-drive-rename_zh_CN.ts \
                translations/peony-drive-rename_de.ts \
                translations/peony-drive-rename_es.ts \
                translations/peony-drive-rename_fr.ts \
                translations/peony-drive-rename_kk_KZ.ts \
                translations/peony-drive-rename_ms.ts \
                translations/peony-drive-rename_th.ts \
                translations/peony-drive-rename_ug_CN.ts \
                translations/peony-drive-rename_ky_KG.ts \
                translations/peony-drive-rename_tr.ts \
                translations/peony-drive-rename_cs.ts \
                translations/peony-drive-rename_bo_CN.ts \
                translations/peony-drive-rename_mn.ts \
                translations/peony-drive-rename_vi.ts \
                translations/peony-drive-rename_zh_HK.ts

#RESOURCES += \
#    resource.qrc

CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /translations/

target.path = $$[QT_INSTALL_LIBS]/peony-extensions

INSTALLS += target

SKIP_TEST = $$(EXTENSIONS_SKIP_TEST)
SKIP_PL_COV = $$(EXTENSIONS_SKIP_PLUGIN_COVERAGE)
isEmpty(SKIP_TEST) {
    equals(SKIP_PL_COV, 0) {
        message("peony-drive-rename: coverage instrumentation enabled")
        QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
        QMAKE_CXXFLAGS += --coverage
        LIBS += -lgcov
    } else {
        message("peony-drive-rename: coverage instrumentation skipped")
    }
}

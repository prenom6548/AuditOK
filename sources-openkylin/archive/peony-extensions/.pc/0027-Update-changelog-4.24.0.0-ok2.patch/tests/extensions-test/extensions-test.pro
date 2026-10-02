TEMPLATE = app
TARGET = extensions-test
CONFIG += console testcase
CONFIG -= app_bundle

QT += core gui dbus sql network remoteobjects widgets testlib

include(../test-utils.pri)

SERVICE_DIR = $$PWD/../../peony-intelligent-data-management-service

INCLUDEPATH += \
    $$SERVICE_DIR \
    $$SERVICE_DIR/model \
    $$SERVICE_DIR/condition \
    $$SERVICE_DIR/file \
    $$SERVICE_DIR/file/qtremoteobject \
    $$SERVICE_DIR/database \
    $$SERVICE_DIR/search \
    $$SERVICE_DIR/ai

CONFIG += link_pkgconfig no_keywords c++17

PKGCONFIG += \
    ukui-search \
    kysdk-alm \
    peony \
    libsystemd \
    kysdk-filesystem \
    kysdk-systime \
    kyai-data-management-client \
    kysdk-ai-common

LIBS += -lukui-log4qt

exists(/usr/include/applications/kabase/log.hpp) {
    message("kysdkbase found (extensions-test)")
    DEFINES += KY_SDK_KABASE
}

DEFINES += COMPLIE_AS_PROJECT USE_AI UKUI_SEARCH_HAS_MATCH_ALL
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += IDM_TEST_RELAX_MODEL_INIT

REPC_SOURCE += $$SERVICE_DIR/fileeventhandler.rep
REPC_REPLICA += $$SERVICE_DIR/fileeventhandler.rep

SOURCES += \
    main.cpp \
    whitebox/wb_spacessettings.cpp \
    whitebox/wb_condition.cpp \
    whitebox/wb_idm_core.cpp \
    whitebox/wb_idm_models.cpp \
    whitebox/wb_idm_controller.cpp \
    whitebox/wb_idm_fileutil.cpp \
    whitebox/wb_idm_filejob.cpp \
    $$SERVICE_DIR/model/modelcontroller.cpp \
    $$SERVICE_DIR/model/spacesfilesfetcher.cpp \
    $$SERVICE_DIR/database/databasemanager.cpp \
    $$SERVICE_DIR/condition/condition.cpp \
    $$SERVICE_DIR/search/searchtask.cpp \
    $$SERVICE_DIR/ai/ailabelhelper.cpp \
    $$SERVICE_DIR/ai/aiconditionshelper.cpp \
    $$SERVICE_DIR/ai/aipreviewhelper.cpp \
    $$SERVICE_DIR/ai/aisearchtask.cpp \
    $$SERVICE_DIR/file/fileinfojob.cpp \
    $$SERVICE_DIR/file/fileinfomanager.cpp \
    $$SERVICE_DIR/file/fileinfo.cpp \
    $$SERVICE_DIR/file/qtremoteobject/fileeventhandlerimpl.cpp \
    $$SERVICE_DIR/model/searchresultmodel.cpp \
    $$SERVICE_DIR/model/spacessettings.cpp \
    $$SERVICE_DIR/3rd-parties/qtsinglecoreapplication.cpp \
    $$SERVICE_DIR/3rd-parties/qtlocalpeer.cpp \
    $$SERVICE_DIR/3rd-parties/qtlockedfile.cpp \
    $$SERVICE_DIR/3rd-parties/qtlockedfile_unix.cpp \
    $$SERVICE_DIR/3rd-parties/qtlockedfile_win.cpp

HEADERS += \
    $$SERVICE_DIR/model/modelcontroller.h \
    $$SERVICE_DIR/model/spacesfilesfetcher.h \
    $$SERVICE_DIR/database/databasemanager.h \
    $$SERVICE_DIR/condition/condition.h \
    $$SERVICE_DIR/search/searchtask.h \
    $$SERVICE_DIR/ai/ailabelhelper.h \
    $$SERVICE_DIR/ai/aiconditionshelper.h \
    $$SERVICE_DIR/ai/aipreviewhelper.h \
    $$SERVICE_DIR/ai/aisearchtask.h \
    $$SERVICE_DIR/file/fileinfojob.h \
    $$SERVICE_DIR/file/fileinfomanager.h \
    $$SERVICE_DIR/file/fileinfo_p.h \
    $$SERVICE_DIR/file/fileinfo.h \
    $$SERVICE_DIR/file/qtremoteobject/fileeventhandlerimpl.h \
    $$SERVICE_DIR/model/searchresultmodel.h \
    $$SERVICE_DIR/model/spacessettings.h \
    $$SERVICE_DIR/3rd-parties/qtsinglecoreapplication.h \
    $$SERVICE_DIR/3rd-parties/qtlocalpeer.h \
    $$SERVICE_DIR/3rd-parties/qtlockedfile.h

QT += core gui dbus sql network remoteobjects

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app
TARGET = peony-intelligent-data-management-service
VERSION = 1.0.0
DEFINES += VERSION='\\"$${VERSION}\\"'
#CONFIG += c++11 no_keywords
CONFIG += create_pc create_prl no_install_prl
CONFIG += link_pkgconfig no_keywords c++11 lrelease hide_symbols embed_translations
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += COMPLIE_AS_PROJECT
DEFINES += USE_AI UKUI_SEARCH_HAS_MATCH_ALL
QMAKE_CXXFLAGS += -Werror=return-type -Werror=return-local-addr -Werror=uninitialized
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#include(../libpeony-qt/libpeony-qt-header.pri)

# ukui-search
PKGCONFIG += ukui-search kysdk-alm peony libsystemd kysdk-filesystem kysdk-systime kyai-data-management-client kysdk-ai-common

LIBS += -lukui-log4qt

## 包含目录
#INCLUDEPATH += $$shell pkg-config --cflags-only-I kysdk-alm \
#                          kysdk-systime \
#                          ukui-search \

#INCLUDEPATH += $$shell pkg-config --cflags-only-I kyai-data-management-client
#include <kysdk/applications/kabase/log.hpp>
exists(/usr/include/applications/kabase/log.hpp) {
    message ("kysdkbase found")
    DEFINES += KY_SDK_KABASE
}

exists(/usr/include/kylin-ai/advai/knowledgebase) {
    message ("kyadvanceidai found")
    DEFINES += KY_SDK_ADVANCED_AI
}

contains(DEFINES,KY_SDK_ADVANCED_AI){
    PKGCONFIG += kysdk-advai-knowledgebase
    LIBS += -lkysdk-advanced-ai
}
TRANSLATIONS += peony-intelligent-data-management-service_zh_CN.ts\
                peony-intelligent-data-management-service_ms.ts \
                peony-intelligent-data-management-service_th.ts \
                peony-intelligent-data-management-service_vi.ts \
                peony-intelligent-data-management-service_zh_HK.ts\
                peony-intelligent-data-management-service_mn.ts\
                peony-intelligent-data-management-service_ug.ts\
                peony-intelligent-data-management-service_kk.ts\
                peony-intelligent-data-management-service_ky.ts\
                peony-intelligent-data-management-service_bo_CN.ts
SOURCES += \
        ./main.cpp\
        ./model/modelcontroller.cpp \
        ./model/spacesfilesfetcher.cpp\
        ./database/databasemanager.cpp\
        ./condition/condition.cpp\
        ./search/searchtask.cpp\
        ./ai/ailabelhelper.cpp\
        ./ai/aiconditionshelper.cpp\
        ./ai/aipreviewhelper.cpp\
        ./ai/aisearchtask.cpp\
        ./ai/semantickbmanager.cpp\
        ./file/fileinfojob.cpp\
        ./file/fileinfomanager.cpp\
        ./file/fileinfo.cpp \
        ./file/qtremoteobject/fileeventhandlerimpl.cpp\
        ./model/searchresultmodel.cpp\
        ./model/spacessettings.cpp\
        ./3rd-parties/qtsinglecoreapplication.cpp\
        ./3rd-parties/qtlocalpeer.cpp\
        ./3rd-parties/qtlockedfile.cpp\
        ./3rd-parties/qtlockedfile_unix.cpp\
        ./3rd-parties/qtlockedfile_win.cpp

HEADERS += \
        ./model/modelcontroller.h \
        ./model/spacesfilesfetcher.h\
        ./database/databasemanager.h\
        ./condition/condition.h\
        ./search/searchtask.h\
        ./ai/ailabelhelper.h\
        ./ai/aiconditionshelper.h\
        ./ai/aipreviewhelper.h\
        ./ai/aisearchtask.h\
        ./ai/semantickbmanager.h\
        ./file/fileinfojob.h\
        ./file/fileinfomanager.h\
        ./file/fileinfo_p.h\
        ./file/fileinfo.h\
        ./file/qtremoteobject/fileeventhandlerimpl.h\
        ./model/searchresultmodel.h\
        ./model/spacessettings.h\
        ./3rd-parties/qtsinglecoreapplication.h\
        ./3rd-parties/qtlocalpeer.h\
        ./3rd-parties/qtlockedfile.h


REPC_SOURCE = fileeventhandler.rep

REPC_REPLICA = fileeventhandler.rep


inst1.files += conf/peony-intelligent-data-management-service.service
inst1.path = /usr/lib/systemd/user
inst2.files +=conf/com.peony.idm.service
inst2.path = /usr/share/dbus-1/services
target.files +=$$OUT_PWD/$$TARGET
target.path = /usr/bin
intro.files += \
    data/智能空间使用手册.pdf \
    data/IntelligentSpacesUserGuide.pdf
intro.path = /usr/share/peony-intelligent-data-management-service/
INSTALLS += \
    target \
    inst1 \
    inst2 \
    intro

QM_FILES_INSTALL_PATH = /usr/share/peony-intelligent-data-management-service

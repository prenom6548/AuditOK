QT += widgets dbus

TARGET = peony-intelligent-search-view

TEMPLATE = lib
DEFINES += PEONYINTELLIGENTSEARCHVIEW_LIBRARY

include(../../common.pri)
CONFIG += debug c++11 no_keywords link_pkgconfig plugin embed_translations
PKGCONFIG +=gio-2.0 glib-2.0 gio-unix-2.0 peony libnotify ukui-file-metadata
# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#TRANSLATIONS = peony-extension-computer-view_zh_CN.ts \
#               peony-extension-computer-view_de.ts \
#               peony-extension-computer-view_es.ts \
#               peony-extension-computer-view_fr.ts \
#               peony-extension-computer-view_kk_KZ.ts \
#               peony-extension-computer-view_ug_CN.ts \
#               peony-extension-computer-view_ky_KG.ts \
#               peony-extension-computer-view_cs.ts \
#               peony-extension-computer-view_tr.ts \
#               peony-extension-computer-view_bo_CN.ts \
#               peony-extension-computer-view_mn.ts \
#               peony-extension-computer-view_zh_HK.ts

#include(computer-view/computer-view.pri)

TRANSLATIONS += peony-intelligent-search-view_zh_CN.ts\
                peony-intelligent-search-view_ms.ts \
                peony-intelligent-search-view_th.ts \
                peony-intelligent-search-view_vi.ts \
                peony-intelligent-search-view_zh_HK.ts\
                peony-intelligent-search-view_mn.ts\
                peony-intelligent-search-view_ug.ts\
                peony-intelligent-search-view_kk.ts\
                peony-intelligent-search-view_ky.ts\
                peony-intelligent-search-view_bo_CN.ts

CONFIG += link_pkgconfig

PKGCONFIG += gio-unix-2.0
SOURCES += \
    peony-intelligent-search-view-plugin.cpp \
    search-view-container.cpp \
    search-view.cpp\
    searchresultmodel.cpp\
    thumbnail/searchthumbnailmanager.cpp\
    search-file-item.cpp
 

HEADERS += \
    peony-intelligent-search-view-plugin.h \
    peony-intelligent-search-view_global.h \
    search-view-container.h \
    search-view.h\
    searchresultmodel.h \
    thumbnail/searchthumbnailmanager.h\
    search-file-item.h


#DESTDIR += ../testdir

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_LIBS]/peony-extensions
}
!isEmpty(target.path): INSTALLS += target


CONFIG += lrelease embed_translations
QM_FILES_RESOURCE_PREFIX = /

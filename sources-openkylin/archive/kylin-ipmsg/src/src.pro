QT += core gui network dbus sql KWindowSystem

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = kylin-ipmsg

TEMPLATE = app

contains(CONFIG, KAIMING){
   message("kaiming")
} else {
   message("not kaiming")
   DEFINES += NOTKAIMING
}

# 使用dpkg-parsechangelog获取版本号
#DEBIAN_VERSION = $$system(dpkg-parsechangelog -l ../debian/changelog --show-field Version 2>/dev/null || echo "Unknown")
# 打印版本号（可选）
#message("Detected package version: $$DEBIAN_VERSION")

# about widget version
VERSION = $$DEBIAN_VERSION
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

QMAKE_CXXFLAGS += -g -Wall
DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += link_pkgconfig c++11
PKGCONFIG += gsettings-qt glib-2.0 kysdk-kabase kysdk-qtwidgets kysdk-waylandhelper kysdk-alm kysdk-ukenv kysdk-log kysdk-diagnostics kysdk-datacollect kysdk-systime kysdk-ukuiwindowhelper

LIBS +=-lpeony

INCLUDEPATH += \
    ../kabase/ \
    ../kabase/Qt

SOURCES += \
    $$PWD/main.cpp \
    $$PWD/view/kyview.cpp \
    $$PWD/view/common/globalsizedata.cpp\
    $$PWD/view/titlebar/menumodule.cpp  \
    $$PWD/view/titlebar/titlebar.cpp    \
    $$PWD/view/titlebar/titleseting.cpp \
    $$PWD/view/localinfo/localinfo.cpp  \
    $$PWD/view/localinfo/localupdatename.cpp \
    $$PWD/view/friendlist/friendlist.cpp\
    $$PWD/view/friendlist/friendItemdelegate.cpp    \
    $$PWD/view/friendlist/mysortfiltermodel.cpp     \
    $$PWD/view/friendlist/friendinfowid.cpp \
    $$PWD/view/chatmsg/chatmsg.cpp          \
    $$PWD/view/chatmsg/chatmsgdelegate.cpp  \
    $$PWD/view/trayicon/trayiconwid.cpp     \
    $$PWD/view/trayicon/trayitemdelegate.cpp\
    $$PWD/view/trayicon/trayfiltermodel.cpp \
    $$PWD/view/chatsearch/chat_search.cpp \
    $$PWD/view/chatsearch/chat_search_delegate.cpp \
    $$PWD/view/chatsearch/chat_search_filter.cpp \
    $$PWD/view/friendlist/homepagesearch/blankpage.cpp \
    $$PWD/view/friendlist/homepagesearch/search_msg_delegate.cpp \
    $$PWD/view/friendlist/homepagesearch/search_msg_model.cpp \
    $$PWD/view/friendlist/homepagesearch/searchfriend_delegate.cpp \
    $$PWD/view/friendlist/homepagesearch/searchmsglist.cpp \
    $$PWD/view/friendlist/homepagesearch/searchpage.cpp \
    $$PWD/model/friendlistmodel.cpp     \
    $$PWD/model/chatmsgmodel.cpp        \
    $$PWD/global/database/database.cpp  \
    $$PWD/global/database/friendlistdb.cpp  \
    $$PWD/global/database/chatmsgdb.cpp \
    $$PWD/global/utils/global_data.cpp  \
    $$PWD/global/utils/tcp_link.cpp     \
    $$PWD/global/utils/globalutils.cpp  \
    $$PWD/global/utils/addrset.cpp \
    $$PWD/global/utils/inisetting.cpp \
    $$PWD/global/utils/rotatechangeinfo.cpp \
    $$PWD/global/utils/globaldateformat.cpp \
    $$PWD/network/protocol_analysis.cpp \
    $$PWD/network/tcp_client.cpp \
    $$PWD/network/tcp_module.cpp \
    $$PWD/network/tcp_server.cpp \
    $$PWD/network/udp_socket.cpp \
    $$PWD/controller/control.cpp \

HEADERS += \
    $$PWD/view/kyview.h \
    $$PWD/view/common/globalsizedata.h  \
    $$PWD/view/titlebar/menumodule.h    \
    $$PWD/view/titlebar/titlebar.h      \
    $$PWD/view/titlebar/titleseting.h\
    $$PWD/view/localinfo/localinfo.h    \
    $$PWD/view/localinfo/localupdatename.h \
    $$PWD/view/friendlist/friendlist.h  \
    $$PWD/view/friendlist/friendItemdelegate.h  \
    $$PWD/view/friendlist/mysortfiltermodel.h   \
    $$PWD/view/friendlist/friendinfowid.h   \
    $$PWD/view/chatmsg/chatmsg.h        \
    $$PWD/view/chatmsg/chatmsgdelegate.h\
    $$PWD/view/trayicon/trayiconwid.h   \
    $$PWD/view/trayicon/trayitemdelegate.h  \
    $$PWD/view/trayicon/trayfiltermodel.h   \
    $$PWD/view/chatsearch/chat_search.h \
    $$PWD/view/chatsearch/chat_search_delegate.h \
    $$PWD/view/chatsearch/chat_search_filter.h \
    $$PWD/view/friendlist/homepagesearch/blankpage.h \
    $$PWD/view/friendlist/homepagesearch/search_msg_delegate.h \
    $$PWD/view/friendlist/homepagesearch/search_msg_model.h \
    $$PWD/view/friendlist/homepagesearch/searchfriend_delegate.h \
    $$PWD/view/friendlist/homepagesearch/searchmsglist.h \
    $$PWD/view/friendlist/homepagesearch/searchpage.h \
    $$PWD/model/friendlistmodel.h       \
    $$PWD/model/chatmsgmodel.h          \
    $$PWD/global/database/database.h    \
    $$PWD/global/database/friendlistdb.h\
    $$PWD/global/database/chatmsgdb.h   \
    $$PWD/global/declare/declare.h      \
    $$PWD/global/utils/global_data.h    \
    $$PWD/global/utils/tcp_link.h       \
    $$PWD/global/utils/globalutils.h    \
    $$PWD/global/utils/addrset.h \
    $$PWD/global/utils/inisetting.h \
    $$PWD/global/utils/rotatechangeinfo.h \
    $$PWD/global/utils/globaldateformat.h \
    $$PWD/network/protocol_analysis.h   \
    $$PWD/network/tcp_client.h \
    $$PWD/network/tcp_module.h \
    $$PWD/network/tcp_server.h \
    $$PWD/network/udp_socket.h \
    $$PWD/controller/control.h \

RESOURCES += \
    ../res.qrc

DISTFILES += \
    ../../pic/icon_plcz_b.svg \
    ../../pic/icon_plcz_w.svg

target.path = /usr/bin
target.source += $$TARGET

INSTALLS += target

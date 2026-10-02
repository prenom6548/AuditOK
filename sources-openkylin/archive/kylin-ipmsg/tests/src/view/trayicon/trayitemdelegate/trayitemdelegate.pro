TEMPLATE = app
TARGET = test_trayitemdelegate

QT += core gui widgets testlib network dbus sql KWindowSystem

CONFIG += link_pkgconfig
CONFIG -= app_bundle

PKGCONFIG += gsettings-qt glib-2.0 kysdk-kabase kysdk-qtwidgets kysdk-waylandhelper kysdk-alm kysdk-ukenv kysdk-log kysdk-diagnostics kysdk-datacollect kysdk-systime kysdk-ukuiwindowhelper peony

LIBS += -lxapian

INCLUDEPATH += ../../../../../src \
               ../../../../../src/view \
               ../../../../../src/view/titlebar \
               ../../../../../src/view/localinfo \
               ../../../../../src/global \
               ../../../../../src/global/utils \
               ../../../../../src/view/friendlist \
               ../../../../../src/controller \
               ../../../../../src/model \
               ../../../../../src/view/trayicon \
               ../../../../../kabase/Qt \
               ../../../../../kabase

SOURCES += main.cpp \
    test_trayitemdelegate.cpp \
    ../../../../../src/view/trayicon/trayitemdelegate.cpp \
    ../../../../../src/view/trayicon/trayfiltermodel.cpp \
    ../../../../../src/view/trayicon/trayiconwid.cpp \
    ../../../../../src/view/titlebar/titlebar.cpp \
    ../../../../../src/view/titlebar/menumodule.cpp \
    ../../../../../src/view/titlebar/titleseting.cpp \
    ../../../../../src/view/localinfo/localinfo.cpp \
    ../../../../../src/view/localinfo/localupdatename.cpp \
    ../../../../../src/view/kyview.cpp \
    ../../../../../src/view/common/globalsizedata.cpp \
    ../../../../../src/global/utils/inisetting.cpp \
    ../../../../../src/global/utils/global_data.cpp \
    ../../../../../src/global/utils/globalutils.cpp \
    ../../../../../src/global/utils/addrset.cpp \
    ../../../../../src/global/utils/rotatechangeinfo.cpp \
    ../../../../../src/global/utils/tcp_link.cpp \
    ../../../../../src/global/utils/globaldateformat.cpp \
    ../../../../../src/controller/control.cpp \
    ../../../../../src/model/friendlistmodel.cpp \
    ../../../../../src/model/chatmsgmodel.cpp \
    ../../../../../src/global/database/friendlistdb.cpp \
    ../../../../../src/global/database/chatmsgdb.cpp \
    ../../../../../src/global/database/database.cpp \
    ../../../../../src/view/friendlist/friendlist.cpp \
    ../../../../../src/view/friendlist/friendItemdelegate.cpp \
    ../../../../../src/view/friendlist/mysortfiltermodel.cpp \
    ../../../../../src/view/friendlist/friendinfowid.cpp \
    ../../../../../src/view/friendlist/homepagesearch/blankpage.cpp \
    ../../../../../src/view/friendlist/homepagesearch/searchmsglist.cpp \
    ../../../../../src/view/friendlist/homepagesearch/searchpage.cpp \
    ../../../../../src/view/friendlist/homepagesearch/search_msg_model.cpp \
    ../../../../../src/view/friendlist/homepagesearch/search_msg_delegate.cpp \
    ../../../../../src/view/friendlist/homepagesearch/searchfriend_delegate.cpp \
    ../../../../../src/view/chatmsg/chatmsg.cpp \
    ../../../../../src/view/chatmsg/chatmsgdelegate.cpp \
    ../../../../../src/view/chatsearch/chat_search.cpp \
    ../../../../../src/view/chatsearch/chat_search_delegate.cpp \
    ../../../../../src/view/chatsearch/chat_search_filter.cpp \
    ../../../../../src/network/tcp_client.cpp \
    ../../../../../src/network/tcp_server.cpp \
    ../../../../../src/network/udp_socket.cpp \
    ../../../../../src/network/tcp_module.cpp \
    ../../../../../src/network/protocol_analysis.cpp

HEADERS += test_trayitemdelegate.h \
    ../../../../../src/view/trayicon/trayitemdelegate.h \
    ../../../../../src/view/trayicon/trayfiltermodel.h \
    ../../../../../src/view/trayicon/trayiconwid.h \
    ../../../../../src/view/titlebar/titlebar.h \
    ../../../../../src/view/titlebar/menumodule.h \
    ../../../../../src/view/titlebar/titleseting.h \
    ../../../../../src/view/localinfo/localinfo.h \
    ../../../../../src/view/localinfo/localupdatename.h \
    ../../../../../src/view/kyview.h \
    ../../../../../src/network/tcp_server.h \
    ../../../../../src/network/tcp_client.h \
    ../../../../../src/network/udp_socket.h \
    ../../../../../src/network/tcp_module.h \
    ../../../../../src/network/protocol_analysis.h \
    ../../../../../src/controller/control.h \
    ../../../../../src/model/friendlistmodel.h \
    ../../../../../src/model/chatmsgmodel.h \
    ../../../../../src/view/friendlist/friendlist.h \
    ../../../../../src/view/friendlist/homepagesearch/searchpage.h \
    ../../../../../src/view/chatmsg/chatmsg.h \
    ../../../../../src/global/utils/inisetting.h \
    ../../../../../src/global/utils/global_data.h \
    ../../../../../src/global/utils/globalutils.h \
    ../../../../../src/global/utils/addrset.h \
    ../../../../../src/global/utils/rotatechangeinfo.h \
    ../../../../../src/global/utils/globaldateformat.h \
    ../../../../../src/global/database/friendlistdb.h \
    ../../../../../src/global/database/chatmsgdb.h \
    ../../../../../src/global/database/database.h \
    ../../../../../src/view/friendlist/friendItemdelegate.h \
    ../../../../../src/view/friendlist/mysortfiltermodel.h \
    ../../../../../src/view/friendlist/friendinfowid.h \
    ../../../../../src/view/friendlist/homepagesearch/blankpage.h \
    ../../../../../src/view/friendlist/homepagesearch/searchmsglist.h \
    ../../../../../src/view/friendlist/homepagesearch/search_msg_model.h \
    ../../../../../src/view/friendlist/homepagesearch/search_msg_delegate.h \
    ../../../../../src/view/friendlist/homepagesearch/searchfriend_delegate.h \
    ../../../../../src/view/chatmsg/chatmsgdelegate.h \
    ../../../../../src/view/chatsearch/chat_search.h \
    ../../../../../src/view/chatsearch/chat_search_delegate.h \
    ../../../../../src/view/chatsearch/chat_search_filter.h

# 添加覆盖率标志
DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

RESOURCES += ../../../../../res.qrc

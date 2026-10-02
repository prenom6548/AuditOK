TEMPLATE = app
TARGET = test_chatmsg

# Qt模块
QT += core widgets testlib  network dbus sql KWindowSystem

# C++标准
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt glib-2.0 kysdk-kabase kysdk-qtwidgets kysdk-waylandhelper kysdk-alm kysdk-ukenv kysdk-log kysdk-diagnostics kysdk-datacollect kysdk-systime kysdk-ukuiwindowhelper

LIBS +=-lpeony
# 包含路径
INCLUDEPATH += \
    ../../../../../../src \
    ../../../../../../src/view/chatmsg \
    ../../../../../../src/view/titlebar \
    ../../../../../../src/model \
    ../../../../../../src/global/database \
    ../../../../../../src/global/utils \
    ../../../../../../kabase/Qt \
    ../../../../../../kabase \

# 源文件（只包含最基本的源文件）
SOURCES += \
    main.cpp \
    test_chatmsg.cpp \
    ../../../../../../src/view/chatmsg/chatmsg.cpp \
    ../../../../../../src/view/chatsearch/chat_search.cpp \
    ../../../../../../src/view/chatmsg/chatmsgdelegate.cpp \
    ../../../../../../src/view/titlebar/titlebar.cpp \
    ../../../../../../src/model/chatmsgmodel.cpp \
    ../../../../../../src/global/database/chatmsgdb.cpp \
    ../../../../../../src/global/database/database.cpp \
    ../../../../../../src/global/utils/globalutils.cpp \
    ../../../../../../src/view/common/globalsizedata.cpp \
    ../../../../../../src/model/friendlistmodel.cpp \
    ../../../../../../src/controller/control.cpp \
    ../../../../../../src/global/utils/globaldateformat.cpp \
    ../../../../../../src/global/utils/global_data.cpp \
    ../../../../../../src/global/utils/inisetting.cpp \
    ../../../../../../src/global/utils/addrset.cpp \
    ../../../../../../src/global/utils/rotatechangeinfo.cpp \
    ../../../../../../src/global/database/friendlistdb.cpp \
    ../../../../../../src/view/friendlist/friendlist.cpp \
    ../../../../../../src/view/trayicon/trayiconwid.cpp \
    ../../../../../../src/view/localinfo/localinfo.cpp \
    ../../../../../../src/view/titlebar/menumodule.cpp \
    ../../../../../../src/view/chatsearch/chat_search_delegate.cpp \
    ../../../../../../src/view/chatsearch/chat_search_filter.cpp \
    ../../../../../../src/network/tcp_client.cpp \
    ../../../../../../src/network/tcp_server.cpp \
    ../../../../../../src/network/udp_socket.cpp \
    ../../../../../../src/view/friendlist/homepagesearch/searchmsglist.cpp \
    ../../../../../../src/view/friendlist/homepagesearch/search_msg_model.cpp \
    ../../../../../../src/view/kyview.cpp \
    ../../../../../../src/view/friendlist/friendItemdelegate.cpp \
    ../../../../../../src/view/friendlist/mysortfiltermodel.cpp \
    ../../../../../../src/view/friendlist/homepagesearch/searchpage.cpp \
    ../../../../../../src/view/friendlist/friendinfowid.cpp \
    ../../../../../../src/view/titlebar/titleseting.cpp \
    ../../../../../../src/view/trayicon/trayfiltermodel.cpp \
    ../../../../../../src/view/trayicon/trayitemdelegate.cpp \
    ../../../../../../src/view/localinfo/localupdatename.cpp \
    ../../../../../../src/global/utils/tcp_link.cpp \
    ../../../../../../src/network/tcp_module.cpp \
    ../../../../../../src/view/friendlist/homepagesearch/search_msg_delegate.cpp \
    ../../../../../../src/view/friendlist/homepagesearch/blankpage.cpp \
    ../../../../../../src/view/friendlist/homepagesearch/searchfriend_delegate.cpp \
    ../../../../../../src/network/protocol_analysis.cpp

# 头文件
HEADERS += \
    test_chatmsg.h \
    ../../../../../../src/view/chatmsg/chatmsg.h \
    ../../../../../../src/view/chatsearch/chat_search.h \
    ../../../../../../src/view/chatmsg/chatmsgdelegate.h \
    ../../../../../../src/view/titlebar/titlebar.h \
    ../../../../../../src/model/chatmsgmodel.h \
    ../../../../../../src/global/database/chatmsgdb.h \
    ../../../../../../src/global/database/database.h \
    ../../../../../../src/global/utils/globalutils.h \
    ../../../../../../src/view/common/globalsizedata.h \
    ../../../../../../src/model/friendlistmodel.h \
    ../../../../../../src/controller/control.h \
    ../../../../../../src/global/utils/globaldateformat.h \
    ../../../../../../src/global/utils/global_data.h \
    ../../../../../../src/global/utils/inisetting.h \
    ../../../../../../src/global/utils/addrset.h \
    ../../../../../../src/global/utils/rotatechangeinfo.h \
    ../../../../../../src/global/database/friendlistdb.h \
    ../../../../../../src/view/friendlist/friendlist.h \
    ../../../../../../src/view/trayicon/trayiconwid.h \
    ../../../../../../src/view/localinfo/localinfo.h \
    ../../../../../../src/view/titlebar/menumodule.h \
    ../../../../../../src/view/chatsearch/chat_search_delegate.h \
    ../../../../../../src/view/chatsearch/chat_search_filter.h \
    ../../../../../../src/network/tcp_client.h \
    ../../../../../../src/network/tcp_server.h \
    ../../../../../../src/network/udp_socket.h \
    ../../../../../../src/view/friendlist/homepagesearch/searchmsglist.h \
    ../../../../../../src/view/friendlist/homepagesearch/search_msg_model.h \
    ../../../../../../src/view/kyview.h \
    ../../../../../../src/view/friendlist/friendItemdelegate.h \
    ../../../../../../src/view/friendlist/mysortfiltermodel.h \
    ../../../../../../src/view/friendlist/homepagesearch/searchpage.h \
    ../../../../../../src/view/friendlist/friendinfowid.h \
    ../../../../../../src/view/titlebar/titleseting.h \
    ../../../../../../src/view/trayicon/trayfiltermodel.h \
    ../../../../../../src/view/trayicon/trayitemdelegate.h \
    ../../../../../../src/view/localinfo/localupdatename.h \
    ../../../../../../src/global/utils/tcp_link.h \
    ../../../../../../src/network/tcp_module.h \
    ../../../../../../src/view/friendlist/homepagesearch/search_msg_delegate.h \
    ../../../../../../src/view/friendlist/homepagesearch/blankpage.h \
    ../../../../../../src/view/friendlist/homepagesearch/searchfriend_delegate.h \
    ../../../../../../src/network/protocol_analysis.h
    

# 添加覆盖率标志
DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

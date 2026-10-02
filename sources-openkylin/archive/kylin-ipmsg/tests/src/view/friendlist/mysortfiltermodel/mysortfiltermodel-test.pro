TEMPLATE = app
TARGET = test_mysortfiltermodel
CONFIG += console gtest gmock
CONFIG -= app_bundle

# Include paths
INCLUDEPATH += \
    $$PWD/../../../../../ \
    ../../../../ \
    ../../../../../src \
    ../../../../../src/view/friendlist \
    ../../../../../src/model \
    ../../../../../src/global/utils \
    ../../../../../kabase \
    ../../../../../kabase/Qt \
    /usr/include/kysdk/kysdk-system

# Source files
SOURCES += \
    main.cpp \
    test_mysortfiltermodel.cpp \
    ../../../../../src/view/kyview.cpp \
    ../../../../../src/view/common/globalsizedata.cpp \
    ../../../../../src/view/titlebar/menumodule.cpp \
    ../../../../../src/view/titlebar/titlebar.cpp \
    ../../../../../src/view/titlebar/titleseting.cpp \
    ../../../../../src/view/localinfo/localinfo.cpp \
    ../../../../../src/view/localinfo/localupdatename.cpp \
    ../../../../../src/view/friendlist/friendlist.cpp \
    ../../../../../src/view/friendlist/friendItemdelegate.cpp \
    ../../../../../src/view/friendlist/mysortfiltermodel.cpp \
    ../../../../../src/view/friendlist/friendinfowid.cpp \
    ../../../../../src/view/chatmsg/chatmsg.cpp \
    ../../../../../src/view/chatmsg/chatmsgdelegate.cpp \
    ../../../../../src/view/trayicon/trayiconwid.cpp \
    ../../../../../src/view/trayicon/trayitemdelegate.cpp \
    ../../../../../src/view/trayicon/trayfiltermodel.cpp \
    ../../../../../src/view/chatsearch/chat_search.cpp \
    ../../../../../src/view/chatsearch/chat_search_delegate.cpp \
    ../../../../../src/view/chatsearch/chat_search_filter.cpp \
    ../../../../../src/view/friendlist/homepagesearch/blankpage.cpp \
    ../../../../../src/view/friendlist/homepagesearch/search_msg_delegate.cpp \
    ../../../../../src/view/friendlist/homepagesearch/search_msg_model.cpp \
    ../../../../../src/view/friendlist/homepagesearch/searchfriend_delegate.cpp \
    ../../../../../src/view/friendlist/homepagesearch/searchmsglist.cpp \
    ../../../../../src/view/friendlist/homepagesearch/searchpage.cpp \
    ../../../../../src/model/friendlistmodel.cpp \
    ../../../../../src/model/chatmsgmodel.cpp \
    ../../../../../src/global/database/database.cpp \
    ../../../../../src/global/database/friendlistdb.cpp \
    ../../../../../src/global/database/chatmsgdb.cpp \
    ../../../../../src/global/utils/global_data.cpp \
    ../../../../../src/global/utils/tcp_link.cpp \
    ../../../../../src/global/utils/globalutils.cpp \
    ../../../../../src/global/utils/addrset.cpp \
    ../../../../../src/global/utils/inisetting.cpp \
    ../../../../../src/global/utils/rotatechangeinfo.cpp \
    ../../../../../src/global/utils/globaldateformat.cpp \
    ../../../../../src/network/protocol_analysis.cpp \
    ../../../../../src/network/tcp_client.cpp \
    ../../../../../src/network/tcp_module.cpp \
    ../../../../../src/network/tcp_server.cpp \
    ../../../../../src/network/udp_socket.cpp \
    ../../../../../src/controller/control.cpp

# Header files
HEADERS += \
    test_mysortfiltermodel.h \
    ../../../../../src/view/kyview.h \
    ../../../../../src/view/common/globalsizedata.h \
    ../../../../../src/view/titlebar/menumodule.h \
    ../../../../../src/view/titlebar/titlebar.h \
    ../../../../../src/view/titlebar/titleseting.h \
    ../../../../../src/view/localinfo/localinfo.h \
    ../../../../../src/view/localinfo/localupdatename.h \
    ../../../../../src/view/friendlist/friendlist.h \
    ../../../../../src/view/friendlist/friendItemdelegate.h \
    ../../../../../src/view/friendlist/mysortfiltermodel.h \
    ../../../../../src/view/friendlist/friendinfowid.h \
    ../../../../../src/view/chatmsg/chatmsg.h \
    ../../../../../src/view/chatmsg/chatmsgdelegate.h \
    ../../../../../src/view/trayicon/trayiconwid.h \
    ../../../../../src/view/trayicon/trayitemdelegate.h \
    ../../../../../src/view/trayicon/trayfiltermodel.h \
    ../../../../../src/view/chatsearch/chat_search.h \
    ../../../../../src/view/chatsearch/chat_search_delegate.h \
    ../../../../../src/view/chatsearch/chat_search_filter.h \
    ../../../../../src/view/friendlist/homepagesearch/blankpage.h \
    ../../../../../src/view/friendlist/homepagesearch/search_msg_delegate.h \
    ../../../../../src/view/friendlist/homepagesearch/search_msg_model.h \
    ../../../../../src/view/friendlist/homepagesearch/searchfriend_delegate.h \
    ../../../../../src/view/friendlist/homepagesearch/searchmsglist.h \
    ../../../../../src/view/friendlist/homepagesearch/searchpage.h \
    ../../../../../src/model/friendlistmodel.h \
    ../../../../../src/model/chatmsgmodel.h \
    ../../../../../src/global/database/database.h \
    ../../../../../src/global/database/friendlistdb.h \
    ../../../../../src/global/database/chatmsgdb.h \
    ../../../../../src/global/utils/global_data.h \
    ../../../../../src/global/utils/tcp_link.h \
    ../../../../../src/global/utils/globalutils.h \
    ../../../../../src/global/utils/addrset.h \
    ../../../../../src/global/utils/inisetting.h \
    ../../../../../src/global/utils/rotatechangeinfo.h \
    ../../../../../src/global/utils/globaldateformat.h \
    ../../../../../src/network/protocol_analysis.h \
    ../../../../../src/network/tcp_client.h \
    ../../../../../src/network/tcp_module.h \
    ../../../../../src/network/tcp_server.h \
    ../../../../../src/network/udp_socket.h \
    ../../../../../src/controller/control.h

# Qt modules
QT += core gui widgets network dbus sql KWindowSystem

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt glib-2.0 kysdk-kabase kysdk-qtwidgets kysdk-waylandhelper kysdk-alm kysdk-ukenv kysdk-log kysdk-diagnostics kysdk-systime kysdk-ukuiwindowhelper kysdk-datacollect

# Coverage flags
DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov

LIBS += -lgtest -lgmock -lpthread -lpeony -lkydate -lkysysinfo -lxapian

RESOURCES += ../../../../../res.qrc

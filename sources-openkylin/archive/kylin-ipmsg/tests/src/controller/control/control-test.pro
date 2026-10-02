QT += core network dbus testlib gui widgets sql KWindowSystem
TEMPLATE = app
TARGET = test_control

CONFIG += console gtest gmock link_pkgconfig

DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov

# 同步 src.pro 的 PKGCONFIG
PKGCONFIG += gsettings-qt glib-2.0 kysdk-kabase kysdk-qtwidgets kysdk-waylandhelper kysdk-alm kysdk-ukenv kysdk-log kysdk-diagnostics kysdk-datacollect kysdk-systime kysdk-ukuiwindowhelper

# 同步 src.pro 的 INCLUDEPATH
INCLUDEPATH += ../../../../src \
               ../../../../src/network \
               ../../../../src/global/utils \
               ../../../../src/global/database \
               ../../../../src/model \
               ../../../../src/view/common \
               ../../../../src/view/friendlist \
               ../../../../src/view/chatmsg \
               ../../../../src/view/trayicon \
               ../../../../src/view/localinfo \
               ../../../../src/view/friendlist/homepagesearch \
               ../../../../src/view/chatsearch \
               ../../../../src/view/titlebar \
               ../../../../kabase \
               ../../../../kabase/Qt

SOURCES += \
    main.cpp \
    test_control.cpp \
    stub_all.cpp \
    stub_view.cpp \
    fake_ui.cpp \
    ../../../../src/controller/control.cpp \
    ../../../../src/global/database/database.cpp \
    ../../../../src/global/database/friendlistdb.cpp \
    ../../../../src/global/database/chatmsgdb.cpp \
    ../../../../src/global/utils/global_data.cpp \
    ../../../../src/global/utils/globalutils.cpp \
    ../../../../src/global/utils/rotatechangeinfo.cpp \
    ../../../../src/view/common/globalsizedata.cpp

HEADERS += \
    test_control.h \
    ../../../../src/controller/control.h \
    ../../../../src/network/udp_socket.h \
    ../../../../src/network/tcp_client.h \
    ../../../../src/network/tcp_module.h \
    ../../../../src/network/tcp_server.h \
    ../../../../src/network/protocol_analysis.h \
    ../../../../src/global/utils/addrset.h \
    ../../../../src/global/utils/inisetting.h \
    ../../../../src/global/utils/global_data.h \
    ../../../../src/global/utils/tcp_link.h \
    ../../../../src/global/utils/globalutils.h \
    ../../../../src/global/utils/rotatechangeinfo.h \
    ../../../../src/global/utils/globaldateformat.h \
    ../../../../src/global/database/database.h \
    ../../../../src/global/database/friendlistdb.h \
    ../../../../src/global/database/chatmsgdb.h \
    ../../../../src/model/friendlistmodel.h \
    ../../../../src/model/chatmsgmodel.h \
    ../../../../src/view/common/globalsizedata.h \
    ../../../../src/view/chatmsg/chatmsg.h \
    ../../../../src/view/friendlist/friendlist.h \
    ../../../../src/view/trayicon/trayiconwid.h \
    ../../../../src/view/localinfo/localinfo.h \
    ../../../../src/view/titlebar/titlebar.h \
    ../../../../src/view/chatsearch/chat_search.h \
    ../../../../src/view/chatmsg/chatmsgdelegate.h \
    ../../../../src/view/chatsearch/chat_search_filter.h \
    ../../../../src/view/friendlist/homepagesearch/searchmsglist.h \
    ../../../../src/view/friendlist/homepagesearch/search_msg_model.h \
    ../../../../src/view/kyview.h

LIBS += -lgtest -lgmock -lpthread -lpeony

target.path = /tmp/tests
INSTALLS += target

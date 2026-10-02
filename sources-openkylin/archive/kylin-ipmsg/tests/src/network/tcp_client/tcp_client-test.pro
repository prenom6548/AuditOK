QT += core network dbus testlib gui widgets sql KWindowSystem
TEMPLATE = app
TARGET = test_tcp_client

CONFIG += console gtest gmock link_pkgconfig

DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov -lgtest -lgmock -lpthread -lpeony

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
               ../../../../kabase \
               ../../../../kabase/Qt

SOURCES += \
    main.cpp \
    test_tcp_client.cpp \
    stub_view.cpp \
    ../../../../src/network/tcp_client.cpp \
    ../../../../src/network/tcp_module.cpp \
    ../../../../src/network/protocol_analysis.cpp \
    ../../../../src/global/utils/addrset.cpp \
    ../../../../src/global/utils/inisetting.cpp \
    ../../../../src/global/utils/global_data.cpp \
    ../../../../src/global/utils/tcp_link.cpp \
    ../../../../src/global/utils/globalutils.cpp \
    ../../../../src/global/utils/rotatechangeinfo.cpp \
    ../../../../src/global/database/database.cpp \
    ../../../../src/global/database/friendlistdb.cpp \
    ../../../../src/global/database/chatmsgdb.cpp \
    ../../../../src/model/friendlistmodel.cpp \
    ../../../../src/model/chatmsgmodel.cpp \
    ../../../../src/view/common/globalsizedata.cpp

HEADERS += \
    test_tcp_client.h \
    ../../../../src/network/tcp_client.h \
    ../../../../src/network/tcp_module.h \
    ../../../../src/network/protocol_analysis.h \
    ../../../../src/global/utils/addrset.h \
    ../../../../src/global/utils/inisetting.h \
    ../../../../src/global/utils/global_data.h \
    ../../../../src/global/utils/tcp_link.h \
    ../../../../src/global/utils/globalutils.h \
    ../../../../src/global/utils/rotatechangeinfo.h \
    ../../../../src/global/database/database.h \
    ../../../../src/global/database/friendlistdb.h \
    ../../../../src/global/database/chatmsgdb.h \
    ../../../../src/model/friendlistmodel.h \
    ../../../../src/model/chatmsgmodel.h \
    ../../../../src/view/common/globalsizedata.h

target.path = /tmp/tests
INSTALLS += target

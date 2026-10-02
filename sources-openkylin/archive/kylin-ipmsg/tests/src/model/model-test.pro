TEMPLATE = app
TARGET = test_model
CONFIG += console gtest gmock
CONFIG -= app_bundle

# Include paths
INCLUDEPATH += ../../../
INCLUDEPATH += ../../../src
INCLUDEPATH += ../../../src/model
INCLUDEPATH += ../../../src/global/database
INCLUDEPATH += ../../../src/global/utils
INCLUDEPATH += ../../../src/view/friendlist
INCLUDEPATH += ../../../src/view/trayicon
INCLUDEPATH += /usr/include/kysdk/kysdk-system

# Source files
SOURCES += \
    main.cpp \
    test_chatmsgmodel.cpp \
    test_friendlistmodel.cpp \
    ../view/friendlist/homepagesearch/fake_ui.cpp \
    ../../../src/model/chatmsgmodel.cpp \
    ../../../src/model/friendlistmodel.cpp \
    ../../../src/global/database/chatmsgdb.cpp \
    ../../../src/global/database/database.cpp \
    ../../../src/global/database/friendlistdb.cpp \
    ../../../src/global/utils/globalutils.cpp \
    ../../../src/global/utils/inisetting.cpp

# Header files
HEADERS += \
    ../../../src/model/chatmsgmodel.h \
    ../../../src/model/friendlistmodel.h \
    ../../../src/global/database/chatmsgdb.h \
    ../../../src/global/database/database.h \
    ../../../src/global/database/friendlistdb.h \
    ../../../src/global/utils/globalutils.h \
    ../../../src/global/utils/inisetting.h

# Qt modules
QT += core gui widgets sql testlib network dbus KWindowSystem

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt glib-2.0 kysdk-kabase kysdk-qtwidgets kysdk-waylandhelper kysdk-alm kysdk-ukenv kysdk-log kysdk-diagnostics kysdk-datacollect kysdk-systime kysdk-ukuiwindowhelper

# Coverage flags
DEFINES += TEST_MODE
QMAKE_CXXFLAGS += --coverage -fbranch-count-reg -fno-exceptions
QMAKE_LDFLAGS += --coverage
LIBS += -lgcov -lgtest -lgmock -lpthread


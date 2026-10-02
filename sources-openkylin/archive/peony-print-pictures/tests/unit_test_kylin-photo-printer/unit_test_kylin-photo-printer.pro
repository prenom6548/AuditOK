QT += \ 
    core          \
    gui           \
    dbus          \
    printsupport \
    testlib

TARGET = unit_test_kylin-photo-printer

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
include ($$PWD/../../3rdparty/kylinimagecodec/kylinimagecodec.pro)
TEMPLATE = app

LIBS += -lpthread

# 配置gsettings
CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt 

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov
TEST_UTILS_PATH = $$PWD/../kt-test-utils
INCLUDEPATH += \
    $$PWD \
    $$TEST_UTILS_PATH/cpp-stub \
    $$TEST_UTILS_PATH/cpp-stub-ext

CONFIG += \
    c++11

INCLUDEPATH += \
    $$PWD/.

SOURCES += \
    $$PWD/../../kylin-photo-printer/photo_printer.cpp \
    $$PWD/unit_test_kylin-photo-printer.cpp \
    $$PWD/main.cpp

HEADERS += \
    $$PWD/../../kylin-photo-printer/photo_printer.h

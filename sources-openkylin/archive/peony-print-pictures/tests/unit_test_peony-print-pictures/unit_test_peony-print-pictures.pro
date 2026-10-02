QT       += core widgets printsupport

TARGET = unit_test_peony-print-pictures

TEMPLATE = app

PKGCONFIG += peony gsettings-qt

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov

CONFIG += link_pkgconfig
LIBS += -lgcov

CONFIG += debug link_pkgconfig

INCLUDEPATH += "/usr/include/peony-qt"
PKGCONFIG += gsettings-qt gio-2.0


DISTFILES += $$PWD/../../peony-print-pictures/common.json


TEST_UTILS_PATH = $$PWD/../kt-test-utils


INCLUDEPATH += \
    $$PWD \
    $$TEST_UTILS_PATH/cpp-stub \
    $$TEST_UTILS_PATH/cpp-stub-ext

SOURCES += \
    $$PWD/../../peony-print-pictures/print-pictures-plugin.cpp \
    $$PWD/../../peony-print-pictures/custom_log.cpp \
    $$PWD/unit_test_peony-print-pictures.cpp \
    $$PWD/main.cpp

HEADERS += \
    $$PWD/../../peony-print-pictures/print-pictures-plugin.h \
    $$PWD/../../peony-print-pictures/peony-print-pictures_global.h \
    $$PWD/../../peony-print-pictures/custom_log.h

#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += core dbus KService KCoreAddons KXmlGui KGlobalAccel x11extras
QT -= gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent sql xml
TEMPLATE = app

TARGET = unit_test_about
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov -lukuiserviceinterface

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0 \
             kysdk-sysinfo \
             gsettings-qt \

#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../../registeredSession/plugins/about/aboutinterface.cpp \
           ../../registeredSession/universalinterface.cpp \
           unit_test_about.cpp \
           main.cpp

HEADERS += ../../registeredSession/plugins/about/aboutinterface.h \
           ../../registeredSession/universalinterface.h


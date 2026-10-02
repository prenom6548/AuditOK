#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus
QT -= gui
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

TARGET = unit_test_boot
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov -lukuiserviceinterface

CONFIG += link_pkgconfig

PKGCONFIG += kysdk-sysinfo \

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

DEFINES += UNIT_TEST_BUILD

#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../../registeredSession/plugins/boot/bootinterface.cpp \
           ../../registeredSession/universalinterface.cpp \
           unit_test_boot.cpp \
           main.cpp

HEADERS += ../../registeredSession/plugins/boot/bootinterface.h \
           ../../registeredSession/universalinterface.h


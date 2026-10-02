#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus widgets
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0 gio-unix-2.0
TEMPLATE = app

TARGET = unit_test_plugin_bioproxy
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../../biometric-plugin/biometricdeviceinfo.cpp \
           ../../biometric-plugin/biometricproxy.cpp \
           ../../biometric-plugin/giodbus.cpp \
           unit_test_plugin_bioproxy.cpp \
           main.cpp

HEADERS += ../../biometric-plugin/biometricdeviceinfo.h \
           ../../biometric-plugin/biometricproxy.h \
           ../../biometric-plugin/giodbus.h

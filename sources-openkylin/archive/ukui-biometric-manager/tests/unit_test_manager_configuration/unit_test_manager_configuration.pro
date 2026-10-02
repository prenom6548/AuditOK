#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus svg
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
CONFIG += link_pkgconfig
PKGCONFIG += gio-2.0 gio-unix-2.0
TEMPLATE = app

DEFINES += APP_API_MAJOR=0  \
            APP_API_MINOR=11    \
            APP_API_FUNC=0

TARGET = unit_test_manager_configuration
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

SOURCES += ../../biometric-manager/src/configuration.cpp \
           ../../biometric-manager/src/customtype.cpp \
           ../../biometric-manager/src/giodbus.cpp \
           ../../biometric-manager/src/imageutil.cpp \
           unit_test_manager_configuration.cpp \
           main.cpp

HEADERS += ../../biometric-manager/src/configuration.h \
            ../../biometric-manager/src/customtype.h \
            ../../biometric-manager/src/giodbus.h \
            ../../biometric-manager/src/imageutil.h


#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

DEFINES += APP_API_MAJOR=0  \
            APP_API_MINOR=11    \
            APP_API_FUNC=0

TARGET = unit_test_service_manager
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

SOURCES += ../../ukui-greeter/greeter/servicemanager.cpp \
           unit_test_service_manager.cpp \
           main.cpp

HEADERS += ../../ukui-greeter/greeter/servicemanager.h


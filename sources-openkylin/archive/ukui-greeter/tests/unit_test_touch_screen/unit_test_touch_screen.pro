#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus x11extras
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

DEFINES += APP_API_MAJOR=0  \
            APP_API_MINOR=11    \
            APP_API_FUNC=0

TARGET = unit_test_touch_screen
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

CONFIG += link_pkgconfig

PKGCONFIG += x11 \
             xi

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

SOURCES += ../../ukui-greeter/common/touchscreenset.cpp \
           ../../ukui-greeter/common/monitorwatcher.cpp \
           unit_test_touch_screen.cpp \
           main.cpp

HEADERS += ../../ukui-greeter/common/touchscreenset.h \
           ../../ukui-greeter/common/monitorwatcher.h

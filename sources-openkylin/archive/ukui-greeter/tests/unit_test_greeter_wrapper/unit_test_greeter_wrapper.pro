#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets x11extras svg
TEMPLATE = app

DEFINES += APP_API_MAJOR=0  \
            APP_API_MINOR=11    \
            APP_API_FUNC=0

TARGET = unit_test_greeter_wrapper
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

CONFIG += link_pkgconfig

PKGCONFIG += x11 \
             liblightdm-qt5-3 \
             gsettings-qt \
             imlib2 \
             gio-2.0 \
             kysdk-sysinfo

#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../../ukui-greeter/greeter/greeterwrapper.cpp \
           ../../ukui-greeter/greeter/rootWindowBackground.cpp \
           ../../ukui-greeter/common/globalv.cpp \
           ../../ukui-greeter/common/configuration.cpp \
           ../../ukui-greeter/common/glibinterface.cpp \
           unit_test_greeter_wrapper.cpp \
           ../kt-test-utils/cpp-stub-ext/stub-shadow.cpp \
           main.cpp

HEADERS += ../../ukui-greeter/greeter/greeterwrapper.h \
           ../../ukui-greeter/greeter/rootWindowBackground.h \
           ../../ukui-greeter/common/globalv.h \
           ../../ukui-greeter/common/configuration.h \
           ../../ukui-greeter/common/glibinterface.h \
           ../kt-test-utils/cpp-stub-ext/stubext.h \
           ../kt-test-utils/cpp-stub-ext/stub-shadow.h


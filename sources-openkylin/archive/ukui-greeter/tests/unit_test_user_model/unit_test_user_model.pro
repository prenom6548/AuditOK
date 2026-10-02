#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus svg x11extras
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

DEFINES += APP_API_MAJOR=0  \
            APP_API_MINOR=11    \
            APP_API_FUNC=0

TARGET = unit_test_user_model
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

PKGCONFIG += gsettings-qt  \
             x11 \
             xtst \
             liblightdm-qt5-3
#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../../ukui-greeter/greeter/usersmodel.cpp \
           ../../ukui-greeter/greeter/proxymodel.cpp \
           ../../ukui-greeter/greeter/securityuser.cpp \
           ../../ukui-greeter/common/xeventmonitor.cpp \
           unit_test_user_model.cpp \
           main.cpp

HEADERS += ../../ukui-greeter/greeter/usersmodel.h \
           ../../ukui-greeter/greeter/proxymodel.h \
           ../../ukui-greeter/greeter/securityuser.h \
           ../../ukui-greeter/common/xeventmonitor.h


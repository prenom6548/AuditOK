#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus xml KService KCoreAddons KXmlGui KGlobalAccel
#greaterThan(QT_MAJOR_VERSION, 4):
QT += widgets concurrent sql xml
QT -= gui
TEMPLATE = app

TARGET = unit_test_default
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

LIBS          += -lgtest_main -lpthread -lpolkit-qt5-core-1
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov -ldconf -lukuiserviceinterface

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt \
	     kysdk-sysinfo \
	     gio-2.0 \

INCLUDEPATH += ../libukcc/interface/ \
               ../libukcc/ \
		/usr/include/dconf

#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../../registeredSession/plugins/default/defaultinterface.cpp \
           unit_test_default.cpp \
           main.cpp

HEADERS += ../../registeredSession/plugins/default/defaultinterface.h \

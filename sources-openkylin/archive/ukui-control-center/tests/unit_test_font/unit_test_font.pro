#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT += dbus xml
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets concurrent sql xml
TEMPLATE = app

TARGET = unit_test_font
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control -fno-exceptions

LIBS          += -lgtest_main -lpthread
LIBS          +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt  -lgtest -lgcov -ldconf -lfontconfig -lfreetype -lukuiserviceinterface

#OBJECTS_DIR = ./obj
#MOC_DIR = ./moc

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt \
	     gio-2.0 \
             freetype2 \
             fontconfig \
             kysdk-sysinfo \
             kysdk-ukenv \

INCLUDEPATH += /usr/include/dconf \
               /usr/include/kysdk

#打桩工具
TEST_UTILS_PATH = $$PWD/../kt-test-utils

# Include directories
INCLUDEPATH += $$PROJECT_ROOTDIR \
               $$TEST_UTILS_PATH/cpp-stub \
               $$TEST_UTILS_PATH/cpp-stub-ext \

SOURCES += ../../registeredSession/plugins/font/libfun.cpp \
           ../../registeredSession/plugins/font/external.cpp \
           ../../registeredSession/plugins/font/fontinterface.cpp \
	   ../../registeredSession/universalinterface.cpp \
           unit_test_font.cpp \
           main.cpp

HEADERS += ../../registeredSession/plugins/font/libfun.h \
          ../../registeredSession/plugins/font/external.h \
          ../../registeredSession/plugins/font/fontinterface.h \
	  ../../registeredSession/universalinterface.h \

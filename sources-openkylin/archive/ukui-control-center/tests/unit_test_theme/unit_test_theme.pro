#-------------------------------------------------
#
# Project created by QtCreator 2019-08-22T11:12:59
#
#-------------------------------------------------
QT       += dbus
QT -= gui
#greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TEMPLATE = app

TARGET = unit_test_theme
target.source += $$TARGET
target.path = ./

#代码覆盖率工具gcov
QMAKE_LFLAGS +=-fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage -fno-inline -fno-access-control

INCLUDEPATH   += $$PROJECT_ROOTDIR

LIBS +=  -lgtest_main -lpthread
LIBS +=  -L$$[QT_INSTALL_LIBS] -lgsettings-qt -lgtest -lgcov -lukuiserviceinterface

CONFIG += link_pkgconfig
PKGCONFIG += gsettings-qt

#OBJECTS_DIR = ./obj/
#MOC_DIR = ./moc/

SOURCES += ../../registeredSession/plugins/theme/themeinterface.cpp \
           ../../registeredSession/universalinterface.cpp \
           unit_test_theme.cpp \
           main.cpp

HEADERS += ../../registeredSession/plugins/theme/themeinterface.h \
           ../../registeredSession/universalinterface.h

INSTALLS += target


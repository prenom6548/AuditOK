#QT -= gui
QT += core widgets

CONFIG += c++17 console link_pkgconfig
CONFIG -= app_bundle

TARGET = inputTest
TEMPLATE = app

LIBS += -lgcov

LIBS += -lgtest -lgtest_main -lpthread

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DEFINES += QT_DEPRECATED_WARNINGS

QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
QMAKE_CXXFLAGS += --coverage

SOURCES += \
    ../gesture-extension-manager/gesture-extension-manager-private.cpp \
    ../gesture-extension-manager/gesture-extension-manager.cpp \
    ../gesture-extensions/slide-gesture.cpp

HEADERS += \
    ../gesture-extension-manager/gesture-extension-manager-private.h \
    ../gesture-extension-manager/gesture-extension-manager.h \
    ../gesture-extensions/gesture-extension.h \
    ../gesture-extensions/slide-gesture.h \

SOURCES += main.cpp

## Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target

test.target = test
test.commands = ./inputTest
test.depends = $(TARGET)
test.CONFIG = no_testcase_targets

QMAKE_EXTRA_TARGETS += test

target.path = $$[QT_INSTALL_TESTS]/$$[TARGET]
INSTALLS += target

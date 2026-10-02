QT += testlib core dbus
QT -= gui

TARGET = test_systemdbusregister
TEMPLATE = app

CONFIG += c++11 console
CONFIG -= app_bundle

# 包含覆盖率标志
QMAKE_CXXFLAGS += --coverage -fno-exceptions
QMAKE_LFLAGS += --coverage -lgcov

# 包含源代码目录和PolkitQt头文件目录
INCLUDEPATH += ../../../registeredQDbus ../../../registeredQDbus/conf /usr/include/polkit-qt5-1

# 包含PolkitQt库
LIBS += -L/usr/lib/x86_64-linux-gnu/ -lpolkit-qt5-core-1
LIBS += -lukui-log4qt

SOURCES += \
    test_systemdbusregister.cpp \
    ../../../registeredQDbus/systemdbusregister.cpp

HEADERS += \
    test_systemdbusregister.h \
    ../../../registeredQDbus/systemdbusregister.h

DISTFILES += \
    ../../../registeredQDbus/conf/com.kylinusbcreator.systemdbus.conf \
    ../../../registeredQDbus/conf/com.kylinusbcreator.systemdbus.policy \
    ../../../registeredQDbus/conf/com.kylinusbcreator.systemdbus.service
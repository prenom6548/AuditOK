QT -= gui
QT += network

TEMPLATE = lib
TARGET = globalManager


#link_pkgconfig no_keywords plugin app_bundlels 必须要加否则会出现so文件版本的链接
CONFIG += c++11  link_pkgconfig no_keywords plugin app_bundlels

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS MODULE_NAME=\\\"{$$TARGET}\\\"

INCLUDEPATH += \
        -I ukui-settings-daemon/

include($$PWD/../../common/common.pri)
# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \ \
    global-manager-plugin.cpp \
    global-manager.cpp


HEADERS += \ \
    global-manager-plugin.h \
    global-manager.h

# Default rules for deployment.
#    OTHER_FILES += org.ukui.peripherals-keyboard.demoplugin.xml


    {$$TARGET}plugin_lib.path = $${PLUGIN_INSTALL_DIRS}
    {$$TARGET}plugin_lib.files =  $$OUT_PWD/lib{$$TARGET}.so

    {$$TARGET}plugin_info.path = $$[QT_INSTALL_LIBS]/ukui-settings-daemon
    {$$TARGET}plugin_info.files = $$OUT_PWD/{$$TARGET}.ukui-settings-plugin

    {$$TARGET}plugin_schema.path = /usr/share/glib-2.0/schemas/
    {$$TARGET}plugin_schema.files = $$OUT_PWD/org.ukui.SettingsDaemon.plugins.{$$TARGET}.gschema.xml

#禁用优化
QMAKE_CFLAGS_DEBUG -= O2
QMAKE_CXXFLAGS_DEBUG -= -O2

QMAKE_CFLAGS_RELEASE -= O2
QMAKE_CXXFLAGS_RELEASE -= -O2

#lib must at last
INSTALLS +=  {$$TARGET}plugin_lib {$$TARGET}plugin_info {$$TARGET}plugin_schema

DISTFILES += \
    {$$TARGET}.ukui-settings-plugin \
    org.ukui.SettingsDaemon.plugins.{$$TARGET}.gschema.xml

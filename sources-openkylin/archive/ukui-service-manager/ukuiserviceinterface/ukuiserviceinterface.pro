QT -= gui

TEMPLATE = lib
DEFINES += UKUISERVICEINTERFACE_LIBRARY

VERSION = 0.1.0

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    serviceobject.cpp

HEADERS += \
    serviceinterface.h \
    serviceobject.h

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

header.path = /usr/include/ukui-service-interface
header.files += serviceinterface.h serviceobject.h
INSTALLS += header

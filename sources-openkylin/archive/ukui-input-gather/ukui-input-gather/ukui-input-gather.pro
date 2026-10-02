QT -= gui
# libqt5network5
QT += network
QT += dbus

CONFIG += c++11 console
CONFIG -= app_bundle

LIBS +=-L/usr/lib/x86_64-linux-gnu/ -linput -ludev -levdev

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += src/ /usr/include/libevdev-1.0

HEADERS += \
    src/event.h \
    src/inputeventgather.h \
    src/libinputdbusserver.h \
    src/libinputeventgather.h \
    src/localserver.h \
    src/socketserver.h

SOURCES += \
    src/inputeventgather.cpp \
    src/libinputdbusserver.cpp \
    src/libinputeventgather.cpp \
    src/localserver.cpp \
    src/main.cpp \
    src/socketserver.cpp

serviceFile.files = installation/ukui-input-gather.service
serviceFile.path  = /lib/systemd/system/

dbusservice.files += installation/org.ukui.libinput.proxy.service
dbusservice.path += /usr/share/dbus-1/system-services/

dbusconf.files += installation/org.ukui.libinput.proxy.conf
dbusconf.path += /usr/share/dbus-1/system.d/

limit1.files += installation/org.ukui.libinput.proxy.limit
limit1.files += installation/org.ukui.libinput.proxy.limit.verify
limit1.path += /etc/dbus-1/conf/
limit2.files += installation/org.ukui.libinput.proxy.limit
limit2.files += installation/org.ukui.libinput.proxy.limit.verify
limit2.path += /usr/share/dbus-1/conf/
conf.files += installation/org.ukui.libinput.proxy.yaml
conf.path += /etc/kylin-config/basic/

# Default rules for deployment.
qnx: target.path = /usr/bin
else: unix:!android: target.path = /usr/bin
!isEmpty(target.path): INSTALLS += target serviceFile dbusconf dbusservice
INSTALLS += limit1 limit2 conf

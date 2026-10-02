TEMPLATE = lib
TARGET = groupManagerServer
INCLUDEPATH += .

QT += dbus core
# The following define makes your compiler warn you if you use any
# feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Input
DBUS_ADAPTORS += org.ukui.groupmanager.xml
DBUS_INTERFACES += org.ukui.groupmanager.xml


CONFIG += console c++11 link_pkgconfig plugin
PKGCONFIG     += gio-2.0 \
                 gio-unix-2.0 \

LIBS          += -L$$[QT_INSTALL_LIBS] -lpolkit-qt5-core-1 -lukuiserviceinterface

HEADERS +=  custom_struct.h \
            group_manager_server.h \
            groupmanagerdbusregisterservice.h
SOURCES +=  group_manager_server.cpp \
            groupmanagerdbusregisterservice.cpp

target.path = /usr/libexec/ukui-services/system
!isEmpty(target.path): INSTALLS += target

dbus_conf.path = /etc/dbus-1/system.d
dbus_conf.files +=  conf/org.ukui.groupmanager.conf
INSTALLS += dbus_conf

systemd_service.path = /usr/share/dbus-1/system-services/
systemd_service.files += conf/org.ukui.groupmanager.service
INSTALLS += systemd_service

polkit.path = /usr/share/polkit-1/actions/
polkit.files += conf/org.ukui.groupmanager.policy
INSTALLS += polkit

limit1.path = /etc/dbus-1/conf/
limit1.files = conf/org.ukui.groupmanager.limit
limit2.path = /usr/share/dbus-1/conf/
limit2.files = conf/org.ukui.groupmanager.limit
verify1.path = /etc/dbus-1/conf/
verify1.files = conf/org.ukui.groupmanager.limit.verify
verify2.path = /usr/share/dbus-1/conf/
verify2.files = conf/org.ukui.groupmanager.limit.verify
yaml.path = /etc/kylin-config/basic/
yaml.files = conf/org.ukui.groupmanager.yaml

INSTALLS += limit1 \
            limit2 \
            verify1 \
            verify2 \
            yaml


DISTFILES += \
    metadata.json








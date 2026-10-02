include(src/src.pri)
QT       += dbus svg sql core gui xml KWindowSystem KService KCoreAddons
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET   = ukui-notifications
TEMPLATE = app
CONFIG  += c++11 link_pkgconfig

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += PROTOCOLVERSION=1.2
DEFINES += SERVERVERSION=3.1
LIBS += -lX11 -lgio-2.0 -lgobject-2.0 -lglib-2.0 -ldl -lukui-com4c -lukui-com4cxx -lukui-log4qt
LIBS += -L/usr/lib/kysdk/kysdk-base/
PKGCONFIG  += gsettings-qt \
            dconf \
            libcanberra \
            kysdk-waylandhelper \

SOURCES += \
        src/main.cpp

HEADERS += \

isEmpty(PREFIX){
    PREFIX = /usr
}

QMAKE_CFLAGS += -D_FORTIFY_SOURCE=2 -O2

#DBUS_ADAPTORS += ./src/org.freedesktop.Notifications.xml
#DBUS_INTERFACES += ./src/org.freedesktop.Notifications.xml

orgDBus.input = file/org.ukui.freedesktop.Notification.service.in
orgDBus.output = file/org.ukui.freedesktop.Notification.service
QMAKE_SUBSTITUTES += orgDBus
QMAKE_CLEAN       += $${orgDBus.output}

# Default rules for deployment.
dbus.files += file/org.ukui.freedesktop.Notification.service
dbus.path = /usr/share/dbus-1/services

schemes.files += data/org.ukui.notification.gschema.xml
schemes.path = /usr/share/glib-2.0/schemas/
schemes1.files += data/org.ukui.log4qt.ukui-notification.gschema.xml
schemes1.path = /usr/share/glib-2.0/schemas/

target.path = $${PREFIX}/lib/ukui-notification-daemon

TRANSLATIONS   += translations/ukui-notification-daemon_zh_CN.ts \
                  translations/ukui-notification-daemon_bo_CN.ts

translation.path = /usr/share/ukui-notification-daemon
translation.files +=  translations/ukui-notification-daemon_zh_CN.qm \
                      translations/ukui-notification-daemon_bo_CN.qm



INSTALLS += target dbus schemes schemes1 translation


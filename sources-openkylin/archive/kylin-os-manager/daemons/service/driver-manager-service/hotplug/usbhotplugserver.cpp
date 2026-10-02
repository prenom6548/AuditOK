#include "usbhotplugserver.h"
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QDebug>
#include "dbus_args.h"

UsbHotplugServer::UsbHotplugServer(QObject *parent) : QObject(parent)
{
}

void UsbHotplugServer::emitSingal(bool plugin, const QString devString)
{
    if (plugin) {
        QDBusMessage devsignal = QDBusMessage::createSignal(
            dbus::DBUS_USB_HOT_INFO_TOOL, dbus::DBUS_USB_HOT_INFO_INTERFACE_TOOL, "newDeviceConnectSignal");
        devsignal.setArguments(QList<QVariant>() << devString);
        QDBusConnection::systemBus().send(devsignal);
    } else {
        QDBusMessage devsignal = QDBusMessage::createSignal(
            dbus::DBUS_USB_HOT_INFO_TOOL, dbus::DBUS_USB_HOT_INFO_INTERFACE_TOOL, "deviceRemoveSignal");
        devsignal.setArguments(QList<QVariant>() << devString);
        QDBusConnection::systemBus().send(devsignal);
    }
}

void UsbHotplugServer::installSucceed(const QString driverinfo) {
    QDBusMessage devsignal = QDBusMessage::createSignal(dbus::DBUS_USB_HOT_INFO_TOOL, dbus::DBUS_USB_HOT_INFO_INTERFACE_TOOL,"installDriverSuccessSignal");
    devsignal.setArguments(QList<QVariant>() << driverinfo);
    QDBusConnection::systemBus().send(devsignal);
}

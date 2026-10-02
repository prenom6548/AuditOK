#include "deviceinfodbusservice.h"
#include "deviceinfodatabase.h"
#include <QJsonDocument>
#include <QDebug>
#include "include/common.h"
DeviceInfoDbusService::DeviceInfoDbusService(QObject *parent)
        : QObject(parent)
{
}

QString DeviceInfoDbusService::QueryDevices(const QString &pid,
                                            const QString &vid,
                                            const QString &vendor,
                                            const QString &model,
                                            const QString &type)
{
    QJsonObject result = DeviceInfoDatabase::instance()->queryDevices(pid, vid, vendor, model, type);
    QString rtr_result = QString::fromUtf8(QJsonDocument(result).toJson(QJsonDocument::Compact));
    qDebug() << "QueryDevices :" << pid << " " << vid << " " << vendor << " " << model << " " << type;
    qDebug() << "QueryDevices result:" << rtr_result;
    return rtr_result;
}

bool DeviceInfoDbusService::rigisterService() {
    QDBusConnection systemBus = QDBusConnection::systemBus();
    if(!systemBus.isConnected()){
        qDebug() << "无法连接到 D-Bus system bus";
        return false;
    }

    if(!systemBus.registerService(DBUS_SERVICE_NAME)){
        qDebug() << "无法注册服务：" << DBUS_SERVICE_NAME;
        return false;
    }

    if (!systemBus.registerObject(DBUS_OBJECT_PATH, DBUS_OBJECT_INTERFACE, this, QDBusConnection::ExportAllSlots)) {
        qDebug() << "无法注册 D-Bus 对象：" << DBUS_OBJECT_PATH;
        return false;
    }

    return true;
}

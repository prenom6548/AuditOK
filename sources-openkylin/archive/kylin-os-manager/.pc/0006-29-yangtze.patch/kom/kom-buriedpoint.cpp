#include "kom-buriedpoint.h"
#include <libkydiagnostics.h>
#include <QDebug>
namespace kom
{

QString BuriedPoint::getBuriedPointTypeString(BuriedPointType type)
{
    switch (type) {
    case BuriedPointType::MainApplication:
        return QString("MainApplication");
    case BuriedPointType::PluginsServiceSupport:
        return QString("PluginsServiceSupport");
    case BuriedPointType::PluginsToolBox:
        return QString("PluginsToolBox");
    case BuriedPointType::PluginsWinDataMigration:
        return QString("PluginsWinDataMigration");
    case BuriedPointType::PluginsRemoteAssistanceTool:
        return QString("PluginsRemoteAssistanceTool");
    case BuriedPointType::ServiceMonitor:
        return QString("ServiceMonitor");
    default:
        return QString("UnknowType");
    }
    return QString("UnknowType");
}

bool BuriedPoint::uploadMessage(BuriedPointType messageType, QMap<QString, QString> data)
{
    KBuriedPoint kp[data.count()];
    memset(kp, '\0', sizeof(kp));
    for (int i = 0; i < data.count(); i++) {
        QString key = data.keys().at(i);
        kp[i].key = strdup(key.toUtf8().data());
        kp[i].value = strdup(data.value(key).toUtf8().data());
    }
    QString typeString = getBuriedPointTypeString(messageType);
#ifdef DEBUG_MODE
    qDebug() << "埋点信息：" << typeString << kp[0].key << kp[0].value;
#endif
    int result = kdk_buried_point("kylin-os-manager", typeString.toUtf8().data(), kp, data.count());
    for (int i = 0; i < data.count(); i++) {
        delete kp[i].key;
        delete kp[i].value;
    }
    if (result == 0) {
        return true;
    }
    qDebug() << "埋点失败！！";
    return false;
}

bool BuriedPoint::uploadMessage(BuriedPointType messageType, const QString &key, const QString &value)
{
    QMap<QString, QString> data;
    data.insert(key, value);
    return uploadMessage(messageType, data);
}

bool BuriedPoint::uploadMessage(BuriedPointType messageType, const QString &msg)
{
    QMap<QString, QString> data;
    data.insert("Event", msg);
    return uploadMessage(messageType, data);
}

} // namespace kom

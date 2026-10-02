#include "batteryinfofinder.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

BatteryInfoFinder::BatteryInfoFinder()
{
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &BatteryInfoFinder::dowork);

    connect(this, &BatteryInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &BatteryInfoFinder::finished, this, &BatteryInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

BatteryInfoFinder::~BatteryInfoFinder()
{
}

QList<BatteryInfo> BatteryInfoFinder::getBatteryList() {
    return m_batteryInfoList;
}

void BatteryInfoFinder::dowork()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()) {
        QDBusMessage response = manager.call("readBTInfo");

        QString result = response.arguments().at(0).toString();
        qDebug() << result;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());

        if (jsonDoc.isObject()) {
            QJsonObject jsonObject = jsonDoc.object();
            if (jsonObject.contains("list") && jsonObject["list"].isArray()) {
                QJsonArray jsonArray = jsonObject["list"].toArray();

                // 遍历数组
                for (const QJsonValue &value: jsonArray) {
                    if (value.isObject()) {
                        QJsonObject monitorObject = value.toObject();

                        BatteryInfo batteryinfo;
                        batteryinfo.vendor = monitorObject.value("manufacturer").toString();
                        batteryinfo.model = monitorObject.value("model").toString();
                        batteryinfo.serailnum = monitorObject.value("serailnum").toString();
                        batteryinfo.state = monitorObject.value("state").toString();
                        batteryinfo.energy = monitorObject.value("energy").toString();
                        batteryinfo.energy_full = monitorObject.value("energy_full").toString();
                        batteryinfo.time_to_empty = monitorObject.value("time_to_empty").toString();
                        batteryinfo.percentage = monitorObject.value("percentage").toString();
                        batteryinfo.used_times = monitorObject.value("used_times").toString();

                        // 将解析出来的 keyboardInfo 添加到 QList
                        m_batteryInfoList.append(batteryinfo);
                    }
                }
            }
        }
    }
    emit succeed();
}

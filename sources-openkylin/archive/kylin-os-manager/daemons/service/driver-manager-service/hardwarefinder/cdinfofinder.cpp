#include "cdinfofinder.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CDInfoFinder::CDInfoFinder()
{
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &CDInfoFinder::dowork);

    connect(this, &CDInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &CDInfoFinder::finished, this, &CDInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

CDInfoFinder::~CDInfoFinder()
{
}

QList<CDInfo> CDInfoFinder::getCDList() {
    return m_CDInfoList;
}

void CDInfoFinder::dowork()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()) {
        QDBusMessage response = manager.call("readCDInfo");

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

                        CDInfo batteryinfo;
                        batteryinfo.vendor = monitorObject.value("manufacturer").toString();
                        batteryinfo.model = monitorObject.value("model").toString();
                        batteryinfo.businfo = monitorObject.value("businfo").toString();
                        batteryinfo.name = monitorObject.value("name").toString();
                        batteryinfo.version = monitorObject.value("version").toString();
                        batteryinfo.driver = monitorObject.value("driver").toString();
                        batteryinfo.devicenum = monitorObject.value("devicenum").toString();
                        batteryinfo.speed = monitorObject.value("speed").toString();
                        batteryinfo.serialnum = monitorObject.value("serialnum").toString();

                        // 将解析出来的 keyboardInfo 添加到 QList
                        m_CDInfoList.append(batteryinfo);
                    }
                }
            }
        }
    }
    emit succeed();
}

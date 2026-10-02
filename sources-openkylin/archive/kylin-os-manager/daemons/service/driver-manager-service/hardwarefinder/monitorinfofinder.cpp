#include "monitorinfofinder.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>

MonitorInfoFinder::MonitorInfoFinder() {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &MonitorInfoFinder::dowork);

    connect(this, &MonitorInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &MonitorInfoFinder::finished, this, &MonitorInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

MonitorInfoFinder::~MonitorInfoFinder() {

}

QList<MonitorInfo> MonitorInfoFinder::getMonitorList() {
    return m_monitorInfoList;
}

void MonitorInfoFinder::dowork() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()) {
        QDBusMessage response = manager.call("readMNInfo");

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

                        MonitorInfo monitorInfo;
                        monitorInfo.vendor = monitorObject.value("manufacturer").toString();
                        monitorInfo.name = monitorObject.value("name").toString();
                        monitorInfo.size = monitorObject.value("size").toString();
                        monitorInfo.ratio = monitorObject.value("ratio").toString();
                        monitorInfo.resolution = monitorObject.value("resolution").toString();
                        monitorInfo.max_resolution = monitorObject.value("max_resolution").toString();
                        monitorInfo.is_main = monitorObject.value("is_main").toString();
                        monitorInfo.gamma = monitorObject.value("gamma").toString();
                        monitorInfo.interface = monitorObject.value("interface").toString();
                        monitorInfo.model = monitorObject.value("model").toString();
                        monitorInfo.visible_area = monitorObject.value("visible_area").toString();
                        monitorInfo.year = monitorObject.value("year").toString();
                        monitorInfo.week = monitorObject.value("week").toString();

                        // 将解析出来的 MonitorInfo 添加到 QList
                        m_monitorInfoList.append(monitorInfo);
                    }
                }
            }
        }
    }
    emit succeed();
}

#include "mouseinfofinder.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MouseInfoFinder::MouseInfoFinder() {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &MouseInfoFinder::dowork);

    connect(this, &MouseInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &MouseInfoFinder::finished, this, &MouseInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

MouseInfoFinder::~MouseInfoFinder() {

}

QList<MouseInfo> MouseInfoFinder::getMouseList() {
    return m_mouseInfoList;
}

void MouseInfoFinder::dowork() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()) {
        QDBusMessage response = manager.call("readMSInfo");

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

                        MouseInfo mouseInfo;
                        mouseInfo.vendor = monitorObject.value("manufacturer").toString();
                        mouseInfo.name = monitorObject.value("name").toString();
                        mouseInfo.driver = monitorObject.value("driver").toString();
                        mouseInfo.devtype = monitorObject.value("dev_type").toString();
                        mouseInfo.interface = monitorObject.value("interface").toString();
                        mouseInfo.model = monitorObject.value("dev_model").toString();

                        // 将解析出来的 keyboardInfo 添加到 QList
                        m_mouseInfoList.append(mouseInfo);
                    }
                }
            }
        }
    }
    emit succeed();
}
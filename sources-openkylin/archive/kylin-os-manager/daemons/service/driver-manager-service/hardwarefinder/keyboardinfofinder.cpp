#include "keyboardinfofinder.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

KeyboardInfoFinder::KeyboardInfoFinder() {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &KeyboardInfoFinder::dowork);

    connect(this, &KeyboardInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &KeyboardInfoFinder::finished, this, &KeyboardInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

KeyboardInfoFinder::~KeyboardInfoFinder() {

}

QList<KeyboardInfo> KeyboardInfoFinder::getKeyBoardList() {
    return m_keyboardInfoList;
}

void KeyboardInfoFinder::dowork() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()) {
        QDBusMessage response = manager.call("readKBInfo");

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

                        KeyboardInfo keyboardInfo;
                        keyboardInfo.vendor = monitorObject.value("manufacturer").toString();
                        keyboardInfo.name = monitorObject.value("name").toString();
                        keyboardInfo.driver = monitorObject.value("driver").toString();
                        keyboardInfo.devtype = monitorObject.value("dev_type").toString();
                        keyboardInfo.interface = monitorObject.value("interface").toString();
                        keyboardInfo.model = monitorObject.value("dev_model").toString();

                        // 将解析出来的 keyboardInfo 添加到 QList
                        m_keyboardInfoList.append(keyboardInfo);
                    }
                }
            }
        }
    }
    emit succeed();
}
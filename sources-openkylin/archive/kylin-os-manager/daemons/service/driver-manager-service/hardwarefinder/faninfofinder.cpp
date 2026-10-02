#include "faninfofinder.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

FanInfoFinder::FanInfoFinder()
{
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &FanInfoFinder::dowork);

    connect(this, &FanInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &FanInfoFinder::finished, this, &FanInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

FanInfoFinder::~FanInfoFinder()
{
}

QList<FanInfo> FanInfoFinder::getFanList() {
    return m_fanInfoList;
}

void FanInfoFinder::dowork()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()) {
        QDBusMessage response = manager.call("readFAInfo");

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

                        FanInfo faninfo;
                        faninfo.name = monitorObject.value("name").toString();
                        faninfo.speed = monitorObject.value("speed").toString();

                        // 将解析出来的 keyboardInfo 添加到 QList
                        m_fanInfoList.append(faninfo);
                    }
                }
            }
        }
    }
    emit succeed();
}

#include "soundcardfinder.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>

SoundCardFinder::SoundCardFinder()
{
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &SoundCardFinder::dowork);

    connect(this, &SoundCardFinder::finished, m_thread, &QThread::quit);
    connect(this, &SoundCardFinder::finished, this, &SoundCardFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

SoundCardFinder::~SoundCardFinder()
{

}

QList<SoundCardInfo> SoundCardFinder::getSoundCardList()
{
    return m_soundCardInfoList;
}

void SoundCardFinder::dowork() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()) {
        QDBusMessage response = manager.call("readVCInfo");

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

                        SoundCardInfo soundcardInfo;
                        soundcardInfo.type = DeviceType::SoundCard;
                        soundcardInfo.vendor = monitorObject.value("Vendor").toString();
                        soundcardInfo.name = monitorObject.value("Product").toString();
                        soundcardInfo.driver_now = monitorObject.value("Driver").toString();
                        soundcardInfo.bus = monitorObject.value("Bus Info").toString();
                        soundcardInfo.width = monitorObject.value("Width").toString();
                        soundcardInfo.clock = monitorObject.value("Clock").toString();
                        soundcardInfo.model = monitorObject.value("Model").toString();

                        // 将解析出来的 MonitorInfo 添加到 QList
                        m_soundCardInfoList.append(soundcardInfo);
                    }
                }
            }
        }
    }
    emit succeed();
}

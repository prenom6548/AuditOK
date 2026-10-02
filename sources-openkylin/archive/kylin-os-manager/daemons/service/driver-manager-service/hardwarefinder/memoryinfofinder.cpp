#include "memoryinfofinder.h"
#include <QProcess>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

MemoryInfoFinder::MemoryInfoFinder(QObject *parent) : QObject(parent) {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &MemoryInfoFinder::dowork);

    connect(this, &MemoryInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &MemoryInfoFinder::finished, this, &MemoryInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

MemoryInfoFinder::~MemoryInfoFinder() {

}

QList<MemoryInfo> MemoryInfoFinder::getMemoryInfo() {
    return m_memoryInfo;
}

void MemoryInfoFinder::dowork() {
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon",
                           "com.kylin.assistant.systemdaemon", bus);
    if (manager.isValid()) {
        QDBusMessage response = manager.call("readMemInfo");

        QString result = response.arguments().at(0).toString();
        qDebug() << "MemoryInfo" << result;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());

        if (jsonDoc.isObject()) {
            QJsonObject jsonObject = jsonDoc.object();
            if (jsonObject.contains("list") && jsonObject["list"].isArray()) {
                QJsonArray jsonArray = jsonObject["list"].toArray();

                // 遍历数组
                for (const QJsonValue &value: jsonArray) {
                    if (value.isObject()) {
                        QJsonObject memoryObject = value.toObject();

                        MemoryInfo memoryinfo;
                        memoryinfo.verdor = memoryObject.value("manufacturer").toString();
                        memoryinfo.totalwidth = memoryObject.value("data_width").toString();
                        memoryinfo.size = memoryObject.value("total_capacity").toString();
                        memoryinfo.serial = memoryObject.value("serail_num").toString();
                        memoryinfo.locator = memoryObject.value("slot").toString();
                        memoryinfo.datawidth = memoryObject.value("data_width").toString();
                        memoryinfo.type = memoryObject.value("type").toString();
                        memoryinfo.speed = memoryObject.value("speed").toString();
                        memoryinfo.configuredspeed = memoryObject.value("config_speed").toString();
                        memoryinfo.banklocator = memoryObject.value("pipe").toString();
                        memoryinfo.partnumber = memoryObject.value("partnum").toString();
                        if(memoryinfo.size.contains("MB")){
                            QString size = memoryinfo.size.split(" MB")[0];
                            int Mbsize = size.toInt();
                            int Gbsize = Mbsize / 1024;
                            memoryinfo.size = QString::number(Gbsize) + " GB";
                        }

                        memoryinfo.displayname = memoryinfo.size + " " + memoryinfo.verdor + " " + memoryinfo.speed;
                        m_memoryInfo.append(memoryinfo);
                    }
                }
            }
        }
    }
    emit succeed();
}

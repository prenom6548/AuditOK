#include "camerainfofinder.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

CameraInfoFinder::CameraInfoFinder()
{
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &CameraInfoFinder::dowork);

    connect(this, &CameraInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &CameraInfoFinder::finished, this, &CameraInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

CameraInfoFinder::~CameraInfoFinder()
{
}

QList<CameraInfo> CameraInfoFinder::getCameraList()
{
    return m_camearInfoList;
}

void CameraInfoFinder::dowork()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()) {
        QDBusMessage response = manager.call("readCAInfo");

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

                        CameraInfo camerainfo;
                        camerainfo.vendor = monitorObject.value("manufacturer").toString();
                        camerainfo.model = monitorObject.value("model").toString();
                        camerainfo.name = monitorObject.value("name").toString();
                        camerainfo.interface = monitorObject.value("interface").toString();
                        camerainfo.driver = monitorObject.value("driver").toString();
                        camerainfo.businfo = monitorObject.value("businfo").toString();
                        camerainfo.speed = monitorObject.value("speed").toString();
                        camerainfo.version = monitorObject.value("version").toString();

                        // 将解析出来的 keyboardInfo 添加到 QList
                        m_camearInfoList.append(camerainfo);
                    }
                }
            }
        }
    }
    emit succeed();
}

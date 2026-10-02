#include "bluetoothfinder.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bluetoothFinder::bluetoothFinder()
{
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &bluetoothFinder::dowork);

    connect(this, &bluetoothFinder::finished, m_thread, &QThread::quit);
    connect(this, &bluetoothFinder::finished, this, &bluetoothFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

bluetoothFinder::~bluetoothFinder()
{

}

QList<BluetoothInfo> bluetoothFinder::getBluetoothDeviceList()
{
    return m_bluetoothInfoList;
}

void bluetoothFinder::dowork()
{
    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);
    if(manager.isValid()){
        QDBusMessage response = manager.call("readBLInfo");

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
                        QJsonObject jsonObj = value.toObject();

                        BluetoothInfo bluetoothInfo;
                        bluetoothInfo.type = DeviceType::Bluetooth;
                        bluetoothInfo.model = jsonObj["dev_model"].toString();
                        bluetoothInfo.vendor = jsonObj["manufacturer"].toString();
                        bluetoothInfo.busNumber = jsonObj["bus"].toString();
                        bluetoothInfo.dev_version = jsonObj["dev_version"].toString();
                        bluetoothInfo.driver_now = jsonObj["driver"].toString();
                        bluetoothInfo.features = jsonObj["features"].toString();
                        bluetoothInfo.link_mode = jsonObj["link_mode"].toString();
                        bluetoothInfo.link_policy = jsonObj["link_policy"].toString();
                        bluetoothInfo.dev_name = jsonObj["name"].toString();
                        bluetoothInfo.packettype = jsonObj["packettype"].toString();
                        bluetoothInfo.scomtu = jsonObj["scomtu"].toString();
                        bluetoothInfo.speed = jsonObj["speed"].toString();
                        bluetoothInfo.address = jsonObj["address"].toString();

                        getDriverModeInfo(bluetoothInfo.driver_now, bluetoothInfo.modeinfo);
                        // 将解析出来的 keyboardInfo 添加到 QList
                        m_bluetoothInfoList.append(bluetoothInfo);
                    }
                }
            }
        }
    }
    emit succeed();
}


#include "networkcardfinder.h"
#include <QProcess>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDBusConnection>
#include <QDBusInterface>

NetworkCardFinder::NetworkCardFinder() {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &NetworkCardFinder::dowork);

    connect(this, &NetworkCardFinder::finished, m_thread, &QThread::quit);
    connect(this, &NetworkCardFinder::finished, this, &NetworkCardFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

NetworkCardFinder::~NetworkCardFinder() {

}

QList<NetworkCardInfo> NetworkCardFinder::getNetworkDeviceList() {
    return m_NetworkCardInfoList;
}

void NetworkCardFinder::dowork() {
    QProcess proc;
    QString cmd = "lshw -c network -json";
    proc.setEnvironment(QStringList() << "LANGUAGE=en_us");
    proc.start(cmd);
    proc.waitForFinished();
    QString jsonData = QString(proc.readAllStandardOutput());

    QJsonDocument doc = QJsonDocument::fromJson(jsonData.toUtf8());
    QJsonArray networkDevices = doc.array();
    for (const QJsonValue& deviceValue : networkDevices){
        QJsonObject device = deviceValue.toObject();
        if (device.contains("children")){
            NetworkCardInfo networkCardinfo;
            networkCardinfo.vendor = deviceValue.toObject().value("vendor").toString();
            networkCardinfo.model = deviceValue.toObject().value("product").toString();
            networkCardinfo.type = DeviceType::NetworkCard;
            QJsonArray children = device["children"].toArray();
            for (const auto& child : children)
            {
                QJsonObject childObj = child.toObject();
                if (childObj.value("logicalname").toString().trimmed() == ""){
                    continue;
                }
                // Process childObj as needed
                networkCardinfo.type = DeviceType::NetworkCard;
                networkCardinfo.name = childObj.value("logicalname").toString();
                networkCardinfo.MAC_addr = childObj.value("serial").toString();
                networkCardinfo.driver_now = childObj.value("configuration").toObject().value("driver").toString();
                getDriverModeInfo(networkCardinfo.driver_now, networkCardinfo.modeinfo);
                networkCardinfo.bus = childObj.value("businfo").toString();
                networkCardinfo.isWireless = checkWireless(networkCardinfo.name);
                networkCardinfo.ip_addr = childObj.value("configuration").toObject().value("ip").toString();
                networkCardinfo.net_musk = childObj.value("configuration").toObject().value("subnet_mask").toString();
                networkCardinfo.MTU = childObj.value("configuration").toObject().value("mtu").toString();

                // 将解析出来的 keyboardInfo 添加到 QList
                m_NetworkCardInfoList.append(networkCardinfo);
            }
        }else{
            break;
        }
    }
    if (m_NetworkCardInfoList.isEmpty()){
        QDBusConnection bus = QDBusConnection::systemBus();
        QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon",
                               "com.kylin.assistant.systemdaemon", bus);
        if (manager.isValid()) {
            QDBusMessage response = manager.call("readNWInfo");

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
                            QJsonObject networkCardObject = value.toObject();
                            if (networkCardObject.value("Logical Name").toString().trimmed() == ""){
                                continue;
                            }

                            NetworkCardInfo networkCardinfo;
                            networkCardinfo.type = DeviceType::NetworkCard;
                            networkCardinfo.vendor = networkCardObject.value("Vendor").toString();
                            networkCardinfo.model = networkCardObject.value("Product").toString();
                            networkCardinfo.name = networkCardObject.value("Logical Name").toString();
                            networkCardinfo.MAC_addr = networkCardObject.value("Serial").toString();
                            networkCardinfo.ip_addr = networkCardObject.value("IP").toString();
                            networkCardinfo.driver_now = networkCardObject.value("Driver").toString();
                            getDriverModeInfo(networkCardinfo.driver_now, networkCardinfo.modeinfo);
                            networkCardinfo.bus = networkCardObject.value("Bus Info").toString();
                            networkCardinfo.isWireless = checkWireless(networkCardinfo.name);
                            networkCardinfo.net_musk = networkCardObject.value("subnet_mask").toString();
                            networkCardinfo.send_bytes = networkCardObject.value("send_bytes").toString();
                            networkCardinfo.recv_bytes = networkCardObject.value("recv_bytes").toString();
                            networkCardinfo.MTU = networkCardObject.value("mtu").toString();

                            // 将解析出来的 keyboardInfo 添加到 QList
                            m_NetworkCardInfoList.append(networkCardinfo);
                        }
                    }
                }
            }
        }
    }
    emit succeed();
}

int NetworkCardFinder::checkWireless(QString name) {
    QProcess process;
    QStringList arguments;
    arguments << "device" << "status";

    // 启动nmcli命令并等待其完成
    process.start("nmcli", arguments);
    process.waitForFinished();

    // 读取命令的输出
    QStringList output = QString(process.readAllStandardOutput()).split("\n");
    for (int i = 0; i < output.length(); ++i) {
        if (output[i].contains(name)){
            if (output[i].contains("wifi")){
                return  1;
            }else{
                return  0;
            }
        }
    }
    return 0;
}

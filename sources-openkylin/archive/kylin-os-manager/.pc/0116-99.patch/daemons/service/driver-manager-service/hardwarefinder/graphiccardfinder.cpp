#include "graphiccardfinder.h"

#include <QFile>
#include <QProcess>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

graphicCardFinder::graphicCardFinder() {

}

graphicCardFinder::~graphicCardFinder() {

}

QList<GraphicCardInfo> graphicCardFinder::getList() {
    return m_cardInfoList;
}

void graphicCardFinder::dowork() {
    const QString basePath = "/sys/class/drm/card";
    const QString devicePath = "/device/";

    int cardNum = 0;

    while (1) {
        GraphicCardInfo cardInfo;
        // 获取modalias
        QString path = basePath + QString::number(cardNum) + devicePath;
        QFile *modaFile = new QFile(path + "modalias");
        if (!modaFile->exists()) {
            break;
        }
        if (!modaFile->open(QIODevice::ReadOnly)) {
            qDebug() << modaFile->errorString();
            break;
        }
        QString modalias = QString(modaFile->readLine()).split("\n")[0];
        modaFile->close();
        cardInfo.modalias = modalias;
        // 获取厂商型号
        QString commond = "udevadm hwdb --test=" + modalias;
        QProcess proc;
        proc.start(commond);
        proc.waitForFinished();
        QStringList outPut = QString(proc.readAllStandardOutput()).split("\n");
        proc.close();
        QString model = "";
        QString vendor = "";
        for (int i = 0; i < outPut.length(); i++) {
            if (QString(outPut[i]).contains("ID_MODEL_FROM_DATABASE", Qt::CaseInsensitive)) {
                model = QString(outPut[i]).split("=")[1];
                cardInfo.model = model;
                continue;
            } else {
                //如果通过上面的udevadm 命令无法获取model,那就比对对应路径modalisas，使用lspci的输出获取model
                QString commond2 = "ls /sys/bus/pci/devices/";
                QProcess proc2;
                proc2.start(commond2);
                proc2.waitForFinished();
                QStringList outPut2 = QString(proc2.readAllStandardOutput()).split("\n");
                proc2.close();
                for (int j = 0; j < outPut2.length() - 1; j++) {
                    // 获取modalias
                    QProcess proc3;
                    QString commond3 = "cat /sys/bus/pci/devices/" + QString(outPut2[j]) + "/modalias";
                    commond3.replace(":", "\:");
                    proc3.start(commond3);
                    proc3.waitForFinished();
                    QString outPut3 = proc3.readAllStandardOutput();
                    outPut3.replace("\n", "");
                    proc3.close();
                    // bus info
                    QString bus = "pci@" + QString(outPut2[j]);
                    if (outPut3 == cardInfo.modalias) {
                        QProcess proc4;
                        QString commond4 = "lshw -c display";
                        proc4.start(commond4);
                        proc4.waitForFinished();
                        QStringList outPut4 = QString(proc4.readAllStandardOutput()).split("*-display");
                        proc4.close();
                        for (int k = 0; k < outPut4.length(); k++) {
                            if (QString(outPut4[k]).contains(bus) && QString(outPut4[k]).contains("product:")) {
                                cardInfo.model = QString(QString(outPut4[k]).split("product: ")[1]).split("\n")[0];
                            }
                        }
                    }
                }
            }

            if (QString(outPut[i]).contains("ID_VENDOR_FROM_DATABASE", Qt::CaseInsensitive)) {
                vendor = QString(outPut[i]).split("=")[1];
                cardInfo.vendor = vendor;
                continue;
            } else {
                //如果通过上面的udevadm 命令无法获取model,那就比对对应路径modalisas，使用lspci的输出获取model
                QString commond2 = "ls /sys/bus/pci/devices/";
                QProcess proc2;
                proc2.start(commond2);
                proc2.waitForFinished();
                QStringList outPut2 = QString(proc2.readAllStandardOutput()).split("\n");
                proc2.close();
                for (int j = 0; j < outPut2.length() - 1; j++) {
                    // 获取modalias
                    QProcess proc3;
                    QString commond3 = "cat /sys/bus/pci/devices/" + QString(outPut2[j]) + "/modalias";
                    commond3.replace(":", "\:");
                    proc3.start(commond3);
                    proc3.waitForFinished();
                    QString outPut3 = proc3.readAllStandardOutput();
                    outPut3.replace("\n", "");
                    proc3.close();
                    // bus info
                    QString bus = "pci@" + QString(outPut2[j]);
                    if (outPut3 == cardInfo.modalias) {
                        QProcess proc4;
                        QString commond4 = "lshw -c display";
                        proc4.start(commond4);
                        proc4.waitForFinished();
                        QStringList outPut4 = QString(proc4.readAllStandardOutput()).split("*-display");
                        proc4.close();
                        for (int k = 0; k < outPut4.length(); k++) {
                            if (QString(outPut4[k]).contains(bus) && QString(outPut4[k]).contains("vendor:")) {
                                cardInfo.vendor = QString(QString(outPut4[k]).split("vendor: ")[1]).split("\n")[0];
                            }
                            if (QString(outPut4[k]).contains("width:")) {
                                cardInfo.width = QString(QString(outPut4[k]).split("width: ")[1]).split("\n")[0];
                            }
                            if (QString(outPut4[k]).contains("driver=")) {
                                cardInfo.driver_now = QString(QString(outPut4[k]).split("driver=")[1]).split(" ")[0];
                            }
                        }
                    }
                }
            }
        }
        if (cardInfo.modalias != "" && cardInfo.model != "") {
            cardInfo.type = DeviceType::GraphicCard;
            m_cardInfoList.append(cardInfo);
        }
        cardNum++;
    }

    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon",
                           "com.kylin.assistant.systemdaemon", bus);
    if (manager.isValid()) {
        QDBusMessage response = manager.call("readGSInfo");
        QString result = response.arguments().at(0).toString();
        qDebug() << result;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(result.toUtf8());
        if (!jsonDoc.isNull()) {
            if (jsonDoc.isObject()) {
                QJsonObject jsonObject = jsonDoc.object();
                if (jsonObject.contains("list") && jsonObject["list"].isArray()) {
                    if (m_cardInfoList.length() != 0) {
                        for (int i = 0; i < m_cardInfoList.length(); ++i) {
                            QJsonArray jsonArray = jsonObject["list"].toArray();
                            for (const QJsonValue &value: jsonArray) {
                                if (value.isObject()) {
                                    QJsonValue jsonObj = value;
                                    if ((m_cardInfoList[i].model == jsonObj["model"].toString() ||
                                         jsonObj["description"].toString().contains(m_cardInfoList[i].model) ||
                                         jsonObj["name"].toString().contains(m_cardInfoList[i].model) ||
                                         jsonObj["subsystem"].toString().contains(m_cardInfoList[i].model)) &&
                                        m_cardInfoList[i].model != "") {
                                        m_cardInfoList[i].subsystem = jsonObj["subsystem"].toString();
                                        m_cardInfoList[i].memorysize = jsonObj["capcity"].toString();
                                        m_cardInfoList[i].businfo = jsonObj["dbusinfo"].toString();
                                        m_cardInfoList[i].clock = jsonObj["clock"].toString();
                                        m_cardInfoList[i].physid = jsonObj["physics_id"].toString();
                                        m_cardInfoList[i].version = jsonObj["version"].toString();
                                        if (m_cardInfoList[i].driver_now == "") {
                                            m_cardInfoList[i].driver_now = jsonObj["driver"].toString();
                                        }
                                        break;
                                    } else {
                                        if (jsonObj == jsonArray.last()){
                                            m_cardInfoList.removeAt(i);
                                        }else{
                                            continue;
                                        }
                                        GraphicCardInfo cardInfo;
                                        cardInfo.type = DeviceType::GraphicCard;
                                        cardInfo.model = jsonObj["model"].toString();
                                        cardInfo.name = jsonObj["name"].toString();
                                        cardInfo.vendor = jsonObj["manufacturer"].toString();
                                        cardInfo.subsystem = jsonObj["subsystem"].toString();
                                        cardInfo.memorysize = jsonObj["capcity"].toString();
                                        cardInfo.businfo = jsonObj["dbusinfo"].toString();
                                        cardInfo.clock = jsonObj["clock"].toString();
                                        cardInfo.physid = jsonObj["physics_id"].toString();
                                        cardInfo.version = jsonObj["version"].toString();
                                        cardInfo.egl_version = jsonObj["egl_version"].toString();
                                        cardInfo.egl_apis = jsonObj["egl_apis"].toString();
                                        cardInfo.gl_version = jsonObj["gl_version"].toString();
                                        cardInfo.glsl_version = jsonObj["glsl_version"].toString();
                                        cardInfo.gddrsize = jsonObj["gddr_capacity"].toString();
                                        m_cardInfoList.insert(i, cardInfo);
                                    }
                                }
                            }
                        }
                    } else {
                        QJsonArray jsonArray = jsonObject["list"].toArray();
                        for (const QJsonValue &value: jsonArray) {
                            if (value.isObject()) {
                                QJsonValue jsonObj = value;
                                GraphicCardInfo cardInfo;
                                cardInfo.type = DeviceType::GraphicCard;
                                cardInfo.model = jsonObj["model"].toString();
                                cardInfo.name = jsonObj["name"].toString();
                                cardInfo.vendor = jsonObj["manufacturer"].toString();
                                cardInfo.businfo = jsonObj["dbusinfo"].toString();
                                cardInfo.subsystem = jsonObj["subsystem"].toString();
                                cardInfo.memorysize = jsonObj["capcity"].toString();
                                cardInfo.businfo = jsonObj["dbusinfo"].toString();
                                cardInfo.clock = jsonObj["clock"].toString();
                                cardInfo.physid = jsonObj["physics_id"].toString();
                                cardInfo.version = jsonObj["version"].toString();
                                cardInfo.gddrsize = jsonObj["gddr_capacity"].toString();
                                m_cardInfoList.append(cardInfo);
                            }
                        }
                    }
                }
            }
        }
    }
}

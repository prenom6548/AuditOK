#include "baseboardinfofinder.h"

#include <QProcess>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

BaseboardInfoFinder::BaseboardInfoFinder(QObject *parent) : QObject(parent) {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &BaseboardInfoFinder::dowork);

    connect(this, &BaseboardInfoFinder::finished, m_thread, &QThread::quit);
    connect(this, &BaseboardInfoFinder::finished, this, &BaseboardInfoFinder::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

BaseboardInfoFinder::~BaseboardInfoFinder() {

}

void BaseboardInfoFinder::dowork() {
    QProcess proc;
    QString cmd = "dmidecode -t baseboard";
    proc.start(cmd);
    proc.waitForFinished();
    QStringList res = QString(proc.readAllStandardOutput()).split("\n\n");

    for (int i = 0; i < res.length(); ++i) {
        if(QString(res[i]).contains("Base Board")){
            QStringList outPut = QString(res[i]).split("\n");
            for (int j = 0; j < outPut.length(); ++j) {
                if(QString(outPut[j]).contains("Manufacturer:")){
                    m_baseboardInfo.vendor = QString(QString(outPut[j]).split("Manufacturer:")[1]).trimmed();
                    continue;
                }

                if(QString(outPut[j]).contains("Product Name:")){
                    m_baseboardInfo.model = QString(QString(outPut[j]).split("Product Name:")[1]).trimmed();
                    continue;
                }

                if(QString(outPut[j]).contains("Version:")){
                    m_baseboardInfo.version = QString(QString(outPut[j]).split("Version:")[1]).trimmed();
                    continue;
                }

                if(QString(outPut[j]).contains("Chassis Handle:")){
                    m_baseboardInfo.chassishandle = QString(QString(outPut[j]).split("Chassis Handle:")[1]).trimmed();
                    continue;
                }

            }
        }
    }
    proc.close();

    QProcess proc2;
    QString cmd2 = "dmidecode -s bios-vendor";
    proc2.start(cmd2);
    proc2.waitForFinished();
    QString res2 = QString(proc2.readAllStandardOutput());
    m_baseboardInfo.biosvendor = res2.remove("\n");
    proc2.close();

    QProcess proc3;
    QString cmd3 = "dmidecode -s bios-version";
    proc3.start(cmd3);
    proc3.waitForFinished();
    QString res3 = QString(proc3.readAllStandardOutput());
    m_baseboardInfo.biosversion = res3.remove("\n");
    proc3.close();

    QDBusConnection bus = QDBusConnection::systemBus();
    QDBusInterface manager("com.kylin.assistant.systemdaemon", "/com/kylin/assistant/systemdaemon", "com.kylin.assistant.systemdaemon", bus);

    if(manager.isValid()) {
        QDBusMessage response = manager.call("readMBInfo");
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
                        QJsonObject baseboardObject = value.toObject();
                        m_baseboardInfo.serial_num = baseboardObject.value("serial_num").toString();
                        m_baseboardInfo.publish_date = baseboardObject.value("publish_date").toString();
                        m_baseboardInfo.chipset = baseboardObject.value("chipset").toString();
                    }
                }
            }
        }
    }
    emit succeed();
}

BaseboardInfo BaseboardInfoFinder::getBaseboardInfo() {
    return m_baseboardInfo;
}

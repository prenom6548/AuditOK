#include "pcinfogetter.h"
#include <QProcess>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

PcIfoGetter::PcIfoGetter() {
    m_thread = new QThread();
    this->moveToThread(m_thread);

    connect(m_thread, &QThread::started, this, &PcIfoGetter::dowork);

    connect(this, &PcIfoGetter::finished, m_thread, &QThread::quit);
    connect(this, &PcIfoGetter::finished, this, &PcIfoGetter::deleteLater);
    connect(m_thread, &QThread::finished, m_thread, &QThread::deleteLater);
}

PcIfoGetter::~PcIfoGetter() {

}

PcInfo PcIfoGetter::getPcInfo() {
    return m_pcInfo;
}

void PcIfoGetter::dowork() {
    m_pcInfo.hostname = getHostName();
    m_pcInfo.sysversion = getSystemVersion();
    m_pcInfo.kernelversion = getKernelVersion();
    m_pcInfo.arch = getKernelArch();
    m_pcInfo.sysbits = getSystemBits();
    m_pcInfo.manufacturer = getManufacturer();
    m_pcInfo.version = getVersion();
    m_pcInfo.model = getModel();
    m_pcInfo.serial = getSerial();

    emit succeed();
}

QString PcIfoGetter::getHostName() {
    QProcess proc;
    QString cmd = "hostname";
    proc.start(cmd);
    proc.waitForFinished();
    QString outPut = QString(proc.readAllStandardOutput());
    outPut.remove("\n");
    return outPut;
}

QString PcIfoGetter::getSystemVersion() {
    QString result;
    QProcess proc;
    QString cmd = "cat /etc/os-release";
    proc.start(cmd);
    proc.waitForFinished();
    QStringList outPut = QString(proc.readAllStandardOutput()).split("\n");
    for (int i = 0; i < outPut.length(); ++i) {
        if(outPut.at(i).contains("VERSION=")){
            result = QString(outPut[i]).split("VERSION=")[1];
            result += " ";
            result.remove("\"");
        }
        if(outPut.at(i).contains("KYLIN_RELEASE_ID=")){
            result += QString(outPut[i]).split("KYLIN_RELEASE_ID=")[1];
            result.remove("\"");
        }
    }
    result.remove("\n");
    return result;
}

QString PcIfoGetter::getKernelVersion() {
    QProcess proc;
    QString cmd = "uname -r";
    proc.start(cmd);
    proc.waitForFinished();
    QString outPut = QString(proc.readAllStandardOutput());
    outPut.remove("\n");
    return outPut;
}

QString PcIfoGetter::getKernelArch() {
    QProcess proc;
    QString cmd = "uname -m";
    proc.start(cmd);
    proc.waitForFinished();
    QString outPut = QString(proc.readAllStandardOutput());
    outPut.remove("\n");
    return outPut;
}

QString PcIfoGetter::getSystemBits() {
    QProcess proc;
    QString cmd = "getconf LONG_BIT";
    proc.start(cmd);
    proc.waitForFinished();
    QString outPut = QString(proc.readAllStandardOutput());
    outPut = outPut + "bit";
    outPut.remove("\n");
    return outPut;
}

QString PcIfoGetter::getManufacturer() {
    QProcess proc;
    QString cmd = "cat /sys/class/dmi/id/sys_vendor";
    proc.start(cmd);
    proc.waitForFinished();
    QString outPut = QString(proc.readAllStandardOutput());
    outPut.remove("\n");
    return outPut;
}

QString PcIfoGetter::getVersion() {
    QProcess proc;
    QString cmd = "cat /sys/class/dmi/id/product_version";
    proc.start(cmd);
    proc.waitForFinished();
    QString outPut = QString(proc.readAllStandardOutput());
    outPut.remove("\n");
    if(outPut.trimmed() == ""){
        return "N/A";
    }
    return outPut;
}

QString PcIfoGetter::getModel() {
    QProcess proc;
    QString cmd = "cat /sys/class/dmi/id/product_name";
    proc.start(cmd);
    proc.waitForFinished();
    QString outPut = QString(proc.readAllStandardOutput());
    outPut.remove("\n");
    if(outPut.trimmed() == ""){
        return "N/A";
    }
    return outPut;
}

QString PcIfoGetter::getSerial() {
    QProcess proc;
    QString cmd = "dmidecode -s system-serial-number";
    proc.start(cmd);
    proc.waitForFinished();
    QString outPut = QString(proc.readAllStandardOutput()).trimmed();
    if (outPut == "" || outPut == "\n"){
        outPut = "N/A";
    }
    if (outPut.contains("\n")){
        outPut.remove("\n");
    }
    return outPut;
}

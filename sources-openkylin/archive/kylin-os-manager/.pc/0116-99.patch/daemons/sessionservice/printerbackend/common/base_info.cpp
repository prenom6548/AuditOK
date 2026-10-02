#include "base_info.h"
#include "common.h"
#include <QProcess>
#include <QDebug>

BaseInfo::BaseInfo()
{
    init();
}

BaseInfo::~BaseInfo()
{

}

void BaseInfo::setSystemVersion()
{
    m_systemVersion = "V10SP1";

    // QProcess proc;
    // QString cmd = "lscpu";
    // proc.setEnvironment(QStringList() << "LANGUAGE=en_us");
    // proc.start(cmd);
    // proc.waitForFinished();
    // QString model;
    // QStringList outPut = QString(proc.readAllStandardOutput()).split("\n");
    // for (int i = 0; i < outPut.length(); ++i) {
    //     if (QString(outPut[ i ]).contains("Model name:")) {
    //         model = QString(QString(outPut[ i ]).split("Model name:")[ 1 ]).trimmed();
    //         break ;
    //     }
    // }
    // if (model.isEmpty()) {
    //     return ;
    // }

    // if (!model.contains("kirin", Qt::CaseInsensitive) && model.contains("huawei", Qt::CaseInsensitive) && !model.contains("pangu", Qt::CaseInsensitive)) {
    //     return ;
    // }

    // if (model.contains("pangux", Qt::CaseInsensitive) || model.contains("9000C", Qt::CaseInsensitive)) {
    //     m_systemVersion = "9000c";
    //     return;
    // }

    // if (model.contains("m900", Qt::CaseInsensitive)) {
    //     m_systemVersion = "m900";
    //     return;
    // }

    // if (model.contains("9006c", Qt::CaseInsensitive)) {
    //     m_systemVersion = "9006c";
    //     return;
    // }

    // if (model.contains("990", Qt::CaseInsensitive)) {
    //     m_systemVersion = "990";
    //     return;
    // }
}

void BaseInfo::init()
{
    m_appDisplayName = tr("Printer");
    m_debianArchitecture = getRetFromCommand(QStringList{"dpkg", "--print-architecture"});
    m_serverAddress = "api.kylinos.cn";
    setSystemVersion();
    qDebug() << "systemVersion is: " << m_systemVersion;
}

QString BaseInfo::getAppDisplayName()
{
    return m_appDisplayName;
}

QString BaseInfo::getDebianArchitecture()
{
    return m_debianArchitecture;
}

QString BaseInfo::getServerAddress()
{
    return m_serverAddress;
}

QString BaseInfo::getSystemVersion()
{
    return m_systemVersion;
}

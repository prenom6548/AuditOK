/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
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

void BaseInfo::initDbus()
{
    if (isV11version()) {
        KUM_DEST = "com.kylin.UpgradeStrategies";
        KUM_PATH = "/com/kylin/UpgradeStrategies";
        KUM_IFACE = "com.kylin.UpgradeStrategies.interface";
        return ;
    }

    QString version = getPackageVersion("kylin-printer-driver-daemon");
    if (version.isEmpty()) {
        KUM_DEST = "com.kylin.systemupgrade";
        KUM_PATH = "/com/kylin/systemupgrade";
        KUM_IFACE = "com.kylin.systemupgrade.interface";
    } else {
        KUM_DEST = "com.kylin.printer.driver.daemon";
        KUM_PATH = "/";
        KUM_IFACE = "com.kylin.printer.driver.daemon.interface";
    }
}

void BaseInfo::init()
{
    initDbus();
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

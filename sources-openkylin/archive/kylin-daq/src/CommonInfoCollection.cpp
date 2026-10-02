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
#include "CommonInfoCollection.h"

CommonInfoCollection::CommonInfoCollection(QObject *parent)
    : InfoCollectionBase(parent)
    , mainBoardSerialNumberCmd("dmidecode -t baseboard 2>/dev/null|grep -i 'Serial Number:'|cut -f2 -d:")
    , systemVersionCmd("dmidecode -t system 2>/dev/null|grep 'Version'|cut -f2 -d:")
    , systemSerialNumberCmd("dmidecode -t system 2>/dev/null|grep 'Serial Number'|cut -f2 -d:")
    , systemUuidCmd("dmidecode -t system 2>/dev/null|grep 'UUID'|cut -f2 -d:")
    , kernelVersionCmd("uname -r 2>/dev/null")
    , processorModelCmd("lscpu  2>/dev/null| grep 'Model name'|awk -F':' '{print $2}'")
    , graphicsCardModelCmd("lshw -c display  2>/dev/null|grep 'product'|awk -F':' '{print $2}'")
    , hardDiskSerialNumberCmd("lsblk -o SERIAL -d 2>/dev/null|sed -r '/^\s*$/d'|tail -n +2|sort|uniq")
    , systemManufacturerCmd("dmidecode -t system 2>/dev/null|grep 'Manufacturer:'|cut -f2 -d:")
    , systemProductNameCmd("dmidecode -t system 2>/dev/null|grep 'Product Name'|cut -f2 -d:")
    , machineIdCmd("cat /etc/machine-id 2>/dev/null")
    , networkInfoCollection(new NetworkInfoCollection(parent))
    , registerNumberInfoCollection(new RegisterNumberInfoCollection(parent))
    , dpkgVersionCmd("dpkg --status dpkg 2>/dev/null|grep 'Version'|awk '{print $2}'")
    , kysecVersionCmd("dpkg --status kysec 2>/dev/null|grep 'Version'|awk '{print $2}'")
    , kysecAuthVersionCmd("dpkg --status kysec-auth 2>/dev/null|grep 'Version'|awk '{print $2}'")
    , kysecDaemonVersionCmd("dpkg --status kysec-daemon 2>/dev/null|grep 'Version'|awk '{print $2}'")
    , kysecUtilsVersionCmd("dpkg --status kysec-utils 2>/dev/null|grep 'Version'|awk '{print $2}'")
    , libkysecVersionCmd("dpkg --status libkysec 2>/dev/null|grep 'Version'|awk '{print $2}'")
    , osVersionCmd("cat /etc/os-release 2>/dev/null|grep VERSION_US|cut -f2 -d=")
    , osReleaseIdCmd("cat /etc/os-release 2>/dev/null|grep RELEASE_ID |cut -f2 -d= ")
    , osBuildInfoCmd("cat /etc/kylin-build|tr '\n' ' '|sed 's/ $//'")
    , systemArchitectureCmd("uname -m 2>/dev/null")
    , systemLanguageCmd("echo ${LANG:-en_US.UTF-8} 2>/dev/null")
{
}

void CommonInfoCollection::getInfo(CommonInfo &commonInfo)
{
    QList<QString> processModelList;
    QList<QString> hardDiskSerialNumberList;
    //  主板序列号
    getOnePopenResult(mainBoardSerialNumberCmd,commonInfo.mainBoardSerialNumber);
    //  系统版本
    getOnePopenResult(systemVersionCmd, commonInfo.systemVersion);
    // 补丁版本
    commonInfo.osPatchVersion = getOsPatchVersion();
    // 系统服务序列号
    commonInfo.osSerialNumber = getOsSerialNumber();
    //  系统序列号
    getOnePopenResult(systemSerialNumberCmd, commonInfo.systemSerialNumber);
    //  系统uuid
    getOnePopenResult(systemUuidCmd, commonInfo.systemUuid);
    //  内核版本
    getOnePopenResult(kernelVersionCmd, commonInfo.kernelVersion);
    //  处理器类型
    getPopenResult(processorModelCmd, processModelList);
    commonInfo.processorModel = processModelList.join(s_separator).simplified();

    //  显卡类型
//    getPopenResult(graphicsCardModelCmd, graphicsCardModelList);
//    commonInfo.graphicsCardModel = graphicsCardModelList.join(s_separator);
    commonInfo.graphicsCardModel = getGraphicsCardModel();

    //OSTREE版本
    // getHardDiskSerialNumber(commonInfo.hardDiskSerialNumber);
    commonInfo.osTreeVersion = getOstreeVersion();
    //  硬盘类型
    getPopenResult(hardDiskSerialNumberCmd, hardDiskSerialNumberList);
    commonInfo.hardDiskSerialNumber = hardDiskSerialNumberList.join(s_separator).simplified();
    //  网卡mac
    commonInfo.networkMacAddress = networkInfoCollection->getNetworkMac();
    //  网卡类型
    commonInfo.networkType = networkInfoCollection->getNetworkType();
    //  厂商名
    getOnePopenResult(systemManufacturerCmd,commonInfo.systemManufacturer);
    //  产品名
    getOnePopenResult(systemProductNameCmd,commonInfo.systemProductName);
    //  获取操作系统机器id
    getOnePopenResult(machineIdCmd, commonInfo.machineId);
    //  注册码
    commonInfo.registerNumber = registerNumberInfoCollection->getSerialNumber();
    //
    getOnePopenResult(dpkgVersionCmd, commonInfo.dpkgVersion);
    getOnePopenResult(kysecVersionCmd, commonInfo.kysecVersion);
    getOnePopenResult(kysecAuthVersionCmd, commonInfo.kysecAuthVersion);
    getOnePopenResult(kysecDaemonVersionCmd, commonInfo.kysecDaemonVersion);
    getOnePopenResult(kysecUtilsVersionCmd, commonInfo.kysecUtilsVersion);
    getOnePopenResult(libkysecVersionCmd, commonInfo.libkysecVersion);
    getOnePopenResult(osVersionCmd,commonInfo.osVersion);
    getOnePopenResult(osReleaseIdCmd,commonInfo.osReleaseId);
    getOnePopenResult(osBuildInfoCmd,commonInfo.osBuildInfo);
    getOnePopenResult(systemArchitectureCmd,commonInfo.systemArchitecture);
    getOnePopenResult(systemLanguageCmd,commonInfo.systemLanguage);
}

void CommonInfoCollection::getHardDiskSerialNumber(QString &serialNumber)
{
    QList<QString> mountNameResultList;
    QList<QString> serialNumberList;
    getPopenResult(hardDiskSerialNumberCmd, mountNameResultList);
    for (auto iter = mountNameResultList.begin();iter != mountNameResultList.end();iter++) {
        QString hardDiskName(iter->simplified());
        QString filePath = QString("/sys/block/%1/device/serial").arg(hardDiskName);
        QFile serialFile(filePath);
        if (!serialFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "serialfile open failed!" << filePath;
        } else {
            serialNumberList<<serialFile.readAll().simplified();
            serialFile.close();
        }
    }
    serialNumber = serialNumberList.join(s_separator);
}

QString CommonInfoCollection::getOstreeVersion()
{
    QString ret = "";
    QSettings settings("/etc/ostree-release",QSettings::IniFormat);
    QString str = settings.value("info/version").toString().remove("\n");
    str.replace("/","-");
    ret = "ostree-" + str;
    return ret;
}

QString CommonInfoCollection::getOsPatchVersion()
{
    QString ret = "";
    QSettings settings("/etc/kylin-version/kylin-system-version.conf",QSettings::IniFormat);
    ret = settings.value("SYSTEM/update_version").toString().remove("\n");
    return ret;
}
QString CommonInfoCollection::getOsSerialNumber()
{
    QString ret = "";
    QSettings settings("/etc/.kyinfo",QSettings::IniFormat);
    ret = settings.value("servicekey/key").toString().remove("\n");
    return ret;
}

QString CommonInfoCollection::getGraphicsCardModel()
{

    QString ret = "";
    QProcess process;
    process.start("lspci");
    process.waitForFinished(10 * 1000);
    QByteArray arr = process.readAllStandardOutput();

    QString tmp = QString(arr);

    QStringList result;

    if (tmp.contains("\n")){
        result = tmp.split("\n");
    }else {
        result << tmp;
    }

    for (int var = 0; var < result.size(); ++var) {
        QString tmp = result.at(var);
        if (tmp.contains("VGA")){
            if (tmp.contains(":")){
                if (tmp.split(":").size() > 2){
                    ret = tmp.split(":").at(2);
                    break;
                }
            }
        }
    }
    return ret;

}



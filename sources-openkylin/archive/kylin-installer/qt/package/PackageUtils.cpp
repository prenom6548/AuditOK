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
#include "PackageUtils.h"
#include <QString>
#include <QProcess>
#include <QDir>
#include <QStorageInfo>
#include <QMessageBox>
#include <QRegExp>
#include <QFileInfo>
#include <QApt/DebFile>
#include <QTextStream>
#include <QDebug>


QString PackageUtils::getOsDistribution()
{
    QString osDistribution;
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return osDistribution;
    }
    QTextStream in(&file);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        if (line.startsWith("PROJECT_CODENAME="))
        {
            osDistribution = line.split("=").at(1);
            osDistribution = osDistribution.replace("\"", "");
            osDistribution = osDistribution.trimmed().toLower();
            break;
        }
    }
    file.close();
    return osDistribution;

}

QStringList PackageUtils::getDebDistribution(QString debPath)
{
    QStringList packageDistributionList;
    QProcess process;
    QString compatFile = PackageUtils::s_packageVersionFileName;

    // 解压 deb 包中的 _compat 文件
    process.start("ar", QStringList() << "-x" << debPath << compatFile);
    process.waitForFinished();

    // 检查 _compat 文件是否存在
    if (QFile::exists(compatFile)) {
        // 打开文件并按行读取内容
        QFile file(compatFile);
        if (file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            QString packageDistributionStr = in.readAll();
            qDebug()<<"packageDistributionStr="<<packageDistributionStr;
            packageDistributionList = packageDistributionStr.simplified().toLower().split(PackageUtils::s_packageVersionSplitKey);
            file.close();
        }
        // 删除 _compat 文件
        QFile::remove(compatFile);
    }
    packageDistributionList.removeAll("");
    qDebug()<<"packageDistributionStr="<<packageDistributionList;
    return packageDistributionList;
}

QStringList PackageUtils::getKareImageDistribution()
{
    QStringList packageDistributionList;
    QProcess process;
    process.start("kare -E");
    process.waitForFinished();
    QByteArray arr = process.readAll();
    QString str = QString(arr).trimmed().toLower();
    if (str.contains("\n"))
    {
        packageDistributionList = str.split("\n");
    }
    else if (!str.isEmpty() )
    {
        packageDistributionList.append(str);
    }

    return packageDistributionList;

}

PackageEnum::Type PackageUtils::getPackageType(QString debPath)
{
    QString osDistribution = getOsDistribution();
    QStringList packageDistributionList = getDebDistribution(debPath);
    if(debPath.contains(".snap",Qt::CaseSensitive))
    {
        return PackageEnum::Type::SNAP;
    }
    else if (debPath.contains(".apk",Qt::CaseSensitive))
    {
        return PackageEnum::Type::APK;
    }
    else if (!packageDistributionList.isEmpty() 
        && !packageDistributionList.contains(osDistribution) 
        && !isInstalledFromDebFilePath(debPath))
//    else if (!packageDistributionList.isEmpty() )
    {
        return PackageEnum::Type::KARE;
    }
    else
    {
        return PackageEnum::Type::DEB;
    }

}


void PackageUtils::getPackageEnumInfo(QString debPath, PackageEnum::Type &type, QString &distribution, bool &isInstalledBaseEnv)
{
    type = getPackageType(debPath);
    isInstalledBaseEnv = false;
    if (type == PackageEnum::Type::KARE)
    {
        QStringList packageDistributionList = PackageUtils::getDebDistribution(debPath);
        QStringList kareInstalledImageList = PackageUtils::getKareImageDistribution();
        foreach (QString dist, packageDistributionList) {
            if (kareInstalledImageList.contains(dist)) {
                distribution = dist;
                isInstalledBaseEnv = true;
                break;
            }
        }
        if (isInstalledBaseEnv != true && packageDistributionList.size() > 0){
            distribution = packageDistributionList.first();
            isInstalledBaseEnv = false;
        }
    }
    else
    {
        distribution = getOsDistribution();
        isInstalledBaseEnv = true;
    }
}

QString PackageUtils::getDebInfoFromDeb(QString debPath, QString type)
{

    debPath = QDir::toNativeSeparators(debPath);


    QApt::DebFile debfile(debPath);
    if (debfile.isValid()){
        if (type == "Package"){
            return debfile.packageName();}
        else if (type == "Version"){
            return debfile.version();
        }
        else{
            return debfile.architecture();
        }
    }
    else{
        return "The software package is damaged";
    }

}

bool PackageUtils::isInstalledFromDebFilePath(QString debpathstring)
{
    QString current_version = PackageUtils::getDebInfoFromDeb(debpathstring, "Version");
    QString installed_version;
    QString current_version_debPath;
    QString current_name = PackageUtils::getDebInfoFromDeb(debpathstring, "Package");

    QString Architecture = PackageUtils::getDebInfoFromDeb(debpathstring, "Architecture");

    QProcess terminal;
    QStringList cmdList;
    QProcess terminal1;
    QStringList cmdList1;
    QProcess terminal2;
    QStringList cmdList2;
    QProcess terminal3;
    QStringList cmdList3;
    QString installed_version_status;
    QString kare_installed_status;
    QString kare_installed_version;
    current_name = current_name.toLower();
    current_version_debPath = current_name;
    current_version_debPath.push_front(' ');
    current_version_debPath.push_front('"');
    current_version_debPath.push_back(' ');
    current_version_debPath.push_back(' ');
    current_version_debPath.push_back('"');


    cmdList1 << "-c" << QString("dpkg -l | grep "+ current_version_debPath + "| awk '{print $1}' | sed -n '1p'").toStdString().c_str();
    terminal1.start("/bin/bash",cmdList1);
    terminal1.waitForFinished();
    installed_version_status = terminal1.readAll();//获取此包已安装的状态
    if (installed_version_status.isEmpty())
    {
        cmdList1.clear();
        current_version_debPath = "";
        current_version_debPath = current_name;
        current_version_debPath.push_front(' ');
        current_version_debPath.push_front('"');
        current_version_debPath.push_back(':');
        current_version_debPath.push_back(Architecture);
        current_version_debPath.push_back('"');
        cmdList1 << "-c" << QString("dpkg -l | grep "+ current_version_debPath + "| awk '{print $1}' | sed -n '1p'").toStdString().c_str();
        terminal1.start("/bin/bash",cmdList1);
        terminal1.waitForFinished();
        installed_version_status = terminal1.readAll();//获取此包已安装的状态
    }

    installed_version_status = installed_version_status.left(installed_version_status.length() - 1);

    if (installed_version_status.contains("ii"))
    {
        qDebug()<<"PackageName is installed";
    }
    else
    {
        qDebug()<<"PackageName isn't installed";
    }
    return installed_version_status.contains("ii");
}

bool PackageUtils::isInstalledFromDpkg(QString packageName)
{
    QProcess process;
    QStringList cmdList;
    QString installed_version_status;
    cmdList << "-c" << QString("dpkg -l | grep "+ packageName + "| awk '{print $1}' | sed -n '1p'").toStdString().c_str();
    process.start("/bin/bash",cmdList);
    process.waitForFinished();
    installed_version_status = process.readAll();//获取此包已安装的状态
    if (installed_version_status.contains("ii"))
    {
        qDebug()<<"package:"<<packageName<<" is installed";
    }
    else
    {
        qDebug()<<"package:"<<packageName<<" isn't installed";
    }
    return installed_version_status.contains("ii");
}

QString PackageUtils::getMd5Sum(QString filePath)
{
    QProcess terminal;
    QStringList cmdList;
    cmdList << "-c" << QString("md5sum " + filePath).toStdString().c_str();
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();

    QString md5sum = terminal.readAll();
    QStringList list = md5sum.split(" ");
    if (list.count() >0)
        return list[0].simplified();
    return "";
}

QString PackageUtils::getMd5SumPart(QString filePath)
{
    QFile file(filePath);
    if (!file.open(QFile::ReadOnly))
        return "";

    QCryptographicHash hash(QCryptographicHash::Md5);
    qint64 totalSize = file.size();
    qint64 partSize  = 50 * 1024 * 1024;

    if (totalSize <= partSize * 2)
        return getMd5Sum(filePath);

    QByteArray headData = file.read(partSize);
    hash.addData(headData);

    file.seek(file.size() - partSize);
    QByteArray tailData = file.read(partSize);
    hash.addData(tailData);

    return hash.result().toHex();
}

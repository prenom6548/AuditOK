#include "ApkPackage.h"
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
#include "ApkPackage.h"

ApkPackage::ApkPackage(QString debPath):Package(debPath)
{
    initPackageInfo();
    initPackageInstallStatus();
    printInfo();
}

void ApkPackage::initPackageInfo()
{
    getApkInfo(this->m_debPath, this->m_name, this->m_version, this->m_architecture, this->m_zhName);
    if (this->m_name.isEmpty() || this->m_zhName.isEmpty()) {
        this->m_status = PackageEnum::Status::DAMAGED;
        return;
    }
    this->m_md5Sum = PackageUtils::getMd5Sum(this->m_debPath);
}

void ApkPackage::getApkInfo(QString debPath, QString &name, QString &version, QString &arch,QString &zhName)
{
    QString application_label = "";
    QString application_zh_label = "";
    QString fileSize = "";
    QString iconPath = "";

    QProcess process;
    QStringList cmdList;



    if (QFileInfo("/usr/bin/aapt").exists()) {
        process.start("aapt", QStringList()<< "d" << "badging" << debPath);
        process.waitForStarted(5*1000);
        process.waitForFinished(10*1000);//process.waitForFinished(-1);
        const QString output = process.readAllStandardOutput();//process.readAll();

        for (const auto &line : output.split('\n')) {
            if (line.startsWith("package:")) {
                const QStringList &info = QString(line).split(' ');
                if (info.size() == 5) {
                    for (int i=0;i<info.size();i++) {
                        if (QString(info.at(i)).startsWith("name=")) {
                            const QStringList &nameinfo = QString(info.at(i)).split('=');
                            name = QString(nameinfo.at(1)).replace("\'", "").trimmed();//com.rightware.BasemarkOSII
                        }
                        if (QString(info.at(i)).startsWith("versionName=")) {
                            const QStringList &versioninfo = QString(info.at(i)).split('=');
                            version = QString(versioninfo.at(1)).replace("\'", "").trimmed();//2.0
                        }
                    }
                }
            }
            if (QString(line).startsWith("application-label:")) {//可能不存在该行
                const QStringList &info = QString(line).split(':');
                if (info.size() == 2) {
                    application_label = QString(info.at(1)).replace("\'", "").trimmed();
                }
            }
            if (line.startsWith("application-label-zh-CN:")) {//可能不存在该行
                const QStringList &info = QString(line).split(':');
                if (info.size() == 2) {
                    application_zh_label = QString(info.at(1)).replace("\'", "").trimmed();
                }
            }
            if (line.startsWith("application-label-zh:")) {//可能不存在该行
                const QStringList &info = QString(line).split(':');
                if (info.size() == 2) {
                    if (application_zh_label.isEmpty()) {
                        application_zh_label = QString(info.at(1)).replace("\'", "").trimmed();
                    }
                }
            }
            if (line.startsWith("application: label=")) {
                int firstIndex = QString(line).indexOf(QChar('\''));
                int secondIndex = QString(line).indexOf(QChar('\''), strlen("application: label='"));
                QString labelContent = QString(line).left(secondIndex);
                QString labelStr = labelContent.right(labelContent.length() - 1 - firstIndex);
                if (application_label.isEmpty()) {
                    application_label = labelStr;
                }
                if (application_zh_label.isEmpty()) {
                    application_zh_label = labelStr;
                }

                const QStringList &info = QString(line).split(' ');
                for (int i=0;i<info.size();i++) {
                    if (QString(info.at(i)).startsWith("icon=")) {
                        //icon path
                        if (iconPath.isEmpty()) {
                            const QStringList &iconinfo = QString(info.at(i)).split('=');
                            iconPath = QString(iconinfo.at(1)).replace("\'", "").trimmed();
                        }
                    }
                }
            }
            if (QString(line).startsWith("launchable-activity:")) {//launchable-activity: name='com.rightware.BasemarkOSII.BasemarkOSII'  label='Basemark OS II' icon=''
                int firstIndex = QString(line).indexOf(QChar('\''));
                int secondIndex = QString(line).indexOf(QChar('\''), strlen("application: label='"));
                QString labelContent = QString(line).left(secondIndex);
                QString labelStr = labelContent.right(labelContent.length() - 1 - firstIndex);
                if (application_label.isEmpty()) {
                    application_label = labelStr;
                }
                if (application_zh_label.isEmpty()) {
                    application_zh_label = labelStr;
                }

                const QStringList &info = QString(line).split(' ');
                for (int i=0;i<info.size();i++) {
                    if (QString(info.at(i)).startsWith("icon=")) {
                        //icon path
                        if (iconPath.isEmpty()) {
                            const QStringList &iconinfo = QString(info.at(i)).split('=');
                            iconPath = QString(iconinfo.at(1)).replace("\'", "").trimmed();
                        }
                    }
                }
            }
            if (!application_label.isEmpty() && !(QString(name).isEmpty())) {
                if (application_zh_label.isEmpty()) {
                    application_zh_label = application_label;
                }
                name = application_label;
                zhName = application_zh_label;
            }
        }
    }

}

void ApkPackage::initPackageInstallStatus()
{
    if (this->m_name.isEmpty () || this->m_version.isEmpty ())
        return;
    // fixme: 获取是否安装状态
    this->m_status = PackageEnum::Status::UNINSTALLED;
}


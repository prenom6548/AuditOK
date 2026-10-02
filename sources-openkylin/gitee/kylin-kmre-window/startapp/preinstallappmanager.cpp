/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "preinstallappmanager.h"

#include <QDir>
#include <QSettings>
#include <QThread>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <syslog.h>

#include "utils.h"

QString PreInstallAppManager::preInstalledFilePath;
QStringList PreInstallAppManager::preInstallApkList;

QVector<PreInstallAppManager::PreInstalledApkStatus> PreInstallAppManager::getPreInstalledApps(QString filePath)
{
    QVector<PreInstalledApkStatus> installedApps;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonParseError json_error;
        QJsonDocument apps = QJsonDocument::fromJson(file.readAll(), &json_error);
        if (!apps.isNull() && (json_error.error == QJsonParseError::NoError)) {
            if (apps.isObject()) {
                QJsonObject obj = apps.object();
                if (obj.contains("data")) {
                    QJsonArray array = obj.value("data").toArray();
                    for (auto app : array) {
                        QJsonObject a = app.toObject();
                        if (a.contains("appname")) {
                            installedApps.push_back({
                                a.value("appname").toString(), 
                                a.value("version").toString(), 
                                a.contains("status") ? a.value("status").toInt() : 0});
                        }
                    }
                }
            }
        }
        file.close();
    }
    return installedApps;
}

// must be called after container started!
bool PreInstallAppManager::isApkPreInstalled(QVector<PreInstalledApkStatus> apps, QString pkgName)
{
    for (auto app : apps) {
        if (app.appName.endsWith(".apk")) {
            if (app.appName == pkgName + ".apk") {
                return true;
            }
        }
        else if (app.appName == pkgName) {
            return true;
        }
    }
    return false;
}

PreInstallAppManager::PRE_INSTALL_STATUS PreInstallAppManager::getApkPreInstallStatus(QString pkgName)
{
    if (!preInstallApkList.contains(pkgName)) {
        return ePreInstallNull;
    }

    QVector<PreInstalledApkStatus> apps = getPreInstalledApps(preInstalledFilePath);
    for (auto app : apps) {
        if (app.appName.endsWith(".apk")) {
            if (app.appName == pkgName + ".apk") {
                return ePreInstallFailed;
            }
        }
        else if (app.appName == pkgName) {
            return (app.status == 0) ? ePreInstallSucceed : ePreInstallFailed;
        }
    }

    return ePreInstallWaiting;
}

// must be called before container started!
void PreInstallAppManager::preInstallApks()
{
    Utils::prepareEnvironment();
    preInstallApkList.clear();

    QDir apkPath("/opt/kmre/preinstall/");
    QDir preInstallPath("/var/lib/kmre/" + Utils::getContainerName() + "/data/local/preinstall/");
    QDir desktopPath(QDir::homePath() + "/.local/share/applications/");
    QDir iconPath(QDir::homePath() + "/.local/share/icons/");

    if ((!apkPath.exists()) || 
        (!QFileInfo(apkPath.absolutePath()).permission(QFile::Permission(0x4444)))) 
    {
        syslog(LOG_ERR, "Directory '%s' isn't exist or have no permission! Pre install failed!", 
            apkPath.absolutePath().toStdString().c_str());
        return;
    }

    if ((!preInstallPath.exists()) || 
        (!QFileInfo(preInstallPath.absolutePath()).permission(QFile::Permission(0x6666)))) 
    {
        syslog(LOG_ERR, "Directory '%s' isn't exist or have no permission! Pre install failed!", 
            preInstallPath.absolutePath().toStdString().c_str());
        return;
    }

    preInstalledFilePath = preInstallPath.absolutePath() + "/preInstallResult.json";
    if (QFileInfo(preInstalledFilePath).exists()) {
        syslog(LOG_INFO, "Apps are only pre installed when first boot docker!");
        return;
    }

    if (!desktopPath.exists()) {
        if (!desktopPath.mkpath(desktopPath.absolutePath())) {
            syslog(LOG_INFO, "Directory '%s' isn't exist and created failed!", 
                desktopPath.absolutePath().toStdString().c_str());
            return;
        }
    }

    if (!iconPath.exists()) {
        if (!iconPath.mkpath(iconPath.absolutePath())) {
            syslog(LOG_INFO, "Directory '%s' isn't exist and created failed!", 
                iconPath.absolutePath().toStdString().c_str());
            return;
        }
    }

    QFileInfoList apkList = apkPath.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (auto fileInfo : apkList) 
    {
        QString pkgName = fileInfo.fileName();
        QDir apkFilesPath = QDir(fileInfo.absoluteFilePath());
        QString apkConfFileName = "preinstall.ini";
        if (apkFilesPath.exists(apkConfFileName)) 
        {
            QString apkConfFilePath = apkFilesPath.absoluteFilePath(apkConfFileName);
            QSettings apkConf(apkConfFilePath, QSettings::IniFormat);
            apkConf.setIniCodec("UTF-8");

            PreInstallApkConf conf;
            conf.pkgName = pkgName;
            apkConf.beginGroup("desktop");
            conf.needinstallDesktop = apkConf.value("desktop_required", "false").toBool();
            apkConf.endGroup();
            apkConf.beginGroup("install");
            conf.needInstallAtBoot = apkConf.value("follow_boot_install", "false").toBool();
            apkConf.endGroup();
            apkConf.beginGroup("version");
            conf.version = apkConf.value("package_version", "0.0.0").toString();
            apkConf.endGroup();

            QFileInfoList apkFileList = apkFilesPath.entryInfoList(QDir::Files);
            for (auto file : apkFileList) 
            {
                QString fileName = file.fileName();
                QString targetFile = "";

                if (fileName.endsWith(".apk")) 
                {
                    targetFile = preInstallPath.absolutePath() + "/" + fileName;
                }
                else if (fileName.endsWith(".desktop") && conf.needinstallDesktop) 
                {
                    targetFile = desktopPath.absolutePath() + "/" + fileName;
                }
                else if ((fileName.endsWith(".png") || fileName.endsWith(".svg")) && conf.needinstallDesktop) 
                {
                    targetFile = iconPath.absolutePath() + "/" + fileName;
                }

                if (!targetFile.isEmpty()) 
                {
                    if (QFileInfo(targetFile).exists()) 
                    {
                        preInstallApkList.append(pkgName);
                    }
                    else if (QFile::copy(file.absoluteFilePath(), targetFile)) 
                    {
                        preInstallApkList.append(pkgName);
                    }
                    else 
                    {
                        syslog(LOG_ERR, "Copy file '%s' failed!", file.absoluteFilePath().toStdString().c_str());
                    }
                }
            }
        }
    }
}

// must be called after container started!
void PreInstallAppManager::waitForPreInstallFinished()
{
    bool finished = true;
    int waitCounter = 0;
    QString failedApks = "";

    do {
        finished = true;
        for (auto apk : preInstallApkList) {
            PRE_INSTALL_STATUS installStatus = getApkPreInstallStatus(apk);
            if (installStatus == ePreInstallFailed) {
                failedApks.append(apk + ",");
            }
            else if (installStatus == ePreInstallWaiting) {
                finished = false;
                break;
            }
        }

        if (!finished) {
            if (waitCounter > 10) {// 10s time out!
                break;
            }
            QThread::msleep(1000);
            waitCounter++;
        }
    }while(!finished);

    if (!finished) {
        syslog(LOG_ERR, "Pre install apks time out!");
        //QMessageBox* Msginfo = new QMessageBox(QMessageBox::Warning, "Warning", "Pre install apks time out!", QMessageBox::Ok);
        //Msginfo->button(QMessageBox::Ok)->setText(QObject::tr("Ok"));
        //Msginfo->exec();
    }
    else if (!failedApks.isEmpty()) {
        syslog(LOG_ERR, "Pre install apks:'%s' failed!", failedApks.toStdString().c_str());
        //QMessageBox* Msginfo = new QMessageBox(QMessageBox::Warning, "Warning", "Pre install apks failed!", QMessageBox::Ok);
        //Msginfo->button(QMessageBox::Ok)->setText(QObject::tr("Ok"));
        //Msginfo->exec();
    }
}

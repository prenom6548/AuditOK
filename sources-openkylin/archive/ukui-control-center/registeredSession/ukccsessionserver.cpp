/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "ukccsessionserver.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QDBusInterface>
#include <QDBusReply>

#ifdef signals
#undef signals
#endif

#include <glib.h>
#include <unistd.h>

const QStringList openkylinFilterPathList = {"/Shortcut/Add", "/Shortcut/Customize Shortcut"};

ukccSessionServer::ukccSessionServer():UKUI::ServiceObject(nullptr)
{
   monitoFileChanged();
}

QMap<QString, QVariant> ukccSessionServer::getJsonInfo(const QString &configFile)
{
    QVariantMap moduleMap;

    QFile file(configFile);
    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        QByteArray readBy=file.readAll();
        QJsonParseError error;
        QJsonDocument readDoc=QJsonDocument::fromJson(readBy,&error);
        QJsonArray obj=readDoc.object().value("ukcc").toArray();

        for (int i = 0 ; i < obj.size(); i++) {
            QJsonObject faObj= obj[i].toObject();
            if (!faObj.contains("name")) {
                continue;
            }
            if (!faObj.contains("visible")) {
                moduleMap.insert(faObj["name"].toString(), true);
            } else {
                moduleMap.insert(faObj["name"].toString(), faObj["visible"].toVariant());
            }

            QJsonArray childNodeAry =  faObj["childnode"].toArray();
            for (int j = 0; j < childNodeAry.size(); j++) {
                QString modeName = childNodeAry.at(j).toObject().value("name").toString();
                QString modeSet = modeName + "Settings";
                QVariant modeVisiable = childNodeAry.at(j).toObject().value("visible").toVariant();
                QString modeEnable = modeName +"Enable";
                QString modeEnableList = childNodeAry.at(j).toObject().value(modeEnable).toString();

                moduleMap.insert(modeName, modeVisiable);
                moduleMap.insert(modeSet, childNodeAry.at(j).toObject().value(modeSet).toString());
                moduleMap.insert(modeEnable, modeEnableList);

                if (modeName.contains("autoboot")) {
                    moduleMap.insert(QString("autostart"), modeVisiable);
                }

                if (modeSet.contains("autobootSettings")) {
                    moduleMap.insert(QString("autostartSettings"), childNodeAry.at(j).toObject().value(modeSet).toString());
                }

                if (modeEnable.contains("autobootEnable")) {
                    if (modeEnableList.contains("autobootAllPage")) {
                        modeEnableList.replace("autobootAllPage", QString("autostartAllPage"), Qt::CaseInsensitive);
                    }
                    moduleMap.insert(QString("autostartEnable"), modeEnableList);
                }


                if (modeName.contains("biometrics")) {
                    moduleMap.insert(QString("login options"), modeVisiable);
                }

                if (modeSet.contains("biometricsSettings")) {
                    moduleMap.insert(QString("login optionsSettings"), childNodeAry.at(j).toObject().value(modeSet).toString());
                }

                if (modeEnable.contains("biometricsEnable")) {
                    if (modeEnableList.contains("biometricsAllPage")) {
                        modeEnableList.replace("login optionsAllPage", QString("login optionsAllpage"), Qt::CaseInsensitive);
                    }
                    moduleMap.insert(QString("login optionsEnable"), modeEnableList);
                }


                if (modeName.contains("defaultapp")) {
                    moduleMap.insert(QString("default"), modeVisiable);
                }

                if (modeSet.contains("defaultappSettings")) {
                    moduleMap.insert(QString("defaultSettings"), childNodeAry.at(j).toObject().value(modeSet).toString());
                }

                if (modeEnable.contains("defaultappEnable")) {
                    if (modeEnableList.contains("defaultappAllPage")) {
                        modeEnableList.replace("defaultappAllPage", QString("defaultAllPage"), Qt::CaseInsensitive);
                    }
                    moduleMap.insert(QString("defaultEnable"), modeEnableList);
                }

                if (modeName.contains("ukccpanel")) {
                    moduleMap.insert(QString("panel"), modeVisiable);
                }

                if (modeSet.contains("ukccpanelSettings")) {
                    moduleMap.insert(QString("panelSettings"), childNodeAry.at(j).toObject().value(modeSet).toString());
                }

                if (modeEnable.contains("ukccpanelEnable")) {
                    if (modeEnableList.contains("ukccpanelAllPage")) {
                        modeEnableList.replace("ukccpanelAllPage", QString("panelAllPage"), Qt::CaseInsensitive);
                    }
                    moduleMap.insert(QString("panelEnable"), modeEnableList);
                }

            }
        }
    }
    return moduleMap;
}

QMap<QString, QVariant> ukccSessionServer::getUSDJsonInfo(const QString &configFile)
{
    QVariantMap moduleMap;

    QFile file(configFile);
    if (file.exists()) {
        file.open(QIODevice::ReadOnly);
        QByteArray readBy=file.readAll();
        QJsonParseError error;
        QJsonDocument readDoc=QJsonDocument::fromJson(readBy,&error);
        QJsonObject rootObj = readDoc.object();
        // 获取 ukui-control-center 对象
        QJsonObject controlCenter = rootObj["ukui-control-center"].toObject();
        for (auto it = controlCenter.begin(); it != controlCenter.end(); ++it) {
            QString key = it.key();
            QStringList keyList = key.split("-");
            QString exchangedModuleName = "";
            /*  json字符串格式：
             * {
             *     "插件类型名-Visible（例如account-Visible）":true,
             *     "插件名-Visible（例如userinfo-Visible）":true,
             *     "插件名-Visible-Setting":"隐藏免密登录（例如noPwdLoginFrame:false）",
             *     "插件名-Enable-Setting":"禁用整个插件页面（例如userinfoAllPage:false）插件名AllPage：true，禁用免密登录（例如noPwdLoginFrame:false）",
             * }
             * return的格式：
             * 插件类型名（例如account）: true,
             * 插件名（例如userinfo）: true,
             * 插件名Settings（例如userinfoSettings）:"noPwdLoginFrame:false",
             * 插件名Enable（例如userinfoEnable）:"userinfoAllPage:false,noPwdLoginFrame:false"
            */
            // 获取插件名,部分插件在不同版本上名称不同，域控统一下发一个名称，以下转化为不同的名称，追加插入的形式加入到返回的数据中
            if (keyList.count() >= 2) {
                exchangedModuleName = exchangeModuleName(keyList.at(0));
            }
            if (keyList.count() == 2) {
                if (keyList.at(1) == "Visible") {
                    moduleMap.insert(QString(keyList.at(0)), it.value().toBool());
                    if (exchangedModuleName.compare(keyList.at(0)) != 0) {
                        moduleMap.insert(exchangedModuleName, it.value().toBool());
                    }
                }
            }

            if (keyList.count() == 3) {
                if (keyList.at(2) == "Setting") {
                    if (keyList.at(1) == "Visible") {
                        moduleMap.insert(QString(QString(keyList.at(0))+"Settings"), it.value().toString());
                        if (exchangedModuleName.compare(keyList.at(0)) != 0) {
                            moduleMap.insert(QString(exchangedModuleName+"Settings"), it.value().toString());
                        }
                    } else if (keyList.at(1) == "Enable") {
                        // 先插入原始的数据
                        QString enableSetting = it.value().toString();
                        QString moduleName = keyList.at(0);
                        moduleMap.insert(QString(moduleName +"Enable"), enableSetting);
                        // 再插入转化后的数据
                        if (enableSetting.contains(moduleName+"AllPage")) {
                            enableSetting.replace(moduleName+"AllPage", QString(exchangedModuleName+"AllPage"), Qt::CaseInsensitive);
                        }
                        if (enableSetting.compare(it.value().toString()) != 0) {
                            moduleMap.insert(QString(exchangedModuleName+"Enable"), enableSetting);
                        }
                    }
                }
            }
        }
    }
    return moduleMap;
}

QString ukccSessionServer::exchangeModuleName(QString originName)
{
    QString exchangeModuleName = originName;
    if (exchangeModuleName.contains("autoboot")) {
        exchangeModuleName = QString("autostart");
    }
    if (exchangeModuleName.contains("biometrics")) {
        exchangeModuleName = QString("login options");
    }
    if (exchangeModuleName.contains("defaultapp")) {
        exchangeModuleName = QString("default");
    }
    if (exchangeModuleName.contains("ukccpanel")) {
        exchangeModuleName = QString("panel");
    }

    return exchangeModuleName;
}

QString ukccSessionServer::getScreenMode()
{
    return mScreenMode;
}

void ukccSessionServer::setScreenMode(QString screenMode)
{
    mScreenMode = screenMode;
    Q_EMIT screenChanged(mScreenMode);
}

void ukccSessionServer::exitService()
{
    this->emitUkuiServiceObjectFinished();
}

void ukccSessionServer::ReloadSecurityConfig()
{
    qDebug() << "Reload Security Config";
    Q_EMIT configChanged();
}

QVariantMap ukccSessionServer::getModuleHideStatus()
{
    if (mUsdJsonFile.isEmpty()) {
        // usd 管控文件是否存在，若存在使用 usd 管控文件
        QDBusInterface usdDbus("com.kylin.ukui.SettingsDaemon",
                               "/securityConfig",
                               "com.kylin.ukui.SettingsDaemon.interface",
                                QDBusConnection::systemBus());

        QDBusReply<QString> res = usdDbus.call("GetConfigPath", QString(g_get_user_name()),"ukui-control-center");

        QString usdFileName = res;
        qDebug() << "get usd file path:" << usdFileName << QString(g_get_user_name());

        if (!usdFileName.isEmpty()) {
            mUsdJsonFile = usdFileName;
        }
    }
    QMap<QString, QVariant> usdConfig = getUSDJsonInfo(mUsdJsonFile);
    if (!usdConfig.isEmpty()) {
        return usdConfig;
    }

    QString userFilename = QDir::homePath() + "/.config/ukui-control-center-security-config.json";
    QString systemFilename = "/usr/share/ukui-control-center/data/ukui-control-center-security-config.json";

    QMap<QString, QVariant> userConfig   = getJsonInfo(userFilename);

    if (!userConfig.isEmpty()) {
        return userConfig;
    }

    QMap<QString, QVariant> systemConfig = getJsonInfo(systemFilename);

    if (userConfig.isEmpty() && !systemConfig.isEmpty()) {
        return systemConfig;
    }

    return userConfig;
}

QString ukccSessionServer::GetSecurityConfigPath()
{
    // usd 管控文件是否存在，若存在使用 usd 管控文件
    QDBusInterface usdDbus("com.kylin.ukui.SettingsDaemon",
                           "/securityConfig",
                           "com.kylin.ukui.SettingsDaemon.interface",
                            QDBusConnection::systemBus());

    QDBusReply<QString> res = usdDbus.call("GetConfigPath", QString(g_get_user_name()),"ukui-control-center");
    QString usdFileName = res;
    qDebug() << "get usd file:" << usdFileName << QString(g_get_user_name());
    if (!usdFileName.isEmpty()) {
        mUsdJsonFile = usdFileName;
        return mUsdJsonFile;
    }

    QString systemFilename = "/usr/share/ukui-control-center/data/ukui-control-center-security-config.json";
    QFile file(systemFilename);
    if (file.exists()) {
        return systemFilename;
    }

    QString userFilename = QDir::homePath() + "/.config/ukui-control-center-security-config.json";
    QFile userFile(userFilename);
    if (userFile.exists()) {
        return userFilename;
    }

    QString moduleFileName = "/usr/share/ukui-control-center/data/ukui-control-center-config.json";
    return moduleFileName;
}

void ukccSessionServer::monitoFileChanged()
{
    QFileSystemWatcher *m_FileWatcher = new QFileSystemWatcher(this);
    m_FileWatcher->addPath(GetSecurityConfigPath());
    connect(m_FileWatcher, &QFileSystemWatcher::fileChanged, this, [=](){
        Q_EMIT configChanged();
    });

    QDBusConnection::systemBus().connect(QString(), QString("/securityConfig"), "com.kylin.ukui.SettingsDaemon.interface", "configChanged",
                                         this, SLOT(ReloadSecurityConfig()));
}

QVariantMap ukccSessionServer::getSearchItems()
{
    QDBusInterface* m_interface = new QDBusInterface("org.ukui.ukcc.search",
                                                     "/",
                                                     "org.ukui.ukcc.search.interface",
                                                     QDBusConnection::sessionBus(),
                                                     this);
    QJsonArray searchItems;
    QJsonObject rootobj;
    rootobj.insert("ukcc", searchItems);
    QJsonDocument document;
    document.setObject(rootobj);
    if (!m_interface->isValid()) {
        qDebug() << "ukcc search Interface Failed : " << QDBusConnection::sessionBus().lastError();
        return rootobj.toVariantMap();
    }
    QDBusReply<QVariantMap> reply = m_interface->call("getSearchItems");
    if (reply.isValid()) {
        return reply.value();
    } else {
        return rootobj.toVariantMap();
    }
}


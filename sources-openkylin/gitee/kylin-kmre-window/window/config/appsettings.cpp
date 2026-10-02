/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
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

#include "appsettings.h"
#include "utils.h"
#include "dbusclient.h"
#include "kmreenv.h"
#include "preferences.h"

#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QTextStream>
#include <QDir>
#include <QFileSystemWatcher>
#include <syslog.h>
#include <unistd.h>


AppSettings::AppSettings()
    : QObject(nullptr)
    , mFileWatcher(new QFileSystemWatcher(this))
{
    mUnsupportShowAppList.clear();
    mUnsupportShowAppList << "com.antutu.benchmark.full.lite";
    mUnsupportShowAppList << "com.antutu.benchmark.full";

    mMultiplierConfigFile = KmreEnv::GetConfigPath() + "/AppMultiplier.json";
    mSupportDDSAppListFile = KmreEnv::GetGlobalConfigPath() + "/config/dynamic_size_app_list.xml";
    mSupportKmreDataAppListFile = KmreEnv::GetGlobalConfigPath() + "/config/kmre_data_app_list.xml";
    mAppConfigurationFile = KmreEnv::GetConfigPath() + "/app_configuration.xml";
    mWarningConfigFile = KmreEnv::GetGlobalConfigPath() + "/warning.json";

    mIsHostSupportDDS = kmre::DbusClient::getInstance()->isHostSupportDDS();
    syslog(LOG_INFO, "[%s] Host %s DDS!", __func__, mIsHostSupportDDS ? "support" : "don't support");

    updateSettings();

    if (mFileWatcher->addPath(mAppConfigurationFile)) {
        connect(mFileWatcher, SIGNAL(fileChanged(const QString &)), this, SLOT(slotConfigFileChanged(const QString &)));
    }
    else {
        syslog(LOG_WARNING, "[%s] Add config file(%s) to file watcher failed!", __func__, mAppConfigurationFile.toStdString().c_str());
    }
}

void AppSettings::updateSettings()
{
    syslog(LOG_INFO, "[%s] Update app settings...", __func__);

    checkDDSSettings();
    if (mIsHostSupportDDS) {
        checkMultiperSettings();
    }
    checkKmreDataAppConfig();
    reLoadAppConfigution();
}

bool AppSettings::isAppSupportShow(const QString &pkgName)
{
    bool support = !mUnsupportShowAppList.contains(pkgName);
    syslog(LOG_INFO, "[%s] App '%s' %s Show!", __func__, pkgName.toStdString().c_str(), support ? "support" : "don't support");
    return support;
}

bool AppSettings::isAppSupportDDS(const QString &pkgName)
{
    bool support = false;

    if (mIsHostSupportDDS) {
        support = mSupportDDSAppList.contains(kmre::utils::getRealPkgName(pkgName));

        if (isAppMultiperEnabled(pkgName)) {//平行界面使能时关闭动态分辨率
            support = false;
        }
    }

    syslog(LOG_INFO, "[%s] App '%s' %s DDS!", __func__, pkgName.toStdString().c_str(), support ? "support" : "don't support");
    return support;
}

bool AppSettings::isMultiperEnabled()
{
    QString multiEnabled = kmre::DbusClient::getInstance()->GetPropOfContainer(
            kmre::utils::getUserName(), getuid(), "persist.kmre.multiwindow");
    return (multiEnabled == "true");
}

bool AppSettings::isAppSupportMultiper(const QString &pkgName)
{
    kmre::lgm lk(mSupportMultiAppMapLk);

    bool support = mSupportMultiAppMap.contains(pkgName);

    syslog(LOG_INFO, "[%s] App '%s' %s Multiper!", __func__, pkgName.toStdString().c_str(), support ? "support" : "don't support");
    return support;
}

bool AppSettings::isAppMultiperEnabled(const QString &pkgName)
{
    bool enable = false;

    if (isMultiperEnabled()) {
        kmre::lgm lk(mSupportMultiAppMapLk);

        if (mSupportMultiAppMap.contains(pkgName)) {
            enable = mSupportMultiAppMap[pkgName];
        }
    }

    syslog(LOG_INFO, "[%s] App '%s' Multiper %s!", __func__, pkgName.toStdString().c_str(), enable ? "enabled" : "disabled");
    return enable;
}

bool AppSettings::isAppSupportKmreData(const QString &pkgName)
{
    bool support = mSupportKmreDataAppList.contains(pkgName);
    syslog(LOG_INFO, "[%s] App '%s' %s Kmre Data!", __func__, pkgName.toStdString().c_str(), support ? "support" : "don't support");
    return support;
}

void AppSettings::initAppMultipliersList(const QString &jsonStr)
{
    //save to local config file
    QFile file(mMultiplierConfigFile);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(jsonStr.toUtf8());
        file.close();
    }

    checkMultiperSettings();
}

void AppSettings::updateAppMultipliersList(const QString &pkgName, int multiplier)
{
    QFile file(mMultiplierConfigFile);
    if (file.exists()) {
        if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            QString content = file.readAll();
            if (!content.isEmpty() && !content.isNull()) {
                QJsonParseError err;
                QJsonDocument jsonDocment = QJsonDocument::fromJson(content.toUtf8(), &err);
                if (err.error != QJsonParseError::NoError) {
                    file.close();
                    return;
                }
                if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
                    file.close();
                    return;
                }
                if (jsonDocment.isObject()) {
                    QJsonObject jsonObj = jsonDocment.object();
                    if (jsonObj.contains("AppMultiplier")) {
                        QJsonValue arrayVaule = jsonObj.value("AppMultiplier");
                        if (arrayVaule.isArray()) {
                            QJsonArray array = arrayVaule.toArray();
                            for (int i = 0;i<array.size();i++) {
                                QJsonValue value = array.at(i);
                                QJsonObject child = value.toObject();
                                if (child.contains("packageName") && child.contains("multiplier")) {
                                    QString key = child.value("packageName").toString();
                                    if (pkgName == key) {
                                        //child["multiplier"] = multiplier;
                                        child.remove("multiplier");
                                        child.insert("multiplier", multiplier);
                                        array.replace(i, child);
                                        jsonObj.insert("AppMultiplier", array);
                                        jsonDocment.setObject(jsonObj);
                                        file.seek(0);
                                        file.write(jsonDocment.toJson());
                                        file.flush();

                                        kmre::lgm lk(mSupportMultiAppMapLk);
                                        mSupportMultiAppMap[pkgName] = multiplier;

                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            file.close();
        }
    }
}

void AppSettings::checkMultiperSettings()
{
    QFile file(mMultiplierConfigFile);
    if (file.exists()) {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            int ret = updateMultiperSettingsFromJson(file.readAll());
            if (ret != 0) {
                syslog(LOG_ERR, "[%s] Parse multiple settings json failed! errno: %d", __func__, ret);
            }
            file.close();
        }
    }
}

int AppSettings::updateMultiperSettingsFromJson(const QByteArray &jsonStr)
{
    QJsonParseError err;
    QJsonDocument jsonDocment = QJsonDocument::fromJson(jsonStr, &err);
    if (err.error != QJsonParseError::NoError) {
        return -1;
    }
    if (jsonDocment.isNull() || jsonDocment.isEmpty()) {
        return -2;
    }
    QJsonObject jsonObj = jsonDocment.object();
    if (jsonObj.isEmpty() || jsonObj.size() == 0) {
        return -3;
    }

    if (jsonObj.contains("AppMultiplier")) {
        kmre::lgm lk(mSupportMultiAppMapLk);
        mSupportMultiAppMap.clear();

        QJsonValue arrayVaule = jsonObj.value("AppMultiplier");
        if (arrayVaule.isArray()) {
            QJsonArray array = arrayVaule.toArray();
            for (auto item : array) {
                QJsonObject child = item.toObject();
                if (child.contains("packageName") && child.contains("multiplier")) {
                    QString pkgName = child.value("packageName").toString();
                    int enabled = child.value("multiplier").toInt();
                    mSupportMultiAppMap[pkgName] = enabled;
                }
            }
        }

        return 0;
    }

    return -4;
}

QString AppSettings::getAppMultipliersList()
{
    QString jsonStr = "[]";
    if (QFile::exists(mMultiplierConfigFile)) {
        jsonStr = kmre::utils::readFileContent(mMultiplierConfigFile);
    }
    return jsonStr;
}

void AppSettings::checkDDSSettings()
{
    getAppListFromXml(mSupportDDSAppListFile, mSupportDDSAppList);
}

void AppSettings::checkKmreDataAppConfig()
{
    getAppListFromXml(mSupportKmreDataAppListFile, mSupportKmreDataAppList);
}

void AppSettings::getAppListFromXml(const QString &filePath, QStringList &appList, 
                                    const QString rootTag, const QString childTag)
{
    appList.clear();
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString errorStr;
        int errorLine;
        int errorColumn;

        QDomDocument doc;
        if (doc.setContent(&file, false, &errorStr, &errorLine, &errorColumn)) {
            QDomElement root = doc.documentElement();
            if (root.tagName() == rootTag) {
                QDomNode child = root.firstChild();
                while (!child.isNull()) {
                    if (child.toElement().tagName() == childTag) {
                        appList << child.toElement().text();
                    }
                    child = child.nextSibling();
                }
            }
        }
        else {
            syslog(LOG_ERR, "[%s] Parse error at line '%d' column '%d' : %s", 
                __func__, errorLine, errorColumn, errorStr.toStdString().c_str());
        }
    }
    else {
        syslog(LOG_ERR, "[%s] Open file:'%s' failed!", __func__, filePath.toStdString().c_str());
    }

    // syslog(LOG_DEBUG, "[%s] Get app list from '%s':", __func__, filePath.toStdString().c_str());
    // for (auto app : appList) {
    //     syslog(LOG_DEBUG, "[%s] %s", __func__, app.toStdString().c_str());
    // }
}

void AppSettings::reLoadAppConfigution()
{
    if (!kmre::utils::checkAndCreateFile(mAppConfigurationFile, 0666)) {
        return;
    }
    
    auto data = kmre::utils::tryReadFile(mAppConfigurationFile, true, 100);
    
    if (data.size() > 0) {
        QString errorStr;
        int errorLine;
        int errorColumn;

        syslog(LOG_DEBUG, "[%s] Load app config file : %s", __func__, mAppConfigurationFile.toStdString().c_str());

        kmre::lgm lk(mAppConfigurationDocLk);
        mAppConfigurationDoc.clear();

        data.replace('+', ':');// char '+' is not allowed in xml
        if (mAppConfigurationDoc.setContent(data, false, &errorStr, &errorLine, &errorColumn)) {
            QDomElement apps = mAppConfigurationDoc.documentElement();
            if (apps.tagName() != "apps") {
                syslog(LOG_ERR, "[%s] Can't find node 'apps' in config file!", __func__);
                mAppConfigurationDoc.clear();
            }
        }
        else {
            syslog(LOG_ERR, "[%s] Parse error at line '%d' column '%d' : %s", 
                    __func__, errorLine, errorColumn, errorStr.toStdString().c_str());
        }

    }
}

AppSettings::AppConfig AppSettings::getAppConfig(const QString& pkgName)
{
    AppConfig appConfig = {
        .bootFullScreen = false,
        .bootMaxSize = false,
        .bootWidth = 0, 
        .bootHeight = 0, 
    };
    
    kmre::lgm lk(mAppConfigurationDocLk);

    if (!mAppConfigurationDoc.isNull()) {
        QDomElement root = mAppConfigurationDoc.documentElement();
        if (root.tagName() == "apps") {
            QString tmpPkgName = pkgName;
            tmpPkgName.replace('+', ':');// char '+' is not allowed in xml

            QDomNodeList apps = root.elementsByTagName(tmpPkgName);
            if (apps.size() > 0) {
                QDomElement app = apps.at(0).toElement();

                QDomNode child = app.firstChild();
                while (!child.isNull()) {
                    if (child.toElement().tagName() == "BootFullScreen") {
                        QString tmpStr = child.toElement().text();
                        appConfig.bootFullScreen = (tmpStr.toLower() == "true");
                    }
                    else if (child.toElement().tagName() == "BootMaxSize") {
                        QString tmpStr = child.toElement().text();
                        appConfig.bootMaxSize = (tmpStr.toLower() == "true");
                    }
                    else if (child.toElement().tagName() == "BootWidth") {
                        QString tmpStr = child.toElement().text();
                        bool ok = false;
                        int width = tmpStr.toInt(&ok);
                        appConfig.bootWidth = ok ? width : 0;
                    }
                    else if (child.toElement().tagName() == "BootHeight") {
                        QString tmpStr = child.toElement().text();
                        bool ok = false;
                        int height = tmpStr.toInt(&ok);
                        appConfig.bootHeight = ok ? height : 0;
                    }
                    else if (child.toElement().tagName() == "Tray") {
                        QString tmpStr = child.toElement().text();
                        appConfig.trayEnabled = (tmpStr.toLower() == "true");
                    }

                    child = child.nextSibling();
                }
            }
        }
    }

    syslog(LOG_DEBUG, "[%s] bootFullScreen = %d, BootMaxSize = %d, bootWidth = %d, \
                            bootHeight = %d, trayEnabled = %d", 
            __func__, appConfig.bootFullScreen, appConfig.bootMaxSize, appConfig.bootWidth, 
            appConfig.bootHeight, appConfig.trayEnabled);
    return appConfig;
}

bool AppSettings::setAppConfig(const QString& pkgName, const AppConfig& appConfig)
{
    // set app config
    QDomElement apps;

    kmre::lgm lk(mAppConfigurationDocLk);

    if (mAppConfigurationDoc.isNull()) {
        QDomProcessingInstruction instruction;
        instruction = mAppConfigurationDoc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\"");
        mAppConfigurationDoc.appendChild(instruction);

        apps = mAppConfigurationDoc.createElement("apps");
        mAppConfigurationDoc.appendChild(apps);
    }
    else {
        apps = mAppConfigurationDoc.documentElement();
    }

    if (apps.tagName() == "apps") {
        QDomElement app;
        QString tmpPkgName = pkgName;
        tmpPkgName.replace('+', ':');// char '+' is not allowed in xml

        QDomNodeList appsList = apps.elementsByTagName(tmpPkgName);

        if (appsList.size() <= 0) {
            app = mAppConfigurationDoc.createElement(tmpPkgName);
            apps.appendChild(app);
        }
        else {
            app = appsList.at(0).toElement();
        }

        typedef struct {
            QString cfgName;
            QString cfgValue;
        }AppConfiguration;

        std::vector<AppConfiguration> AppConfigurationList = {
            {"BootFullScreen",  appConfig.bootFullScreen ? "true" : "false"},
            {"BootMaxSize",     appConfig.bootMaxSize ? "true" : "false"},
            {"BootWidth",       QString::number(appConfig.bootWidth)},
            {"BootHeight",      QString::number(appConfig.bootHeight)},
            {"Tray",            appConfig.trayEnabled ? "true" : "false"},
        };

        for (const auto &cfg : AppConfigurationList) {
            QDomNodeList nodeList = app.elementsByTagName(cfg.cfgName);
            if (nodeList.size() > 0) {
                QDomNode cfgNode = nodeList.at(0);
                QDomNode oldNode = cfgNode.firstChild();
                cfgNode.firstChild().setNodeValue(cfg.cfgValue);
                QDomNode newNode = cfgNode.firstChild();
                cfgNode.replaceChild(newNode, oldNode);
            }
            else {
                QDomElement cfgElement = mAppConfigurationDoc.createElement(cfg.cfgName);
                QDomText value = mAppConfigurationDoc.createTextNode(cfg.cfgValue);
                cfgElement.appendChild(value);
                app.appendChild(cfgElement);
            }
        }
    }
 
    // save configuration to file
    mFileWatcher->removePath(mAppConfigurationFile);
    bool ret = kmre::utils::tryWriteFile(mAppConfigurationFile, mAppConfigurationDoc.toByteArray(), true, 100);
    mFileWatcher->addPath(mAppConfigurationFile);

    if (ret) {
        syslog(LOG_DEBUG, "Set app configuration success.");
    }
    else {
        syslog(LOG_ERR, "[%s] Write app configuration file:'%s' failed!", 
                __func__, mAppConfigurationFile.toStdString().c_str());
    }
    return ret;
}

bool AppSettings::setAppBootSize(const QString& pkgName, const QSize& size)
{
    AppConfig appConfig = getAppConfig(pkgName);

    if ((appConfig.bootWidth != size.width()) || (appConfig.bootHeight != size.height())) {
        appConfig.bootWidth = size.width();
        appConfig.bootHeight = size.height();
        return setAppConfig(pkgName, appConfig);
    }
    
    return true;
}

bool AppSettings::setAppBootMaxSize(const QString& pkgName, bool maxsize)
{
    AppConfig appConfig = getAppConfig(pkgName);

    if (appConfig.bootMaxSize != maxsize) {
        appConfig.bootMaxSize = maxsize;
        return setAppConfig(pkgName, appConfig);
    }

    return true;
}

bool AppSettings::setAppBootFullScreen(const QString& pkgName, bool fullScreen)
{
    AppConfig appConfig = getAppConfig(pkgName);

    if (appConfig.bootFullScreen != fullScreen) {
        appConfig.bootFullScreen = fullScreen;
        return setAppConfig(pkgName, appConfig);
    }

    return true;
}

bool AppSettings::setAppTray(const QString& pkgName, bool enable)
{
    AppConfig appConfig = getAppConfig(pkgName);

    if (appConfig.trayEnabled != enable) {
        appConfig.trayEnabled = enable;
        return setAppConfig(pkgName, appConfig);
    }
    
    return true;
}

void AppSettings::slotConfigFileChanged(const QString &path)
{
    syslog(LOG_INFO, "[%s] Config file(%s) changed!", __func__, path.toStdString().c_str());

    if (path == mAppConfigurationFile) {
        reLoadAppConfigution();
    }

    if (!mFileWatcher->files().contains(path)) {
        if (!mFileWatcher->addPath(path)) {
            syslog(LOG_WARNING, "[%s] re-add file(%s) to file watcher failed!", __func__, path.toStdString().c_str());
        }
    }
}

void AppSettings::initAndroidConfigFile()
{
    const QString srcPath = KmreEnv::GetGlobalConfigPath() + "/config/";
    const QString targetPath = KmreEnv::GetAndroidPath() + "/data/local/config/";

    QDir path(targetPath);
    if (!path.exists()) {
        syslog(LOG_ERR, "[%s] config path:'%s' isn't exists!", __func__, targetPath.toStdString().c_str());
        return;
    }

    QStringList configFileList;
    configFileList << mSupportDDSAppListFile.split("/", QString::SkipEmptyParts).back();
    configFileList << mSupportKmreDataAppListFile.split("/", QString::SkipEmptyParts).back();

    for (auto configFile : configFileList) {
        QString srcFile = srcPath + configFile;
        QString targetFile = targetPath + configFile;

        if (!QFile::exists(srcFile)) {
            syslog(LOG_ERR, "[%s] file '%s' isn't exists!", __func__, srcFile.toStdString().c_str());
            continue;
        }
        // always copy config file, for manager deb update can sync config file
        if (QFile::exists(targetFile)) {
            QFile::remove(targetFile);
        }

        if (!QFile::copy(srcFile, targetFile)) {
            syslog(LOG_ERR, "[%s] copy file failed, src file '%s', target file '%s'.", 
                __func__, srcFile.toStdString().c_str(), targetFile.toStdString().c_str());
        }
    }
}

void AppSettings::initAndroidDataPath()
{
    QString dataPath = KmreEnv::GetAndroidDataPath() + "/KmreData/";

    QDir dir;
    if (!dir.mkpath(dataPath)) {
        syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, dataPath.toStdString().c_str());
    }

    // QString downloadPath = dataPath + "/下载";
    // if (!dir.mkpath(downloadPath)) {
    //     syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, downloadPath.toStdString().c_str());
    // }

    QString picturePath = dataPath + "/图片";
    if (!dir.mkpath(picturePath)) {
        syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, picturePath.toStdString().c_str());
    }

    QString moviePath = dataPath + "/视频";
    if (!dir.mkpath(moviePath)) {
        syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, moviePath.toStdString().c_str());
    }

    QString musicPath = dataPath + "/音乐";
    if (!dir.mkpath(musicPath)) {
        syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, musicPath.toStdString().c_str());
    }

    QString documentPath = dataPath + "/文档";
    if (!dir.mkpath(documentPath)) {
        syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, documentPath.toStdString().c_str());
    }

    QString otherPath = dataPath + "/其他";
    if (!dir.mkpath(otherPath)) {
        syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, otherPath.toStdString().c_str());
    }
}

std::tuple<bool, QString> AppSettings::isAppWarned(const QString& pkgName)
{
    QString content;
    QFile file;
    file.setFileName(mWarningConfigFile);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    content = file.readAll();
    file.close();

    QJsonParseError jsonError;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(content.toUtf8(), &jsonError);
    if (!jsonDocument.isNull() && (jsonError.error == QJsonParseError::NoError)) {
        if (jsonDocument.isObject()) {
            QJsonObject obj = jsonDocument.object();
            if (obj.contains("games")) {
                QJsonValue arrayVaule = obj.value("games");
                if (arrayVaule.isArray()) {
                    QJsonArray array = arrayVaule.toArray();
                    for (int i = 0;i<array.size();i++) {
                        QJsonValue value = array.at(i);
                        QJsonObject child = value.toObject();
                        QString pkgname = child["pkgName"].toString();
                        if (pkgname == pkgName) {
                            return {true, tr("Playing with KMRE may result in a ban")};
                        }
                    }
                }
            }

            if (obj.contains("pay")) {
                QJsonValue arrayVaule = obj.value("pay");
                if (arrayVaule.isArray()) {
                    QJsonArray array = arrayVaule.toArray();
                    for (int i = 0;i<array.size();i++) {
                        QJsonValue value = array.at(i);
                        QJsonObject child = value.toObject();
                        QString pkgname = child["pkgName"].toString();
                        if (pkgname == pkgName) {
                            return {true, tr("Payment is risky and use with caution")};
                        }
                    }
                }
            }
        }
    }

    return {false, ""};
}
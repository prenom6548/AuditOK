/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "save-security-config.h"
#include "usdqjsonconf.h"

SaveSecurityConfig::SaveSecurityConfig(QObject *parent)
{
    m_gsettingsModuleList = getConfigFileList("securityConf", "gsettings");
    SYS_LOG(LOG_NOTICE, "Use dconf config module list is {%s}", QString(m_gsettingsModuleList.join(",")).toUtf8().data());
    if (!initSecurityStat() || !isClientSecurityFileExists()) {
        SYS_LOG(LOG_NOTICE, "Desktop components is not under security control");
        exitSecurity();
        return;
    }
    initSecurityConf();
}

bool SaveSecurityConfig::exitSecurity()
{
    setSecurityStat(false);
    SYS_LOG(LOG_INFO, "ExitSecurity Success! Delete all app config");
    deleteDconf();
    deleteAllAppConfig();
    Q_EMIT SecurityExit();
    return true;
}

void SaveSecurityConfig::initSecurityConf()
{
    //删除客户端不存在的本地用户配置
    QStringList clientUserList = getDirList(CLIENT_SECURITYFILE_PATH);
    QStringList localUserList = getDirList(APP_SECURITYFILE_PATH);
    for (const auto localUser : localUserList) {
        if (!clientUserList.contains(localUser)) {
            deleteSecurityConfig(localUser);
        }
    }

    //更新全局域控配置，若不存在则删除组件全局配置
    QString globalConfig = getClientGlobalSecurityFile();
    if (globalConfig.isEmpty()) {
        SYS_LOG(LOG_INFO, "Client don't have global security config file, delete local file!");
        deleteSecurityConfig("");
    } else {
        //初始化更新全局配置时不更新用户配置，下面会更新用户配置
        SYS_LOG(LOG_INFO, "Init client global security config file, file:%s", globalConfig.toUtf8().data());
        updateGlobalConfig(globalConfig, false);
    }

    //更新客户端存在的本地用户配置
    QStringList clinetUserFileList = getClientUserSecurityFileList();
    for (const auto clientUserFile : clinetUserFileList) {
        QFileInfo fileInfo(clientUserFile);
        SYS_LOG(LOG_INFO, "Init client user security config file, file:%s", clientUserFile.toUtf8().data());
        updateUserConfig(clientUserFile, fileInfo.dir().dirName());
    }
}

bool SaveSecurityConfig::UpdateConfig(QString configPath, QString userName)
{
    if (configPath.isEmpty()) {
        SYS_LOG(LOG_WARNING,"UpdateConfig Error! ConfigPath arg is empty!");
        return false;
    }

    if (!QFile(configPath).exists() || QFileInfo(configPath).isDir()) {
        SYS_LOG(LOG_WARNING,"UpdateConfig Error! File %s is not exists!", configPath.toUtf8().data());
        return false;
    }

    if (userName.isEmpty() && configPath != getClientGlobalSecurityFile()) {
        SYS_LOG(LOG_WARNING,"UpdateConfig Error! File %s is not global config!", configPath.toUtf8().data());
        return false;
    }

    if (!userName.isEmpty() && configPath != getAppFilePathStr(CLIENT_SECURITYFILE_PATH, CLIENT_SECURITYFILE_USER, userName)) {
        SYS_LOG(LOG_WARNING,"UpdateConfig Error! File %s is not user %s config!", configPath.toUtf8().data(), userName.toUtf8().data());
        return false;
    }


    return updateConfigBegin(configPath, userName);
}

bool SaveSecurityConfig::ClearConfig(QString userName)
{
    return deleteSecurityConfig(userName);
}

bool SaveSecurityConfig::ExitSecurity()
{
    return exitSecurity();
}

QString SaveSecurityConfig::GetConfigPath(QString userName, QString fileName)
{
    if (fileName.isEmpty() || !getSecurityStat()) {
        return QString();
    }

    return getConfigPath(userName, fileName);
}

QString SaveSecurityConfig::GetUkuiSupport()
{
    QDir dir(UKUI_SUPPORT_PATH);
    if (!dir.exists()) {
        return QString();
    }

    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    QJsonObject clientAllObj;
    for (auto fileInfo : fileInfoList) {
        if (UKUI_SUPPORT_FILE == fileInfo.fileName()) {
            continue;
        }
        QJsonObject clientObj = UsdQJsonConf::loadJsonObject(fileInfo.filePath());
        UsdQJsonConf::mergeJsonObjects(clientAllObj, clientObj);
    }

    QString supportFile(UKUI_SUPPORT_PATH UKUI_SUPPORT_FILE);
    if (clientAllObj.keys().isEmpty()) {
        QFile::remove(supportFile);
        return QString();
    }

    UsdQJsonConf::saveJsonObject(supportFile, clientAllObj);
    return supportFile;
}

bool SaveSecurityConfig::deleteAllAppConfig()
{
    QDir dir(APP_SECURITYFILE_PATH);
    // 检查目录是否存在
    if (!dir.exists()) {
        SYS_LOG(LOG_NOTICE, "Security config dir is not exists!");
        return true;
    }

    if (dir.removeRecursively()) {
        SYS_LOG(LOG_NOTICE, "All security config delete!");
        return true;
    } else {
        SYS_LOG(LOG_WARNING, "Delete security config fail!");
        return false;
    }
}

bool SaveSecurityConfig::deleteSecurityConfig(QString userName)
{
    if (userName.isEmpty()) {
        SYS_LOG(LOG_NOTICE, "Delete global security config");
        deleteDconf();
        return deleteGlobalSecurityConfig();
    } else {
        SYS_LOG(LOG_NOTICE, "Delete user %s security config", userName.toUtf8().data());
        return deleteUserSecurityConfig(userName);
    }

    return false;
}

bool SaveSecurityConfig::deleteUserSecurityConfig(const QString &userName)
{
    if (userName.isEmpty()) {
        SYS_LOG(LOG_DEBUG, "User name is empty");
        return false;
    }

    QString configDir = QString("%1/%2").arg(APP_SECURITYFILE_PATH, userName);
    QDir dir(configDir);
    if (!dir.exists()) {
        return true;
    }

    QStringList updateConfig;
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &fileInfo : fileInfoList) {
        if (fileInfo.suffix() == "json") {
            if (!fileInfo.dir().remove(fileInfo.fileName())) {
                SYS_LOG(LOG_WARNING, "Failed to delete file:%s",fileInfo.absoluteFilePath().toUtf8().data());
            } else {
                updateConfig.append(fileInfo.completeBaseName());
            }
        }
    }
    // 尝试删除目录（非空也要删除）
    if (!dir.removeRecursively()) {
        SYS_LOG(LOG_WARNING, "Failed to delete directory, it may not be empty:%s", configDir.toUtf8().data());
    }
    Q_EMIT ConfigChanged(userName, updateConfig);
    return true;
}

bool SaveSecurityConfig::deleteGlobalSecurityConfig()
{
    QString configDir = QString(APP_SECURITYFILE_PATH);
    QDir appDir(configDir);
    if (!appDir.exists()) {
        return true;
    }

    QStringList updateConfigList = getAllConfigNamefromDir(configDir);
    for (const auto updateConfig : updateConfigList) {
        QFile file(getAppFilePathStr(APP_SECURITYFILE_PATH, updateConfig));
        file.remove();
    }

    QStringList clientUserFileList = getClientUserSecurityFileList();
    for (const auto clientUserFile : clientUserFileList) {
        QFileInfo info(clientUserFile);
        QString userName = info.dir().dirName();

        QString updatePath = QString("%1/%2").arg(APP_SECURITYFILE_PATH, userName);
        QStringList updateAppList = getUpdateAppList(clientUserFile, updatePath);
        if (updateAppList.isEmpty()) {
            continue;
        }
        writeUserConf(clientUserFile, updateAppList, userName);
    }

    Q_EMIT ConfigChanged("", updateConfigList);
    return true;
}

QString SaveSecurityConfig::getAppFilePathStr(const QString &path, const QString &file, QString user)
{
    if (!user.isEmpty()) {
        return QString("%1/%2/%3.json").arg(path, user, file);
    }
    return QString("%1/%2.json").arg(path, file);
}

QString SaveSecurityConfig::getConfigPath(const QString &userName, const QString &appName)
{
    if (appName.isEmpty()) {
        return QString();
    }

    QString filePath;
    if (!userName.isEmpty()) {
        filePath = getAppFilePathStr(APP_SECURITYFILE_PATH, appName, userName);
        if (QFile(filePath).exists()) {
            return filePath;
        }
    }

    filePath = getAppFilePathStr(APP_SECURITYFILE_PATH, appName);
    return QFile(filePath).exists() ? filePath : QString();
}

QStringList SaveSecurityConfig::getAllConfigNamefromDir(const QString &dirName)
{
    QDir dir(dirName);
    if (!dir.exists()) {
        return QStringList();
    }
    QStringList configNameList;
    QFileInfoList fileInfoList = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo &fileInfo : fileInfoList) {
        if (fileInfo.suffix() == "json") {
            configNameList.append(fileInfo.completeBaseName());
        }
    }
    return configNameList;
}

bool SaveSecurityConfig::updateConfigBegin(const QString &updatefile, const QString &userName)
{
    setSecurityStat(true);
    SYS_LOG(LOG_INFO, "UpdateConfig Success! File is %s, userName is %s", updatefile.toUtf8().data(), userName.toUtf8().data());

    if (!m_updateTimerMap.keys().contains(updatefile)) {
        updateTimerInit(updatefile, userName);
    }
    updateTimerStart(updatefile);

    return true;
}

bool SaveSecurityConfig::updateConfig(const QString& configPath, const QString& userName)
{
    if (!QFile(configPath).exists()) {
        return false;
    }

    if (userName.isEmpty()) {
        updateGlobalConfig(configPath);
    } else {
        updateUserConfig(configPath, userName);
    }
    return true;
}

QStringList SaveSecurityConfig::getUpdateAppList(const QString &clientFile, const QString &updatePath)
{
    QStringList changeAppList;
    if (!QFile(clientFile).exists()) {
        return changeAppList;
    }
    createSecurityDir(updatePath);
    QJsonObject clientAllObj = UsdQJsonConf::loadJsonObject(clientFile);
    if (clientAllObj.keys().contains("config")) {
        clientAllObj.remove("config");
    }

    for (auto key : clientAllObj.keys()) {
        if (clientAllObj.value(key).isNull() || clientAllObj.value(key).toObject().isEmpty()) {
            clientAllObj.remove(key);
        }
    }
    QStringList clientAppList = clientAllObj.keys();
    QStringList appList = getAllConfigNamefromDir(updatePath);
    QString appFile;

    for (const QString &app : appList) {
        if (!clientAppList.contains(app)) {
            appFile = getAppFilePathStr(updatePath, app);
            if (QFile(appFile).exists()) {
                QFile(appFile).remove();
                changeAppList.append(app);
            }
        }
    }

    QJsonObject clientObj;
    for (const QString &clientApp : clientAppList) {
        appFile = getAppFilePathStr(updatePath, clientApp);
        if (!QFile(appFile).exists()) {
            changeAppList.append(clientApp);
            continue;
        }

        clientObj = clientAllObj[clientApp].toObject();
        if (updatePath != APP_SECURITYFILE_PATH) {
            QString globalFile = getAppFilePathStr(APP_SECURITYFILE_PATH, clientApp);
            if (QFile(globalFile).exists()) {
                UsdQJsonConf::mergeJsonObjects(clientObj, UsdQJsonConf::loadJsonObject(globalFile).value(clientApp).toObject());
            }
        }

        QJsonObject appObj = UsdQJsonConf::loadJsonObject(appFile);
        if (clientObj != appObj.value(clientApp).toObject()) {
            changeAppList.append(clientApp);
        }
    }

    return changeAppList;
}

void SaveSecurityConfig::updateUserConfig(const QString &clientFile, const QString &userName)
{
    QString updatePath = QString("%1/%2").arg(APP_SECURITYFILE_PATH, userName);
    QStringList updateAppList = getUpdateAppList(clientFile, updatePath);
    if (updateAppList.isEmpty()) {
        SYS_LOG(LOG_NOTICE, "There is no application security config file changed, don't send signal!");
        return;
    }

    writeUserConf(clientFile, updateAppList, userName);

    Q_EMIT ConfigChanged(userName, updateAppList);
}

void SaveSecurityConfig::updateGlobalConfig(const QString& clientFile, bool updateUser)
{
    QStringList updateAppList = getUpdateAppList(clientFile, APP_SECURITYFILE_PATH);
    if (updateAppList.isEmpty()) {
        SYS_LOG(LOG_NOTICE, "There is no application security config file changed, don't send signal!");
        return;
    }

    updateDconf(clientFile);
    writeGlobalConf(clientFile, updateAppList);
    if (updateUser) {
        writeGlobalConfToUserConf(clientFile, updateAppList);
    }

    Q_EMIT ConfigChanged("", updateAppList);
}

void SaveSecurityConfig::writeGlobalConf(const QString &clientFile, const QStringList &updateAppList)
{
    QJsonObject clientGlobalObj = UsdQJsonConf::loadJsonObject(clientFile);

    for (const auto updateApp : updateAppList) {
        if (!clientGlobalObj.keys().contains(updateApp)) {
            continue;
        }
        QJsonObject appObj = clientGlobalObj.value(updateApp).toObject();
        if (appObj.isEmpty()) {
            continue;
        }
        QJsonObject writeObj;
        writeObj.insert(updateApp, appObj);
        QString appFile = getAppFilePathStr(APP_SECURITYFILE_PATH, updateApp);
        UsdQJsonConf::saveJsonObject(appFile, writeObj);
    }
}

void SaveSecurityConfig::writeUserConf(const QString &clientFile, const QStringList &updateAppList, const QString& userName)
{
    QJsonObject clientObj = UsdQJsonConf::loadJsonObject(clientFile);
    for (const auto updateApp : updateAppList) {
        if (!clientObj.keys().contains(updateApp)) {
            continue;
        }
        QJsonObject clientGlobalObj;
        QString globalFile = getAppFilePathStr(APP_SECURITYFILE_PATH, updateApp);
        if (QFile(globalFile).exists()) {
            clientGlobalObj = UsdQJsonConf::loadJsonObject(globalFile).value(updateApp).toObject();
        }
        UsdQJsonConf::mergeJsonObjects(clientGlobalObj, clientObj[updateApp].toObject());
        if (clientGlobalObj.isEmpty()) {
            continue;
        }
        QJsonObject writeObj;
        writeObj.insert(updateApp, clientGlobalObj);
        QString appFile = getAppFilePathStr(APP_SECURITYFILE_PATH, updateApp, userName);
        UsdQJsonConf::saveJsonObject(appFile, writeObj);
    }
}

void SaveSecurityConfig::writeGlobalConfToUserConf(const QString& clientFile, const QStringList& updateAppList)
{
    QJsonObject clientGlobalObj = UsdQJsonConf::loadJsonObject(clientFile);

    QStringList clientUserFileList = getClientUserSecurityFileList();
    for (const auto clientUserFile : clientUserFileList) {
        QFileInfo info(clientUserFile);
        QString userName = info.dir().dirName();

        QJsonObject clientUserObj = UsdQJsonConf::loadJsonObject(clientUserFile);
        for (const auto updateApp : updateAppList) {
            if (clientUserObj.keys().contains(updateApp)) {
                QJsonObject appObj = clientGlobalObj.value(updateApp).toObject();
                UsdQJsonConf::mergeJsonObjects(appObj, clientUserObj.value(updateApp).toObject());
                if (appObj.isEmpty()) {
                    continue;
                }
                QJsonObject writeObj;
                writeObj.insert(updateApp, appObj);
                QString appFile = getAppFilePathStr(APP_SECURITYFILE_PATH, updateApp, userName);
                if(!QFile(appFile).exists()) {
                    QFileInfo info(appFile);
                    createSecurityDir(info.absolutePath());
                }
                UsdQJsonConf::saveJsonObject(appFile, writeObj);
            } else {
                QString appFile = getAppFilePathStr(APP_SECURITYFILE_PATH, updateApp, userName);
                if (QFile(appFile).exists()) {
                    QFile(appFile).remove();
                }
            }
        }
    }
}

void SaveSecurityConfig::updateTimerInit(const QString &updatefile, const QString &userName)
{
    QSharedPointer<QTimer> timer(new QTimer());
    timer.data()->setSingleShot(true);
    m_updateTimerMap.insert(updatefile, timer);
    connect(timer.data(), &QTimer::timeout, this, [this, updatefile, userName] () {
        if (!getSecurityStat()) {
            SYS_LOG(LOG_NOTICE, "Already exited security control! user:%s, config file:%s", userName.toUtf8().data(), updatefile.toUtf8().data());
            m_updateTimerMap.remove(updatefile);
            return;
        }
        SYS_LOG(LOG_NOTICE, "On update config, user:%s, config file:%s", userName.toUtf8().data(), updatefile.toUtf8().data());
        updateConfig(updatefile, userName);
        m_updateTimerMap.remove(updatefile);
    });
}

bool SaveSecurityConfig::createSecurityDir(const QString &dirPath)
{
    // 检查目录是否存在
    return QDir(dirPath).exists() ? true : UsdBaseClass::peekDir(dirPath, QFileDevice::Permission(0x755));
}

void SaveSecurityConfig::updateTimerStart(const QString &updatefile)
{
    auto timer = m_updateTimerMap.value(updatefile);
    if (!timer.isNull()) {
        SYS_LOG(LOG_NOTICE, "config file:%s start timer", updatefile.toUtf8().data());
        timer->start(UPDATE_TIME);
    }
}

QStringList jsonArrayToStringList(const QJsonArray &array)
{
    QStringList list;
    for (const QJsonValue &value : array) {
        if (value.isString()) {
            list.append(QString("'%1'").arg(value.toString()));
        }
    }
    return list;
}

QString processPath(const QString& input)
{
    QString result = input;
    if (result.startsWith("/")) {
        result = result.mid(1);
    }
    if (result.endsWith("/")) {
        result = result.left(result.length() - 1);
    }
    return result;
}

QStringList intersectStringLists(const QStringList& list1, const QStringList& list2)
{
    QStringList result;
    for (const QString& item : list1) {
        if (list2.contains(item)) {
            result << item;
        }
    }
    return result;
}

bool SaveSecurityConfig::createDconfDbFile()
{
    QString msg;
    msg.append("user-db:user\n");
    msg.append("system-db:local\n");
    return writeFile(DCONF_DB_FILE, msg);
}

bool SaveSecurityConfig::writeFile(const QString &file, const QString &msg)
{
    QFileInfo fileInfo(file);
    QString filePath = fileInfo.path();
    if (!createSecurityDir(filePath)) {
        SYS_LOG(LOG_WARNING, "File path %s is not exists, create file path failed!", filePath.toUtf8().data());
        return false;
    }

    QFile targetFile(file);
    if (!targetFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        SYS_LOG(LOG_WARNING, "Write file error! Can not open file %s!", file.toUtf8().data());
        return false;
    }

    QTextStream out(&targetFile);
    out << msg;
    targetFile.close();
    return true;
}

QJsonObject SaveSecurityConfig::getValidJsonObj(const QString &updateApp, const QJsonObject &updateJsonObj)
{
    QJsonObject jsonObj = updateJsonObj;
    QGSettings settings(updateApp.toUtf8());
    auto jsonKeyList = jsonObj.keys();
    for (auto key : jsonKeyList) {
        if (!settings.keys().contains(key)) {
            jsonObj.remove(key);
            SYS_LOG(LOG_WARNING, "Gsettings id: %s no such key: %s!", updateApp.toUtf8().data(), key.toUtf8().data());
            continue;
        }

        QJsonValue value = jsonObj.value(key);
        switch(value.type()) {
        case QJsonValue::Bool:
            if (settings.getKeyType(key) != "b" && settings.getKeyType(key) != "i") {
                jsonObj.remove(key);
                SYS_LOG(LOG_WARNING, "Gsettings id: %s key: %s type is not Bool/Int!", updateApp.toUtf8().data(), key.toUtf8().data());
                continue;
            }
            break;
        case QJsonValue::Double:
            if (settings.getKeyType(key) != "i" && settings.getKeyType(key) != "d") {
                jsonObj.remove(key);
                SYS_LOG(LOG_WARNING, "Gsettings id: %s key: %s type is not Int/Double!", updateApp.toUtf8().data(), key.toUtf8().data());
                continue;
            }
            break;
        case QJsonValue::String:
            if (settings.getKeyType(key) != "s") {
                jsonObj.remove(key);
                SYS_LOG(LOG_WARNING, "Gsettings id: %s key: %s type is not String!", updateApp.toUtf8().data(), key.toUtf8().data());
                continue;
            }
            break;
        case QJsonValue::Array:
            if (settings.getKeyType(key) != "as") {
                jsonObj.remove(key);
                SYS_LOG(LOG_WARNING, "Gsettings id: %s key: %s type is not StringList!", updateApp.toUtf8().data(), key.toUtf8().data());
                continue;
            }
            break;
        }
    }

    return jsonObj;
}

QJsonObject SaveSecurityConfig::getGsetJsonObj(const QString &clientFile)
{
    QJsonObject gsetJsonObj;
    QJsonObject clientGlobalObj = UsdQJsonConf::loadJsonObject(clientFile);
    QStringList updateAppList = intersectStringLists(m_gsettingsModuleList, clientGlobalObj.keys());

    for (const auto updateApp : updateAppList) {
        QJsonObject appObj = clientGlobalObj.value(updateApp).toObject();
        if (appObj.isEmpty()) {
            continue;
        }

        gsetJsonObj.insert(updateApp, getValidJsonObj(updateApp, appObj));
    }
    return gsetJsonObj;
}

void SaveSecurityConfig::updateDconf(const QString &clientFile)
{
    if (!createDconfDbFile()) {
        SYS_LOG(LOG_WARNING, "Create dcong db file failed!");
        return;
    }

    QJsonObject updateJsonObj = getGsetJsonObj(clientFile);
    if (updateJsonObj.isEmpty()) {
        SYS_LOG(LOG_NOTICE, "Dconf updateJsonObj is Empty! Don't update dconf");
        return;
    }
    if (!writeDconfLocaldFile(updateJsonObj)) {
        SYS_LOG(LOG_WARNING, "Write dcong local.d file failed!");
        return;
    }
    if (!writeDconfLocksFile(updateJsonObj)) {
        SYS_LOG(LOG_WARNING, "Write dcong locks file failed!");
        return;
    }
    dconfUpdate();
}

bool SaveSecurityConfig::writeDconfLocaldFile(const QJsonObject &updateJsonObj)
{
    QString str;
    for (const auto updateApp : updateJsonObj.keys()) {
        QJsonObject appObj = updateJsonObj.value(updateApp).toObject();

        QGSettings settings(updateApp.toUtf8());
        QString strTmp = QString("[%1]\n").arg(processPath(settings.getGsettingsPath()));
        for (auto key : appObj.keys()) {
            QJsonValue value = appObj.value(key);
            switch(value.type()) {
            case QJsonValue::Bool:
                strTmp.append(QString("%1=%2\n").arg(key).arg(value.toBool()?"true":"false"));
                break;
            case QJsonValue::Double:
                if (settings.getKeyType(key) == "i") {
                    strTmp.append(QString("%1=%2\n").arg(key).arg(value.toInt()));
                } else if (settings.getKeyType(key) == "d") {
                    strTmp.append(QString("%1=%2\n").arg(key).arg(value.toDouble()));
                }
                break;
            case QJsonValue::String:
                strTmp.append(QString("%1='%2'\n").arg(key).arg(value.toString()));
                break;
            case QJsonValue::Array:
                value.toArray().isEmpty() ? strTmp.append(QString("%1='[]'\n").arg(key)) :
                                            strTmp.append(QString("%1=[%2]\n").arg(key).arg(jsonArrayToStringList(value.toArray()).join(",")));
                break;
            }
        }
        str.append(strTmp);
    }
    return writeFile(DCONF_FILE_CONFIG, str);
}

bool SaveSecurityConfig::writeDconfLocksFile(const QJsonObject &updateJsonObj)
{
    QString str;
    for (const auto updateApp : updateJsonObj.keys()) {
        QJsonObject appObj = updateJsonObj.value(updateApp).toObject();
        QGSettings settings(updateApp.toUtf8());
        for (auto key : appObj.keys()) {
            str.append(QString("%1%2\n").arg(settings.getGsettingsPath()).arg(key));
        }
    }
    return writeFile(DCONF_FILE_LOCK, str);
}

bool SaveSecurityConfig::dconfUpdate()
{
    QProcess process;
    process.setProgram("dconf");
    process.setArguments(QStringList("update"));
    return process.startDetached();
}

void SaveSecurityConfig::deleteDconf()
{
    QFile::remove(DCONF_FILE_CONFIG);
    QFile::remove(DCONF_FILE_LOCK);
    dconfUpdate();
}

QVariant SaveSecurityConfig::getConfigFromQSettingsFile(const QString& file, const QString &group, const QString &key)
{
    QVariant value;
    if (file.isEmpty() || group.isEmpty() || key.isEmpty()) {
        SYS_LOG(LOG_WARNING, "Arguments is empty");
        return value;
    }

    if(!QFile(file).exists()) {
        SYS_LOG(LOG_WARNING, "Config file %s does not exists", file.toUtf8().data());
        return value;
    }

    QSettings usdSettings(file, QSettings::IniFormat);
    usdSettings.beginGroup(group);
    if (!usdSettings.allKeys().contains(key)) {
        SYS_LOG(LOG_NOTICE, "Config file %s don't have key %s", file.toUtf8().data(), key.toUtf8().data());
        usdSettings.endGroup();
        return value;
    }
    value = usdSettings.value(key);
    usdSettings.endGroup();

    return value;
}

QStringList SaveSecurityConfig::getConfigFileList(const QString &group, const QString& key)
{
    QVariant value = getConfigFromQSettingsFile(SECURITYCONFIGFILE_PATH, group, key);

    switch (value.type()) {
    case QVariant::StringList:
    case QVariant::String:
        return value.toStringList();
        break;

    default:
        SYS_LOG(LOG_WARNING, "QVariant value type is %d", value.type());
        break;
    }

    return QStringList();
}

QString SaveSecurityConfig::getClientGlobalSecurityFile()
{
    QString globalFile = getAppFilePathStr(CLIENT_SECURITYFILE_PATH, CLIENT_SECURITYFILE_GLOBAL);
    return QFile(globalFile).exists()? globalFile : QString();
}

QStringList SaveSecurityConfig::getDirList(const QString &path)
{
    QDir dir(path);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    return dir.entryList();
}

QStringList SaveSecurityConfig::getClientUserSecurityFileList()
{
    QStringList userFileList;
    QStringList userDirList = getDirList(CLIENT_SECURITYFILE_PATH);
    for (const auto userDir : userDirList) {
        QString userFilePath = getAppFilePathStr(CLIENT_SECURITYFILE_PATH, CLIENT_SECURITYFILE_USER, userDir);
        if (QFile(userFilePath).exists()) {
            userFileList.append(userFilePath);
        }
    }

    return userFileList;
}

bool SaveSecurityConfig::isClientSecurityFileExists()
{
    return !getClientGlobalSecurityFile().isEmpty() || !getClientUserSecurityFileList().isEmpty();
}

bool SaveSecurityConfig::initSecurityStat()
{
    QVariant state = getConfigFromQSettingsFile(SECURITYCONFIGFILE_PATH, SECURITYSTAT_GROUP, SECURITYSTAT_KEY_STAT);
    if (state.isNull() || !state.isValid()) {
        setQSettingsFile(SECURITYCONFIGFILE_PATH, SECURITYSTAT_GROUP, SECURITYSTAT_KEY_STAT, false);
        return false;
    }

    return state.toBool();
}

bool SaveSecurityConfig::getSecurityStat()
{
    return getConfigFromQSettingsFile(SECURITYCONFIGFILE_PATH, SECURITYSTAT_GROUP, SECURITYSTAT_KEY_STAT).toBool();
}

void SaveSecurityConfig::setSecurityStat(bool stat)
{
    if (getSecurityStat() == stat) {
        return;
    }
    setQSettingsFile(SECURITYCONFIGFILE_PATH, SECURITYSTAT_GROUP, SECURITYSTAT_KEY_STAT, stat);
}


void SaveSecurityConfig::setQSettingsFile(const QString &filePath, const QString &group, const QString &key, const QVariant &value)
{
    if(!QFile(filePath).exists()) {
        QFileInfo info(filePath);
        createSecurityDir(info.absolutePath());
        QFile file(filePath);
        file.setPermissions(QFileDevice::Permission(0x644));
        file.close();
    }

    QSettings usdSettings(filePath, QSettings::IniFormat);
    usdSettings.beginGroup(group);
    if (value.isNull()) {
        usdSettings.remove(key);
    } else {
        usdSettings.setValue(key, value);
    }
    usdSettings.endGroup();
    usdSettings.sync();
}

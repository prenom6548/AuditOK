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

#ifndef SAFEGUARDCUSTOMIZED_H
#define SAFEGUARDCUSTOMIZED_H

#include <errno.h>
#include <QObject>
#include <QVector>
#include <QtDBus/QtDBus>
#include <QtCore/QCoreApplication>
#include <QTimer>

#include "clib-syslog.h"
#include "authority-base.h"
#include "usd_base_class.h"
#include "usd_global_define.h"

#include "QGSettings/qgsettings.h"

#define CLIENT_SECURITYFILE_PATH       "/opt/security-ukui"
#define CLIENT_SECURITYFILE_USER       "settings-user"
#define CLIENT_SECURITYFILE_GLOBAL     "settings-ukui"

#define APP_SECURITYFILE_PATH          "/opt/system/conf/ukui-security-config"

#define SECURITYCONFIGFILE_PATH    "/opt/system/conf/usd/SecurityConf.ini"
#define CLIENTFILEPATH_GRUOP        "clientFilePath"
#define METHODSPERMITED_GRUOP       "methodsPermitied"
#define SECURITYCONFIG_KEY_CLIENT  "client"
#define SECURITYCONFIG_KEY_APP     "app"

#define SECURITYSTAT_GROUP          "securityStat"
#define SECURITYSTAT_KEY_STAT       "stat"

#define SECURITYCLIENT_NAME         "/usr/bin/hedron-client"

#define UKUI_SUPPORT_PATH           "/opt/system/conf/ukui-security-support/"
#define UKUI_SUPPORT_FILE           "ukui-support.json"

#define DCONF_DB_FILE               "/etc/dconf/profile/user"
#define DCONF_PATH_LOCALD           "/etc/dconf/db/local.d/"
#define DCONF_FILE_CONFIG           DCONF_PATH_LOCALD "01-security-config"
#define DCONF_PATH_LOCKS            DCONF_PATH_LOCALD "locks/"
#define DCONF_FILE_LOCK             DCONF_PATH_LOCKS "01-security-lock"

#define UPDATE_TIME 1000

class SaveSecurityConfig : public QObject,protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface",SETTINGS_DAEMON_SYSTEMDBUS_INTERFACE)
public:
    SaveSecurityConfig(QObject *parent = nullptr);

public Q_SLOTS:
    /**  更新管控配置
     *  @brief UpdateConfig
     *  @param configPath   客户端下发配置文件绝对路径
     *  @param userName     管控用户名，为空则为终端配置
     *  @return bool        接口调用状态
    */
    Q_SCRIPTABLE bool UpdateConfig(QString configPath, QString userName);
    /**  清除管控配置
     *  @brief ClearConfig
     *  @param userName     清除配置用户名，为空则清除终端配置
     *  @return bool        接口调用状态
    */
    Q_SCRIPTABLE bool ClearConfig(QString userName);
    /**  退域
     *  @brief ExitSecurity
     *  @return bool        接口调用状态
    */
    Q_SCRIPTABLE bool ExitSecurity();

    /**  获取组件配置文件路径
     *  @brief GetConfigPath
     *  @param userName     获取配置用户名
     *  @return QString     配置文件路径
    */
    Q_SCRIPTABLE QString GetConfigPath(QString userName, QString fileName);

    /** 获取UKUI能力集
     * @brief GetUkuiSupport
     * @return QString      能力集文档路径
     */
    Q_SCRIPTABLE QString GetUkuiSupport();
Q_SIGNALS:
    /** 管控更新信号
     * @brief ConfigChanged
     * @param userName      配置更新用户名，为空则为终端配置更新
     * @param appConfigName 配置更新组件
     */
    void ConfigChanged(QString userName, QStringList appConfigList);
    /** 退域信号
     * @brief SecurityExit
     */
    void SecurityExit();
private:
    bool exitSecurity();
    void initSecurityConf();
    QStringList getDirList(const QString& path);

    QVariant getConfigFromQSettingsFile(const QString& file, const QString& group, const QString& key);
    QStringList getConfigFileList(const QString& group, const QString& key);
    QString getClientGlobalSecurityFile();
    QStringList getClientUserSecurityFileList();

    bool isClientSecurityFileExists();
    bool initSecurityStat();
    bool getSecurityStat();
    void setSecurityStat(bool stat);
    void setQSettingsFile(const QString& filePath, const QString& group, const QString& key, const QVariant& value);

    bool deleteAllAppConfig();
    bool deleteSecurityConfig(QString userName);
    bool deleteUserSecurityConfig(const QString& userName);
    bool deleteGlobalSecurityConfig();

    QString getAppFilePathStr(const QString& path, const QString& file, QString user = QString());
    QString getConfigPath(const QString& userName, const QString& appName);
    QStringList getAllConfigNamefromDir(const QString& dirName);

    bool updateConfig(const QString& configPath, const QString& userName);
    QStringList getUpdateAppList(const QString& clientFile, const QString& updatePath);
    void updateUserConfig(const QString& clientFile, const QString& userName);
    void updateGlobalConfig(const QString& clientFile, bool updateUser = true);
    void writeGlobalConf(const QString& clientFile, const QStringList& updateAppList);
    void writeUserConf(const QString& clientFile, const QStringList& updateAppList, const QString& userName);
    void writeGlobalConfToUserConf(const QString& clientFile, const QStringList& updateAppList);

    bool updateConfigBegin(const QString& updatefile, const QString& userName);
    void updateTimerInit(const QString& updatefile, const QString& userName);
    bool createSecurityDir(const QString& dirPath);
    void updateTimerStart(const QString& updatefile);

    bool createDconfDbFile();
    bool writeFile(const QString& file, const QString& msg);
    QJsonObject getValidJsonObj(const QString &updateApp, const QJsonObject &updateJsonObj);
    QJsonObject getGsetJsonObj(const QString &clientFile);
    void updateDconf(const QString &clientFile);
    bool writeDconfLocaldFile(const QJsonObject &updateJsonObj);
    bool writeDconfLocksFile(const QJsonObject &updateJsonObj);
    bool dconfUpdate();
    void deleteDconf();

private:
    QMap<QString, QSharedPointer<QTimer>> m_updateTimerMap;

    QStringList m_gsettingsModuleList;
};

#endif // SAFEGUARDCUSTOMIZED_H

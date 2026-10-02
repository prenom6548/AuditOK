/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#include "settings-manager.h"
#include "settings-manager-private.h"
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QList>
#include <QLockFile>
#include <QCoreApplication>
#include <mutex>
#include "settings-properties-info.h"
#include "utils.h"

using namespace UkuiNotification;

static const QString SETTINGS_FILE_PATH = QDir::homePath() + QStringLiteral("/.config/org.ukui/ukui-notification");
static const QString SETTINGS_FILE_PATH_NAME = SETTINGS_FILE_PATH + QStringLiteral("/ukui-notification-settings.json");
static const QString SETTINGS_ALLOW_LIST_NAME =  QStringLiteral("/etc/ukui/ukui-notification/notification-allowlist.json");
static const QString DEFAULT_APP_LIST_PATH =  QStringLiteral("/etc/ukui/ukui-notification/default-app/");

static std::once_flag flag;
static SettingsManager *s_self;
SettingsManagerPrivate::SettingsManagerPrivate(QObject* parent)
    : QObject(parent)
    , m_applicationInfo(Utils::applicationInfo())
    , m_allowListData(loadAllowListData())
{
    for(SettingsProperty::Property property : SINGLE_APPLICATION_SETTINGS) {
        SettingsPropertiesInfo info(property);
        m_applicationDefaultSettings.insert(info.name(), info.defaultValue());
    }
}

SettingsManagerPrivate::~SettingsManagerPrivate()
{
}

void SettingsManagerPrivate::init()
{
    m_watcher = new QFileSystemWatcher(this);
    connect(m_watcher, &QFileSystemWatcher::fileChanged, this, &SettingsManagerPrivate::checkAndLoad);
    connect(&m_applicationInfo, &UkuiSearch::ApplicationInfo::appDBItems2BDelete, this, &SettingsManagerPrivate::desktopFileDelete);
    checkAndLoad();
}
void SettingsManagerPrivate::checkAndLoad()
{
    QLockFile lockFile(SETTINGS_FILE_PATH + QStringLiteral("ukui-notification-settings.json.lock"));
    lockFile.lock();
    QFile settingsFile(SETTINGS_FILE_PATH_NAME);
    if (!settingsFile.exists()) {
        qDebug() << SETTINGS_FILE_PATH_NAME << " is not exist, start create settings file.";
        createSettingsFile();
        lockFile.unlock();
        Q_EMIT settingsUpdateFinished();
        return;
    }

    if (!settingsFile.open(QFile::ReadOnly)) {
        qWarning() << "SettingsManagerPrivate: configuration file " << settingsFile.fileName() << "open failed !";
        m_watcher->addPath(SETTINGS_FILE_PATH_NAME);
        lockFile.unlock();
        return;
    }
    // 读取json数据
    QByteArray byteArray = settingsFile.readAll();
    settingsFile.close();
    QJsonParseError errRpt;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray, &errRpt));
    if (errRpt.error != QJsonParseError::NoError) {
        qWarning() << "SettingsManagerPrivate: Incorrect configuration files. JSON parse error";
        createSettingsFile();
        lockFile.unlock();
        Q_EMIT settingsUpdateFinished();
        return;
    }

    // 读取配置文件
    QJsonObject settingsData = jsonDocument.object();
    QString version = settingsData.value("VERSION").toString();
    bool updated = updateSettingsFile(version, QStringLiteral(NOTIFICATION_SETTINGS_VERSION), settingsData);
    lockFile.unlock();
    if (updated) {
        Q_EMIT settingsUpdateFinished();
    }
}

void SettingsManagerPrivate::createSettingsFile()
{
    initSettingsData(m_settingsData);
    save2SettingsFile(m_settingsData);
    m_watcher->addPath(SETTINGS_FILE_PATH_NAME);
}

bool SettingsManagerPrivate::updateSettingsFile(const QString& oldVersion, const QString& newVersion, QJsonObject& data)
{
    if (oldVersion.toDouble() > newVersion.toDouble()) {
        qCritical() << "Notification configuration version has been updated to "<< oldVersion << ", current version is " << newVersion <<", please restart the application";
        return false;
    }

    bool needUpdateFileVersion = false;
    if (oldVersion == "2.0" && newVersion.toDouble() > 2.0) {
        needUpdateFileVersion = true;
    }
    //版本号不变， 或者版本号2.0升级到高版本，只获取配置，其他不做处理
    if (oldVersion == newVersion || needUpdateFileVersion) {
        if(m_needCheckApplist) {
            checkApplicationList(data);
            m_needCheckApplist = false;
        }
        m_settingsData.swap(data);
        if (needUpdateFileVersion) {
            m_settingsData.insert("VERSION", newVersion);
            save2SettingsFile(m_settingsData);
        }
        m_watcher->addPath(SETTINGS_FILE_PATH_NAME);
        return true;
    }

    //版本号1.0升级到高版本，配置清空
    if (oldVersion == "1.0" && newVersion.toDouble() > 1.0) {
        m_watcher->removePath(SETTINGS_FILE_PATH_NAME);
        QJsonObject newData;

        //初始化当前版本默认值
        initSettingsData(newData);
        //写入数据
        m_settingsData.swap(newData);
        save2SettingsFile(m_settingsData);
        m_watcher->addPath(SETTINGS_FILE_PATH_NAME);
        qDebug() << "Notification settings version is different, need update! configFile:" << oldVersion << " new:" << newVersion;
        return true;
    }
    return false;
}

QStringList SettingsManagerPrivate::getAppInfo()
{
    QSet<QString> result;
    for (const QString& desktopPath : m_applicationInfo.getInfo(UkuiSearch::ApplicationProperties{}).keys()) {
        if (const auto& appDesktop = Utils::desktopEntry2Name(desktopPath); !appDesktop.isEmpty()) {
            result.insert(appDesktop);
        }
    }
    return result.values();
}

void SettingsManagerPrivate::initSettingsData(QJsonObject &data)
{
    data.insert("VERSION", QString(NOTIFICATION_SETTINGS_VERSION));

    //全局设置
    QJsonObject globalSettings;
    for(SettingsProperty::Property property : GLOBAL_SETTINGS) {
        SettingsPropertiesInfo info(property);
        globalSettings.insert(info.name(), info.defaultValue());
    }
    data.insert(QStringLiteral("Global"), globalSettings);
}

void SettingsManagerPrivate::desktopFileDelete(const QStringList &data)
{
    if (data.isEmpty()) {
        return;
    }
    if (qApp->property("IS_UKUI_NOTIFICATION_SERVER").toBool()) {
        QJsonObject applicationsSettings = m_settingsData.value(QStringLiteral("Applications")).toObject();
        for (const QString &app : data) {
            applicationsSettings.remove(UkuiNotification::Utils::desktopEntry2Name(app));
        }
        m_settingsData.insert(QStringLiteral("Applications"), applicationsSettings);
        QLockFile lockFile(SETTINGS_FILE_PATH + QStringLiteral("ukui-notification-settings.json.lock"));
        lockFile.lock();
        m_watcher->removePath(SETTINGS_FILE_PATH_NAME);
        save2SettingsFile(m_settingsData);
        m_watcher->addPath(SETTINGS_FILE_PATH_NAME);
        lockFile.unlock();
        Q_EMIT settingsUpdateFinished();
    }
    for(const QString &app : data) {
        Q_EMIT appUninstalled(UkuiNotification::Utils::desktopEntry2Name(app));
    }
}

void SettingsManagerPrivate::save2SettingsFile(const QJsonObject &jsonDocData)
{
    QDir dir;
    QString configFileDir(SETTINGS_FILE_PATH);
    if (!dir.exists(configFileDir)) {
        if (!dir.mkdir(configFileDir)) {
            qWarning() << "Unable to create notification settings config file.";
            return;
        }
    }

    QFile settingsFile(SETTINGS_FILE_PATH_NAME);
    settingsFile.open(QIODevice::WriteOnly | QIODevice::Truncate);

    if (settingsFile.write(QJsonDocument(jsonDocData).toJson()) == -1) {
        qWarning() << "SettingsManagerPrivate: Error saving configuration file.";
    }
    settingsFile.flush();
    settingsFile.close();
}

void SettingsManagerPrivate::checkApplicationList(QJsonObject &data)
{
    QStringList appList = getAppInfo();
    QStringList appNeedRemoved;
    QJsonObject applicationSettings = data.value(QStringLiteral("Applications")).toObject();
    for(const QString &desktopFile : applicationSettings.keys()) {
        if(!appList.contains(desktopFile)) {
            appNeedRemoved.append(desktopFile);
        }
    }
    //移除应用列表里已经不存在的应用
    for(const QString &desktopFile : appNeedRemoved) {
        applicationSettings.remove(desktopFile);
    }

    data.insert(QStringLiteral("Applications"), applicationSettings);
    save2SettingsFile(data);
}

QJsonObject SettingsManagerPrivate::loadAllowListData()
{
    QFile file(SETTINGS_ALLOW_LIST_NAME);
    if (!file.exists()) {
        qWarning() << "SettingsManagerPrivate::loadAllowListData" << file.fileName() << "is not exists";
        return QJsonObject();
    }

    if (!file.open(QFile::ReadOnly)) {
        qWarning() << "SettingsManagerPrivate: configuration file " << file.fileName() << "open failed !";
        return QJsonObject();
    }
    // 读取json数据
    QByteArray byteArray = file.readAll();
    file.close();
    QJsonParseError errRpt;
    QJsonDocument jsonDocument(QJsonDocument::fromJson(byteArray, &errRpt));
    if (errRpt.error != QJsonParseError::NoError) {
        qWarning() << "SettingsManagerPrivate: Incorrect configuration files " << file.fileName() << "JSON parse error";
        return QJsonObject();
    }
    return jsonDocument.object();
}

SettingsManager::SettingsManager(QObject *parent) : QObject(parent), d(new SettingsManagerPrivate(this))
{
    connect(d, &SettingsManagerPrivate::settingsUpdateFinished, this, &SettingsManager::settingsDataChanged);
    connect(d, &SettingsManagerPrivate::appUninstalled, this, &SettingsManager::appUninstalled);
    d->init();
}

QJsonObject SettingsManager::getGlobalSettings()
{
    return d->m_settingsData.value(QStringLiteral("Global")).toObject();
}

QJsonObject SettingsManager::getAllAppSettings()
{
    return d->m_settingsData.value(QStringLiteral("Applications")).toObject();
}

QJsonObject SettingsManager::getAppSettings(const QString &appDesktopName)
{
    if(appDesktopName == QStringLiteral("default") || appDesktopName.isEmpty()) {
        return d->m_applicationDefaultSettings;
    }
    QJsonValue value = d->m_settingsData.value(QStringLiteral("Applications")).toObject().value(appDesktopName);
    if(value.isUndefined()) {
        return {};
    }
    return value.toObject();
}

QJsonObject SettingsManager::getAppDefaultSettings()
{
    return d->m_applicationDefaultSettings;
}

void SettingsManager::getAppList(QStringList &defaultAppList, QStringList &otherAppList)
{
    auto checkDefaultApp = [](const QJsonObject& appData) -> bool{
        if (appData.value("ConfigurationSource").toString() == QStringLiteral("Default")) {
            return true;
        }
        return false;
    };

    QJsonObject applicationsData = d->m_settingsData.value(QStringLiteral("Applications")).toObject();
    for(const QString &appName : applicationsData.keys()) {
        QJsonObject appData = applicationsData.value(appName).toObject();
        if (checkDefaultApp(appData)) {
            defaultAppList << appName;
        } else {
            otherAppList << appName;
        }
    }
}

SettingsManager *SettingsManager::self()
{
    std::call_once(flag, [ & ] {
        s_self = new SettingsManager();
    });
    return s_self;
}

bool SettingsManager::setGlobalSettings(SettingsProperty::Property key, const QVariant &value)
{
    if(!GLOBAL_SETTINGS.contains(key)) {
        return false;
    }
    auto info = SettingsPropertiesInfo(key);
    QString valueToSet = info.valueToString(value);

    QJsonObject data = d->m_settingsData.value(QStringLiteral("Global")).toObject();
    if (data.value(info.name()).toString() == valueToSet) {
        qWarning() << "SettingsManager::setGlobalSettings" << info.name() << "value is same";
        return false;
    }
    data.insert(info.name(), valueToSet);
    d->m_settingsData.insert(QStringLiteral("Global"), data);
    QLockFile lockFile(SETTINGS_FILE_PATH + QStringLiteral("ukui-notification-settings.json.lock"));
    lockFile.lock();
    d->save2SettingsFile(d->m_settingsData);
    lockFile.unlock();
    return true;
}

bool SettingsManager::setGlobalSettings(const QList<QPair<SettingsProperty::Property, QVariant>> &values)
{
    if (values.isEmpty()) {
        return false;
    }

    QJsonObject data = d->m_settingsData.value(QStringLiteral("Global")).toObject();
    bool changed = false;

    for (const auto &item : values) {
        const auto key = item.first;
        const auto &value = item.second;

        if (!GLOBAL_SETTINGS.contains(key)) {
            return false;
        }

        const SettingsPropertiesInfo info(key);
        const QString valueToSet = info.valueToString(value);
        if (data.value(info.name()).toString() != valueToSet) {
            data.insert(info.name(), valueToSet);
            changed = true;
        }
    }

    if (!changed) {
        return false;
    }

    d->m_settingsData.insert(QStringLiteral("Global"), data);

    QLockFile lockFile(SETTINGS_FILE_PATH + QStringLiteral("ukui-notification-settings.json.lock"));
    lockFile.lock();
    d->save2SettingsFile(d->m_settingsData);
    lockFile.unlock();
    return true;
}

bool SettingsManager::setAppSettings(const QString &appDesktopName, SettingsProperty::Property key, const QVariant &value)
{
    if(!SINGLE_APPLICATION_SETTINGS.contains(key)) {
        return false;
    }

    QJsonObject appsData = d->m_settingsData.value(QStringLiteral("Applications")).toObject();
    QJsonValue dataValue = appsData.value(appDesktopName);
    if(dataValue.isUndefined()) {
        return false;
    }
    QJsonObject oneAppData = dataValue.toObject();
    auto info = SettingsPropertiesInfo(key);
    QString valueToSet = info.valueToString(value);
    if(oneAppData.value(info.name()).toString() == valueToSet) {
        return false;
    }
    oneAppData.insert(info.name(), valueToSet);
    appsData.insert(appDesktopName, oneAppData);
    d->m_settingsData.insert(QStringLiteral("Applications"), appsData);

    QLockFile lockFile(SETTINGS_FILE_PATH + QStringLiteral("ukui-notification-settings.json.lock"));
    lockFile.lock();
    d->save2SettingsFile(d->m_settingsData);
    lockFile.unlock();
    return true;
}

bool SettingsManager::addAppSettings(const QString &appDesktopName, const QString &configurationSource)
{
    if (appDesktopName.isEmpty()) {
        qWarning() << "SettingsManager::addAppSettings:"  << "app desktop name is empty.";
        return false;
    }

    QJsonObject appsData = d->m_settingsData.value(QStringLiteral("Applications")).toObject();
    //标记配置来源
    d->m_applicationDefaultSettings.insert(QStringLiteral("ConfigurationSource"), configurationSource);
    appsData.insert(appDesktopName, d->m_applicationDefaultSettings);
    d->m_settingsData.insert(QStringLiteral("Applications"), appsData);
    QLockFile lockFile(SETTINGS_FILE_PATH + QStringLiteral("ukui-notification-settings.json.lock"));
    lockFile.lock();
    d->m_watcher->removePath(SETTINGS_FILE_PATH_NAME);
    d->save2SettingsFile(d->m_settingsData);
    d->m_watcher->addPath(SETTINGS_FILE_PATH_NAME);
    lockFile.unlock();
    return true;
}

bool SettingsManager::deleteAppSettings(const QString &appDesktopName)
{
    if (appDesktopName.isEmpty()) {
        qWarning() << "SettingsManager::deleteAppSettings:"  << "app desktop name is empty.";
        return false;
    }
    QJsonObject appsData = d->m_settingsData.value(QStringLiteral("Applications")).toObject();
    if (!appsData.contains(appDesktopName)) {
        qWarning() << "SettingsManager::deleteAppSettings:"  << "Applications settings not containns "<< appDesktopName;
        return false;
    }
    appsData.remove(appDesktopName);
    d->m_settingsData.insert(QStringLiteral("Applications"), appsData);
    QLockFile lockFile(SETTINGS_FILE_PATH + QStringLiteral("ukui-notification-settings.json.lock"));
    lockFile.lock();
    d->m_watcher->removePath(SETTINGS_FILE_PATH_NAME);
    d->save2SettingsFile(d->m_settingsData);
    d->m_watcher->addPath(SETTINGS_FILE_PATH_NAME);
    lockFile.unlock();
    return true;
}

/*!
    删除指定配置来源的应用设置
 */
bool SettingsManager::deleteAppSettingsByConfigurationSource(const QString &configurationSource)
{
    auto appsData = d->m_settingsData.value(QStringLiteral("Applications")).toObject();
    for (auto& [desktop, appValue] : appsData.toVariantMap().toStdMap()) {
        QJsonObject appJson = appValue.toJsonObject();
        if (appJson.value(QStringLiteral("ConfigurationSource")).toString() == configurationSource) {
            appsData.remove(desktop);
        }
    }

    d->m_settingsData.insert(QStringLiteral("Applications"), appsData);
    QLockFile lockFile(SETTINGS_FILE_PATH + QStringLiteral("ukui-notification-settings.json.lock"));
    lockFile.lock();
    d->m_watcher->removePath(SETTINGS_FILE_PATH_NAME);
    d->save2SettingsFile(d->m_settingsData);
    d->m_watcher->addPath(SETTINGS_FILE_PATH_NAME);
    lockFile.unlock();
    return true;
}

QStringList SettingsManager::getAppListByConfigurationSource(const QString& configurationSource) const
{
    QStringList appList;
    auto appsData = d->m_settingsData.value(QStringLiteral("Applications")).toObject();
    for (const auto& [desktop, appValue] : appsData.toVariantMap().toStdMap()) {
        if (appValue.toJsonObject().value(QStringLiteral("ConfigurationSource")).toString() == configurationSource) {
            appList.append(desktop);
        }
    }
    return appList;
}

/*!
    获取系统全量应用desktop名称
 */
QStringList SettingsManager::getAllApps() const
{
    return d->getAppInfo();
}

bool SettingsManager::hasAppSettings(const QString &appDesktopName)
{
    if (appDesktopName.isEmpty()) {
        qWarning() << "SettingsManager::hasAppSettings" << "app desktop name is empty.";
        return false;
    }
    return !d->m_settingsData.value(QStringLiteral("Applications")).toObject().value(appDesktopName).isUndefined();
}

bool SettingsManager::isInAllowList(const QString &appDesktopName)
{
    if (appDesktopName.isEmpty()) {
        qWarning() << "SettingsManager::isInAllowList:" <<"app desktop name is empty.";
        return false;
    }
    return d->m_allowListData.value(QStringLiteral("Applications")).toObject().keys().contains(appDesktopName);
}

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

#include "applications-settings.h"
#include <mutex>
#include <QQmlEngine>
#include <QDebug>
#include "settings-manager.h"
#include <QJsonObject>

#include "utils.h"

using namespace UkuiNotification;
static std::once_flag flag;
static ApplicationsSettings *s_self;
namespace UkuiNotification{
class ApplicationsSettingsPrivate
{
    friend class ApplicationsSettings;
private:
    ApplicationsSettingsMap m_cache;
    QJsonObject m_settingsData;
};
}

/*!
    获取 ApplicationsSettings 全局单例实例.
 */
ApplicationsSettings *UkuiNotification::ApplicationsSettings::self()
{
    std::call_once(flag, [ & ] {
        s_self = new ApplicationsSettings();
    });
    return s_self;
}

/*!
    \class UkuiNotification::ApplicationsSettings
    \inmodule UkuiNotification
    \inheaderfile notifications-settings/applications-settings.h
    \brief 提供应用程序通知设置的管理功能.

    该类负责管理所有应用程序的通知设置，使用单例模式设计，通过self()方法获取实例.
 */

/*!
    \fn void UkuiNotification::ApplicationsSettings::applicationUninstalled(const QString &desktopEntry)

    此信号在应用程序被卸载时触发.
 */

/*!
    \fn void UkuiNotification::ApplicationsSettings::settingsUpdated(const QString& desktopEntry)

    此信号在应用程序通知设置被更新时触发.
 */

/*!
    \fn void UkuiNotification::ApplicationsSettings::applicationInstalled(const QString &desktopEntry)

    此信号在应用程序安装时触发.
 */

/*!
    Constructor
 */
ApplicationsSettings::ApplicationsSettings(QObject *parent) : QObject(parent), d(new ApplicationsSettingsPrivate)
{
    d->m_settingsData = SettingsManager::self()->getAllAppSettings();
    connect(SettingsManager::self(), &SettingsManager::appUninstalled, this, &ApplicationsSettings::onApplicationUninstalled);
    connect(SettingsManager::self(), &SettingsManager::settingsDataChanged, this, &ApplicationsSettings::settingsDataChanged);
    qmlRegisterUncreatableType<UkuiNotification::SingleApplicationSettings>("org.ukui.notification.client", 1, 0, "SingleApplicationSettings", "");
}

/*!
    Destructor
 */
ApplicationsSettings::~ApplicationsSettings()
{
    if(d) {
        qDeleteAll(d->m_cache);
        d->m_cache.clear();
        delete d;
        d = nullptr;
    }
}

/*!
    \brief 获取某条通知的配置。
    \sa creatSettings(QString desktopEntry)
 */
SingleApplicationSettings *ApplicationsSettings::creatSettings(const PopupNotification &notification)
{
    return creatSettings(notification.desktopEntry());
}

/*!
    获取所有应用的通知配置。
 */
ApplicationsSettingsMap &ApplicationsSettings::getAllApplicationsSettings()
{
    for(const QString &desktopEntry : d->m_settingsData.keys()) {
        if(!d->m_cache.contains(desktopEntry)) {
            d->m_cache.insert(desktopEntry, new SingleApplicationSettings(desktopEntry));
        }
    }
    return d->m_cache;
}

/*!
    根据 \a appDesktopName 获取当前用户某个应用通知相关配置信息。 传入 "default" 获取配置默认值。
 */
QJsonObject ApplicationsSettings::getAppSettings(const QString &appDesktopName)
{
    if(appDesktopName == QStringLiteral("default") || appDesktopName.isEmpty()) {
        return SettingsManager::self()->getAppDefaultSettings();
    }
    QJsonValue value = d->m_settingsData.value(appDesktopName);
    if(value.isUndefined()) {
        return SettingsManager::self()->getAppDefaultSettings();;
    }
    return value.toObject();
}

/*!
    设置应用通知相关配置。
    \a appDesktopName 应用 desktop 名称。
    \a key 通知的某个属性配置项。
    \a value 对应属性配置值。
    返回 \c true 表示设置成功，\c false 表示设置失败。
 */
bool ApplicationsSettings::setAppSetting(const QString &appDesktopName, SettingsProperty::Property key,
                                         const QVariant &value)
{
    return SettingsManager::self()->setAppSettings(appDesktopName, key, value);
}

/*!
    根据 \a appDesktopName, 查询是否存在应用通知相关配置。
 */
bool ApplicationsSettings::hasAppSettings(const QString &appDesktopName)
{
    return SettingsManager::self()->hasAppSettings(appDesktopName);
}

/*!
    通过一条通知 \a notification , 查询是否存在应用通知相关配置。
 */
bool ApplicationsSettings::hasAppSettings(const PopupNotification &notification)
{
    return hasAppSettings(notification.desktopEntry());
}

/*!
    根据 \a appDesktopName , 查询是否在白名单内。
    白名单内的应用相关的通知配置不会暴露给应用设置。
 */
bool ApplicationsSettings::isInAllowList(const QString &appDesktopName)
{
    return SettingsManager::self()->isInAllowList(appDesktopName);
}

/*!
    \sa isInAllowList(const QString &appDesktopName)
 */
bool ApplicationsSettings::isInAllowList(const PopupNotification &notification)
{
    return SettingsManager::self()->isInAllowList(notification.desktopEntry());
}

void ApplicationsSettings::settingsDataChanged()
{
    QJsonObject data = SettingsManager::self()->getAllAppSettings();
    if(d->m_settingsData == data) {
        return;
    }

    //更新现有app 配置
    for (const QString &desktopEntry : data.keys()) {
        if (d->m_settingsData.contains(desktopEntry)) {
            const QJsonValue&& oldValue = d->m_settingsData.value(desktopEntry);
            const QJsonValue&& newValue = data.value(desktopEntry);
            if ( oldValue != newValue) {
                d->m_settingsData.insert(desktopEntry, newValue);
                Q_EMIT settingsUpdated(desktopEntry);
            }
        }
    }

    //新增
    QStringList needAddList =  StringListDiff(data.keys(), d->m_settingsData.keys());
    //删除
    QStringList needRemoveList = StringListDiff(d->m_settingsData.keys(), data.keys());

    for(const QString &desktopEntry : needAddList) {
        d->m_settingsData.insert(desktopEntry, data.value(desktopEntry));
        Q_EMIT applicationInstalled(desktopEntry);
    }

    for(const QString &desktopEntry : needRemoveList) {
        d->m_settingsData.remove(desktopEntry);
        Q_EMIT applicationUninstalled(desktopEntry);

        auto* singleSetting = d->m_cache.take(desktopEntry);
        if (singleSetting) {
            singleSetting->deleteLater();
        }
    }
}

void ApplicationsSettings::onApplicationUninstalled(const QString &desktopEntry)
{
    if (d->m_settingsData.contains(desktopEntry)) {
        d->m_settingsData.remove(desktopEntry);
        Q_EMIT applicationUninstalled(desktopEntry);

        auto* singleSetting = d->m_cache.take(desktopEntry);
        if (singleSetting) {
            singleSetting->deleteLater();
        }
    }
}

/*!
    \brief 根据 \a desktopEntry 获取应用的通知配置
 */
SingleApplicationSettings *ApplicationsSettings::creatSettings(QString desktopEntry)
{
    if(desktopEntry.isEmpty()) {
        desktopEntry = QStringLiteral("default");
    }
    if(d->m_cache.contains(desktopEntry)) {
        return d->m_cache[desktopEntry];
    } else {
        return d->m_cache.insert(desktopEntry, new SingleApplicationSettings(desktopEntry)).value();
    }
}

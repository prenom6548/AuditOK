/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: sundagao <sundagao@kylinos.cn>
 */
#include "notificaiton-settings-query.h"
#include <utils.h>

using namespace UkuiNotification;
NotificaitonSettingsQuery::NotificaitonSettingsQuery(QObject* parent)
    :QObject(parent)
{
    m_notificationGlobalSettings = new NotificationGlobalSettings(this);
    connect(m_notificationGlobalSettings, &NotificationGlobalSettings::dndChanged, this, &NotificaitonSettingsQuery::checkAlarmSoundAllowed);
    connect(m_notificationGlobalSettings, &NotificationGlobalSettings::dndChanged, this, &NotificaitonSettingsQuery::dndChanged);
    connect(m_notificationGlobalSettings, &NotificationGlobalSettings::receiveNotificationsFromAppsChanged, this, &NotificaitonSettingsQuery::checkAlarmSoundAllowed);
    connect(m_notificationGlobalSettings, &NotificationGlobalSettings::notifyAlarmWhileDNDChanged, this, &NotificaitonSettingsQuery::checkAlarmSoundAllowed);

    //获取闹钟配置
    m_alarmSingleAppSettings = new SingleApplicationSettings(QStringLiteral("ukui-clock"), this);
    if (m_alarmSingleAppSettings) {
        connect(m_alarmSingleAppSettings, &SingleApplicationSettings::allowNotifyChanged, this, &NotificaitonSettingsQuery::checkAlarmSoundAllowed);
        connect(m_alarmSingleAppSettings, &SingleApplicationSettings::allowSoundChanged, this, &NotificaitonSettingsQuery::checkAlarmSoundAllowed);
    }
}

bool NotificaitonSettingsQuery::isAlarmSoundAllowed() const
{
    //勿扰模式
    bool isDND = m_notificationGlobalSettings->isDND();
    //勿扰模式允许闹钟提示
    bool notifyAlarmWhileDND = m_notificationGlobalSettings->notifyAlarmWhileDND();

    if (isDND && !notifyAlarmWhileDND)
    {
        //勿扰模式，不允许闹钟提示
        return false;
    }
    //允许获取应用的通知
    bool isAllowedReceiveNotifications = m_notificationGlobalSettings->receiveNotificationsFromApps();

    //闹钟配置
    bool clockAllowedNotify = m_alarmSingleAppSettings->allowNotify();
    bool clockAllowedSound = m_alarmSingleAppSettings->allowSound();
    return isAllowedReceiveNotifications && clockAllowedNotify && clockAllowedSound;
}

bool NotificaitonSettingsQuery::isSoundAllowed(const QString& desktopEntry) const
{
    if (desktopEntry.isEmpty()) {
        qWarning() << "NotificaitonSettingsQuery::isSoundAllowed : desktop is empty";
        return false;
    }
    QString desktop = desktopEntry;
    if (desktop.startsWith("/")) {
        desktop = Utils::desktopEntry2Name(desktop);
    }
    qDebug() << "NotificaitonSettingsQuery::isSoundAllowed : " << desktop << "obtains the playback prompt sound status .";
    //勿扰模式
    bool isDND = m_notificationGlobalSettings->isDND();
    //获取是否允许获取通知
    bool isAllowedReceiveNotifications = m_notificationGlobalSettings->receiveNotificationsFromApps();
    //勿扰模式允许闹钟提示
    bool notifyAlarmWhileDND = false;
    //默认单个应用配置
    bool singleAppAllowedNotify = true;
    bool SingleAppAllowedSound = true;
    if (desktop == QStringLiteral("ukui-clock")) {
        //查询闹钟时才获取notifyAlarmWhileDND的实际状态
        notifyAlarmWhileDND = m_notificationGlobalSettings->notifyAlarmWhileDND();
        singleAppAllowedNotify = m_alarmSingleAppSettings->allowNotify();
        SingleAppAllowedSound = m_alarmSingleAppSettings->allowSound();
    } else {
        const SingleApplicationSettings singleAppSettings(desktop);
        singleAppAllowedNotify = singleAppSettings.allowNotify();
        SingleAppAllowedSound = singleAppSettings.allowSound();
    }
    return (!isDND || notifyAlarmWhileDND) && isAllowedReceiveNotifications && singleAppAllowedNotify && SingleAppAllowedSound;
}

void NotificaitonSettingsQuery::checkAlarmSoundAllowed()
{
    bool alarmSoundAllowed = isAlarmSoundAllowed();
    if (m_alarmSoundAllowed != alarmSoundAllowed)
    {
        qDebug() << "NotificaitonSettingsQuery::checkAlarmSoundAllowed AlarmSoundAllowed changed: " << alarmSoundAllowed;
        m_alarmSoundAllowed = alarmSoundAllowed;
        Q_EMIT alarmSoundAllowedChanged(alarmSoundAllowed);
    }
}

void NotificaitonSettingsQuery::setDND(bool enable)
{
    m_notificationGlobalSettings->setDND(enable);
}

bool NotificaitonSettingsQuery::isDND()
{
    return m_notificationGlobalSettings->isDND();
}

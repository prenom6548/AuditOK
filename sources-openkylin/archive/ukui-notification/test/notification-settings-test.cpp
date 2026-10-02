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

#include "notification-settings-test.h"
#include "popup-notification.h"

NotificationSettingsTest::NotificationSettingsTest(QObject *parent) : QObject(parent)
{
    UkuiNotification::ApplicationsSettingsMap &map = UkuiNotification::ApplicationsSettings::self()->getAllApplicationsSettings();
    for(const QString &key : map.keys()) {
        initConnection(map.value(key));
    }
    m_globalSettings = new UkuiNotification::NotificationGlobalSettings(this);
    connect(m_globalSettings, &UkuiNotification::NotificationGlobalSettings::scheduleTurnOnDNDChanged, this, [&](bool value){
        qDebug() << "scheduleTurnOnDNDChanged" << value;
    });
    connect(m_globalSettings, &UkuiNotification::NotificationGlobalSettings::scheduleTurnOnDNDTimeChanged, this, [&](QTime value){
        qDebug() << "scheduleTurnOnDNDTimeChanged" << value;
    });
    connect(m_globalSettings, &UkuiNotification::NotificationGlobalSettings::scheduleTurnOffDNDTimeChanged, this, [&](QTime value){
        qDebug() << "scheduleTurnOffDNDTimeChanged" << value;
    });
    connect(m_globalSettings, &UkuiNotification::NotificationGlobalSettings::DNDWhileMultiScreenChanged, this, [&](bool value){
        qDebug() << "DNDWhileMultiScreenChanged" << value;
    });
    connect(m_globalSettings, &UkuiNotification::NotificationGlobalSettings::DNDWhileFullScreenChanged, this, [&](bool value){
        qDebug() << "DNDWhileFullScreenChanged" << value;
    });
    connect(m_globalSettings, &UkuiNotification::NotificationGlobalSettings::notifyAlarmWhileDNDChanged, this, [&](bool value){
        qDebug() << "notifyAlarmWhileDNDChanged" << value;
    });
    connect(m_globalSettings, &UkuiNotification::NotificationGlobalSettings::receiveNotificationsFromAppsChanged, this, [&](bool value){
        qDebug() << "receiveNotificationsFromAppsChanged" << value;
    });
}

NotificationSettingsTest::~NotificationSettingsTest()
{

}

void NotificationSettingsTest::initConnection(UkuiNotification::SingleApplicationSettings *setting)
{
    connect(setting, &UkuiNotification::SingleApplicationSettings::allowNotifyChanged, this, [&](bool value){
        qDebug() << "allowNotifyChanged" << value;
    });
    connect(setting, &UkuiNotification::SingleApplicationSettings::allowSoundChanged, this, [&](bool value){
        qDebug() << "allowSoundChanged" << value;
    });
    connect(setting, &UkuiNotification::SingleApplicationSettings::showContentOnLockScreenChanged, this, [&](bool value){
        qDebug() << "showContentOnLockScreenChanged" << value;
    });
    connect(setting, &UkuiNotification::SingleApplicationSettings::showNotificationOnLockScreenChanged, this, [&](bool value){
        qDebug() << "showNotificationOnLockScreenChanged" << value;
    });
    connect(setting, &UkuiNotification::SingleApplicationSettings::popupStyleChanged, this, [&](UkuiNotification::SettingsProperty::Property value){
        qDebug() << "popupStyleChanged" << value;
    });
    connect(setting, &UkuiNotification::SingleApplicationSettings::uninstalled, this, [&](){
        qDebug() << "uninstalled";
    });
}

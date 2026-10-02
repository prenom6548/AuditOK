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

#ifndef UKUI_NOTIFICATION_NOTIFICATION_GLOBAL_SETTINGS_H
#define UKUI_NOTIFICATION_NOTIFICATION_GLOBAL_SETTINGS_H
#include <QObject>
#include <QTime>
#include "settings-properties.h"
namespace UkuiNotification {
class NotificationGlobalSettingsPrivate;
/**
 * @short 通知的全局设置.
 * @author iaom
 */
class NotificationGlobalSettings : public QObject
{
    Q_OBJECT
public:
    enum DNDSource {
        None,
        Auto,
        Manual
    };
    Q_ENUM(DNDSource)
    Q_PROPERTY(bool scheduleTurnOnDND READ scheduleTurnOnDND WRITE setScheduleTurnOnDND NOTIFY scheduleTurnOnDNDChanged)
    Q_PROPERTY(QTime scheduleTurnOnDNDTime READ scheduleTurnOnDNDTime WRITE setScheduleTurnOnDNDTime NOTIFY scheduleTurnOnDNDTimeChanged)
    Q_PROPERTY(QTime scheduleTurnOffDNDTime READ scheduleTurnOffDNDTime WRITE setScheduleTurnOffDNDTime NOTIFY scheduleTurnOffDNDTimeChanged)
    Q_PROPERTY(bool DNDWhileMultiScreen READ DNDWhileMultiScreen WRITE setDNDWhileMultiScreen NOTIFY DNDWhileMultiScreenChanged)
    Q_PROPERTY(bool DNDWhileFullScreen READ DNDWhileFullScreen WRITE setDNDWhileFullScreen NOTIFY DNDWhileFullScreenChanged)
    Q_PROPERTY(bool notifyAlarmWhileDND READ notifyAlarmWhileDND WRITE setNotifyAlarmWhileDND NOTIFY notifyAlarmWhileDNDChanged)
    Q_PROPERTY(bool receiveNotificationsFromApps READ receiveNotificationsFromApps WRITE setReceiveNotificationsFromApps NOTIFY receiveNotificationsFromAppsChanged)
    Q_PROPERTY(bool DND READ isDND NOTIFY dndChanged)

public:
    NotificationGlobalSettings(QObject *parent = nullptr);
    ~NotificationGlobalSettings();
    /**
     * 自动开启勿扰模式
     * @return true 自动开启 false 关闭
     */
    bool scheduleTurnOnDND();
    void setScheduleTurnOnDND(bool enable);
    /**
     * 自动开启勿扰模式时间
     * @return
     */
    QTime scheduleTurnOnDNDTime();
    void setScheduleTurnOnDNDTime(const QTime &time);
    /**
     * 自动关闭勿扰模式时间
     * @return
     */
    QTime scheduleTurnOffDNDTime();
    void setScheduleTurnOffDNDTime(const QTime &time);
    /**
     * 多屏连接时自动开启勿扰模式
     * @return
     */
    bool DNDWhileMultiScreen();
    void setDNDWhileMultiScreen(bool enable);
    /**
     * 全屏模式下自动开启勿扰模式
     * @return
     */
    bool DNDWhileFullScreen();
    void setDNDWhileFullScreen(bool enable);
    /**
     * 勿扰模式下允许闹钟提示
     * @return
     */
    bool notifyAlarmWhileDND();
    void setNotifyAlarmWhileDND(bool enable);
    /**
     * 获取来自应用的通知
     * @return
     */
    bool receiveNotificationsFromApps();
    void setReceiveNotificationsFromApps(bool enable);

    /**
     * 当前是否处于勿扰模式
     * @return
     */
    bool isDND();
    void setDND(bool enable);
Q_SIGNALS:
    void scheduleTurnOnDNDChanged(bool);
    void scheduleTurnOnDNDTimeChanged(QTime);
    void scheduleTurnOffDNDTimeChanged(QTime);
    void DNDWhileMultiScreenChanged(bool);
    void DNDWhileFullScreenChanged(bool);
    void notifyAlarmWhileDNDChanged(bool);
    void receiveNotificationsFromAppsChanged(bool);
    void dndChanged(bool);

private Q_SLOTS:
    void settingsDataChanged();
    void screenModeChanged(int mode);
    void checkDNDMode();
    void checkTimeForDND();
    void updateDNDTime();
    bool manualDND();
    bool autoDND();
private:
    NotificationGlobalSettingsPrivate *d = nullptr;
};

} // UkuiNotification

#endif //UKUI_NOTIFICATION_NOTIFICATION_GLOBAL_SETTINGS_H

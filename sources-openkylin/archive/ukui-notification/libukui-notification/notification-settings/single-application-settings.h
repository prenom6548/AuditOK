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

#ifndef UKUI_NOTIFICATION_SINGLE_APPLICATION_SETTINGS_H
#define UKUI_NOTIFICATION_SINGLE_APPLICATION_SETTINGS_H
#include <QObject>
#include "settings-properties.h"
#include <QJsonObject>
namespace UkuiNotification {
class SingleApplicationSettingsPrivate;
/**
 * @short 表示单个应用的设置通知设置.
 * @author iaom
 */
class SingleApplicationSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool allowNotify READ allowNotify WRITE setAllowNotify NOTIFY allowNotifyChanged)
    Q_PROPERTY(bool allowSound READ allowSound WRITE setAllowSound NOTIFY allowSoundChanged)
    Q_PROPERTY(bool showContentOnLockScreen READ showContentOnLockScreen WRITE setShowContentOnLockScreen NOTIFY showContentOnLockScreenChanged)
    Q_PROPERTY(bool showNotificationOnLockScreen READ showNotificationOnLockScreen WRITE setShowNotificationOnLockScreen NOTIFY showNotificationOnLockScreenChanged)
//    Q_PROPERTY(SettingsProperty::Property PopupStyle READ popupStyle WRITE setPopupStyle NOTIFY popupStyleChanged)
public:
    explicit SingleApplicationSettings(const QString &desktopEntry = QString(), QObject *parent = nullptr);
    SingleApplicationSettings(const SingleApplicationSettings &other);
    SingleApplicationSettings &operator=(const SingleApplicationSettings &other);
    SingleApplicationSettings &operator=(SingleApplicationSettings &&other) Q_DECL_NOEXCEPT;
    ~SingleApplicationSettings();

    bool allowNotify() const;
    void setAllowNotify(bool enable);

    bool allowSound() const ;
    void setAllowSound(bool enable);

    bool showContentOnLockScreen() const ;
    void setShowContentOnLockScreen(bool enable);

    bool showNotificationOnLockScreen() const;
    void setShowNotificationOnLockScreen(bool enable);

    bool isNotificationSwitchEditable() const;
    bool isShowNotificationSwitchOnLockScreenEditable() const;

//    SettingsProperty::Property popupStyle() const;
//    void setPopupStyle(SettingsProperty::Property style);

    QString desktopEntry() const;

Q_SIGNALS:
    void allowNotifyChanged(bool);
    void allowSoundChanged(bool);
    void showContentOnLockScreenChanged(bool);
    void showNotificationOnLockScreenChanged(bool);
    void popupStyleChanged(SettingsProperty::Property);
    void uninstalled();

private Q_SLOTS:
    void init();
    void settingsDataChanged(const QString& desktopEntry);
    void appUninstalled(const QString &desktopEntry);

private:
    SingleApplicationSettingsPrivate *d = nullptr;
};
}

#endif //UKUI_NOTIFICATION_SINGLE_APPLICATION_SETTINGS_H

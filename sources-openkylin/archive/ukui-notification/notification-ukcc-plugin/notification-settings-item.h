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

#ifndef NOTIFICATION_SETTINGS_ITEM_H
#define NOTIFICATION_SETTINGS_ITEM_H

#include <QFrame>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include "notice-menu.h"
#include "single-application-settings.h"

using namespace UkuiNotification;

class NotificationSettingsItem : public QFrame {
    Q_OBJECT
public:
    explicit NotificationSettingsItem(const QString &localName, const QString &nameEn, SingleApplicationSettings* settings, QWidget *parent = nullptr);
    ~NotificationSettingsItem() override = default;
    void setIcon(const QString &icon);
public Q_SLOTS:
    void updateState() const;

private:
    QLabel *m_iconLabel = nullptr;
    QLabel *m_nameLabel = nullptr;
    QHBoxLayout *m_horLayout = nullptr;
    QToolButton *m_menuButton = nullptr;
    NoticeMenu *m_menu = nullptr;
    KSwitchButton *m_notificationSwitch = nullptr;
    QPointer<SingleApplicationSettings> m_settings;
};


#endif //NOTIFICATION_SETTINGS_ITEM_H

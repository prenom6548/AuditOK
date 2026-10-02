/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 */

#ifndef TEST_APPLICATIONS_SETTINGS_H
#define TEST_APPLICATIONS_SETTINGS_H

#include <QObject>

#include "popup-notification.h"

namespace UkuiNotification {

class SingleApplicationSettings : public QObject
{
    Q_OBJECT
public:
    explicit SingleApplicationSettings(QObject *parent = nullptr);

    bool allowNotify() const;
    bool allowSound() const;
    bool showNotificationOnLockScreen() const;
    bool showContentOnLockScreen() const;

    void setAllowNotify(bool value);
    void setAllowSound(bool value);
    void setShowNotificationOnLockScreen(bool value);
    void setShowContentOnLockScreen(bool value);

private:
    bool m_allowNotify {true};
    bool m_allowSound {false};
    bool m_showOnLockScreen {true};
    bool m_showContentOnLockScreen {true};
};

class ApplicationsSettings
{
public:
    static ApplicationsSettings *self();
    bool isInAllowList(const QString &) const;
    SingleApplicationSettings *creatSettings(const PopupNotification &notification);
    static SingleApplicationSettings *settings();
};

} // namespace UkuiNotification

#endif

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

#ifndef UKUI_NOTIFICATION_NOTIFICATION_SETTINGS_TEST_H
#define UKUI_NOTIFICATION_NOTIFICATION_SETTINGS_TEST_H
#include <QObject>
#include "applications-settings.h"
#include "notification-global-settings.h"

class NotificationSettingsTest : public QObject
{
Q_OBJECT
public:
    explicit NotificationSettingsTest(QObject *parent = nullptr);
    ~NotificationSettingsTest();

    void initConnection(UkuiNotification::SingleApplicationSettings *setting);
private:
    UkuiNotification::NotificationGlobalSettings *m_globalSettings = nullptr;
};


#endif //UKUI_NOTIFICATION_NOTIFICATION_SETTINGS_TEST_H

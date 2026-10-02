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

#ifndef UKUI_NOTIFICATION_NOTIFICATION_CLIENT_TEST_H
#define UKUI_NOTIFICATION_NOTIFICATION_CLIENT_TEST_H
#include "notification-client.h"
#include <QObject>

class NotificationClientTest : public QObject
{
    Q_OBJECT
public:
    explicit NotificationClientTest(QObject *parent = nullptr);
    ~NotificationClientTest();
private:
    UkuiNotification::NotificationClient *m_client = nullptr;
};


#endif //UKUI_NOTIFICATION_NOTIFICATION_CLIENT_TEST_H

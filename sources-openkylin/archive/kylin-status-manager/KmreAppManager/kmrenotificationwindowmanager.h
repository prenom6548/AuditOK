/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 */

#ifndef KMRENOTIFICATIONWINDOWMANAGER_H
#define KMRENOTIFICATIONWINDOWMANAGER_H

#include <QObject>

class QDBusPendingCallWatcher;
class KmreNotificationWindowManager : public QObject
{
    Q_OBJECT
public:
    explicit KmreNotificationWindowManager(QObject *parent = nullptr);
    void show(QString notificationText);

private:
    void connectNotificationWindowOperationSignal();
    void disconnectNotificationWindowSignal();
    void setNotificationWindowContent(const QString &notificationContent);

private:
    uint m_notificationWindwoId = 0;
    bool m_switchModeToTablet;

signals:
    void canSwitchMode(bool canModeSwicth);

public slots:
    void notificationWindowOperationHandle(uint id, const QString &value);
    void notificationWindowClosed(uint id, uint type);
    void saveRepliedNotificationWindowId(QDBusPendingCallWatcher *watcher);
};

#endif // KMRENOTIFICATIONWINDOWMANAGER_H

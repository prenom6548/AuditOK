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

#ifndef AUTOTEST_APPLICATION_FAKE_KEEP_ALIVE_CLIENT_H
#define AUTOTEST_APPLICATION_FAKE_KEEP_ALIVE_CLIENT_H

#include <QObject>
#include <QString>

namespace UkuiQuick::HeartBeat {

class KeepAliveClient : public QObject
{
public:
    explicit KeepAliveClient(const QString &appName, QObject *parent = nullptr)
        : QObject(parent), m_appName(appName)
    {
    }

    bool start()
    {
        ++m_startCount;
        m_started = true;
        m_registered = true;
        return true;
    }

    void stop()
    {
        m_started = false;
        m_registered = false;
    }

    bool isStarted() const { return m_started; }
    bool isRegistered() const { return m_registered; }
    void sendStatusResponse() const {}

    QString appName() const { return m_appName; }
    int startCount() const { return m_startCount; }

private:
    QString m_appName;
    int m_startCount {0};
    bool m_started {false};
    bool m_registered {false};
};

}

#endif

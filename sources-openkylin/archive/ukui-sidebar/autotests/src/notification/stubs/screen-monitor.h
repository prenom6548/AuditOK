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

#ifndef TEST_SCREEN_MONITOR_H
#define TEST_SCREEN_MONITOR_H

#include <QObject>

namespace Sidebar {

class ScreenMonitor : public QObject
{
    Q_OBJECT
public:
    static ScreenMonitor *getInstance();
    bool getScreenLockState() const;
    bool getSessionToolsState() const;
    void setScreenLockState(bool state);
    void setSessionToolsState(bool state);

Q_SIGNALS:
    void screenLockStateChanged(bool state);
    void sessionToolsStateChanged(bool state);

private:
    explicit ScreenMonitor(QObject *parent = nullptr);
    bool m_screenLockState {false};
    bool m_sessionToolsState {false};
};

} // namespace Sidebar

#endif

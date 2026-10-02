/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

/*
 * This file is part of the pipecontrol project.
 * Copyright (c) 2022 Matteo De Carlo.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
//#include <QString>
//#include <QtDBus/QtDBus>

#include <string>

namespace UkuiAudioFramwork
{
class SystemdService
{
    void runningChanged(bool);

private:
    std::string serviceName;
//    QDBusConnection bus;
//    QDBusInterface *systemd = nullptr;
    bool isRunning = false;

public:
    SystemdService(const std::string& serviceName, bool userService = true);
    ~SystemdService();

    bool running() { return isRunning; }
    void setRunning(bool);

    bool checkIsRunning();
//    Q_INVOKABLE void start();
//    Q_INVOKABLE void stop();
//    Q_INVOKABLE void restart();

    void start();
    void stop();
    void restart();

private:
    // Sets variable, emits signal but does not communicate with systemd
    void _setRunning(bool);
};
}

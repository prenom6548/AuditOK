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

#ifndef AUTOTESTS_UTILS_STUB_APP_MANAGER_H
#define AUTOTESTS_UTILS_STUB_APP_MANAGER_H

#include <QObject>
#include <QString>

namespace Sidebar {

class AppManager : public QObject
{
public:
    explicit AppManager(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    static AppManager *getInstance(QObject *parent = nullptr)
    {
        Q_UNUSED(parent)
        static AppManager manager;
        return &manager;
    }

    static void resetTestData()
    {
        s_launchCount = 0;
        s_lastDesktopFile.clear();
        s_lastApplicationName.clear();
    }

    static int launchCount()
    {
        return s_launchCount;
    }

    static QString lastDesktopFile()
    {
        return s_lastDesktopFile;
    }

    static QString lastApplicationName()
    {
        return s_lastApplicationName;
    }

    void launchApp(const QString &desktopFile, const QString &applicationName)
    {
        ++s_launchCount;
        s_lastDesktopFile = desktopFile;
        s_lastApplicationName = applicationName;
    }

private:
    inline static int s_launchCount = 0;
    inline static QString s_lastDesktopFile;
    inline static QString s_lastApplicationName;
};

} // namespace Sidebar

#endif

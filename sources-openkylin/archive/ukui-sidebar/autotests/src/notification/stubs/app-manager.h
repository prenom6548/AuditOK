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

#ifndef TEST_APP_MANAGER_H
#define TEST_APP_MANAGER_H

#include <QObject>
#include <QStringList>

namespace Sidebar {

class AppManager : public QObject
{
    Q_OBJECT
public:
    static AppManager *getInstance(QObject *parent = nullptr);
    bool launchAppWithArguments(const QString &desktopFile, const QStringList &args, const QString &appName);

    static void reset();
    static QString lastDesktopFile();
    static QStringList lastArgs();
    static QString lastAppName();

private:
    explicit AppManager(QObject *parent = nullptr);
};

} // namespace Sidebar

#endif

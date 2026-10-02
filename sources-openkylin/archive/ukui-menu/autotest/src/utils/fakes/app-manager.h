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

#ifndef AUTOTEST_UTILS_FAKE_APP_MANAGER_H
#define AUTOTEST_UTILS_FAKE_APP_MANAGER_H

#include <QStringList>

namespace UkuiMenu {

class AppManager
{
public:
    static AppManager *instance();

    void launchApp(const QString &appid);
    void launchBinaryApp(const QString &app, const QString &args = QString());
    void launchAppWithArguments(const QString &appid, const QStringList &args, const QString &applicationName);
    void changeFavoriteState(const QString &appid, bool isFavorite);

    static QStringList parseDesktopFile(const QString &desktopFilePath);
};

} // UkuiMenu

#endif

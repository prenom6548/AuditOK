/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

//
// Created by hxf on 22-10-11.
//

#ifndef UKUI_SIDEBAR_APP_MANAGER_H
#define UKUI_SIDEBAR_APP_MANAGER_H

#include <QObject>

class QDBusInterface;

namespace Sidebar {

class AppManager : public QObject
{
    Q_OBJECT
public:
    static AppManager *getInstance(QObject *parent = nullptr);
    ~AppManager() override;

    Q_INVOKABLE void launchApp(const QString &desktopFile, const QString &applicationName);
    Q_INVOKABLE void launchAppWithArguments(const QString &desktopFile, const QStringList &args, const QString &applicationName);

private:
    explicit AppManager(QObject *parent);

private:
    QDBusInterface *m_appManagerDbusInterface = nullptr;
};

} // Sidebar

#endif //UKUI_SIDEBAR_APP_MANAGER_H

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

#ifndef AUTOTEST_FAKE_SETTINGS_H
#define AUTOTEST_FAKE_SETTINGS_H

#include <QObject>
#include <QStringList>

namespace UkuiMenu {

class GlobalSetting : public QObject
{
    Q_OBJECT
public:
    static GlobalSetting *instance();

    bool isSystemApp(const QString &appid) const;
    QStringList defaultFavoriteApps() const;

    void setSystemApps(const QStringList &apps);
    void setDefaultFavoriteApps(const QStringList &apps);

private:
    explicit GlobalSetting(QObject *parent = nullptr);

    QStringList m_systemApps;
    QStringList m_defaultFavoriteApps;
};

}

#endif

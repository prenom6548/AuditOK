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

#ifndef AUTOTEST_APPLICATION_FAKE_SETTINGS_H
#define AUTOTEST_APPLICATION_FAKE_SETTINGS_H

#include <QObject>
#include <QMap>
#include <QVariant>

#define FOLLOW_UKUI_PANEL            "followUkuiPanel"
#define MENU_WIDTH                   "width"
#define MENU_HEIGHT                  "height"
#define MENU_MARGIN                  "margin"

namespace UkuiMenu {

class SettingModule
{
public:
    static void defineModule(const char *uri, int versionMajor, int versionMinor);
};

class MenuSetting : public QObject
{
    Q_OBJECT
public:
    static MenuSetting *instance();

    QVariant get(const QString &key) const;
    bool set(const QString &key, const QVariant &value);
    void reset();

Q_SIGNALS:
    void changed(const QString &key);

private:
    explicit MenuSetting(QObject *parent = nullptr);

    QMap<QString, QVariant> m_cache;
};

} // namespace UkuiMenu

#endif

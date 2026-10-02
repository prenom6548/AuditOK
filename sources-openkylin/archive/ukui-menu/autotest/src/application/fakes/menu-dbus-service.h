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

#ifndef AUTOTEST_APPLICATION_FAKE_MENU_DBUS_SERVICE_H
#define AUTOTEST_APPLICATION_FAKE_MENU_DBUS_SERVICE_H

#include <QObject>

namespace UkuiMenu {

class MenuDbusService : public QObject
{
    Q_OBJECT
public:
    explicit MenuDbusService(const QString &display, QObject *parent = nullptr);

    bool visible() const;
    void setVisible(bool visible);
    QString display() const;
    int visibleSetCount() const;

Q_SIGNALS:
    void activeRequest(QString display);
    void menuActive();
    void reloadConfigSignal();

private:
    QString m_display;
    bool m_visible = false;
    int m_visibleSetCount = 0;
};

} // namespace UkuiMenu

#endif

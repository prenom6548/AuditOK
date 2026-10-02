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

#ifndef AUTOTESTS_WINDOWS_STUB_SIDEBAR_WINDOW_HELPER_H
#define AUTOTESTS_WINDOWS_STUB_SIDEBAR_WINDOW_HELPER_H

#include <QObject>
#include <QRect>

#include "window-type.h"

namespace Sidebar {

class SidebarWindowHelper : public QObject
{
    Q_OBJECT
public:
    static SidebarWindowHelper *instance(QObject *parent = nullptr);

    QRect getWindowGeometry(SidebarWindowType::Type type);
    Qt::LayoutDirection getLayoutDirection();
    int getPanelPos();

    void setWindowGeometry(SidebarWindowType::Type type, const QRect &geometry);
    void setLayoutDirection(Qt::LayoutDirection direction);
    void reset();

Q_SIGNALS:
    void geometryChanged();

private:
    explicit SidebarWindowHelper(QObject *parent = nullptr);

    QMap<int, QRect> m_geometries;
    Qt::LayoutDirection m_direction = Qt::LeftToRight;
};

class SidebarWindowDefineModule
{
public:
    static void defineModules(const char *uri, int versionMajor, int versionMinor)
    {
        Q_UNUSED(uri)
        Q_UNUSED(versionMajor)
        Q_UNUSED(versionMinor)
    }
};

} // namespace Sidebar

#endif

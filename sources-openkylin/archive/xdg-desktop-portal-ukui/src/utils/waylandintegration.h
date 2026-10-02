/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef WAYLANDINTEGRATION_H
#define WAYLANDINTEGRATION_H

#include <QObject>
#include <QWindow>
#include <KWayland/Client/registry.h>

class WaylandIntegration : public QObject
{
    Q_OBJECT
public:
    explicit WaylandIntegration(QObject *parent = nullptr);
    void setParentWindow(QWindow *window, const QString &parentHandle);

private:
    void initWayland();

private:
    KWayland::Client::Registry *m_registry{nullptr};
    KWayland::Client::XdgImporter *m_xdgImporter{nullptr};
};

#endif // WAYLANDINTEGRATION_H

/*
 * Peony-Qt's Library
 *
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

#include "windowutils.h"

#include <QWidget>
#include <QWindow>
#include <KWindowSystem>
#include "waylandintegration.h"

void WindowUtils::setParentWindow(QWidget *widget, const QString &parentWindow)
{
    if (parentWindow.startsWith(QLatin1String("x11:"))) {
        widget->setAttribute(Qt::WA_NativeWindow, true);
        KWindowSystem::setMainWindow(widget->windowHandle(), parentWindow.midRef(4).toULongLong(nullptr, 16));
    }

    if (parentWindow.startsWith((QLatin1String("wayland:")))) {
        static WaylandIntegration waylandIntegration;
        waylandIntegration.setParentWindow(widget->windowHandle(), parentWindow.mid(strlen("wayland:")));
    }
}

WindowUtils::WindowUtils()
{

}

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

#ifndef AUTOTEST_WIDGET_CONTAINER_ITEM_H
#define AUTOTEST_WIDGET_CONTAINER_ITEM_H

#include <QQuickItem>

#include "widget-container.h"

namespace UkuiQuick {

class WidgetQuickItem : public QQuickItem
{
    Q_OBJECT
public:
    explicit WidgetQuickItem(QQuickItem *parent = nullptr);

    static WidgetQuickItem *loadWidgetItem(Widget *widget, QQmlContext *context = nullptr);
};

class WidgetContainerItem : public WidgetQuickItem
{
    Q_OBJECT
public:
    explicit WidgetContainerItem(QQuickItem *parent = nullptr);
};

} // namespace UkuiQuick

#endif // AUTOTEST_WIDGET_CONTAINER_ITEM_H

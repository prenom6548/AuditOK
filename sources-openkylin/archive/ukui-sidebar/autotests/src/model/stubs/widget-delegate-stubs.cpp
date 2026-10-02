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

#include "widget-container-item.h"
#include "shared-engine-component.h"

#include <QQmlContext>

namespace UkuiQuick {

static WidgetLoaderStubState s_state;
static QQmlEngine *s_engine = nullptr;

WidgetLoaderStubState &widgetLoaderStubState()
{
    return s_state;
}

void resetWidgetLoaderStubState()
{
    s_state = WidgetLoaderStubState();
}

Widget::Widget(const QString &id, QObject *parent)
    : QObject(parent)
    , m_id(id)
{
}

QString Widget::id() const
{
    return m_id;
}

WidgetContainer::WidgetContainer(QObject *parent)
    : Widget(QStringLiteral("org.ukui.shortcutContainer"), parent)
{
}

void WidgetContainer::addWidget(Widget *widget)
{
    if (widget) {
        widget->setParent(this);
        m_widgets.append(widget);
    }
}

void WidgetContainer::removeWidget(Widget *widget)
{
    m_widgets.removeOne(widget);
}

QVector<Widget *> WidgetContainer::widgets() const
{
    return m_widgets;
}

WidgetQuickItem::WidgetQuickItem(QQuickItem *parent)
    : QQuickItem(parent)
{
}

WidgetQuickItem *WidgetQuickItem::loadWidgetItem(Widget *widget, QQmlContext *context)
{
    Q_UNUSED(widget)
    delete context;
    if (!s_state.containerItemLoadSucceeds) {
        return nullptr;
    }

    auto *item = new WidgetContainerItem;
    item->setSize(QSizeF(64, 32));
    return item;
}

WidgetContainerItem::WidgetContainerItem(QQuickItem *parent)
    : WidgetQuickItem(parent)
{
    auto *child = new QQuickItem(this);
    child->setWidth(64);
    child->setHeight(32);
}

WidgetLoader::WidgetLoader(QObject *parent)
    : QObject(parent)
{
}

Widget *WidgetLoader::loadWidget(const QString &id)
{
    s_state.loadCalls++;
    s_state.loadedIds.append(id);

    if (id == QStringLiteral("org.ukui.shortcutContainer")) {
        if (!s_state.containerLoadSucceeds) {
            return new Widget(id);
        }
        return new WidgetContainer;
    }

    if (!s_state.widgetLoadSucceeds) {
        return nullptr;
    }
    return new Widget(id);
}

void WidgetLoader::addWidgetSearchPath(const QString &path)
{
    s_state.addPathCalls++;
    s_state.searchPaths.append(path);
}

void WidgetLoader::setShowInFilter(WidgetMetadata::Hosts hosts)
{
    s_state.setFilterCalls++;
    s_state.lastHosts = hosts;
}

QQmlEngine *SharedEngineComponent::sharedEngine()
{
    if (!s_engine) {
        s_engine = new QQmlEngine;
    }
    return s_engine;
}

} // namespace UkuiQuick

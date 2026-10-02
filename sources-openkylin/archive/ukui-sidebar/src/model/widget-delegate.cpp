/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "widget-delegate.h"
#include <widget-loader.h>
#include <widget-container.h>
#include <widget-container-item.h>
#include <shared-engine-component.h>
#include <QQmlContext>
namespace UkuiShortcut {
WidgetDelegate::WidgetDelegate(QQuickItem* parent)
{
    m_widgetLoader = new UkuiQuick::WidgetLoader(this);
    m_widgetLoader->addWidgetSearchPath(QStringLiteral(":/shortcut"));
    m_widgetLoader->setShowInFilter(UkuiQuick::WidgetMetadata::Host::Shortcut);
    loadContainer();
}

WidgetDelegate::~WidgetDelegate()
{
    if(m_container) {
        Q_EMIT m_container->aboutToDeleted();
        m_container->deleteLater();
        m_container = nullptr;
    }
}

QString WidgetDelegate::quickWidgetId() const
{
    return m_quickWidgetId;
}

void WidgetDelegate::setQuickWidgetId(const QString& id)
{
    if(m_quickWidgetId != id) {
        m_quickWidgetId = id;
        Q_EMIT quickWidgetIdChanged();
    }
    loadWidget();
}

qreal WidgetDelegate::preferredWidth() const
{
    return m_preferredWidth;
}

qreal WidgetDelegate::preferredHeight() const
{
    return m_preferredHeight;
}

void WidgetDelegate::loadContainer()
{
    auto widget = m_widgetLoader->loadWidget(QStringLiteral("org.ukui.shortcutContainer"));
    auto container = qobject_cast<UkuiQuick::WidgetContainer *>(widget);
    if (!container) {
        qWarning() << "Load shortcutContainer failed!";
        delete widget;
        return;
    }
    m_container = container;
    auto contItem = qobject_cast<UkuiQuick::WidgetContainerItem *>(UkuiQuick::WidgetQuickItem::loadWidgetItem(m_container, new QQmlContext(UkuiQuick::SharedEngineComponent::sharedEngine(), this)));
    if (!contItem) {
        qWarning() << "Load taskManagerContainerItem failed!";
        return;
    }
    contItem->setParentItem(this);
    m_preferredWidth = contItem->childrenRect().width();
    m_preferredHeight = contItem->childrenRect().height();
    setSize(contItem->childrenRect().size());
    connect(contItem, &QQuickItem::childrenRectChanged, this, [&, contItem]() {
        setSize(contItem->childrenRect().size());
        m_preferredWidth = contItem->childrenRect().width();
        m_preferredHeight = contItem->childrenRect().height();
        Q_EMIT preferredWidthChanged();
        Q_EMIT preferredHeightChanged();
    });
}

void WidgetDelegate::loadWidget()
{
    if(m_widget) {
        m_container->removeWidget(m_widget);
    }
    m_widget = m_widgetLoader->loadWidget(m_quickWidgetId);
    if (m_widget) qDebug() << "Loading widget:" <<  m_widget->id();
    m_container->addWidget(m_widget);
}
}

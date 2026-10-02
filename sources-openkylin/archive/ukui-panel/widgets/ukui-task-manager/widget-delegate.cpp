/*
*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "widget-delegate.h"

#include <QFileInfo>
#include <QGuiApplication>
#include <QtConcurrentRun>
#include <QMutexLocker>
#include <QQmlContext>
#include <widget-container-item.h>
#include <widget-container.h>
#include <widget-item.h>
#include <widget.h>
#include <widget-loader.h>
#include <shared-engine-component.h>

using namespace TaskManager;
static std::once_flag flag;
static WidgetInfo *widgetInfo = nullptr;
int WidgetUpdateEvent::m_eventType = QEvent::registerEventType();
int WidgetUpdateEvent::eventType()
{
    return m_eventType;
}

WidgetInfo::WidgetInfo(QObject* parent)
{
    m_widgetLoader = new UkuiQuick::WidgetLoader(this);
    m_widgetLoader->addWidgetSearchPath(QStringLiteral(":/task-manager"));
    m_widgetLoader->setShowInFilter( UkuiQuick::WidgetMetadata::Host::TaskManager);
}

WidgetInfo* WidgetInfo::self()
{
    std::call_once(flag, [ & ] {
        widgetInfo = new WidgetInfo();
    });
    return widgetInfo;
}

bool WidgetInfo::hasWidgetFor(const QString& appId)
{
    QMutexLocker locker(&m_mutex);
    return !m_widgetLoader->widgets(appId).isEmpty();
}

QString WidgetInfo::widgetFor(const QString& appId)
{
    if (appId.isEmpty()) {
        return {};
    }

    if(m_quickWidgets.contains(appId)) {
        return m_quickWidgets.value(appId);
    }
    QtConcurrent::run([this, appId]() {
        QMutexLocker locker(&m_mutex);
        QStringList widgets = m_widgetLoader->widgets(appId);

        if(!widgets.isEmpty()) {
            QGuiApplication::postEvent(this, new WidgetUpdateEvent(appId, widgets.first()));
        }
    });
    return {};
}

UkuiQuick::Widget* WidgetInfo::loadWidget(const QString& id)
{
    QMutexLocker locker(&m_mutex);
    return m_widgetLoader->loadWidget(id);
}

void WidgetInfo::customEvent(QEvent* event)
{
    if(event->type() == WidgetUpdateEvent::eventType()) {
        auto e = dynamic_cast<WidgetUpdateEvent*>(event);
        m_quickWidgets.insert(e->appId(), e->widgetId());
        Q_EMIT widgetInfoUpdate(e->appId(), e->widgetId());
    }
    QObject::customEvent(event);
}

WidgetDelegate::WidgetDelegate(QQuickItem* parent) : QQuickItem(parent)
{
    loadContainer();
    connect(this, &QQuickItem::childrenRectChanged, this, [this]() {
        Q_EMIT preferredWidthChanged();
    });
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

int WidgetDelegate::preferredWidth()
{
    return m_preferredWidth;
}

void WidgetDelegate::loadContainer()
{
    auto widget = WidgetInfo::self()->loadWidget(QStringLiteral("org.ukui.taskManagerContainer"));
    auto container = qobject_cast<UkuiQuick::WidgetContainer *>(widget);
    if (!container) {
        qWarning() << "Load taskManagerContainer failed!";
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
    connect(contItem, &QQuickItem::childrenRectChanged, this, [&, contItem]() {
        m_preferredWidth = contItem->childrenRect().width();
        Q_EMIT preferredWidthChanged();
    });
}

void WidgetDelegate::loadWidget()
{
    if(m_widget) {
        m_container->removeWidget(m_widget);
    }
    m_widget = WidgetInfo::self()->loadWidget(m_quickWidgetId);
    m_container->addWidget(m_widget);
}

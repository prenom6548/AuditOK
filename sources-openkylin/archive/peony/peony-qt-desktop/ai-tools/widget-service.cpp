/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "widget-service.h"

#include "desktop-widgets/base/desktop-widgets-manager.h"
#include "platformwindow.h"

#include <QSet>

#include <widget-loader.h>
#include <widget.h>

namespace PeonyDesktop {

// 与 WidgetManagerDialog::showInWidgetManager 一致：仅 Contents.ShowInManager（或兼容 ManagerVisible）为真时可由 AI/管理器添加
static bool showInWidgetManager(const UkuiQuick::WidgetMetadata &metadata)
{
    const QVariantMap contents = metadata.contents();
    if (contents.contains(QStringLiteral("ShowInManager"))) {
        return contents.value(QStringLiteral("ShowInManager")).toBool();
    }
    if (contents.contains(QStringLiteral("ManagerVisible"))) {
        return contents.value(QStringLiteral("ManagerVisible")).toBool();
    }
    return false;
}

static bool isDesktopWidgetPlugin(const UkuiQuick::WidgetMetadata &metadata)
{
    return (metadata.showIn() & UkuiQuick::WidgetMetadata::Desktop) && showInWidgetManager(metadata);
}

static bool isDesktopWidgetPluginId(const QString &pluginId)
{
    for (const UkuiQuick::WidgetMetadata &meta :
         UkuiQuick::WidgetLoader::globalLoader().widgetsMetadata()) {
        if (meta.id() == pluginId && isDesktopWidgetPlugin(meta)) {
            return true;
        }
    }
    return false;
}

static PlatformWindow *platformWindowForScreen(int screenId)
{
    PlatformWindow *fallback = nullptr;
    for (PlatformWindow *window : PlatformWindow::allWindows()) {
        if (!window) {
            continue;
        }
        if (window->id() == screenId) {
            return window;
        }
        if (!fallback) {
            fallback = window;
        }
        if (window->id() == 0) {
            fallback = window;
        }
    }
    if (screenId < 0) {
        return fallback;
    }
    return nullptr;
}

static QJsonObject widgetJsonFromInstance(UkuiQuick::Widget *widget, PlatformWindow *window)
{
    QJsonObject obj;
    if (!widget) {
        return obj;
    }
    obj.insert(QStringLiteral("instance_id"), widget->instanceId());
    obj.insert(QStringLiteral("plugin_id"), widget->id());
    obj.insert(QStringLiteral("screen_id"), window ? window->id() : -1);
    if (widget->config()) {
        const QString variantId = widget->config()->getValue(QStringLiteral("variantId")).toString();
        if (!variantId.isEmpty()) {
            obj.insert(QStringLiteral("variant_id"), variantId);
        }
    }
    return obj;
}

bool WidgetService::widgetExists(int instanceId)
{
    if (instanceId < 0) {
        return false;
    }
    for (PlatformWindow *window : PlatformWindow::allWindows()) {
        if (!window || !window->mainView()) {
            continue;
        }
        if (window->mainView()->widget(instanceId)) {
            return true;
        }
        auto containerConfig = window->mainView()->config();
        if (containerConfig && containerConfig->child(QStringLiteral("widgets"), instanceId)) {
            return true;
        }
    }
    return false;
}

QJsonArray WidgetService::listAvailableWidgets()
{
    QJsonArray widgets;
    for (const UkuiQuick::WidgetMetadata &meta :
         UkuiQuick::WidgetLoader::globalLoader().widgetsMetadata()) {
        if (!isDesktopWidgetPlugin(meta)) {
            continue;
        }
        QJsonObject obj;
        obj.insert(QStringLiteral("plugin_id"), meta.id());
        QString name = meta.name();
        if (name.isEmpty()) {
            name = meta.contents().value(QStringLiteral("Name")).toString();
        }
        if (!name.isEmpty()) {
            obj.insert(QStringLiteral("name"), name);
        }
        widgets.append(obj);
    }
    return widgets;
}

QJsonArray WidgetService::listWidgets()
{
    QJsonArray widgets;
    for (PlatformWindow *window : PlatformWindow::allWindows()) {
        if (!window || !window->mainView()) {
            continue;
        }
        for (UkuiQuick::Widget *widget : window->mainView()->widgets()) {
            const QJsonObject obj = widgetJsonFromInstance(widget, window);
            if (!obj.isEmpty()) {
                widgets.append(obj);
            }
        }
    }
    return widgets;
}

ToolResponse WidgetService::addWidget(const QString &requestId,
                                      const QString &pluginId,
                                      int screenId,
                                      const QJsonObject &variantJson)
{
    PlatformWindow *window = platformWindowForScreen(screenId);
    if (!window) {
        return ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("NO_DESKTOP_WINDOW"),
                                QStringLiteral("no desktop window for screen"),
                                {{QStringLiteral("screen_id"), screenId}}));
    }

    if (!isDesktopWidgetPluginId(pluginId)) {
        return ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("PLUGIN_NOT_FOUND"),
                                QStringLiteral("plugin_id not found or not available for desktop "
                                               "(requires ShowIn Desktop and ShowInManager true)"),
                                {{QStringLiteral("plugin_id"), pluginId}}));
    }

    QSet<int> instanceIdsBefore;
    for (UkuiQuick::Widget *widget : window->mainView()->widgets()) {
        if (widget) {
            instanceIdsBefore.insert(widget->instanceId());
        }
    }

    const QVariantMap variant = variantJson.toVariantMap();
    if (!window->addWidgetFromVariant(pluginId, variant)) {
        return ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("ADD_WIDGET_FAILED"),
                                QStringLiteral("failed to add widget (no free area or load error)"),
                                {{QStringLiteral("plugin_id"), pluginId}}));
    }

    int instanceId = -1;
    for (UkuiQuick::Widget *widget : window->mainView()->widgets()) {
        if (!widget || widget->id() != pluginId) {
            continue;
        }
        if (!instanceIdsBefore.contains(widget->instanceId())) {
            instanceId = widget->instanceId();
            break;
        }
    }

    for (PlatformWindow *w : PlatformWindow::allWindows()) {
        w->updateDesktopWidgetsUsedRegion();
    }

    QJsonObject data;
    if (instanceId >= 0) {
        data.insert(QStringLiteral("instance_id"), instanceId);
    }
    data.insert(QStringLiteral("plugin_id"), pluginId);
    data.insert(QStringLiteral("screen_id"), window->id());
    data.insert(QStringLiteral("widgets"), listWidgets());
    return ToolResponse::completed(requestId, data);
}

ToolResponse WidgetService::removeWidget(const QString &requestId,
                                         int instanceId,
                                         int screenId)
{
    if (!widgetExists(instanceId)) {
        return ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("WIDGET_NOT_FOUND"),
                                QStringLiteral("desktop widget not found"),
                                {{QStringLiteral("instance_id"), instanceId}}));
    }

    QString pluginId;
    for (PlatformWindow *window : PlatformWindow::allWindows()) {
        if (!window || !window->mainView()) {
            continue;
        }
        UkuiQuick::Widget *widget = window->mainView()->widget(instanceId);
        if (widget) {
            pluginId = widget->id();
            break;
        }
    }

    if (pluginId == QStringLiteral("desktop-organization-tool")) {
        return ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("WIDGET_NOT_REMOVABLE"),
                                QStringLiteral("partition widgets must use desktop.partition.disband"),
                                {{QStringLiteral("instance_id"), instanceId}}));
    }

    PlatformWindow *window = nullptr;
    if (screenId >= 0) {
        window = platformWindowForScreen(screenId);
    }
    DesktopWidgetsManager::getInstance()->removeWidget(instanceId, window);

    for (PlatformWindow *w : PlatformWindow::allWindows()) {
        w->updateDesktopWidgetsUsedRegion();
    }

    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("widgets"), listWidgets());
    return ToolResponse::completed(requestId, data);
}

} // namespace PeonyDesktop

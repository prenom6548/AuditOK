/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "partition-service.h"

#include "common.h"
#include "desktop-item-proxy-model.h"
#include "desktop-window-manager.h"
#include "desktop-widgets/base/desktop-widgets-menu.h"
#include "file-action-controler.h"
#include "file-meta-info.h"
#include "global-settings.h"
#include "advanced-desktop-item-model.h"
#include "peony-desktop-application.h"
#include "desktop-widgets/base/widgets-proxy-model.h"
#include "platformwindow.h"
#include "screen-manager.h"

#include <QQuickItem>
#include <QScreen>
#include <QSet>
#include <QTimer>
#include <QtMath>
#include <widget-item.h>
#include <widget.h>

namespace PeonyDesktop {

using Peony::DesktopItemProxyModel;
using Peony::FileMetaInfo;
using Peony::GlobalSettings;

static DesktopOrganizationManager *orgManager()
{
    return DesktopOrganizationManager::instance();
}

static UkuiQuick::WidgetItem *partitionWidgetItem(int instanceId, PlatformWindow **windowOut = nullptr)
{
    for (auto *window : PlatformWindow::allWindows()) {
        if (!window) {
            continue;
        }
        const QHash<UkuiQuick::WidgetItem *, QRect> widgetRects = window->getCurrentWidgetUsedRects();
        for (auto it = widgetRects.constBegin(); it != widgetRects.constEnd(); ++it) {
            UkuiQuick::WidgetItem *widgetItem = it.key();
            if (!widgetItem || !widgetItem->widget()) {
                continue;
            }
            if (widgetItem->widget()->id() != QStringLiteral("desktop-organization-tool")) {
                continue;
            }
            if (widgetItem->widget()->instanceId() == instanceId) {
                if (windowOut) {
                    *windowOut = window;
                }
                return widgetItem;
            }
        }

        if (!window->mainView()) {
            continue;
        }
        UkuiQuick::Widget *widget = window->mainView()->widget(instanceId);
        if (!widget || widget->id() != QStringLiteral("desktop-organization-tool")) {
            continue;
        }
        if (auto *widgetItem = qobject_cast<UkuiQuick::WidgetItem *>(widget->parent())) {
            if (windowOut) {
                *windowOut = window;
            }
            return widgetItem;
        }
    }
    return nullptr;
}

// 对外坐标与 getCurrentWidgetUsedRects 一致：widgetItem->mapToItem(rootItem)，左上角为原点。
static QPoint partitionWidgetScreenPosition(PlatformWindow *window, UkuiQuick::WidgetItem *widgetItem)
{
    if (window && widgetItem && window->isPlatformWindowHasRootItem()) {
        return window->widgetItemScreenPosition(widgetItem);
    }
    return QPoint();
}

static int partitionWidgetConfigHeight(int instanceId)
{
    for (auto *window : PlatformWindow::allWindows()) {
        if (!window || !window->mainView()) {
            continue;
        }
        UkuiQuick::Widget *widget = window->mainView()->widget(instanceId);
        if (!widget || widget->id() != QStringLiteral("desktop-organization-tool")) {
            continue;
        }
        return widget->config()->getValue(QStringLiteral("height")).toInt();
    }
    return -1;
}

static QJsonObject partitionJsonFromWidget(UkuiQuick::Widget *widget, PlatformWindow *window)
{
    QJsonObject obj;
    if (!widget || widget->id() != QStringLiteral("desktop-organization-tool")) {
        return obj;
    }
    const int instanceId = widget->instanceId();
    auto *widgetItem = qobject_cast<UkuiQuick::WidgetItem *>(widget->parent());
    auto *mgr = orgManager();
    obj.insert(QStringLiteral("instance_id"), instanceId);
    obj.insert(QStringLiteral("screen_id"), window ? window->id() : -1);
    obj.insert(QStringLiteral("name"), widget->config()->getValue(QStringLiteral("name")).toString());
    obj.insert(QStringLiteral("organization_type"),
               widget->config()->getValue(QStringLiteral("organizationType")).toInt());
    obj.insert(QStringLiteral("is_manual"),
               mgr->isManulCreatePartition(instanceId));
    obj.insert(QStringLiteral("is_list_mode"), mgr->getIsListMode(instanceId));
    obj.insert(QStringLiteral("is_expanded"), mgr->isExpand(instanceId));
    obj.insert(QStringLiteral("init_height"), mgr->getInitHeight(instanceId));
    if (window && widgetItem && widgetItem->isVisible()) {
        const QPoint screenPos = partitionWidgetScreenPosition(window, widgetItem);
        obj.insert(QStringLiteral("x"), screenPos.x());
        obj.insert(QStringLiteral("y"), screenPos.y());
    }
    obj.insert(QStringLiteral("uris"),
               QJsonArray::fromStringList(PartitionService::urisForPartition(instanceId)));
    return obj;
}

bool PartitionService::partitionExists(int instanceId)
{
    if (instanceId < 0) {
        return false;
    }
    for (auto *window : PlatformWindow::allWindows()) {
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

QJsonArray PartitionService::listPartitions()
{
    QJsonArray partitions;
    for (auto *window : PlatformWindow::allWindows()) {
        if (!window || !window->mainView()) {
            continue;
        }
        for (UkuiQuick::Widget *widget : window->mainView()->widgets()) {
            QJsonObject obj = partitionJsonFromWidget(widget, window);
            if (!obj.isEmpty()) {
                partitions.append(obj);
            }
        }
    }
    return partitions;
}

QJsonObject PartitionService::getPartition(int instanceId)
{
    for (auto *window : PlatformWindow::allWindows()) {
        if (!window || !window->mainView()) {
            continue;
        }
        UkuiQuick::Widget *widget = window->mainView()->widget(instanceId);
        if (widget && widget->id() == QStringLiteral("desktop-organization-tool")) {
            return partitionJsonFromWidget(widget, window);
        }
    }
    return QJsonObject();
}

QStringList PartitionService::urisForPartition(int instanceId)
{
    QStringList uris;
    auto model = PeonyDesktopApplication::getModel();
    if (!model) {
        return uris;
    }
    for (int i = 0; i < model->rowCount(); ++i) {
        auto modelIndex = model->index(i, 0);
        const QString uri = modelIndex.data(Qt::UserRole).toString();
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (!metaInfo) {
            continue;
        }
        if (metaInfo->getMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID) == instanceId) {
            uris.append(uri);
        }
    }
    return uris;
}

ToolResponse PartitionService::createPartition(const QString &requestId,
                                               const QStringList &uris,
                                               const QString &name)
{
    orgManager()->CreatePartition(uris);

    int targetInstanceId = -1;
    for (auto *window : PlatformWindow::allWindows()) {
        if (!window || !window->mainView()) {
            continue;
        }
        for (UkuiQuick::Widget *widget : window->mainView()->widgets()) {
            if (widget->id() != QStringLiteral("desktop-organization-tool")) {
                continue;
            }
            if (orgManager()->isManulCreatePartition(widget->instanceId())) {
                targetInstanceId = qMax(targetInstanceId, widget->instanceId());
            }
        }
    }

    if (!name.isEmpty() && targetInstanceId >= 0) {
        orgManager()->renameSpace(targetInstanceId, name);
    }

    QJsonObject data;
    if (targetInstanceId >= 0) {
        data.insert(QStringLiteral("instance_id"), targetInstanceId);
    }
    data.insert(QStringLiteral("partitions"), listPartitions());
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::renamePartition(const QString &requestId,
                                               int instanceId,
                                               const QString &name)
{
    orgManager()->renameSpace(instanceId, name);
    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("name"), name);
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::disbandPartition(const QString &requestId, int instanceId)
{
    orgManager()->disbandPartition(instanceId);
    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::addFiles(const QString &requestId,
                                      int instanceId,
                                      const QStringList &uris)
{
    return moveFiles(requestId, -1, instanceId, uris);
}

ToolResponse PartitionService::moveFiles(const QString &requestId,
                                         int fromInstanceId,
                                         int toInstanceId,
                                         const QStringList &uris)
{
    QSet<int> changedInstanceIds;
    if (fromInstanceId >= -1) {
        changedInstanceIds.insert(fromInstanceId);
    }
    changedInstanceIds.insert(toInstanceId);

    const bool moveToDesktop = (toInstanceId == -1);
    const bool targetIsManualPartition = (!moveToDesktop && orgManager()->isManulCreatePartition(toInstanceId));

    for (const QString &uri : uris) {
        auto metaInfo = FileMetaInfo::fromUri(uri);
        if (!metaInfo) {
            continue;
        }

        auto oldPartitionVariant = metaInfo->getMetaInfoVariant(ORIGANIZATION_SPACE_INSTANCEID);
        if (oldPartitionVariant.isValid()) {
            changedInstanceIds.insert(oldPartitionVariant.toInt());
        }

        if (moveToDesktop) {
            metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, -1);
            metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER, -1);
            metaInfo->setMetaInfoInt(ORIGANIZATION_MANUAL_CREATE_PARTITION, 0);
        } else {
            metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID, toInstanceId);
            metaInfo->setMetaInfoInt(ORIGANIZATION_SPACE_INSTANCEID_UNORDER, toInstanceId);
            if (targetIsManualPartition) {
                metaInfo->setMetaInfoInt(ORIGANIZATION_MANUAL_CREATE_PARTITION, 1);
            }
        }
    }

    for (int instanceId : changedInstanceIds) {
        orgManager()->updateOrganizationView(instanceId);
    }
    orgManager()->updateOrganizationView(-1);

    if (!moveToDesktop) {
        auto config = SpacesSettings::globalInstance()->getSpacesSortConfig(toInstanceId);
        const int type = config.value("partitionSortType").toInt();
        const int order = config.value("partitionSortAsc").toInt();
        orgManager()->doSort(toInstanceId, type, order);
    }

    for (auto *window : PlatformWindow::allWindows()) {
        if (window) {
            window->updateView();
        }
    }

    QJsonObject data;
    data.insert(QStringLiteral("from_instance_id"), fromInstanceId);
    data.insert(QStringLiteral("to_instance_id"), toInstanceId);
    data.insert(QStringLiteral("uris"), QJsonArray::fromStringList(uris));
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::sortPartition(const QString &requestId,
                                             int instanceId,
                                             int sortType,
                                             int sortOrder)
{
    orgManager()->doSort(instanceId, sortType, sortOrder);
    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("sort_type"), sortType);
    data.insert(QStringLiteral("sort_order"), sortOrder);
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::setExpanded(const QString &requestId,
                                           int instanceId,
                                           bool expanded,
                                           int initHeight)
{
    int height = initHeight;
    if (!expanded) {
        // 与 QML 一致：收起时传入收起前高度，供 SpacesSettings 记录
        if (height <= 0) {
            height = partitionWidgetConfigHeight(instanceId);
        }
        if (height <= 0) {
            height = orgManager()->getInitHeight(instanceId);
        }
    } else if (height < 0) {
        height = 0;
    }

    orgManager()->expandStatusChanged(instanceId, expanded, height);
    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("expanded"), expanded);
    if (!expanded) {
        data.insert(QStringLiteral("init_height"), height);
    }
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::getExpanded(const QString &requestId, int instanceId)
{
    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("expanded"), orgManager()->isExpand(instanceId));
    data.insert(QStringLiteral("init_height"), orgManager()->getInitHeight(instanceId));
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::setListMode(const QString &requestId,
                                           int instanceId,
                                           bool listMode)
{
    SpacesSettings::globalInstance()->setViewModel(instanceId, listMode);
    orgManager()->updateOrganizationView(instanceId);
    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("list_mode"), listMode);
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::getListMode(const QString &requestId, int instanceId)
{
    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("list_mode"), orgManager()->getIsListMode(instanceId));
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::updateView(const QString &requestId, int instanceId)
{
    orgManager()->updateOrganizationView(instanceId);
    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::setPosition(const QString &requestId,
                                           int instanceId,
                                           int x,
                                           int y)
{
    PlatformWindow *window = nullptr;
    UkuiQuick::WidgetItem *widgetItem = partitionWidgetItem(instanceId, &window);
    if (!widgetItem || !widgetItem->widget()) {
        return ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("PARTITION_NOT_FOUND"),
                                QStringLiteral("partition widget not found"),
                                {{QStringLiteral("instance_id"), instanceId}}));
    }

    const QPoint requestedVisualPos(x, y);

    WidgetsProxyModel *proxyModel = nullptr;
    if (window && window->rootItem()) {
        proxyModel = window->rootItem()->findChild<WidgetsProxyModel *>();
    }

    QPoint requestedConfigPos(requestedVisualPos);
    if (QQuickItem *delegate = widgetItem->parentItem()) {
        if (window) {
            requestedConfigPos = window->delegateConfigPosForScreenPosition(delegate,
                                                                          widgetItem,
                                                                          requestedVisualPos.x(),
                                                                          requestedVisualPos.y());
        }
    }

    QPoint finalConfigPos = requestedConfigPos;
    if (QQuickItem *delegate = widgetItem->parentItem()) {
        if (window) {
            finalConfigPos = window->applyWidgetItemConfigPosition(delegate,
                                                                   widgetItem,
                                                                   proxyModel,
                                                                   requestedConfigPos.x(),
                                                                   requestedConfigPos.y());
        } else {
            delegate->setX(requestedConfigPos.x());
            delegate->setY(requestedConfigPos.y());
            if (proxyModel) {
                proxyModel->updateItemPositionConfig(widgetItem, requestedConfigPos);
            } else {
                widgetItem->widget()->config()->setValue(QStringLiteral("x"), requestedConfigPos.x());
                widgetItem->widget()->config()->setValue(QStringLiteral("y"), requestedConfigPos.y());
            }
        }
    } else if (proxyModel) {
        proxyModel->updateItemPositionConfig(widgetItem, requestedConfigPos);
    } else {
        widgetItem->widget()->config()->setValue(QStringLiteral("x"), requestedConfigPos.x());
        widgetItem->widget()->config()->setValue(QStringLiteral("y"), requestedConfigPos.y());
    }

    if (window) {
        window->updateView();
    }

    const QPoint finalVisualPos = partitionWidgetScreenPosition(window, widgetItem);
    const bool positionAdjusted = (finalVisualPos != requestedVisualPos);

    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("x"), finalVisualPos.x());
    data.insert(QStringLiteral("y"), finalVisualPos.y());
    data.insert(QStringLiteral("requested_x"), requestedVisualPos.x());
    data.insert(QStringLiteral("requested_y"), requestedVisualPos.y());
    data.insert(QStringLiteral("position_adjusted"), positionAdjusted);
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::getPosition(const QString &requestId, int instanceId)
{
    PlatformWindow *window = nullptr;
    UkuiQuick::WidgetItem *widgetItem = partitionWidgetItem(instanceId, &window);
    if (!widgetItem || !widgetItem->widget()) {
        return ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("PARTITION_NOT_FOUND"),
                                QStringLiteral("partition widget not found"),
                                {{QStringLiteral("instance_id"), instanceId}}));
    }

    const QPoint visualPos = partitionWidgetScreenPosition(window, widgetItem);

    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("x"), visualPos.x());
    data.insert(QStringLiteral("y"), visualPos.y());
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::setSize(const QString &requestId,
                                       int instanceId,
                                       int width,
                                       int height)
{
    PlatformWindow *window = nullptr;
    UkuiQuick::WidgetItem *widgetItem = partitionWidgetItem(instanceId, &window);
    if (!widgetItem || !widgetItem->widget()) {
        return ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("PARTITION_NOT_FOUND"),
                                QStringLiteral("partition widget not found"),
                                {{QStringLiteral("instance_id"), instanceId}}));
    }

    auto *widget = widgetItem->widget();
    const int requestedWidth = width;
    const int requestedHeight = height;

    int gridWidth = requestedWidth;
    int gridHeight = requestedHeight;
    int appliedWidth = requestedWidth;
    int appliedHeight = requestedHeight;

    WidgetsProxyModel *proxyModel = nullptr;
    if (window && window->rootItem()) {
        proxyModel = window->rootItem()->findChild<WidgetsProxyModel *>();
    }

    QQuickItem *delegate = widgetItem->parentItem();
    QPoint configPos(widget->config()->getValue(QStringLiteral("x")).toInt(),
                     widget->config()->getValue(QStringLiteral("y")).toInt());
    if (delegate) {
        configPos = QPoint(int(delegate->x()), int(delegate->y()));
    }

    if (WidgetsProxyModel::isItemLayoutWithGrid(widgetItem)) {
        int gridUnitSize = WidgetsProxyModel::itemWidgetGridUnitSize(widgetItem);
        if (gridUnitSize <= 0) {
            gridUnitSize = 60;
        }

        const double gridSpanWidth = (requestedWidth + 2.0 * WIDGET_MARGIN + WIDGET_SPACING)
                                     / (gridUnitSize + WIDGET_SPACING);
        const double gridSpanHeight = (requestedHeight + 2.0 * WIDGET_MARGIN + WIDGET_SPACING)
                                      / (gridUnitSize + WIDGET_SPACING);
        gridWidth = qMax(1, qCeil(gridSpanWidth));
        gridHeight = qMax(1, qCeil(gridSpanHeight));

        appliedWidth = gridWidth * (gridUnitSize + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;
        appliedHeight = gridHeight * (gridUnitSize + WIDGET_SPACING) - WIDGET_SPACING - 2 * WIDGET_MARGIN;

        // 清理旧的像素尺寸配置，避免覆盖 SizeHint 导致新网格尺寸不生效。
        widget->config()->removeKey(QStringLiteral("width"));
        widget->config()->removeKey(QStringLiteral("height"));
        widget->config()->setValue(QStringLiteral("initGridWidth"), gridWidth);
        widget->config()->setValue(QStringLiteral("initGridHeight"), gridHeight);
        DesktopUKUIpluginnManager::instance()->setGirdSize(gridWidth, gridHeight, instanceId);

        // 立即把配置映射到当前可见 widgetItem，避免只写配置但界面不刷新的情况。
        if (proxyModel) {
            proxyModel->initWithConfig(widgetItem, delegate);
        } else {
            widgetItem->setWidth(appliedWidth);
            widgetItem->setHeight(appliedHeight);
        }
    } else {
        widget->config()->setValue(QStringLiteral("width"), requestedWidth);
        widget->config()->setValue(QStringLiteral("height"), requestedHeight);
        widgetItem->setWidth(requestedWidth);
        widgetItem->setHeight(requestedHeight);
    }

    if (window && delegate) {
        window->applyWidgetItemConfigPosition(delegate,
                                              widgetItem,
                                              proxyModel,
                                              configPos.x(),
                                              configPos.y());
    } else if (window) {
        window->updateView();
    }

    const int finalWidth = qRound(widgetItem->width());
    const int finalHeight = qRound(widgetItem->height());

    QJsonObject data;
    data.insert(QStringLiteral("instance_id"), instanceId);
    data.insert(QStringLiteral("requested_width"), requestedWidth);
    data.insert(QStringLiteral("requested_height"), requestedHeight);
    data.insert(QStringLiteral("grid_width"), gridWidth);
    data.insert(QStringLiteral("grid_height"), gridHeight);
    data.insert(QStringLiteral("width"), finalWidth > 0 ? finalWidth : appliedWidth);
    data.insert(QStringLiteral("height"), finalHeight > 0 ? finalHeight : appliedHeight);
    data.insert(QStringLiteral("size_adjusted"),
                (requestedWidth != (finalWidth > 0 ? finalWidth : appliedWidth))
                || (requestedHeight != (finalHeight > 0 ? finalHeight : appliedHeight)));
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::organizationEnable(const QString &requestId)
{
    GlobalSettings::getInstance()->setGSettingValue(DESKTOP_USE_ORIGANIZATION, true);
    auto *ctrl = FileActionController::getInstance();
    QTimer::singleShot(0, ctrl, &FileActionController::triggerDesktopOrganization);
    QJsonObject data;
    data.insert(QStringLiteral("enabled"), true);
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::organizationDisable(const QString &requestId)
{
    GlobalSettings::getInstance()->setGSettingValue(DESKTOP_USE_ORIGANIZATION, false);
    auto *ctrl = FileActionController::getInstance();
    QTimer::singleShot(0, ctrl, &FileActionController::triggerQuitDesktopOrganization);
    QJsonObject data;
    data.insert(QStringLiteral("enabled"), false);
    return ToolResponse::completed(requestId, data);
}

ToolResponse PartitionService::organizationStatus(const QString &requestId)
{
    QJsonObject data;
    data.insert(QStringLiteral("gsettings_enabled"),
                GlobalSettings::getInstance()->getValue(DESKTOP_USE_ORIGANIZATION).toBool());
    data.insert(QStringLiteral("config_exists"), orgManager()->isOriganizationToolConfigExist());
    data.insert(QStringLiteral("partitions"), listPartitions());
    return ToolResponse::completed(requestId, data);
}

} // namespace PeonyDesktop

/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Thin service layer over DesktopOrganizationManager (partition manager).
 * No new business logic — delegates to existing APIs only.
 */

#ifndef PEONY_DESKTOP_PARTITION_SERVICE_H
#define PEONY_DESKTOP_PARTITION_SERVICE_H

#include "tool-response.h"

#include <QJsonArray>
#include <QJsonObject>

namespace PeonyDesktop {

class PartitionService
{
public:
    static bool partitionExists(int instanceId);
    static QJsonArray listPartitions();
    static QJsonObject getPartition(int instanceId);
    static QStringList urisForPartition(int instanceId);

    static ToolResponse createPartition(const QString &requestId,
                                        const QStringList &uris,
                                        const QString &name);
    static ToolResponse renamePartition(const QString &requestId,
                                        int instanceId,
                                        const QString &name);
    static ToolResponse disbandPartition(const QString &requestId, int instanceId);
    static ToolResponse addFiles(const QString &requestId,
                                 int instanceId,
                                 const QStringList &uris);
    static ToolResponse moveFiles(const QString &requestId,
                                  int fromInstanceId,
                                  int toInstanceId,
                                  const QStringList &uris);
    static ToolResponse sortPartition(const QString &requestId,
                                      int instanceId,
                                      int sortType,
                                      int sortOrder);
    static ToolResponse setExpanded(const QString &requestId,
                                    int instanceId,
                                    bool expanded,
                                    int initHeight = -1);
    static ToolResponse getExpanded(const QString &requestId, int instanceId);
    static ToolResponse setListMode(const QString &requestId,
                                    int instanceId,
                                    bool listMode);
    static ToolResponse getListMode(const QString &requestId, int instanceId);
    static ToolResponse updateView(const QString &requestId, int instanceId);
    static ToolResponse setPosition(const QString &requestId,
                                    int instanceId,
                                    int x,
                                    int y);
    static ToolResponse getPosition(const QString &requestId, int instanceId);
    static ToolResponse setSize(const QString &requestId,
                                int instanceId,
                                int width,
                                int height);

    static ToolResponse organizationEnable(const QString &requestId);
    static ToolResponse organizationDisable(const QString &requestId);
    static ToolResponse organizationStatus(const QString &requestId);
};

} // namespace PeonyDesktop

#endif // PEONY_DESKTOP_PARTITION_SERVICE_H

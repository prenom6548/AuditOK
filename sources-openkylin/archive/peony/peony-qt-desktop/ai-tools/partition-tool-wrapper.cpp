/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "partition-tool-wrapper.h"

#include "partition-job-manager.h"
#include "partition-service.h"
#include "partition-tool-validator.h"
#include "widget-service.h"

#include <QJsonArray>

namespace PeonyDesktop {

QStringList PartitionToolWrapper::supportedTools()
{
    return {
        QStringLiteral("desktop.partition.list"),
        QStringLiteral("desktop.partition.get"),
        QStringLiteral("desktop.partition.create"),
        QStringLiteral("desktop.partition.rename"),
        QStringLiteral("desktop.partition.disband"),
        QStringLiteral("desktop.partition.add_files"),
        QStringLiteral("desktop.partition.move_files"),
        QStringLiteral("desktop.partition.sort"),
        QStringLiteral("desktop.partition.set_expanded"),
        QStringLiteral("desktop.partition.get_expanded"),
        QStringLiteral("desktop.partition.set_list_mode"),
        QStringLiteral("desktop.partition.get_list_mode"),
        QStringLiteral("desktop.partition.update_view"),
        QStringLiteral("desktop.partition.set_position"),
        QStringLiteral("desktop.partition.get_position"),
        QStringLiteral("desktop.partition.set_size"),
        QStringLiteral("desktop.partition.organization.enable"),
        QStringLiteral("desktop.partition.organization.disable"),
        QStringLiteral("desktop.partition.organization.status"),
        QStringLiteral("desktop.widget.list_available"),
        QStringLiteral("desktop.widget.list"),
        QStringLiteral("desktop.widget.add"),
        QStringLiteral("desktop.widget.remove"),
        QStringLiteral("desktop.job.get"),
    };
}

static ToolResponse dispatchSync(const QString &tool, const QString &requestId,
                                 const QJsonObject &args)
{
    if (tool == QStringLiteral("desktop.partition.list")) {
        QJsonObject data;
        data.insert(QStringLiteral("partitions"), PartitionService::listPartitions());
        return ToolResponse::completed(requestId, data);
    }

    if (tool == QStringLiteral("desktop.partition.get")) {
        const int instanceId = args.value(QStringLiteral("instance_id")).toInt();
        QJsonObject partition = PartitionService::getPartition(instanceId);
        if (partition.isEmpty()) {
            return ToolResponse::failed(requestId,
                                        ToolError::fromCode(QStringLiteral("PARTITION_NOT_FOUND"),
                                                            QStringLiteral("partition not found"),
                                                            {{QStringLiteral("instance_id"), instanceId}}));
        }
        return ToolResponse::completed(requestId, partition);
    }

    if (tool == QStringLiteral("desktop.partition.create")) {
        QStringList uris;
        if (args.contains(QStringLiteral("uris"))) {
            const QJsonArray arr = args.value(QStringLiteral("uris")).toArray();
            for (const QJsonValue &v : arr) {
                uris.append(v.toString());
            }
        }
        const QString name = args.value(QStringLiteral("name")).toString();
        return PartitionService::createPartition(requestId, uris, name);
    }

    if (tool == QStringLiteral("desktop.partition.rename")) {
        return PartitionService::renamePartition(requestId,
                                                 args.value(QStringLiteral("instance_id")).toInt(),
                                                 args.value(QStringLiteral("new_name")).toString());
    }

    if (tool == QStringLiteral("desktop.partition.disband")) {
        return PartitionService::disbandPartition(requestId,
                                                 args.value(QStringLiteral("instance_id")).toInt());
    }

    if (tool == QStringLiteral("desktop.partition.add_files")) {
        QStringList uris;
        for (const QJsonValue &v : args.value(QStringLiteral("uris")).toArray()) {
            uris.append(v.toString());
        }
        return PartitionService::addFiles(requestId,
                                         args.value(QStringLiteral("instance_id")).toInt(),
                                         uris);
    }

    if (tool == QStringLiteral("desktop.partition.move_files")) {
        QStringList uris;
        for (const QJsonValue &v : args.value(QStringLiteral("uris")).toArray()) {
            uris.append(v.toString());
        }
        const int fromInstanceId = args.contains(QStringLiteral("from_instance_id"))
            ? args.value(QStringLiteral("from_instance_id")).toInt()
            : -1;
        return PartitionService::moveFiles(requestId,
                                           fromInstanceId,
                                           args.value(QStringLiteral("to_instance_id")).toInt(),
                                           uris);
    }

    if (tool == QStringLiteral("desktop.partition.sort")) {
        return PartitionService::sortPartition(requestId,
                                               args.value(QStringLiteral("instance_id")).toInt(),
                                               args.value(QStringLiteral("sort_type")).toInt(),
                                               args.value(QStringLiteral("sort_order")).toInt());
    }

    if (tool == QStringLiteral("desktop.partition.set_expanded")) {
        const int initHeight = args.contains(QStringLiteral("init_height"))
            ? args.value(QStringLiteral("init_height")).toInt()
            : -1;
        return PartitionService::setExpanded(requestId,
                                             args.value(QStringLiteral("instance_id")).toInt(),
                                             args.value(QStringLiteral("expanded")).toBool(),
                                             initHeight);
    }

    if (tool == QStringLiteral("desktop.partition.get_expanded")) {
        return PartitionService::getExpanded(requestId,
                                             args.value(QStringLiteral("instance_id")).toInt());
    }

    if (tool == QStringLiteral("desktop.partition.set_list_mode")) {
        return PartitionService::setListMode(requestId,
                                             args.value(QStringLiteral("instance_id")).toInt(),
                                             args.value(QStringLiteral("list_mode")).toBool());
    }

    if (tool == QStringLiteral("desktop.partition.get_list_mode")) {
        return PartitionService::getListMode(requestId,
                                             args.value(QStringLiteral("instance_id")).toInt());
    }

    if (tool == QStringLiteral("desktop.partition.update_view")) {
        return PartitionService::updateView(requestId,
                                            args.value(QStringLiteral("instance_id")).toInt());
    }

    if (tool == QStringLiteral("desktop.partition.set_position")) {
        return PartitionService::setPosition(requestId,
                                             args.value(QStringLiteral("instance_id")).toInt(),
                                             args.value(QStringLiteral("x")).toInt(),
                                             args.value(QStringLiteral("y")).toInt());
    }

    if (tool == QStringLiteral("desktop.partition.get_position")) {
        return PartitionService::getPosition(requestId,
                                             args.value(QStringLiteral("instance_id")).toInt());
    }

    if (tool == QStringLiteral("desktop.partition.set_size")) {
        return PartitionService::setSize(requestId,
                                         args.value(QStringLiteral("instance_id")).toInt(),
                                         args.value(QStringLiteral("width")).toInt(),
                                         args.value(QStringLiteral("height")).toInt());
    }

    if (tool == QStringLiteral("desktop.partition.organization.enable")) {
        return PartitionService::organizationEnable(requestId);
    }

    if (tool == QStringLiteral("desktop.partition.organization.disable")) {
        return PartitionService::organizationDisable(requestId);
    }

    if (tool == QStringLiteral("desktop.partition.organization.status")) {
        return PartitionService::organizationStatus(requestId);
    }

    if (tool == QStringLiteral("desktop.widget.list_available")) {
        QJsonObject data;
        data.insert(QStringLiteral("widgets"), WidgetService::listAvailableWidgets());
        return ToolResponse::completed(requestId, data);
    }

    if (tool == QStringLiteral("desktop.widget.list")) {
        QJsonObject data;
        data.insert(QStringLiteral("widgets"), WidgetService::listWidgets());
        return ToolResponse::completed(requestId, data);
    }

    if (tool == QStringLiteral("desktop.widget.add")) {
        const QString pluginId = args.value(QStringLiteral("plugin_id")).toString();
        const int screenId = args.contains(QStringLiteral("screen_id"))
            ? args.value(QStringLiteral("screen_id")).toInt()
            : 0;
        const QJsonObject variant = args.value(QStringLiteral("variant")).toObject();
        return WidgetService::addWidget(requestId, pluginId, screenId, variant);
    }

    if (tool == QStringLiteral("desktop.widget.remove")) {
        const int instanceId = args.value(QStringLiteral("instance_id")).toInt();
        const int screenId = args.contains(QStringLiteral("screen_id"))
            ? args.value(QStringLiteral("screen_id")).toInt()
            : -1;
        return WidgetService::removeWidget(requestId, instanceId, screenId);
    }

    if (tool == QStringLiteral("desktop.job.get")) {
        const QString jobId = args.value(QStringLiteral("job_id")).toString();
        return PartitionToolWrapper::getJob(jobId);
    }

    return ToolResponse::failed(requestId,
                                ToolError::fromCode(QStringLiteral("UNKNOWN_TOOL"),
                                                    QStringLiteral("unsupported tool"),
                                                    {{QStringLiteral("tool"), tool}}));
}

ToolResponse PartitionToolWrapper::invoke(const QString &tool,
                                          const QString &requestId,
                                          const QJsonObject &args,
                                          bool async)
{
    if (!supportedTools().contains(tool)) {
        return ToolResponse::failed(requestId,
                                    ToolError::fromCode(QStringLiteral("UNKNOWN_TOOL"),
                                                        QStringLiteral("unsupported tool"),
                                                        {{QStringLiteral("tool"), tool}}));
    }

    const ToolResponse validation = PartitionToolValidator::validate(tool, args, requestId);
    if (!validation.success) {
        return validation;
    }

    if (!async) {
        return dispatchSync(tool, requestId, args);
    }

    const QString jobId = PartitionJobManager::instance()->submit(
        tool, requestId, [=]() { return dispatchSync(tool, requestId, args); });

    QJsonObject data;
    data.insert(QStringLiteral("poll_tool"), QStringLiteral("desktop.job.get"));
    data.insert(QStringLiteral("tool"), tool);
    return ToolResponse::pending(requestId, jobId, data);
}

ToolResponse PartitionToolWrapper::getJob(const QString &jobId)
{
    const QString requestId;
    if (!PartitionJobManager::instance()->hasJob(jobId)) {
        return ToolResponse::failed(requestId,
                                    ToolError::fromCode(QStringLiteral("JOB_NOT_FOUND"),
                                                        QStringLiteral("job not found"),
                                                        {{QStringLiteral("job_id"), jobId}}));
    }

    const PartitionJobManager::JobRecord record = PartitionJobManager::instance()->job(jobId);
    ToolResponse resp = record.result;
    resp.jobId = jobId;
    resp.requestId = record.requestId;
    resp.status = record.status;

    if (record.status == QStringLiteral("completed")) {
        resp.success = true;
    } else if (record.status == QStringLiteral("failed")) {
        resp.success = false;
        if (resp.error.code.isEmpty()) {
            resp.error = ToolError::fromCode(QStringLiteral("JOB_FAILED"),
                                             QStringLiteral("job failed"));
        }
    } else {
        resp.success = true;
    }

    QJsonObject data = resp.data;
    data.insert(QStringLiteral("job_id"), jobId);
    data.insert(QStringLiteral("tool"), record.tool);
    resp.data = data;
    return resp;
}

} // namespace PeonyDesktop

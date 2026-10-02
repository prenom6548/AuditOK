/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "partition-tool-validator.h"

#include "partition-service.h"
#include "widget-service.h"

#include <QJsonArray>
#include <QUrl>

namespace PeonyDesktop {

bool PartitionToolValidator::isValidUri(const QString &uri, QString *errorOut)
{
    if (uri.isEmpty()) {
        if (errorOut) {
            *errorOut = QStringLiteral("uri must not be empty");
        }
        return false;
    }
    const QUrl url(uri);
    if (!url.isValid() || url.scheme() != QStringLiteral("file")) {
        if (errorOut) {
            *errorOut = QStringLiteral("uri must be a valid file:// URI");
        }
        return false;
    }
    return true;
}

bool PartitionToolValidator::parseUriList(const QJsonValue &value, QStringList *urisOut,
                                          QString *errorOut)
{
    if (!value.isArray()) {
        if (errorOut) {
            *errorOut = QStringLiteral("uris must be a JSON array");
        }
        return false;
    }
    const QJsonArray arr = value.toArray();
    QStringList uris;
    for (const QJsonValue &item : arr) {
        if (!item.isString()) {
            if (errorOut) {
                *errorOut = QStringLiteral("each uri must be a string");
            }
            return false;
        }
        const QString uri = item.toString();
        if (!isValidUri(uri, errorOut)) {
            return false;
        }
        uris.append(uri);
    }
    if (urisOut) {
        *urisOut = uris;
    }
    return true;
}

bool PartitionToolValidator::requireInstanceId(const QJsonObject &args, int *instanceIdOut,
                                               QString *errorOut)
{
    if (!args.contains(QStringLiteral("instance_id"))) {
        if (errorOut) {
            *errorOut = QStringLiteral("instance_id is required");
        }
        return false;
    }
    const int instanceId = args.value(QStringLiteral("instance_id")).toInt(-1);
    if (instanceId < 0) {
        if (errorOut) {
            *errorOut = QStringLiteral("instance_id must be >= 0");
        }
        return false;
    }
    if (!PartitionService::partitionExists(instanceId)) {
        if (errorOut) {
            *errorOut = QStringLiteral("partition not found");
        }
        return false;
    }
    if (instanceIdOut) {
        *instanceIdOut = instanceId;
    }
    return true;
}

ToolResponse PartitionToolValidator::validate(const QString &tool,
                                              const QJsonObject &args,
                                              const QString &requestId)
{
    auto fail = [&](const QString &code, const QString &message,
                    const QJsonObject &details = QJsonObject()) {
        return ToolResponse::failed(requestId, ToolError::fromCode(code, message, details));
    };

    if (tool == QStringLiteral("desktop.partition.list")
        || tool == QStringLiteral("desktop.partition.organization.status")
        || tool == QStringLiteral("desktop.partition.organization.enable")
        || tool == QStringLiteral("desktop.partition.organization.disable")
        || tool == QStringLiteral("desktop.widget.list_available")
        || tool == QStringLiteral("desktop.widget.list")) {
        return ToolResponse::completed(requestId);
    }

    if (tool == QStringLiteral("desktop.widget.add")) {
        const QString pluginId = args.value(QStringLiteral("plugin_id")).toString();
        if (pluginId.isEmpty()) {
            return fail(QStringLiteral("INVALID_PLUGIN_ID"), QStringLiteral("plugin_id is required"));
        }
        if (args.contains(QStringLiteral("screen_id"))
            && args.value(QStringLiteral("screen_id")).toInt() < 0) {
            return fail(QStringLiteral("INVALID_SCREEN_ID"), QStringLiteral("screen_id must be >= 0"));
        }
        if (args.contains(QStringLiteral("variant")) && !args.value(QStringLiteral("variant")).isObject()) {
            return fail(QStringLiteral("INVALID_VARIANT"), QStringLiteral("variant must be a JSON object"));
        }
        return ToolResponse::completed(requestId);
    }

    if (tool == QStringLiteral("desktop.widget.remove")) {
        if (!args.contains(QStringLiteral("instance_id"))) {
            return fail(QStringLiteral("INVALID_INSTANCE_ID"), QStringLiteral("instance_id is required"));
        }
        const int instanceId = args.value(QStringLiteral("instance_id")).toInt(-1);
        if (instanceId < 0) {
            return fail(QStringLiteral("INVALID_INSTANCE_ID"), QStringLiteral("instance_id must be >= 0"),
                        {{QStringLiteral("instance_id"), args.value(QStringLiteral("instance_id"))}});
        }
        if (!WidgetService::widgetExists(instanceId)) {
            return fail(QStringLiteral("WIDGET_NOT_FOUND"), QStringLiteral("desktop widget not found"),
                        {{QStringLiteral("instance_id"), instanceId}});
        }
        if (args.contains(QStringLiteral("screen_id"))
            && args.value(QStringLiteral("screen_id")).toInt() < 0) {
            return fail(QStringLiteral("INVALID_SCREEN_ID"), QStringLiteral("screen_id must be >= 0"));
        }
        return ToolResponse::completed(requestId);
    }

    if (tool == QStringLiteral("desktop.job.get")) {
        const QString jobId = args.value(QStringLiteral("job_id")).toString();
        if (jobId.isEmpty()) {
            return fail(QStringLiteral("INVALID_JOB_ID"), QStringLiteral("job_id is required"));
        }
        return ToolResponse::completed(requestId);
    }

    if (tool == QStringLiteral("desktop.partition.create")) {
        if (args.contains(QStringLiteral("uris"))) {
            QStringList uris;
            QString err;
            if (!parseUriList(args.value(QStringLiteral("uris")), &uris, &err)) {
                return fail(QStringLiteral("INVALID_URIS"), err);
            }
        }
        if (args.contains(QStringLiteral("name"))) {
            const QString name = args.value(QStringLiteral("name")).toString();
            if (name.isEmpty()) {
                return fail(QStringLiteral("INVALID_NAME"), QStringLiteral("name must not be empty"));
            }
        }
        return ToolResponse::completed(requestId);
    }

    if (tool == QStringLiteral("desktop.partition.get")
        || tool == QStringLiteral("desktop.partition.rename")
        || tool == QStringLiteral("desktop.partition.disband")
        || tool == QStringLiteral("desktop.partition.add_files")
        || tool == QStringLiteral("desktop.partition.sort")
        || tool == QStringLiteral("desktop.partition.set_expanded")
        || tool == QStringLiteral("desktop.partition.get_expanded")
        || tool == QStringLiteral("desktop.partition.set_list_mode")
        || tool == QStringLiteral("desktop.partition.get_list_mode")
        || tool == QStringLiteral("desktop.partition.update_view")
        || tool == QStringLiteral("desktop.partition.set_position")
        || tool == QStringLiteral("desktop.partition.get_position")
        || tool == QStringLiteral("desktop.partition.set_size")) {
        int instanceId = -1;
        QString err;
        if (!requireInstanceId(args, &instanceId, &err)) {
            return fail(QStringLiteral("INVALID_INSTANCE_ID"), err,
                        {{QStringLiteral("instance_id"), args.value(QStringLiteral("instance_id"))}});
        }
        Q_UNUSED(instanceId);
    }

    if (tool == QStringLiteral("desktop.partition.rename")) {
        const QString name = args.value(QStringLiteral("new_name")).toString();
        if (name.isEmpty()) {
            return fail(QStringLiteral("INVALID_NAME"), QStringLiteral("new_name is required"));
        }
    }

    if (tool == QStringLiteral("desktop.partition.add_files")) {
        QStringList uris;
        QString err;
        if (!parseUriList(args.value(QStringLiteral("uris")), &uris, &err)) {
            return fail(QStringLiteral("INVALID_URIS"), err);
        }
        if (uris.isEmpty()) {
            return fail(QStringLiteral("INVALID_URIS"), QStringLiteral("uris must not be empty"));
        }
    }

    if (tool == QStringLiteral("desktop.partition.move_files")) {
        QStringList uris;
        QString err;
        if (!parseUriList(args.value(QStringLiteral("uris")), &uris, &err)) {
            return fail(QStringLiteral("INVALID_URIS"), err);
        }
        if (uris.isEmpty()) {
            return fail(QStringLiteral("INVALID_URIS"), QStringLiteral("uris must not be empty"));
        }

        if (!args.contains(QStringLiteral("to_instance_id"))) {
            return fail(QStringLiteral("INVALID_INSTANCE_ID"),
                        QStringLiteral("to_instance_id is required"));
        }
        const int toInstanceId = args.value(QStringLiteral("to_instance_id")).toInt(-2);
        if (toInstanceId < -1) {
            return fail(QStringLiteral("INVALID_INSTANCE_ID"),
                        QStringLiteral("to_instance_id must be -1 (desktop) or a partition instance id"));
        }
        if (toInstanceId >= 0 && !PartitionService::partitionExists(toInstanceId)) {
            return fail(QStringLiteral("INVALID_INSTANCE_ID"),
                        QStringLiteral("target partition not found"),
                        {{QStringLiteral("to_instance_id"), toInstanceId}});
        }

        if (args.contains(QStringLiteral("from_instance_id"))) {
            const int fromInstanceId = args.value(QStringLiteral("from_instance_id")).toInt(-2);
            if (fromInstanceId < -1) {
                return fail(QStringLiteral("INVALID_INSTANCE_ID"),
                            QStringLiteral("from_instance_id must be -1 (desktop) or a partition instance id"));
            }
            if (fromInstanceId >= 0 && !PartitionService::partitionExists(fromInstanceId)) {
                return fail(QStringLiteral("INVALID_INSTANCE_ID"),
                            QStringLiteral("source partition not found"),
                            {{QStringLiteral("from_instance_id"), fromInstanceId}});
            }
        }
    }

    if (tool == QStringLiteral("desktop.partition.sort")) {
        const int sortType = args.value(QStringLiteral("sort_type")).toInt(-1);
        const int sortOrder = args.value(QStringLiteral("sort_order")).toInt(-1);
        if (sortType < 0 || sortType > 3) {
            return fail(QStringLiteral("INVALID_SORT_TYPE"),
                        QStringLiteral("sort_type must be 0..3"));
        }
        if (sortOrder < 0 || sortOrder > 1) {
            return fail(QStringLiteral("INVALID_SORT_ORDER"),
                        QStringLiteral("sort_order must be 0 (asc) or 1 (desc)"));
        }
    }

    if (tool == QStringLiteral("desktop.partition.set_expanded")) {
        if (!args.contains(QStringLiteral("expanded"))) {
            return fail(QStringLiteral("INVALID_ARGUMENT"),
                        QStringLiteral("expanded is required"));
        }
        if (args.contains(QStringLiteral("init_height"))
            && args.value(QStringLiteral("init_height")).toInt() < 0) {
            return fail(QStringLiteral("INVALID_ARGUMENT"),
                        QStringLiteral("init_height must be >= 0 when provided"));
        }
    }

    if (tool == QStringLiteral("desktop.partition.set_list_mode")) {
        if (!args.contains(QStringLiteral("list_mode"))) {
            return fail(QStringLiteral("INVALID_ARGUMENT"),
                        QStringLiteral("list_mode is required"));
        }
    }

    if (tool == QStringLiteral("desktop.partition.set_position")) {
        if (!args.contains(QStringLiteral("x"))) {
            return fail(QStringLiteral("INVALID_ARGUMENT"), QStringLiteral("x is required"));
        }
        if (!args.contains(QStringLiteral("y"))) {
            return fail(QStringLiteral("INVALID_ARGUMENT"), QStringLiteral("y is required"));
        }
        if (args.value(QStringLiteral("x")).toInt() < 0 || args.value(QStringLiteral("y")).toInt() < 0) {
            return fail(QStringLiteral("INVALID_ARGUMENT"),
                        QStringLiteral("x and y must be >= 0"));
        }
    }

    if (tool == QStringLiteral("desktop.partition.set_size")) {
        if (!args.contains(QStringLiteral("width"))) {
            return fail(QStringLiteral("INVALID_ARGUMENT"), QStringLiteral("width is required"));
        }
        if (!args.contains(QStringLiteral("height"))) {
            return fail(QStringLiteral("INVALID_ARGUMENT"), QStringLiteral("height is required"));
        }
        if (args.value(QStringLiteral("width")).toInt() <= 0
            || args.value(QStringLiteral("height")).toInt() <= 0) {
            return fail(QStringLiteral("INVALID_ARGUMENT"),
                        QStringLiteral("width and height must be > 0"));
        }
    }

    return ToolResponse::completed(requestId);
}

} // namespace PeonyDesktop

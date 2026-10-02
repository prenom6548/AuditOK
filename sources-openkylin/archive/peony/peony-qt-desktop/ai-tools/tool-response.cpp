/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "tool-response.h"

#include <QJsonDocument>

namespace PeonyDesktop {

QJsonObject ToolError::toJson() const
{
    QJsonObject obj;
    obj.insert(QStringLiteral("code"), code);
    obj.insert(QStringLiteral("message"), message);
    if (!details.isEmpty()) {
        obj.insert(QStringLiteral("details"), details);
    }
    return obj;
}

ToolError ToolError::fromCode(const QString &code, const QString &message,
                              const QJsonObject &details)
{
    ToolError err;
    err.code = code;
    err.message = message;
    err.details = details;
    return err;
}

QString ToolResponse::toJsonString() const
{
    QJsonObject root;
    root.insert(QStringLiteral("success"), success);
    if (success) {
        root.insert(QStringLiteral("error"), QJsonValue::Null);
    } else {
        root.insert(QStringLiteral("error"), error.toJson());
    }
    root.insert(QStringLiteral("request_id"), requestId);
    if (jobId.isEmpty()) {
        root.insert(QStringLiteral("job_id"), QJsonValue::Null);
    } else {
        root.insert(QStringLiteral("job_id"), jobId);
    }
    root.insert(QStringLiteral("status"), status);
    if (data.isEmpty()) {
        root.insert(QStringLiteral("data"), QJsonValue::Null);
    } else {
        root.insert(QStringLiteral("data"), data);
    }
    if (!meta.isEmpty()) {
        root.insert(QStringLiteral("meta"), meta);
    }
    return QString::fromUtf8(QJsonDocument(root).toJson(QJsonDocument::Compact));
}

ToolResponse ToolResponse::completed(const QString &requestId, const QJsonObject &data)
{
    ToolResponse resp;
    resp.success = true;
    resp.requestId = requestId;
    resp.status = QStringLiteral("completed");
    resp.data = data;
    resp.meta.insert(QStringLiteral("api_version"), QStringLiteral("1.0"));
    return resp;
}

ToolResponse ToolResponse::failed(const QString &requestId, const ToolError &error)
{
    ToolResponse resp;
    resp.success = false;
    resp.error = error;
    resp.requestId = requestId;
    resp.status = QStringLiteral("failed");
    resp.meta.insert(QStringLiteral("api_version"), QStringLiteral("1.0"));
    return resp;
}

ToolResponse ToolResponse::pending(const QString &requestId, const QString &jobId,
                                   const QJsonObject &data)
{
    ToolResponse resp;
    resp.success = true;
    resp.requestId = requestId;
    resp.jobId = jobId;
    resp.status = QStringLiteral("pending");
    resp.data = data;
    resp.meta.insert(QStringLiteral("api_version"), QStringLiteral("1.0"));
    return resp;
}

} // namespace PeonyDesktop

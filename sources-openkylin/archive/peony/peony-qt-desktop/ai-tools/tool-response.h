/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef PEONY_DESKTOP_TOOL_RESPONSE_H
#define PEONY_DESKTOP_TOOL_RESPONSE_H

#include <QJsonObject>
#include <QString>

namespace PeonyDesktop {

struct ToolError {
    QString code;
    QString message;
    QJsonObject details;

    QJsonObject toJson() const;
    static ToolError fromCode(const QString &code, const QString &message,
                              const QJsonObject &details = QJsonObject());
};

struct ToolResponse {
    bool success = false;
    ToolError error;
    QString requestId;
    QString jobId;
    QString status; // completed | pending | running | failed | cancelled
    QJsonObject data;
    QJsonObject meta;

    QString toJsonString() const;
    static ToolResponse completed(const QString &requestId, const QJsonObject &data = QJsonObject());
    static ToolResponse failed(const QString &requestId, const ToolError &error);
    static ToolResponse pending(const QString &requestId, const QString &jobId,
                                const QJsonObject &data = QJsonObject());
};

} // namespace PeonyDesktop

#endif // PEONY_DESKTOP_TOOL_RESPONSE_H

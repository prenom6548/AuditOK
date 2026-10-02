/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "desktop-partition-dbus-service.h"

#include "partition-job-manager.h"
#include "partition-tool-wrapper.h"

#include <QDBusConnection>
#include <QDBusError>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace PeonyDesktop {

static const char kServiceName[] = "org.ukui.peony.desktop";
static const char kObjectPath[] = "/org/ukui/peony/desktop/partition";

DesktopPartitionDbusService::DesktopPartitionDbusService(QObject *parent)
    : QObject(parent)
{
    connect(PartitionJobManager::instance(), &PartitionJobManager::jobFinished,
            this, &DesktopPartitionDbusService::JobFinished);
}

void DesktopPartitionDbusService::registerService()
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) {
        qWarning() << "DesktopPartitionDbusService: session bus not connected";
        return;
    }

    bus.unregisterService(QString::fromLatin1(kServiceName));
    if (!bus.registerService(QString::fromLatin1(kServiceName))) {
        qWarning() << "DesktopPartitionDbusService: registerService failed"
                   << bus.lastError().message();
    }

    if (!bus.registerObject(QString::fromLatin1(kObjectPath),
                            this,
                            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qWarning() << "DesktopPartitionDbusService: registerObject failed"
                   << bus.lastError().message();
    }
}

QString DesktopPartitionDbusService::Invoke(const QString &tool,
                                            const QString &requestId,
                                            const QString &argsJson,
                                            const QString &optionsJson)
{
    Q_UNUSED(optionsJson);

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(argsJson.toUtf8(), &parseError);
    QJsonObject args;
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        args = doc.object();
    } else if (!argsJson.isEmpty()) {
        ToolResponse resp = ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("INVALID_JSON"),
                                QStringLiteral("args_json is not a valid JSON object")));
        return resp.toJsonString();
    }

    const ToolResponse resp = PartitionToolWrapper::invoke(tool, requestId, args, false);
    return resp.toJsonString();
}

QString DesktopPartitionDbusService::InvokeAsync(const QString &tool,
                                                 const QString &requestId,
                                                 const QString &argsJson,
                                                 const QString &optionsJson)
{
    Q_UNUSED(optionsJson);

    QJsonParseError parseError;
    const QJsonDocument doc = QJsonDocument::fromJson(argsJson.toUtf8(), &parseError);
    QJsonObject args;
    if (parseError.error == QJsonParseError::NoError && doc.isObject()) {
        args = doc.object();
    } else if (!argsJson.isEmpty()) {
        ToolResponse resp = ToolResponse::failed(
            requestId,
            ToolError::fromCode(QStringLiteral("INVALID_JSON"),
                                QStringLiteral("args_json is not a valid JSON object")));
        return resp.toJsonString();
    }

    const ToolResponse resp = PartitionToolWrapper::invoke(tool, requestId, args, true);
    return resp.toJsonString();
}

QString DesktopPartitionDbusService::GetJob(const QString &jobId)
{
    return PartitionToolWrapper::getJob(jobId).toJsonString();
}

QString DesktopPartitionDbusService::ListTools(const QString &prefix)
{
    QJsonArray tools;
    for (const QString &tool : PartitionToolWrapper::supportedTools()) {
        if (prefix.isEmpty() || tool.startsWith(prefix)) {
            tools.append(tool);
        }
    }
    QJsonObject data;
    data.insert(QStringLiteral("tools"), tools);
    ToolResponse resp = ToolResponse::completed(QString(), data);
    return resp.toJsonString();
}

} // namespace PeonyDesktop

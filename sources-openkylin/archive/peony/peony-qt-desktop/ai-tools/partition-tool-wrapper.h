/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef PEONY_DESKTOP_PARTITION_TOOL_WRAPPER_H
#define PEONY_DESKTOP_PARTITION_TOOL_WRAPPER_H

#include "tool-response.h"

#include <QJsonObject>
#include <QStringList>

namespace PeonyDesktop {

class PartitionToolWrapper
{
public:
    static QStringList supportedTools();

    static ToolResponse invoke(const QString &tool,
                               const QString &requestId,
                               const QJsonObject &args,
                               bool async);

    static ToolResponse getJob(const QString &jobId);
};

} // namespace PeonyDesktop

#endif // PEONY_DESKTOP_PARTITION_TOOL_WRAPPER_H

/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef PEONY_DESKTOP_PARTITION_TOOL_VALIDATOR_H
#define PEONY_DESKTOP_PARTITION_TOOL_VALIDATOR_H

#include "tool-response.h"

#include <QJsonObject>

namespace PeonyDesktop {

class PartitionToolValidator
{
public:
    static ToolResponse validate(const QString &tool,
                                 const QJsonObject &args,
                                 const QString &requestId);

private:
    static bool isValidUri(const QString &uri, QString *errorOut = nullptr);
    static bool parseUriList(const QJsonValue &value, QStringList *urisOut,
                             QString *errorOut = nullptr);
    static bool requireInstanceId(const QJsonObject &args, int *instanceIdOut,
                                  QString *errorOut = nullptr);
};

} // namespace PeonyDesktop

#endif // PEONY_DESKTOP_PARTITION_TOOL_VALIDATOR_H

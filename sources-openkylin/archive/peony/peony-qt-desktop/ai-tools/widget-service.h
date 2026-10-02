/*
 * Peony-Qt Desktop AI Tools
 *
 * Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Thin service layer over DesktopWidgetsManager / PlatformWindow widget APIs.
 */

#ifndef PEONY_DESKTOP_WIDGET_SERVICE_H
#define PEONY_DESKTOP_WIDGET_SERVICE_H

#include "tool-response.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QStringList>

namespace PeonyDesktop {

class WidgetService
{
public:
    static bool widgetExists(int instanceId);
    static QJsonArray listAvailableWidgets();
    static QJsonArray listWidgets();

    static ToolResponse addWidget(const QString &requestId,
                                  const QString &pluginId,
                                  int screenId = 0,
                                  const QJsonObject &variant = QJsonObject());
    static ToolResponse removeWidget(const QString &requestId,
                                     int instanceId,
                                     int screenId = -1);
};

} // namespace PeonyDesktop

#endif // PEONY_DESKTOP_WIDGET_SERVICE_H

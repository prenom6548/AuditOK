// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include "ukuishellintegration.h"

#include <QtWaylandClient/private/qwaylandshellintegrationplugin_p.h>

namespace UKUI
{

class UkuiShellIntegrationPlugin : public QtWaylandClient::QWaylandShellIntegrationPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QWaylandShellIntegrationFactoryInterface_iid FILE "ukui-shell.json")
public:
    QtWaylandClient::QWaylandShellIntegration *create(const QString &key, const QStringList &paramList) override;
};

QtWaylandClient::QWaylandShellIntegration *UkuiShellIntegrationPlugin::create(const QString &key, const QStringList &paramList)
{
    return new UkuiShellIntegration();
}

}

#include "main.moc"

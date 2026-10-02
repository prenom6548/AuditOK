// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#pragma once

#include "ukuishell.h"

#include <QtWaylandClient/private/qwaylandshellintegration_p.h>

namespace UKUI
{

class UkuiShellIntegration : public QtWaylandClient::QWaylandShellIntegration
{
public:
    UkuiShellIntegration() = default;
    bool initialize(QtWaylandClient::QWaylandDisplay *display) override;
    QtWaylandClient::QWaylandShellSurface *createShellSurface(QtWaylandClient::QWaylandWindow *window) override;

private:
    std::unique_ptr<UkuiShell> m_ukuiShell;
};

} // namespace UKUI

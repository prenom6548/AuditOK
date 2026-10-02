/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef WAYLAND_INTEGRATION_P_H
#define WAYLAND_INTEGRATION_P_H

#include <QObject>
#include <QPointer>

#include "ukui-window-management.h"

namespace KWayland::Client {
class ConnectionThread;
}


namespace UkuiQuick::WaylandClient
{
class UkuiShell;
class Registry;
class UkuiBlurManager;
class Compositor;
}

class WaylandIntegration : public QObject
{
    Q_OBJECT

public:
    explicit WaylandIntegration(QObject *parent = nullptr);
    ~WaylandIntegration() override;

    UkuiQuick::WaylandClient::Registry *registry() const;
    UkuiQuick::WaylandClient::UkuiShell *waylandUkuiShell();
    UkuiQuick::WaylandClient::Compositor *waylandCompositor() const;
    UkuiQuick::WaylandClient::UkuiBlurManager *waylandBlurManager();

    static WaylandIntegration *self();
    void sync();

private:
    void setupKWaylandIntegration();

    QPointer<UkuiQuick::WaylandClient::Registry> m_registry;
    QPointer<UkuiQuick::WaylandClient::UkuiShell> m_waylandUkuiShell;
    QPointer<KWayland::Client::ConnectionThread> m_connection;
    QPointer<UkuiQuick::WaylandClient::Compositor> m_compositor;
    QPointer<UkuiQuick::WaylandClient::UkuiBlurManager> m_blurManager;

    Q_DISABLE_COPY(WaylandIntegration)
};

#endif // WAYLAND_INTEGRATION_P_H

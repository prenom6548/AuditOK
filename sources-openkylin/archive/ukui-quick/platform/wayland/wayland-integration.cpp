/*
    SPDX-FileCopyrightText: 2020 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    Based on WaylandIntegration from kwayland-integration

    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "wayland-integration_p.h"
#include <KWayland/Client/connection_thread.h>

#include <QGuiApplication>
#include <QDebug>

#include "ukui-shell-v1.h"
#include "compositor.h"
#include "ukui-blur-manager.h"
#include "registry.h"

class WaylandIntegrationSingleton
{
public:
    WaylandIntegration self;
};

Q_GLOBAL_STATIC(WaylandIntegrationSingleton, privateWaylandIntegrationSelf)

WaylandIntegration::WaylandIntegration(QObject *parent)
    : QObject(parent)
{
    setupKWaylandIntegration();
}

WaylandIntegration::~WaylandIntegration() {}

UkuiQuick::WaylandClient::Registry *WaylandIntegration::registry() const
{
    return m_registry;
}

UkuiQuick::WaylandClient::UkuiShell *WaylandIntegration::waylandUkuiShell()
{
    if (!m_waylandUkuiShell && m_registry) {
        const UkuiQuick::WaylandClient::Registry::AnnouncedInterface interface = m_registry->interface(UkuiQuick::WaylandClient::Registry::Interface::UkuiShell);

        if (interface.name == 0) {
            qWarning() << "The compositor does not support the ukui shell protocol";
            return nullptr;
        }

        m_waylandUkuiShell = m_registry->createUkuiShell(interface.name, interface.version, qGuiApp);

        connect(m_waylandUkuiShell, &UkuiQuick::WaylandClient::UkuiShell::removed, this, [this]() {
            m_waylandUkuiShell->deleteLater();
        });
    }

    return m_waylandUkuiShell;
}

UkuiQuick::WaylandClient::Compositor *WaylandIntegration::waylandCompositor() const
{
    return m_compositor;
}

UkuiQuick::WaylandClient::UkuiBlurManager* WaylandIntegration::waylandBlurManager()
{
    if (!m_blurManager && m_registry) {
        const UkuiQuick::WaylandClient::Registry::AnnouncedInterface wmInterface = m_registry->interface(UkuiQuick::WaylandClient::Registry::Interface::UkuiBlurManager);

        if (wmInterface.name == 0) {
            return nullptr;
        }

        m_blurManager = m_registry->createUkuiBlurManager(wmInterface.name, wmInterface.version, qGuiApp);

        connect(m_blurManager, &UkuiQuick::WaylandClient::UkuiBlurManager::removed, this, [this]() {
            m_blurManager->deleteLater();
        });
    }

    return m_blurManager;
}

WaylandIntegration *WaylandIntegration::self()
{
    return &privateWaylandIntegrationSelf()->self;
}

void WaylandIntegration::sync()
{
    wl_display_roundtrip(m_connection->display());
}

void WaylandIntegration::setupKWaylandIntegration()
{
    m_connection = KWayland::Client::ConnectionThread::fromApplication(this);
    if (!m_connection) {
        qWarning() << "Failed getting Wayland connection from QPA";
        return;
    }

    m_registry = new UkuiQuick::WaylandClient::Registry(qGuiApp);
    m_registry->create(m_connection);
    m_compositor = UkuiQuick::WaylandClient::Compositor::fromApplication(this);
    m_registry->setup();
    m_connection->roundtrip();
}

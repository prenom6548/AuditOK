// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QDebug>
#include <QGuiApplication>

#include "xdg-dialog-v1-client-protocol.h"
#include "xdg-foreign-unstable-v2-client-protocol.h"

#include "WaylandClient.h"

WaylandClient::WaylandClient(Protocols protocols, QObject *parent)
    : QObject(parent), m_activeProtocols(protocols)
{
    nativeInterface = QGuiApplication::platformNativeInterface();
    if (!nativeInterface) {
        qWarning() << "WaylandClient: Failed to get platform native interface";
        return;
    }

    display =
        reinterpret_cast<wl_display *>(nativeInterface->nativeResourceForIntegration("wl_display"));
    if (!display) {
        qWarning() << "WaylandClient: Failed to get wl_display.";
        return;
    }

    m_registry = wl_display_get_registry(display);
    wl_registry_add_listener(m_registry, &s_registryListener, this);
    wl_display_roundtrip(display);
}

WaylandClient::~WaylandClient()
{
    if (xdgImporter) {
        zxdg_importer_v2_destroy(xdgImporter);
    }
    if (xdgWmDialog) {
        xdg_wm_dialog_v1_destroy(xdgWmDialog);
    }
    if (m_registry) {
        wl_registry_destroy(m_registry);
    }
}

void WaylandClient::registryGlobalHandler(void *data, struct wl_registry *registry, uint32_t name,
                                          const char *interface, uint32_t /*version*/)
{
    auto *self = static_cast<WaylandClient *>(data);
    if (!self) {
        return;
    }

    if (self->m_activeProtocols.testFlag(Protocol::XDG_DIALOG) &&
        strcmp(interface, xdg_wm_dialog_v1_interface.name) == 0) {
        self->xdgWmDialog = reinterpret_cast<struct xdg_wm_dialog_v1 *>(
            wl_registry_bind(registry, name, &xdg_wm_dialog_v1_interface, 1));
    } else if (self->m_activeProtocols.testFlag(Protocol::XDG_FOREIGN) &&
               strcmp(interface, zxdg_importer_v2_interface.name) == 0) {
        self->xdgImporter = reinterpret_cast<struct zxdg_importer_v2 *>(
            wl_registry_bind(registry, name, &zxdg_importer_v2_interface, 1));
    }
}

void WaylandClient::registryGlobalRemoveHandler(void *data, struct wl_registry * /*registry*/,
                                                uint32_t /*name*/)
{
    auto *self = static_cast<WaylandClient *>(data);
    if (!self) {
        return;
    }
}

const struct wl_registry_listener WaylandClient::s_registryListener = {
    .global = registryGlobalHandler,
    .global_remove = registryGlobalRemoveHandler,
};

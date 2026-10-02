// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef WAYLANDCLIENT_H
#define WAYLANDCLIENT_H

#include <QObject>
#include <qpa/qplatformnativeinterface.h>

#include <wayland-client.h>

class WaylandClient : public QObject
{
    Q_OBJECT
  public:
    enum Protocol {
        XDG_DIALOG = 0x01,
        XDG_FOREIGN = 0x02,
    };
    Q_DECLARE_FLAGS(Protocols, Protocol)

    explicit WaylandClient(Protocols protocols, QObject *parent = nullptr);
    ~WaylandClient() override;

    QPlatformNativeInterface *nativeInterface = nullptr;
    wl_display *display = nullptr;
    struct zxdg_importer_v2 *xdgImporter = nullptr;
    struct xdg_wm_dialog_v1 *xdgWmDialog = nullptr;

  private:
    static void registryGlobalHandler(void *data, struct wl_registry *registry, uint32_t name,
                                      const char *interface, uint32_t version);
    static void registryGlobalRemoveHandler(void *data, struct wl_registry *registry,
                                            uint32_t name);

    static const struct wl_registry_listener s_registryListener;

    wl_registry *m_registry = nullptr;

    Protocols m_activeProtocols;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(WaylandClient::Protocols)
#endif // WAYLANDCLIENT_H

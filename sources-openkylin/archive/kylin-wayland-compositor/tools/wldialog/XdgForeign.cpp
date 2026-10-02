// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QDebug>
#include <QString>
#include <QWindow>

#include <wayland-client.h>

#include "xdg-foreign-unstable-v2-client-protocol.h"

#include "XdgForeign.h"

XdgForeign::XdgForeign(WaylandClient *client, QObject *parent) : QObject(parent), m_client(client)
{
}

XdgForeign::~XdgForeign()
{
    if (m_imported) {
        zxdg_imported_v2_destroy(m_imported);
        m_imported = nullptr;
    }
}

void XdgForeign::importedDestroyedHandler(void *data, struct zxdg_imported_v2 *)
{
    auto *self = static_cast<XdgForeign *>(data);
    if (!self) {
        return;
    }

    qDebug() << "XDGForeign: imported destroyed";

    self->m_imported = nullptr;
    emit self->parentWindowLost();
}

const struct zxdg_imported_v2_listener XdgForeign::s_importedListener = {
    .destroyed = importedDestroyedHandler,
};

bool XdgForeign::setParentWindow(QWindow *window, const QString &parentHandle)
{
    if (!m_client || !m_client->xdgImporter) {
        return false;
    }

    if (!window) {
        qWarning() << "XDGForeign: Invalid child window.";
        return false;
    }

    if (parentHandle.isEmpty()) {
        qWarning() << "XDGForeign: parentHandle is empty.";
        return false;
    }

    wl_surface *surface = reinterpret_cast<wl_surface *>(
        m_client->nativeInterface->nativeResourceForWindow("surface", window));

    if (m_imported) {
        zxdg_imported_v2_destroy(m_imported);
        m_imported = nullptr;
    }

    m_imported =
        zxdg_importer_v2_import_toplevel(m_client->xdgImporter, parentHandle.toUtf8().constData());
    if (!m_imported) {
        qWarning()
            << QString("XDGForeign: Failed to import toplevel with handle: %1").arg(parentHandle);
        return false;
    }

    zxdg_imported_v2_add_listener(m_imported, &s_importedListener, this);
    zxdg_imported_v2_set_parent_of(m_imported, surface);

    wl_display_flush(m_client->display);
    wl_display_roundtrip(m_client->display);

    qDebug() << QString("Parent window set successfully for handle: %1").arg(parentHandle);
    return true;
}

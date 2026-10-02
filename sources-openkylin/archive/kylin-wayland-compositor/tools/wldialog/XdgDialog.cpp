// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QDebug>
#include <QGuiApplication>

#include "xdg-dialog-v1-client-protocol.h"

#include "XdgDialog.h"

XdgDialog::XdgDialog(WaylandClient *client, QObject *parent) : QObject(parent), m_client(client) {}

XdgDialog::~XdgDialog()
{
    if (m_xdgDialog) {
        xdg_dialog_v1_destroy(m_xdgDialog);
        m_xdgDialog = nullptr;
    }
}

void XdgDialog::setDialogModal(QWindow *window, bool flag)
{
    if (!m_client || !m_client->xdgWmDialog) {
        return;
    }

    struct xdg_toplevel *xdgToplevel = reinterpret_cast<struct xdg_toplevel *>(
        m_client->nativeInterface->nativeResourceForWindow("xdg_toplevel", window));
    if (!xdgToplevel) {
        qDebug() << "Failed to get xdg_toplevel";
        return;
    }

    m_xdgDialog = xdg_wm_dialog_v1_get_xdg_dialog(m_client->xdgWmDialog, xdgToplevel);
    if (!m_xdgDialog) {
        qDebug() << "Failed to create xdg_dialog";
        return;
    }

    if (flag) {
        xdg_dialog_v1_set_modal(m_xdgDialog);
    } else {
        xdg_dialog_v1_unset_modal(m_xdgDialog);
    }
}

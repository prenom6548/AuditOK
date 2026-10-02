// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef XDGDIALOG_H
#define XDGDIALOG_H

#include "WaylandClient.h"

class XdgDialog : public QObject
{
    Q_OBJECT
  public:
    XdgDialog(WaylandClient *client, QObject *parent = nullptr);
    ~XdgDialog() override;

    void setDialogModal(QWindow *window, bool flag);

  private:
    WaylandClient *m_client = nullptr;
    struct xdg_dialog_v1 *m_xdgDialog = nullptr;
};

#endif // XDGDIALOG_H

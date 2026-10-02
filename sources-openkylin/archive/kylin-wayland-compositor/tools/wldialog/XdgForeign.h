// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef XDGFOREIGN_H
#define XDGFOREIGN_H

#include "WaylandClient.h"

class XdgForeign : public QObject
{
    Q_OBJECT
  public:
    XdgForeign(WaylandClient *client, QObject *parent = nullptr);
    ~XdgForeign() override;

    bool setParentWindow(QWindow *window, const QString &parentHandle);

  Q_SIGNALS:
    void parentWindowLost();

  private:
    static void importedDestroyedHandler(void *data, struct zxdg_imported_v2 *zxdg_imported_v2);

    static const struct zxdg_imported_v2_listener s_importedListener;

    struct zxdg_imported_v2 *m_imported = nullptr;

    WaylandClient *m_client = nullptr;
};

#endif // XDGFOREIGN_H

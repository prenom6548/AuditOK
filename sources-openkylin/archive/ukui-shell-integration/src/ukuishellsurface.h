// SPDX-FileCopyrightText: 2024 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#pragma once

#include "ukuishell.h"

#include <QtWaylandClient/private/qwaylandshellsurface_p.h>

#include <QSize>

class QWindow;

namespace UKUI
{

class Popup;
class Toplevel;
class XdgDecorationManagerV1;
class XdgDialogV1;

class UkuiShellSurface : public QtWaylandClient::QWaylandShellSurface, public xdg_surface, public ukui_surface
{
    Q_OBJECT
public:
    UkuiShellSurface(UkuiShell *shell, ::xdg_surface *xgdSurface, ::ukui_surface *ukuiSurface, QtWaylandClient::QWaylandWindow *window);
    ~UkuiShellSurface() override;

    UkuiShell *shell();
    Toplevel *toplevel();
    Popup *popup();

    // --------- QWaylandShellSurface interface begin------------//
    bool resize(QtWaylandClient::QWaylandInputDevice *inputDevice, Qt::Edges edges) override;
    bool move(QtWaylandClient::QWaylandInputDevice *inputDevice) override;
    bool showWindowMenu(QtWaylandClient::QWaylandInputDevice *seat) override;
    void setTitle(const QString &title) override;
    void setAppId(const QString &appId) override;
    void setWindowFlags(Qt::WindowFlags flags) override;
    bool isExposed() const override;
    bool handleExpose(const QRegion &) override;
    void applyConfigure() override;
    bool wantsDecorations() const override;
    void propagateSizeHints() override;
    void setContentGeometry(const QRect &rect) override;
    void setWindowPosition(const QPoint &rect) override;
    void setSizeHints();
    // Not implemented yet
    void raise();
    void lower();
    void setContentOrientationMask(Qt::ScreenOrientations orientation);
    void sendProperty(const QString &name, const QVariant &value);
    // --------- QWaylandShellSurface interface end------------//

    void setSkipTaskbar(bool skip);
    void setSkipSwitcher(bool skip);
    void setNoTitlebar(bool noTitlebar);
    void setTheme(int theme);
    void setWindowRadius(int radius);
    void setBorderWidth(int width);
    void setBorderColor(QColor color);
    void setShadowRadius(int radius);
    void setShadowOffset(int offset);
    void setShadowColor(QColor color);
    void setWindowRole(int role);
    void setWindowIcon(const QString &icon);

protected:
    void requestWindowStates(Qt::WindowStates states) override;
    void xdg_surface_configure(uint32_t serial) override;
    void ukui_surface_position(int32_t x, int32_t y) override;

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void initWindowByProperties();
    void slotWindowPropertyChanged(const char *property_name);

    void setToplevel();
    void setPopup(QtWaylandClient::QWaylandWindow *parent);
    void setGrabPopup(QtWaylandClient::QWaylandWindow *parent, QtWaylandClient::QWaylandInputDevice *device, int serial);

    UkuiShell *m_shell = nullptr;
    QtWaylandClient::QWaylandWindow *m_window = nullptr;
    Toplevel *m_toplevel = nullptr;
    Popup *m_popup = nullptr;
    bool m_configured = false;
    QRegion m_exposeRegion;
    uint m_pendingConfigureSerial = 0;
    uint m_appliedConfigureSerial = 0;

    bool m_pendingPosition = false;
    QPoint m_surfacePosition;
};

class Toplevel : public xdg_toplevel
{
public:
    Toplevel(UkuiShellSurface *ukuiShellSurface);
    ~Toplevel() override;

    void applyConfigure();
    bool wantsDecorations();

    void xdg_toplevel_configure(int32_t width, int32_t height, wl_array *states) override;
    void xdg_toplevel_close() override;

    void requestWindowFlags(Qt::WindowFlags flags);
    void requestWindowStates(Qt::WindowStates states);

    static resize_edge convertToResizeEdges(Qt::Edges edges);

private:
    struct
    {
        QSize size = {0, 0};
        Qt::WindowStates states = Qt::WindowNoState;
    } m_pending, m_applied;
    QSize m_normalSize;

    UkuiShellSurface *m_ukuiShellSurface = nullptr;
    XdgToplevelDecorationV1 *m_decoration = nullptr;
    std::unique_ptr<XdgDialogV1> m_xdgDialog;
};

class Popup : public xdg_popup
{
public:
    Popup(UkuiShellSurface *ukuiShellSurface, QtWaylandClient::QWaylandWindow *parent, xdg_positioner *positioner);
    ~Popup() override;

    UkuiShellSurface *ukuiShellSurface();

    void grab(QtWaylandClient::QWaylandInputDevice *seat, uint serial);
    void xdg_popup_popup_done() override;

private:
    UkuiShellSurface *m_ukuiShellSurface = nullptr;
    UkuiShellSurface *m_parentUkuiShellSurface = nullptr;
    QtWaylandClient::QWaylandWindow *m_parent = nullptr;
    bool m_grabbing = false;
};

} // namespace UKUI

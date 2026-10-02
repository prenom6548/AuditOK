#include "ukuiwindowhelper.h"
#include "qapplication.h"
#include "qguiapplication.h"
#include "ukuidisplay_p.h"

#include "wayland-ukui-shell-v1-client-protocol.h"

#include <QDebug>
#include <QGuiApplication>
#include <QLibrary>
#include <QPair>
#include <QScreen>
#include <QVariant>
#include <QWidget>
#include <QWindow>
#include <QX11Info>

#include <X11/X.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

using nativeResourceForWindowFunPtr = void *(*)(void *, QByteArray const &, QWindow *);
static nativeResourceForWindowFunPtr nativeResourceForWindowFun = nullptr;

ukui_surface_v1 *getUkuiSurfaceV1Object(QWindow *window)
{
    if (!window)
        return nullptr;

    void *native = qApp->platformNativeInterface();
    if (!native) {
        qWarning() << "Failed to get QPlatformNativeInterface";
        return nullptr;
    }

    if (!nativeResourceForWindowFun)
        nativeResourceForWindowFun = (nativeResourceForWindowFunPtr)QLibrary::resolve("libQt5WaylandClient.so.5", "_ZN15QtWaylandClient23QWaylandNativeInterface23nativeResourceForWindowERK10QByteArrayP7QWindow");

    if (!nativeResourceForWindowFun) {
        qWarning() << "Failed to resolve nativeResourceForIntegration function";
        return nullptr;
    }

    return static_cast<ukui_surface_v1 *>(nativeResourceForWindowFun(native, QByteArrayLiteral("ukui_surface_v1"), window));
}

class UkuiWindowHelperPrivate
{
public:
    explicit UkuiWindowHelperPrivate(UkuiWindowHelper *windowHelper, QWidget *widget);
    explicit UkuiWindowHelperPrivate(UkuiWindowHelper *windowHelper, QWindow *window);
    ~UkuiWindowHelperPrivate();

    void removeTitleBar();
    void setWindowRole(UkuiWindowHelper::WindowRole role);
    void setWindowState(UkuiWindowHelper::States state);
    void setWindowIcon(const QString &iconName);
    void setSkipTaskBar(bool skip);
    void setSkipSwitcher(bool skip);
    void setPanelAutoHide(bool autoHide);
    void grabKeyboard();
    void openUnderCursor(QPoint pos);
    void setBlurEffect(QRegion region, int level = 0, bool enable = true);
    void setSlideEffect(UkuiWindowHelper::Position position, int offset = 0, bool enable = true);
    void showTileFlyout(QRect rect);
    void setDecorationCompoents(UkuiWindowHelper::DecorationComponents components);
    void requestGetCurrentScreen();
    QScreen *currentScreen() const;
    void setOpaqueRegion(QRegion);
    void setRestoreGeometry(QRect geometry);

private:
    static void ukui_shell_v1_current_output(void *data, ukui_shell_v1 *ukui_shell_v1, const char *output_name, const char *seat_name);
    static void ukui_shell_v1_done(void *data, ukui_shell_v1 *ukui_shell_v1);

    bool initUkuiShellV1Interface();

    ukui_shell_v1 *m_ukuiShellv1 = nullptr;
    static ukui_shell_v1_listener s_ukui_shell_v1_listener;

    UkuiWindowHelper *m_windowHelper = nullptr;
    QWidget *m_widget = nullptr;
    QWindow *m_window = nullptr;
    ukui_surface_v1 *m_ukuiSurfacev1 = nullptr;
    QString m_currentOutputName;
    QString m_currentSeatName;
    QScreen *m_currentScreen = nullptr;
};

UkuiWindowHelperPrivate::UkuiWindowHelperPrivate(UkuiWindowHelper *windowHelper, QWidget *widget)
    : m_windowHelper(windowHelper)
    , m_widget(widget)
{
    if (!m_widget) {
        qWarning() << "Creating a UkuiWindowHelperPrivate for a null QWidget(nullptr)!";
    }
}

UkuiWindowHelperPrivate::UkuiWindowHelperPrivate(UkuiWindowHelper *windowHelper, QWindow *window)
    : m_windowHelper(windowHelper)
    , m_window(window)
{
    if (!m_window) {
        qWarning() << "Creating a UkuiWindowHelperPrivate for a QWindow(nullptr)!";
    }
}

UkuiWindowHelperPrivate::~UkuiWindowHelperPrivate()
{
    if (m_ukuiShellv1) {
        ukui_shell_v1_destroy(m_ukuiShellv1);
        m_ukuiShellv1 = nullptr;
    }
}

void UkuiWindowHelperPrivate::removeTitleBar()
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_no_titlebar", true);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_no_titlebar", true);
    }

    if (QGuiApplication::platformName() == "xcb") {
        Display *display = QX11Info::display();
        if (!display) {
            return;
        }

        Atom atom = XInternAtom(display, "_KWIN_UKUI_DECORAION", False);
        if (atom == None) {
            return;
        }

        unsigned long data = 1;
        Window winId = m_widget ? m_widget->winId() : (m_window ? m_window->winId() : 0);
        if (winId != 0) {
            XChangeProperty(display, winId, atom, atom, 32, PropModeReplace, reinterpret_cast<unsigned char *>(&data), 1);
            XFlush(display);
        }
    }
}

static QMap<UkuiWindowHelper::WindowRole, QStringList> roleMap = {
    {UkuiWindowHelper::WindowRole::Normal, {"_NET_WM_WINDOW_TYPE_NORMAL"}},
    {UkuiWindowHelper::WindowRole::Desktop, {"_NET_WM_WINDOW_TYPE_DESKTOP"}},
    {UkuiWindowHelper::WindowRole::Panel, {"_UKUI_NET_WM_WINDOW_TYPE_PANEL", "_NET_WM_WINDOW_TYPE_DOCK"}},
    {UkuiWindowHelper::WindowRole::OnScreenDisplay, {"_KDE_NET_WM_WINDOW_TYPE_ON_SCREEN_DISPLAY", "_NET_WM_WINDOW_TYPE_NOTIFICATION"}},
    {UkuiWindowHelper::WindowRole::Notification, {"_NET_WM_WINDOW_TYPE_NOTIFICATION", "_NET_WM_WINDOW_TYPE_UTILITY"}},
    {UkuiWindowHelper::WindowRole::Tooltip, {"_NET_WM_WINDOW_TYPE_TOOLTIP"}},
    {UkuiWindowHelper::WindowRole::CriticalNotification, {"_NET_WM_WINDOW_TYPE_CRITICAL_NOTIFICATION", "_NET_WM_WINDOW_TYPE_NOTIFICATION"}},
    {UkuiWindowHelper::WindowRole::AppletPopup, {"_NET_WM_WINDOW_TYPE_APPLET_POPUP"}},
    {UkuiWindowHelper::WindowRole::ScreenLock, {"_UKUI_NET_WM_WINDOW_TYPE_SCREENLOCK"}},
    {UkuiWindowHelper::WindowRole::Watermark, {"_UKUI_NET_WM_WINDOW_TYPE_WATERMARK"}},
    {UkuiWindowHelper::WindowRole::SystemWindow, {"_UKUI_NET_WM_WINDOW_TYPE_SYSTEMWINDOW"}},
    {UkuiWindowHelper::WindowRole::InputPanel, {"_UKUI_NET_WM_WINDOW_TYPE_INPUTPANEL"}},
    {UkuiWindowHelper::WindowRole::Logout, {"_UKUI_NET_WM_WINDOW_TYPE_LOGOUT"}},
    {UkuiWindowHelper::WindowRole::ScreenLockNotification, {"_UKUI_NET_WM_WINDOW_TYPE_SCREENLOCKNOTIFICATION"}},
    {UkuiWindowHelper::WindowRole::Switcher, {"_UKUI_NET_WM_WINDOW_TYPE_SWITCHER"}},
    {UkuiWindowHelper::WindowRole::Authentication, {"_UKUI_NET_WM_WINDOW_TYPE_AUTHENTICATION"}},
    {UkuiWindowHelper::WindowRole::Override, {"_NET_WM_WINDOW_TYPE_OVERRIDE", "_NET_WM_WINDOW_TYPE_NORMAL"}},
    {UkuiWindowHelper::WindowRole::Dialog, {"_NET_WM_WINDOW_TYPE_DIALOG"}},
    {UkuiWindowHelper::WindowRole::Menu, {"_NET_WM_WINDOW_TYPE_MENU"}},
    {UkuiWindowHelper::WindowRole::TopMenu, {"_KDE_NET_WM_WINDOW_TYPE_TOPMENU", "_NET_WM_WINDOW_TYPE_DOCK"}},
    {UkuiWindowHelper::WindowRole::ToolBar, {"_NET_WM_WINDOW_TYPE_TOOLBAR"}},
    {UkuiWindowHelper::WindowRole::Dock, {"_NET_WM_WINDOW_TYPE_DOCK"}},
    {UkuiWindowHelper::WindowRole::Utility, {"_NET_WM_WINDOW_TYPE_UTILITY", "_NET_WM_WINDOW_TYPE_DIALOG"}},
    {UkuiWindowHelper::WindowRole::Splash, {"_NET_WM_WINDOW_TYPE_SPLASH", "_NET_WM_WINDOW_TYPE_DOCK"}},
    {UkuiWindowHelper::WindowRole::DropdownMenu, {"_NET_WM_WINDOW_TYPE_DROPDOWN_MENU", "_NET_WM_WINDOW_TYPE_MENU"}},
    {UkuiWindowHelper::WindowRole::PopupMenu, {"_NET_WM_WINDOW_TYPE_POPUP_MENU", "_NET_WM_WINDOW_TYPE_MENU"}},
    {UkuiWindowHelper::WindowRole::ComboBox, {"_NET_WM_WINDOW_TYPE_COMBO"}},
    {UkuiWindowHelper::WindowRole::DNDIcon, {"_NET_WM_WINDOW_TYPE_DND"}},
};

void UkuiWindowHelperPrivate::setWindowRole(UkuiWindowHelper::WindowRole role)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_role_v1", qBound(0, (int)role, 100));
    } else if (m_window) {
        m_window->setProperty("ukui_surface_role_v1", qBound(0, (int)role, 100));
    }

    if (QGuiApplication::platformName() == "xcb") {
        Display *display = QX11Info::display();
        if (!display) {
            return;
        }

        Atom prop = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
        if (prop == None) {
            return;
        }

        QStringList types = roleMap.value(role);
        if (types.isEmpty()) {
            return;
        }

        QVector<Atom> atomList;
        for (const QString &type : types) {
            Atom atom = XInternAtom(display, type.toUtf8().constData(), False);
            if (atom != None) {
                atomList.append(atom);
            }
        }

        if (atomList.isEmpty()) {
            return;
        }

        Window win = m_widget ? m_widget->winId() : (m_window ? m_window->winId() : 0);
        if (!win)
            return;

        XChangeProperty(QX11Info::display(), win, prop, XA_ATOM, 32, PropModeReplace, (unsigned char *)atomList.constData(), atomList.size());
    }
}

void UkuiWindowHelperPrivate::setWindowState(UkuiWindowHelper::States states)
{
    QPair<uint32_t, uint32_t> pair(0x3ff, states);
    if (m_widget) {
        m_widget->setProperty("ukui_surface_state", QVariant::fromValue(pair));
    } else if (m_window) {
        m_window->setProperty("ukui_surface_state", QVariant::fromValue(pair));
    }

    if (QGuiApplication::platformName() == "xcb") {
        Display *display = QX11Info::display();
        if (!display) {
            return;
        }

        Atom prop = XInternAtom(display, "_NET_WM_STATE", False);
        if (prop == None) {
            return;
        }

        Window win = m_widget ? m_widget->winId() : (m_window ? m_window->winId() : 0);
        if (!win)
            return;

        Atom actualType;
        int actualFormat;
        unsigned long nItems, bytesAfter;
        Atom *currentAtoms = nullptr;

        QVector<Atom> finalAtoms;
        QStringList atomList = {"_NET_WM_STATE_ABOVE", "_NET_WM_STATE_BELOW"};

        if (Success == XGetWindowProperty(display, win, prop, 0, 1024, False, XA_ATOM, &actualType, &actualFormat, &nItems, &bytesAfter, (unsigned char **)&currentAtoms)) {
            for (unsigned long i = 0; i < nItems; ++i) {
                char *name = XGetAtomName(display, currentAtoms[i]);
                if (name) {
                    QString nameStr = QString::fromUtf8(name);
                    if (!atomList.contains(nameStr))
                        finalAtoms.append(currentAtoms[i]);
                    XFree(name);
                }
            }
            if (currentAtoms)
                XFree(currentAtoms);
        }

        if (states & UkuiWindowHelper::State::KeepAbove) {
            Atom atom = XInternAtom(display, "_NET_WM_STATE_ABOVE", False);
            if (atom != None) {
                finalAtoms.append(atom);
            }
        }
        if (states & UkuiWindowHelper::State::KeepBelow) {
            Atom atom = XInternAtom(display, "_NET_WM_STATE_BELOW", False);
            if (atom != None) {
                finalAtoms.append(atom);
            }
        }

        if (finalAtoms.isEmpty()) {
            return;
        }

        XChangeProperty(QX11Info::display(), win, prop, XA_ATOM, 32, PropModeReplace, (unsigned char *)finalAtoms.constData(), finalAtoms.size());
    }
}

void UkuiWindowHelperPrivate::setWindowIcon(const QString &iconName)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_icon", iconName);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_icon", iconName);
    }
}

void UkuiWindowHelperPrivate::setSkipTaskBar(bool skip)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_skip_taskbar", skip);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_skip_taskbar", skip);
    }

    if (QGuiApplication::platformName() == "xcb") {
        Display *display = QX11Info::display();
        if (!display) {
            return;
        }

        Atom prop = XInternAtom(display, "_NET_WM_STATE", False);
        if (prop == None) {
            return;
        }

        Window win = m_widget ? m_widget->winId() : (m_window ? m_window->winId() : 0);
        if (!win)
            return;

        Atom actualType;
        int actualFormat;
        unsigned long nItems, bytesAfter;
        Atom *currentAtoms = nullptr;

        QVector<Atom> finalAtoms;
        QString atomStr = "_NET_WM_STATE_SKIP_TASKBAR";

        if (Success == XGetWindowProperty(display, win, prop, 0, 1024, False, XA_ATOM, &actualType, &actualFormat, &nItems, &bytesAfter, (unsigned char **)&currentAtoms)) {
            for (unsigned long i = 0; i < nItems; ++i) {
                char *name = XGetAtomName(display, currentAtoms[i]);
                if (name) {
                    QString nameStr = QString::fromUtf8(name);
                    if (nameStr != atomStr)
                        finalAtoms.append(currentAtoms[i]);
                    XFree(name);
                }
            }
            if (currentAtoms)
                XFree(currentAtoms);
        }

        if (skip) {
            Atom skipAtom = XInternAtom(display, atomStr.toUtf8().constData(), False);
            if (skipAtom != None) {
                finalAtoms.append(skipAtom);
            }
        }

        if (finalAtoms.isEmpty()) {
            XDeleteProperty(display, win, prop);
        } else {
            XChangeProperty(QX11Info::display(), win, prop, XA_ATOM, 32, PropModeReplace, (unsigned char *)finalAtoms.constData(), finalAtoms.size());
        }
    }
}

void UkuiWindowHelperPrivate::setSkipSwitcher(bool skip)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_skip_switcher", skip);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_skip_switcher", skip);
    }
    if (QGuiApplication::platformName() == "xcb") {
        Display *display = QX11Info::display();
        if (!display) {
            return;
        }

        Atom prop = XInternAtom(display, "_NET_WM_STATE", False);
        if (prop == None) {
            return;
        }

        Window win = m_widget ? m_widget->winId() : (m_window ? m_window->winId() : 0);
        if (!win)
            return;

        Atom actualType;
        int actualFormat;
        unsigned long nItems, bytesAfter;
        Atom *currentAtoms = nullptr;

        QVector<Atom> finalAtoms;
        QString atomStr = "_KDE_NET_WM_STATE_SKIP_SWITCHER";

        if (Success == XGetWindowProperty(display, win, prop, 0, 1024, False, XA_ATOM, &actualType, &actualFormat, &nItems, &bytesAfter, (unsigned char **)&currentAtoms)) {
            for (unsigned long i = 0; i < nItems; ++i) {
                char *name = XGetAtomName(display, currentAtoms[i]);
                if (name) {
                    QString nameStr = QString::fromUtf8(name);
                    if (nameStr != atomStr)
                        finalAtoms.append(currentAtoms[i]);
                    XFree(name);
                }
            }
            if (currentAtoms)
                XFree(currentAtoms);
        }

        if (skip) {
            Atom skipAtom = XInternAtom(display, atomStr.toUtf8().constData(), False);
            if (skipAtom != None) {
                finalAtoms.append(skipAtom);
            }
        }

        if (finalAtoms.isEmpty()) {
            XDeleteProperty(display, win, prop);
        } else {
            XChangeProperty(display, win, prop, XA_ATOM, 32, PropModeReplace, (unsigned char *)finalAtoms.constData(), finalAtoms.size());
        }
    }
}

void UkuiWindowHelperPrivate::setPanelAutoHide(bool autoHide)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_panel_auto_hide", autoHide);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_panel_auto_hide", autoHide);
    }
}

void UkuiWindowHelperPrivate::grabKeyboard()
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_grab_keyboard", true);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_grab_keyboard", true);
    }
}

void UkuiWindowHelperPrivate::openUnderCursor(QPoint pos)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_open_under_cursor", pos);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_open_under_cursor", pos);
    }
}

void UkuiWindowHelperPrivate::setBlurEffect(QRegion region, int level, bool enable)
{
    QPair<QRegion, int> pair(region, level);
    if (!enable)
        pair.second = -1;

    if (m_widget) {
        m_widget->setProperty("ukui_surface_blur", QVariant::fromValue(pair));
    } else if (m_window) {
        m_window->setProperty("ukui_surface_blur", QVariant::fromValue(pair));
    }

    if (QGuiApplication::platformName() == "xcb") {
        Display *display = QX11Info::display();
        if (!display)
            return;

        Atom blurAtom = XInternAtom(display, "_KDE_NET_WM_BLUR_BEHIND_REGION", False);
        if (blurAtom == None)
            return;

        Window win = m_widget ? m_widget->winId() : (m_window ? m_window->winId() : 0);
        if (!win)
            return;

        if (enable) {
            QVector<unsigned long> data;
            data.reserve(region.rectCount() * 4 + 1); // 每个 QRect 占4个uint，最后加1是 level

            qreal dpr = qApp->devicePixelRatio(); // 设备像素缩放
            for (const QRect &r : region) {
                data << static_cast<unsigned long>(std::floor(r.x() * dpr))
                     << static_cast<unsigned long>(std::floor(r.y() * dpr))
                     << static_cast<unsigned long>(std::ceil(r.width() * dpr))
                     << static_cast<unsigned long>(std::ceil(r.height() * dpr));
            }

            data << static_cast<unsigned long>(level); // 添加 level（模糊强度）

            XChangeProperty(display, win, blurAtom, XA_CARDINAL, 32, PropModeReplace, reinterpret_cast<const unsigned char *>(data.constData()), data.size());
            XFlush(display);
        } else {
            XDeleteProperty(display, win, blurAtom);
            XFlush(display);
        }
    }
}

void UkuiWindowHelperPrivate::setSlideEffect(UkuiWindowHelper::Position position, int offset, bool enable)
{
    QPair<int, int> pair((int)position, offset);

    if (!enable)
        pair.first = -1;

    if (m_widget) {
        m_widget->setProperty("ukui_surface_slide", QVariant::fromValue(pair));
    } else if (m_window) {
        m_window->setProperty("ukui_surface_slide", QVariant::fromValue(pair));
    }

    if (QGuiApplication::platformName() == "xcb") {
        Display *display = QX11Info::display();
        if (!display)
            return;

        Atom slideAtom = XInternAtom(display, "_KDE_SLIDE", False);
        if (slideAtom == None)
            return;

        Window winId = m_widget ? m_widget->winId() : (m_window ? m_window->winId() : 0);
        if (!winId)
            return;

        if (!enable) {
            XDeleteProperty(display, winId, slideAtom);
            XFlush(display);
            return;
        }

        unsigned long data[2];
        data[0] = offset;

        switch (position) {
        case UkuiWindowHelper::Position::Left:
            data[1] = 0;
            break;
        case UkuiWindowHelper::Position::Top:
            data[1] = 1;
            break;
        case UkuiWindowHelper::Position::Right:
            data[1] = 2;
            break;
        case UkuiWindowHelper::Position::Bottom:
        default:
            data[1] = 3;
            break;
        }

        XChangeProperty(display, winId, slideAtom, slideAtom, 32, PropModeReplace, reinterpret_cast<unsigned char *>(data), 2);
        XFlush(display);
    }
}

void UkuiWindowHelperPrivate::showTileFlyout(QRect rect)
{
    if (qApp->platformName() != "wayland") {
        qWarning() << "showTileFlyout() is only available on Wayland!";
        return;
    }

    m_ukuiSurfacev1 = getUkuiSurfaceV1Object(m_window ? m_window : m_widget->windowHandle());

    if (!m_ukuiSurfacev1) {
        qWarning() << "Failed to get ukui_surface_v1";
        return;
    }

    ukui_surface_v1_show_tile_flyout(m_ukuiSurfacev1, nullptr, rect.x(), rect.y(), rect.width(), rect.height());
}

void UkuiWindowHelperPrivate::setDecorationCompoents(UkuiWindowHelper::DecorationComponents components)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_decoration_components", (uint32_t)components);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_decoration_components", (uint32_t)components);
    }
}

void UkuiWindowHelperPrivate::requestGetCurrentScreen()
{
    if (!initUkuiShellV1Interface())
        ukui_shell_v1_get_current_output(m_ukuiShellv1);
}

QScreen *UkuiWindowHelperPrivate::currentScreen() const
{
    return m_currentScreen;
}

void UkuiWindowHelperPrivate::setOpaqueRegion(QRegion region)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_opaque_region", region);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_opaque_region", region);
    }
}

void UkuiWindowHelperPrivate::setRestoreGeometry(QRect geometry)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_restore_geometry", geometry);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_restore_geometry", geometry);
    }
}

void UkuiWindowHelperPrivate::ukui_shell_v1_current_output(void *data, ukui_shell_v1 *ukui_shell_v1, const char *output_name, const char *seat_name)
{
    UkuiWindowHelperPrivate *self = static_cast<UkuiWindowHelperPrivate *>(data);
    if (!self)
        return;

    self->m_currentOutputName = QString::fromUtf8(output_name);
    self->m_currentSeatName = QString::fromUtf8(seat_name);
}

void UkuiWindowHelperPrivate::ukui_shell_v1_done(void *data, ukui_shell_v1 *ukui_shell_v1)
{
    UkuiWindowHelperPrivate *self = static_cast<UkuiWindowHelperPrivate *>(data);
    if (!self)
        return;

    for (QScreen *screen : QGuiApplication::screens()) {
        if (screen->name() == self->m_currentOutputName) {
            self->m_currentScreen = screen;
            Q_EMIT self->m_windowHelper->currentScreenChanged(screen);
            return;
        }
    }
}

ukui_shell_v1_listener UkuiWindowHelperPrivate::s_ukui_shell_v1_listener = {
    &UkuiWindowHelperPrivate::ukui_shell_v1_current_output,
    &UkuiWindowHelperPrivate::ukui_shell_v1_done};

bool UkuiWindowHelperPrivate::initUkuiShellV1Interface()
{
    if (m_ukuiShellv1) {
        return false;
    }

    m_ukuiShellv1 = static_cast<ukui_shell_v1 *>(ukuiDisplay()->bind(ukui_shell_v1_interface.name, &ukui_shell_v1_interface, ukui_shell_v1_interface.version));

    if (m_ukuiShellv1) {
        ukui_shell_v1_add_listener(m_ukuiShellv1, &s_ukui_shell_v1_listener, this);
    }

    return true;
}

UkuiWindowHelper::UkuiWindowHelper(QWidget *widget)
    : p(new UkuiWindowHelperPrivate(this, widget))
{
    connect(widget, &QObject::destroyed, this, &QObject::deleteLater);
}

UkuiWindowHelper::UkuiWindowHelper(QWindow *window)
    : p(new UkuiWindowHelperPrivate(this, window))
{
    connect(window, &QObject::destroyed, this, &QObject::deleteLater);
}

UkuiWindowHelper::~UkuiWindowHelper()
{
    delete p;
    p = nullptr;
}

void UkuiWindowHelper::removeTitleBar()
{
    p->removeTitleBar();
}

void UkuiWindowHelper::setWindowRole(WindowRole role)
{
    p->setWindowRole(role);
}

void UkuiWindowHelper::setWindowState(States states)
{
    p->setWindowState(states);
}

void UkuiWindowHelper::setWindowIcon(const QString &iconName)
{
    p->setWindowIcon(iconName);
}

void UkuiWindowHelper::setSkipTaskBar(bool skip)
{
    p->setSkipTaskBar(skip);
}

void UkuiWindowHelper::setSkipSwitcher(bool skip)
{
    p->setSkipSwitcher(skip);
}

void UkuiWindowHelper::setPanelAutoHide(bool autoHide)
{
    p->setPanelAutoHide(autoHide);
}

void UkuiWindowHelper::grabKeyboard()
{
    p->grabKeyboard();
}

void UkuiWindowHelper::openUnderCursor(QPoint pos)
{
    p->openUnderCursor(pos);
}

void UkuiWindowHelper::setBlurEffect(QRegion region, int level, bool enable)
{
    p->setBlurEffect(region, level, enable);
}

void UkuiWindowHelper::setSlideEffect(Position position, int offset, bool enable)
{
    p->setSlideEffect(position, offset, enable);
}

UkuiWindowHelper::States UkuiWindowHelper::defaultState() const
{
    return UkuiWindowHelper::States(0xff);
}

void UkuiWindowHelper::showTileFlyout()
{
    p->showTileFlyout(QRect());
}

void UkuiWindowHelper::showTileFlyout(QRect rect)
{
    p->showTileFlyout(rect);
}

void UkuiWindowHelper::setDecorationState(DecorationStates state)
{
    p->setDecorationCompoents((DecorationComponents)(int)state);
}

void UkuiWindowHelper::setDecorationCompoents(DecorationComponents components)
{
    p->setDecorationCompoents(components);
}

void UkuiWindowHelper::requestGetCurrentScreen()
{
    p->requestGetCurrentScreen();
}

QScreen *UkuiWindowHelper::currentScreen() const
{
    return p->currentScreen();
}

QScreen *UkuiWindowHelper::getCurrentScreenBlocking()
{
    if (QGuiApplication::platformName() != "wayland") {
        qWarning() << "getCurrentScreenBlocking() is only available on Wayland!";
        return nullptr;
    }

    p->requestGetCurrentScreen();
    wl_display_roundtrip(ukuiDisplay()->display());
    return p->currentScreen();
}

void UkuiWindowHelper::setOpaqueRegion(QRegion region)
{
    p->setOpaqueRegion(region);
}

void UkuiWindowHelper::setRestoreGeometry(QRect geometry)
{
    p->setRestoreGeometry(geometry);
}

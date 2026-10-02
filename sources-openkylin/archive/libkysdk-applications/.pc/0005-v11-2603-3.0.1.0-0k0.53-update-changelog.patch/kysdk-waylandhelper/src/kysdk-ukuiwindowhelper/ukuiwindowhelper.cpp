#include "ukuiwindowhelper.h"

#include "wayland-ukui-shell-v1-client-protocol.h"

#include <QDebug>
#include <QGuiApplication>
#include <QLibrary>
#include <QPair>
#include <QVariant>
#include <QWidget>
#include <QWindow>

ukui_surface_v1 *getUkuiSurfaceV1Object(QWindow *window)
{
    if (!window)
        return nullptr;

    void *native = qApp->platformNativeInterface();
    if (!native) {
        qWarning() << "Failed to get QPlatformNativeInterface";
        return nullptr;
    }

    using nativeResourceForWindowFunPtr = void *(*)(void *, QByteArray const &, QWindow *);
    auto fun = (nativeResourceForWindowFunPtr)QLibrary::resolve("libQt5WaylandClient.so.5", "_ZN15QtWaylandClient23QWaylandNativeInterface23nativeResourceForWindowERK10QByteArrayP7QWindow");
    if (!fun) {
        qWarning() << "Failed to resolve nativeResourceForIntegration function";
        return nullptr;
    }

    return (ukui_surface_v1 *)fun(native, QByteArrayLiteral("ukui_surface_v1"), window);
}

class UkuiWindowHelperPrivate
{
public:
    explicit UkuiWindowHelperPrivate(QWidget *widget);
    explicit UkuiWindowHelperPrivate(QWindow *window);

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

private:
    QWidget *m_widget = nullptr;
    QWindow *m_window = nullptr;
    ukui_surface_v1 *m_ukuiSurfacev1 = nullptr;

    friend class UkuiWindowHelper;
};

UkuiWindowHelperPrivate::UkuiWindowHelperPrivate(QWidget *widget)
    : m_widget(widget)
{
    if (!m_widget) {
        qWarning() << "Creating a UkuiWindowHelperPrivate for a null QWidget(nullptr)!";
    }
}

UkuiWindowHelperPrivate::UkuiWindowHelperPrivate(QWindow *window)
    : m_window(window)
{
    if (!m_window) {
        qWarning() << "Creating a UkuiWindowHelperPrivate for a QWindow(nullptr)!";
    }
}

void UkuiWindowHelperPrivate::removeTitleBar()
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_no_titlebar", true);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_no_titlebar", true);
    }
}

void UkuiWindowHelperPrivate::setWindowRole(UkuiWindowHelper::WindowRole role)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_role_v1", (int)role);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_role_v1", (int)role);
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
}

void UkuiWindowHelperPrivate::setSkipSwitcher(bool skip)
{
    if (m_widget) {
        m_widget->setProperty("ukui_surface_skip_switcher", skip);
    } else if (m_window) {
        m_window->setProperty("ukui_surface_skip_switcher", skip);
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
}

void UkuiWindowHelperPrivate::setSlideEffect(UkuiWindowHelper::Position position, int offset, bool enable)
{
    QPair<int, int> pair((int)position, offset);

    if (!enable)
        pair.second = -1;

    if (m_widget) {
        m_widget->setProperty("ukui_surface_slide", QVariant::fromValue(pair));
    } else if (m_window) {
        m_window->setProperty("ukui_surface_slide", QVariant::fromValue(pair));
    }
}

void UkuiWindowHelperPrivate::showTileFlyout(QRect rect)
{
    if (qApp->platformName() != "wayland") {
        qWarning() << "showTileFlyout() is only available on Wayland!";
        return;
    }

    if (!m_ukuiSurfacev1)
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

UkuiWindowHelper::UkuiWindowHelper(QWidget *widget)
    : p(new UkuiWindowHelperPrivate(widget))
{
    connect(widget, &QObject::destroyed, this, &QObject::deleteLater);
}

UkuiWindowHelper::UkuiWindowHelper(QWindow *window)
    : p(new UkuiWindowHelperPrivate(window))
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

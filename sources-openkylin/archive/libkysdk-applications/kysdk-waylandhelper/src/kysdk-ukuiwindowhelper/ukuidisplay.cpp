#include "ukuidisplay_p.h"

#include <QDebug>

#include <QGuiApplication>
#include <QLibrary>

static UkuiDisplay *s_ukuiDisplay = nullptr;
UkuiDisplay *ukuiDisplay()
{
    if (!s_ukuiDisplay) {
        s_ukuiDisplay = new UkuiDisplay();
    }

    if (!s_ukuiDisplay->m_registry) {
        qWarning() << "UkuiDisplay init failed";
        delete s_ukuiDisplay;
        s_ukuiDisplay = nullptr;
    }

    return s_ukuiDisplay;
}

void UkuiDisplay::handle_global(void *data, struct ::wl_registry *object, uint32_t name, const char *interface, uint32_t version)
{
    Q_UNUSED(object);
    static_cast<UkuiDisplay *>(data)->handle_global(name, QString::fromUtf8(interface), version);
}

void UkuiDisplay::handle_global_remove(void *data, struct ::wl_registry *object, uint32_t name)
{
    Q_UNUSED(object);
    static_cast<UkuiDisplay *>(data)->handle_global_remove(name);
}

const wl_registry_listener UkuiDisplay::s_wl_registry_listener = {
    UkuiDisplay::handle_global,
    UkuiDisplay::handle_global_remove};

UkuiDisplay::UkuiDisplay(QObject *parent)
    : QObject(parent)
{
    void *native = qApp->platformNativeInterface();
    if (!native) {
        qWarning() << "Failed to get QPlatformNativeInterface";
        return;
    }

    using nativeResourceForIntegrationFunPtr = void *(*)(void *, QByteArray const &);
    auto fun = (nativeResourceForIntegrationFunPtr)QLibrary::resolve("libQt5WaylandClient.so.5", "_ZN15QtWaylandClient23QWaylandNativeInterface28nativeResourceForIntegrationERK10QByteArray");
    if (!fun) {
        qWarning() << "Failed to resolve nativeResourceForIntegration function";
        return;
    }

    m_display = reinterpret_cast<wl_display *>(fun(native, QByteArrayLiteral("wl_display")));

    if (!m_display) {
        qWarning() << "Failed to get wl_display";
        return;
    }

    m_seat = reinterpret_cast<wl_seat *>(fun(native, QByteArrayLiteral("wl_seat")));
    if (!m_seat) {
        qWarning() << "Failed to get wl_seat";
        return;
    }

    m_registry = wl_display_get_registry(m_display);
    if (!m_registry) {
        qWarning() << "Failed to get wl_registry";
        return;
    }

    wl_registry_add_listener(m_registry, &s_wl_registry_listener, this);

    // roundtrip to get all global interfaces
    wl_display_roundtrip(m_display);
}

UkuiDisplay::~UkuiDisplay()
{
    if (m_registry) {
        wl_registry_destroy(m_registry);
        m_registry = nullptr;
    }
}

wl_display *UkuiDisplay::display() const
{
    return m_display;
}

wl_seat *UkuiDisplay::seat() const
{
    return m_seat;
}

void UkuiDisplay::handle_global(uint32_t name, const QString &interface, uint32_t version)
{
    m_globalInterfaces.append({name, interface, version});
}

void UkuiDisplay::handle_global_remove(uint32_t name)
{
    for (int i = 0; i < m_globalInterfaces.size(); ++i) {
        if (m_globalInterfaces.at(i).name == name) {
            m_globalInterfaces.removeAt(i);
            break;
        }
    }
}

QList<UkuiDisplay::WaylandInterface> UkuiDisplay::globalInterfaces()
{
    return m_globalInterfaces;
}

void *UkuiDisplay::bind(const char *name, const wl_interface *t, uint32_t minVersion)
{
    for (const WaylandInterface &interface : m_globalInterfaces) {
        if (interface.interface == QString::fromUtf8(name)) {
            return wl_registry_bind(m_registry, interface.name, t, qMin(interface.version, minVersion));
        }
    }
    return nullptr;
}

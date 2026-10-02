#include <QObject>

#include <wayland-client.h>

class UkuiDisplay;

UkuiDisplay *ukuiDisplay();

class UkuiDisplay : public QObject
{
    Q_OBJECT
public:
    ~UkuiDisplay();

    wl_display *display() const;
    wl_seat *seat() const;

    void *bind(const char *name, const wl_interface *t, uint32_t minVersion = 1);

private:
    static const wl_registry_listener s_wl_registry_listener;
    static void handle_global(void *data, ::wl_registry *object, uint32_t name, const char *interface, uint32_t version);
    static void handle_global_remove(void *data, ::wl_registry *object, uint32_t name);

    explicit UkuiDisplay(QObject *parent = nullptr);

    void handle_global(uint32_t name, const QString &interface, uint32_t version);
    void handle_global_remove(uint32_t name);

    struct WaylandInterface
    {
        uint32_t name;
        QString interface;
        uint32_t version;
    };
    QList<WaylandInterface> globalInterfaces();

    wl_display *m_display = nullptr;
    wl_seat *m_seat = nullptr;
    wl_registry *m_registry = nullptr;

    QList<WaylandInterface> m_globalInterfaces;

    friend UkuiDisplay * ::ukuiDisplay();
};

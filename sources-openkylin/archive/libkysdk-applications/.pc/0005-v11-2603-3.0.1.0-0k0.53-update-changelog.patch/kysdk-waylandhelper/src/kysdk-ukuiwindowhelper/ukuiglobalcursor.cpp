#include "ukuiglobalcursor.h"
#include "ukuidisplay_p.h"

#include "wayland-kywc-capture-v1-client-protocol.h"

#include <QDebug>

class UkuiGolbalCursorPrivate
{
public:
    UkuiGolbalCursorPrivate();
    ~UkuiGolbalCursorPrivate();

    std::function<void()> enter;
    std::function<void()> leave;
    std::function<void(int, int)> position;
    std::function<void(int, int)> hotspot;

private:
    static kywc_capture_cursor_v1_listener s_kywc_capture_cursor_v1_listener;

    static void handle_enter(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1);
    static void handle_leave(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1);
    static void handle_position(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1, int32_t x, int32_t y);
    static void handle_hotspot(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1, int32_t x, int32_t y);

    kywc_capture_manager_v1 *m_captureManager = nullptr;
    kywc_capture_cursor_v1 *m_captureCursor = nullptr;
};

kywc_capture_cursor_v1_listener UkuiGolbalCursorPrivate::s_kywc_capture_cursor_v1_listener = {
    UkuiGolbalCursorPrivate::handle_enter,
    UkuiGolbalCursorPrivate::handle_leave,
    UkuiGolbalCursorPrivate::handle_position,
    UkuiGolbalCursorPrivate::handle_hotspot,
};

UkuiGolbalCursorPrivate::UkuiGolbalCursorPrivate()
{
    m_captureManager = static_cast<kywc_capture_manager_v1 *>(ukuiDisplay()->bind(kywc_capture_manager_v1_interface.name, &kywc_capture_manager_v1_interface, kywc_capture_manager_v1_interface.version));
    if (!m_captureManager) {
        qWarning() << "Failed to bind kywc_capture_manager_v1";
        return;
    }
    m_captureCursor = kywc_capture_manager_v1_capture_cursor(m_captureManager, ukuiDisplay()->seat(), 0);
    if (!m_captureCursor) {
        qWarning() << "Failed to capture cursor";
        return;
    }
    kywc_capture_cursor_v1_add_listener(m_captureCursor, &s_kywc_capture_cursor_v1_listener, this);
}

UkuiGolbalCursorPrivate::~UkuiGolbalCursorPrivate()
{
    if (m_captureCursor) {
        kywc_capture_cursor_v1_destroy(m_captureCursor);
        m_captureCursor = nullptr;
    }
    if (m_captureManager) {
        kywc_capture_manager_v1_destroy(m_captureManager);
        m_captureManager = nullptr;
    }
}

void UkuiGolbalCursorPrivate::handle_enter(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1)
{
    UkuiGolbalCursorPrivate *self = static_cast<UkuiGolbalCursorPrivate *>(data);
    if (self->enter) {
        self->enter();
    }
}

void UkuiGolbalCursorPrivate::handle_leave(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1)
{
    UkuiGolbalCursorPrivate *self = static_cast<UkuiGolbalCursorPrivate *>(data);
    if (self->leave) {
        self->leave();
    }
}

void UkuiGolbalCursorPrivate::handle_position(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1, int32_t x, int32_t y)
{
    UkuiGolbalCursorPrivate *self = static_cast<UkuiGolbalCursorPrivate *>(data);
    if (self->position) {
        self->position(x, y);
    }
}

void UkuiGolbalCursorPrivate::handle_hotspot(void *data, struct kywc_capture_cursor_v1 *kywc_capture_cursor_v1, int32_t x, int32_t y)
{
    UkuiGolbalCursorPrivate *self = static_cast<UkuiGolbalCursorPrivate *>(data);
    if (self->hotspot) {
        self->hotspot(x, y);
    }
}

UkuiGolbalCursor *ukuiGolbalCursor()
{
    static UkuiGolbalCursor *instance = nullptr;
    if (!instance) {
        instance = new UkuiGolbalCursor();
    }
    return instance;
}

UkuiGolbalCursor::UkuiGolbalCursor()
    : m_ukuiGolbalCursorPrivate(new UkuiGolbalCursorPrivate())
{
}

UkuiGolbalCursor::~UkuiGolbalCursor()
{
    delete m_ukuiGolbalCursorPrivate;
    m_ukuiGolbalCursorPrivate = nullptr;
}

std::function<void()> &UkuiGolbalCursor::on_enter()
{
    return m_ukuiGolbalCursorPrivate->enter;
}

std::function<void()> &UkuiGolbalCursor::on_leave()
{
    return m_ukuiGolbalCursorPrivate->leave;
}

std::function<void(int, int)> &UkuiGolbalCursor::on_position()
{
    return m_ukuiGolbalCursorPrivate->position;
}

std::function<void(int, int)> &UkuiGolbalCursor::on_hotspot()
{
    return m_ukuiGolbalCursorPrivate->hotspot;
}
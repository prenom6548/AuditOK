#include "ukuivirtualkeyboard.h"
#include "ukuidisplay_p.h"

#include "wayland-virtual-keyboard-unstable-v1-client-protocol.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <QDebug>

class UkuiVirtualKeyboardPrivate
{
public:
    explicit UkuiVirtualKeyboardPrivate(const char *keymap);
    ~UkuiVirtualKeyboardPrivate();

    void keyPress(uint32_t key);
    void keyRelease(uint32_t key);
    void modifiers(uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked);

private:
    zwp_virtual_keyboard_manager_v1 *m_virtualKeyboardManager = nullptr;
    zwp_virtual_keyboard_v1 *m_virtualKeyboard = nullptr;

    uint32_t symToKeyCode(uint32_t sym) const;
};

UkuiVirtualKeyboardPrivate::UkuiVirtualKeyboardPrivate(const char *keymap)
{
    if (strlen(keymap) < 1) {
        qWarning() << "Keymap is empty";
        return;
    }

    m_virtualKeyboardManager = static_cast<zwp_virtual_keyboard_manager_v1 *>(ukuiDisplay()->bind(zwp_virtual_keyboard_manager_v1_interface.name, &zwp_virtual_keyboard_manager_v1_interface, zwp_virtual_keyboard_manager_v1_interface.version));
    if (!m_virtualKeyboardManager) {
        qWarning() << "Failed to bind zwp_virtual_keyboard_manager_v1";
        return;
    }
    m_virtualKeyboard = zwp_virtual_keyboard_manager_v1_create_virtual_keyboard(m_virtualKeyboardManager, ukuiDisplay()->seat());
    std::string keymap_file = "/tmp/waylandpp-XXXXXX";
    int fd = mkstemp(const_cast<char *>(keymap_file.c_str()));
    if (fd == -1) {
        qWarning() << "Failed to create keymap file";
        return;
    }
    unlink(keymap_file.c_str());
    FILE *f = fdopen(fd, "w");
    fprintf(f, "%s", keymap);
    fflush(f);

    size_t keymap_size = ftell(f);

    zwp_virtual_keyboard_v1_keymap(m_virtualKeyboard, 0, fd, keymap_size);
    fclose(f);
}

UkuiVirtualKeyboardPrivate::~UkuiVirtualKeyboardPrivate()
{
    if (m_virtualKeyboard) {
        zwp_virtual_keyboard_v1_destroy(m_virtualKeyboard);
        m_virtualKeyboard = nullptr;
    }
    if (m_virtualKeyboardManager) {
        zwp_virtual_keyboard_manager_v1_destroy(m_virtualKeyboardManager);
        m_virtualKeyboardManager = nullptr;
    }
}

void UkuiVirtualKeyboardPrivate::keyPress(uint32_t key)
{
    if (!m_virtualKeyboard) {
        qWarning() << "Virtual keyboard is not initialized";
        return;
    }
    zwp_virtual_keyboard_v1_key(m_virtualKeyboard, 0, key, 1);
}

void UkuiVirtualKeyboardPrivate::keyRelease(uint32_t key)
{
    if (!m_virtualKeyboard) {
        qWarning() << "Virtual keyboard is not initialized";
        return;
    }
    zwp_virtual_keyboard_v1_key(m_virtualKeyboard, 0, key, 0);
}

void UkuiVirtualKeyboardPrivate::modifiers(uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked)
{
    if (!m_virtualKeyboard) {
        qWarning() << "Virtual keyboard is not initialized";
        return;
    }
    zwp_virtual_keyboard_v1_modifiers(m_virtualKeyboard, mods_depressed, mods_latched, mods_locked, 0);
}

UkuiVirtualKeyboard::UkuiVirtualKeyboard(const char *keymap)
    : m_ukuiVirtualKeyboardPrivate(new UkuiVirtualKeyboardPrivate(keymap))
{
}

UkuiVirtualKeyboard::~UkuiVirtualKeyboard()
{
    delete m_ukuiVirtualKeyboardPrivate;
    m_ukuiVirtualKeyboardPrivate = nullptr;
}

void UkuiVirtualKeyboard::keyPress(uint32_t key)
{
    if (m_ukuiVirtualKeyboardPrivate) {
        m_ukuiVirtualKeyboardPrivate->keyPress(key);
    }
}

void UkuiVirtualKeyboard::keyRelease(uint32_t key)
{
    if (m_ukuiVirtualKeyboardPrivate) {
        m_ukuiVirtualKeyboardPrivate->keyRelease(key);
    }
}

void UkuiVirtualKeyboard::modifiers(uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked)
{
    if (m_ukuiVirtualKeyboardPrivate) {
        m_ukuiVirtualKeyboardPrivate->modifiers(mods_depressed, mods_latched, mods_locked);
    }
}

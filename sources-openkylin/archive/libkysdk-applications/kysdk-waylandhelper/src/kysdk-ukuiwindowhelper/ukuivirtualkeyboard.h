#pragma once

#include <cstdint>
#include <functional>

class UkuiVirtualKeyboard;
class UkuiVirtualKeyboardPrivate;

class UkuiVirtualKeyboard
{
public:
    /**
     * @brief Constructor for UkuiVirtualKeyboard
     * @param keymap The keymap to use for the virtual keyboard
     */
    UkuiVirtualKeyboard(const char *keymap);

    ~UkuiVirtualKeyboard();

    /**
     * @brief Press a key on the virtual keyboard
     * @param key The key to press
     */
    void keyPress(uint32_t key);

    /**
     * @brief Release a key on the virtual keyboard
     * @param key The key to release
     */
    void keyRelease(uint32_t key);

    /**
     * @brief Set the modifiers on the virtual keyboard
     * @param mods_depressed The modifiers that are currently depressed
     * @param mods_latched The modifiers that are latched
     * @param mods_locked The modifiers that are locked
     */
    void modifiers(uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked);

private:
    UkuiVirtualKeyboardPrivate *m_ukuiVirtualKeyboardPrivate = nullptr;
};

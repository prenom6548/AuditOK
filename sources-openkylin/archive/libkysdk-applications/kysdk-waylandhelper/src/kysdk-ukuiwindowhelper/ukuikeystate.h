#pragma once

#include <cstdint>
#include <functional>

class UkuiKeyState;
class UkuiKeyStatePrivate;

UkuiKeyState *ukuiKeyState();

class UkuiKeyState
{
public:
    enum Key {
        CapsLock = 0,
        NumLock = 1,
        ScrollLock = 2,
        Alt = 3,
        Control = 4,
        Shift = 5,
        Meta = 6,
        AltGr = 7,
    };

    enum State {
        Unlocked = 0,
        Latched = 1,
        Locked = 2,
        Pressed = 3,
    };

    ~UkuiKeyState();

    /*
     * @brief fetch the current state of all keys
     */
    void fetchStates();

    /*
     * @brief handler for key state changed
     * @param1  The key that changed state
     * @param2  The new state of the key
     */
    std::function<void(Key /* key */, State /* key */)> &on_state_changed();

private:
    UkuiKeyStatePrivate *m_ukuiKeyStatePrivate = nullptr;

    explicit UkuiKeyState();
    friend UkuiKeyState *ukuiKeyState();
};

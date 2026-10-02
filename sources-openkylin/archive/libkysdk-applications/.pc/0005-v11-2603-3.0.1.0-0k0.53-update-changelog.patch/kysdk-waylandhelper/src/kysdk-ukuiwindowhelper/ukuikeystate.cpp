#include "ukuikeystate.h"
#include "ukuidisplay_p.h"

#include "wayland-keystate-client-protocol.h"

#include <QDebug>

class UkuiKeyStatePrivate
{
public:
    UkuiKeyStatePrivate();
    ~UkuiKeyStatePrivate();

    void fetchStates();
    std::function<void(UkuiKeyState::Key, UkuiKeyState::State)> stateChanged;

private:
    static org_kde_kwin_keystate_listener s_org_kde_kwin_keystate_listener;
    static void handle_state_changed(void *data, struct org_kde_kwin_keystate *org_kde_kwin_keystate, uint32_t key, uint32_t state);

    org_kde_kwin_keystate *m_keystate = nullptr;
};

org_kde_kwin_keystate_listener UkuiKeyStatePrivate::s_org_kde_kwin_keystate_listener = {
    UkuiKeyStatePrivate::handle_state_changed,
};

UkuiKeyStatePrivate::UkuiKeyStatePrivate()
{
    m_keystate = static_cast<org_kde_kwin_keystate *>(ukuiDisplay()->bind(org_kde_kwin_keystate_interface.name, &org_kde_kwin_keystate_interface, org_kde_kwin_keystate_interface.version));
    if (!m_keystate) {
        qWarning() << "Failed to bind org_kde_kwin_keystate";
        return;
    }
    org_kde_kwin_keystate_add_listener(m_keystate, &s_org_kde_kwin_keystate_listener, this);
}

UkuiKeyStatePrivate::~UkuiKeyStatePrivate()
{
    org_kde_kwin_keystate_destroy(m_keystate);
    m_keystate = nullptr;
}

void UkuiKeyStatePrivate::fetchStates()
{
    if (m_keystate) {
        org_kde_kwin_keystate_fetchStates(m_keystate);
    }
}

void UkuiKeyStatePrivate::handle_state_changed(void *data, struct org_kde_kwin_keystate *org_kde_kwin_keystate, uint32_t key, uint32_t state)
{
    UkuiKeyStatePrivate *self = static_cast<UkuiKeyStatePrivate *>(data);
    if (self->stateChanged) {
        self->stateChanged(UkuiKeyState::Key(key), UkuiKeyState::State(state));
    }
}

UkuiKeyState *ukuiKeyState()
{
    static UkuiKeyState *instance = nullptr;
    if (!instance) {
        instance = new UkuiKeyState();
    }
    return instance;
}

UkuiKeyState::UkuiKeyState()
    : m_ukuiKeyStatePrivate(new UkuiKeyStatePrivate())
{
}

UkuiKeyState::~UkuiKeyState()
{
    delete m_ukuiKeyStatePrivate;
    m_ukuiKeyStatePrivate = nullptr;
}

void UkuiKeyState::fetchStates()
{
    if (m_ukuiKeyStatePrivate) {
        m_ukuiKeyStatePrivate->fetchStates();
    }
}

std::function<void(UkuiKeyState::Key, UkuiKeyState::State)> &UkuiKeyState::on_state_changed()
{
    return m_ukuiKeyStatePrivate->stateChanged;
}

#include "capslockstatehelper.h"
#include <QDebug>

CapslockStateHelper *CapslockStateHelper::m_instance = nullptr;

CapslockStateHelper::CapslockStateHelper(QObject *parent)
    : QObject{parent}
{
    m_keyState = ukuiKeyState();
    m_keyState->fetchStates();
    m_keyState->on_state_changed() = [=](UkuiKeyState::Key key, UkuiKeyState::State state) {
        if (key == UkuiKeyState::CapsLock) {
            qDebug() << " CapsLock changed !!! " << " state = " << state;
            Q_EMIT capslockChanged(state);
        }
    };
}

CapslockStateHelper *CapslockStateHelper::instance(QObject *parent)
{
    if (m_instance == nullptr)
        m_instance = new CapslockStateHelper(parent);
    return m_instance;
}

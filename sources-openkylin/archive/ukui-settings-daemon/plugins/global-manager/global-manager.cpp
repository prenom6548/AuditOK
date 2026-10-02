#include "global-manager.h"

GlobalManager *GlobalManager::m_globalManager = nullptr;

GlobalManager::GlobalManager()
{
    m_pBright = new Brightness();
    m_pSignal = new GlobalSignal();
    QDBusConnection sessionBug = QDBusConnection::sessionBus();
    if (sessionBug.registerService("org.ukui.SettingsDaemon")) {
        sessionBug.registerObject(DBUS_GC_BRIGHTNESS_PATH, m_pBright,
                                 QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals
                                 );

        sessionBug.registerObject("/GlobalSignal", m_pSignal,
                                 QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals
                                 );

    }
}

GlobalManager::~GlobalManager()
{
    if (m_pBright != nullptr) {
        delete m_pBright;
        m_pBright = nullptr;
    }

    if (m_pSignal != nullptr) {
        delete m_pSignal;
        m_pSignal = nullptr;
    }
}

GlobalManager *GlobalManager::GlobalManagerNew()
{
    if (m_globalManager == nullptr) {
        m_globalManager = new GlobalManager();
    }

    return m_globalManager;
}

void GlobalManager::start()
{

}

void GlobalManager::stop()
{

}

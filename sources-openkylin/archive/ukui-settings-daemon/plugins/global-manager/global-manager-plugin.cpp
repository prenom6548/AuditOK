#include "global-manager-plugin.h"

PluginInterface *GlobalManagerPlugin::m_Instance = nullptr;
GlobalManager *GlobalManagerPlugin::m_pManager = nullptr;

GlobalManagerPlugin::~GlobalManagerPlugin()
{
    if (m_pManager != nullptr) {
        delete m_pManager;
        m_pManager = nullptr;
    }
}

GlobalManagerPlugin::GlobalManagerPlugin()
{
    if (m_pManager == nullptr) {
        m_pManager = GlobalManager::GlobalManagerNew();
    }
}

PluginInterface *GlobalManagerPlugin::getInstance()
{
    if (nullptr == m_Instance) {
        m_Instance = new GlobalManagerPlugin();
    }
    return m_Instance;
}

void GlobalManagerPlugin::activate()
{
    m_pManager->start();
}

void GlobalManagerPlugin::deactivate()
{
    m_pManager->stop();
}

PluginInterface *createSettingsPlugin()
{
    return GlobalManagerPlugin::getInstance();
}

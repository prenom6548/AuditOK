#include "global-manager-plugin.h"

PluginInterface *GlobalManagerPlugin::m_Instance = nullptr;
GlobalManager *GlobalManagerPlugin::m_pManager = nullptr;

GlobalManagerPlugin::~GlobalManagerPlugin()
{

}

GlobalManagerPlugin::GlobalManagerPlugin()
{
    if (m_pManager == nullptr) {
        m_pManager = new GlobalManager();
    }
}

PluginInterface *GlobalManagerPlugin::getInstance()
{
    if (nullptr == m_Instance) {
        m_Instance = new GlobalManagerPlugin();
    }
    return m_Instance;
}

PluginInterface *createSettingsPlugin()
{
    return GlobalManagerPlugin::getInstance();
}

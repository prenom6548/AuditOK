#include "displayeffectmanager.h"
#include "loghelper.h"
#include "schedpolicy.h"
#include <memory>

DisplayEffectManager *DisplayEffectManager::instance = nullptr;
DisplayEffectManager* DisplayEffectManager::getInstance()
{
    // TODO: 在此处插入 return 语句
    if (instance == nullptr) {
        instance = new DisplayEffectManager();
    }

    return instance;
}

DisplayEffectManager::DisplayEffectManager()
    : m_effectDbusInterface(std::make_unique<EffectDbusInterface>())
{
    m_EffectsStatus = true;
}

DisplayEffectManager::~DisplayEffectManager()
{
}


QString DisplayEffectManager::getCurrentMaintainMode()
{
    PRINT_DBG("DisplayEffectManager::getCurrentMaintainMode()");
    return m_effectDbusInterface->getCurrentMaintainMode();
}

void DisplayEffectManager::printEffectStatus()
{
    PRINT_DBG("DisplayEffectManager::printEffectStatus()");
    m_effectDbusInterface->printEffectStatus();
}


void DisplayEffectManager::suspendEffectEffects(std::vector<std::string> &filterList)
{
    PRINT_DBG("EffectDbusInterface::suspendEffectEffects\n");
    m_effectDbusInterface->suspendEffectEffects(filterList);
}

void DisplayEffectManager::resumeEffectEffects()
{
    PRINT_DBG("EffectDbusInterface::resumeEffectEffects\n");
    m_effectDbusInterface->resumeEffectEffects();
}
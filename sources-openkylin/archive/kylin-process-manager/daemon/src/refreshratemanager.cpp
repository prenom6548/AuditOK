#include "refreshratemanager.h"

RefreshRateManager::RefreshRateManager()
    : m_psrStateManager(new PSRStateManager())
    , m_vrrStateManager(new VRRStateManager())
{

}

void RefreshRateManager::openPSR()
{
    PRINT_DBG("RefreshRateManager::openPSR()\n");
    m_psrStateManager->openPSR();
}

void RefreshRateManager::closePSR()
{
    PRINT_DBG("RefreshRateManager::closePSR()\n");
    m_psrStateManager->closePSR();
}

void RefreshRateManager::setVRRRefreshRate(std::string mode)
{
    PRINT_DBG("RefreshRateManager::setRefreshRate()\n");
    m_vrrStateManager->setVRRRefreshRate(mode);
}

void RefreshRateManager::reverseVRRRefreshRate()
{
    PRINT_DBG("RefreshRateManager::reverseRefreshRate()\n");
    m_vrrStateManager->reverseVRRRefreshRate();
}

bool RefreshRateManager::isSupportVRR() 
{
    return m_vrrStateManager->isSupportVRR();
}

bool RefreshRateManager::isSupportPSR() 
{
    return m_psrStateManager->isSupportPSR();
}
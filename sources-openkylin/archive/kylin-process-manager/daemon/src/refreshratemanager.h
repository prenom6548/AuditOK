#ifndef REFRESHRATEMANAGER_H
#define REFRESHRATEMANAGER_H

#include <memory>
#include "psrstatemanagerinterface.h"
#include "vrrstatemanagerinterface.h"
#include "loghelper.h"

class RefreshRateManager {
public:
    RefreshRateManager();
    ~RefreshRateManager() = default;

    void openPSR();
    void closePSR();
    bool isSupportPSR() ;

    void setVRRRefreshRate(std::string mode);
    void reverseVRRRefreshRate();
    bool isSupportVRR();

private:
    std::unique_ptr<PSRStateManager> m_psrStateManager;
    std::unique_ptr<VRRStateManager> m_vrrStateManager;

};

#endif
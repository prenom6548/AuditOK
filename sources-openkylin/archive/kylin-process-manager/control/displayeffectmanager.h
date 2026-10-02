#ifndef DISPLAYEFFECTMANAGER_H
#define DISPLAYEFFECTMANAGER_H
#include "displayeffectdbusinterface.h"
#include <mutex>
#include <memory>

enum DISPLAY_CONTROL_TYPE 
{
    DISPLAY_COMPOSITOR,
    DISPLAY_EFFECTS,
    DISPLAY_NONE
};

class DisplayEffectManager {
public:
    ~DisplayEffectManager();
    static DisplayEffectManager* getInstance();
    void printEffectStatus();
    void suspendEffectEffects(std::vector<std::string> &filterList);
    void resumeEffectEffects();
    QString getCurrentMaintainMode();

private:
    static DisplayEffectManager* instance;
    DisplayEffectManager();
    std::unique_ptr<EffectDbusInterface> m_effectDbusInterface;
    std::mutex m_EffectsMutex;
    bool m_EffectsStatus;

};


#endif
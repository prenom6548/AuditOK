#ifndef PULSEAUDIOMANAGERFACTORY_H
#define PULSEAUDIOMANAGERFACTORY_H

#include "ManagerFactory.h"

namespace UkuiAudioFramwork
{
class PulseaudioManagerFactory : public ManagerFactory
{
public:
    PulseaudioManagerFactory() = default;

    virtual auto createManager() -> std::shared_ptr<AudioManager> override;
};
}
#endif // PULSEAUDIOMANAGERFACTORY_H

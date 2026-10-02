#ifndef PIPEWIREMANAGERFACTORY_H
#define PIPEWIREMANAGERFACTORY_H

#include "ManagerFactory.h"

namespace UkuiAudioFramwork
{
class PipewireManagerFactory : public ManagerFactory
{
public:
    PipewireManagerFactory() = default;

    virtual auto createManager() -> std::shared_ptr<AudioManager> override;
};
}
#endif // PIPEWIREMANAGERFACTORY_H

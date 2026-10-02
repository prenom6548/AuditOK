#ifndef PULSEAUDIOBACKENDFACTORY_H
#define PULSEAUDIOBACKENDFACTORY_H

#include "BackendFactory.h"

namespace UkuiAudioFramwork
{
class PulseaudioBackendFactory : public BackendFactory
{
public:
    PulseaudioBackendFactory() = default;

    virtual auto createBackend() -> std::shared_ptr<IBackend> override;

    ~PulseaudioBackendFactory() = default;
};
}
#endif // PULSEAUDIOBACKENDFACTORY_H

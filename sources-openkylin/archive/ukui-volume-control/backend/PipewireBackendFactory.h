#ifndef PIPEWIREBACKENDFACTORY_H
#define PIPEWIREBACKENDFACTORY_H

#include "BackendFactory.h"

namespace UkuiAudioFramwork
{
class PipewireBackendFactory : public BackendFactory
{
public:
    PipewireBackendFactory();

    virtual auto createBackend() -> std::shared_ptr<IBackend> override;
};
}
#endif // PIPEWIREBACKENDFACTORY_H

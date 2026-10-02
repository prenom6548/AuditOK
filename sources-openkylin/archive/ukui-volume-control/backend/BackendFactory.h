#ifndef BACKENDFACTORY_H
#define BACKENDFACTORY_H

#include "IBackend.h"

namespace UkuiAudioFramwork
{
class BackendFactory
{
public:
    BackendFactory() = default;
    virtual ~BackendFactory() = default;

public:
    virtual std::shared_ptr<IBackend> createBackend() = 0;

private:
    BackendFactory(const BackendFactory&) = delete;
    BackendFactory(BackendFactory&&) = delete;
    void operator=(const BackendFactory&) = delete;
    void operator=(BackendFactory&&) = delete;
};
}
#endif // BACKENDFACTORY_H

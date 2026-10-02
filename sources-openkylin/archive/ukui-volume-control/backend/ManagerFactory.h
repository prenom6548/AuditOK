#ifndef MANAGERFACTORY_H
#define MANAGERFACTORY_H

#include <memory>

#include "AudioManager.h"

namespace UkuiAudioFramwork
{
class ManagerFactory
{
public:
    ManagerFactory() = default;
    virtual ~ManagerFactory() = default;

public:
    virtual std::shared_ptr<AudioManager> createManager() = 0;

private:
    ManagerFactory(const ManagerFactory&) = delete;
    ManagerFactory(ManagerFactory&&) = delete;
    void operator=(const ManagerFactory&) = delete;
    void operator=(ManagerFactory&&) = delete;
};
}
#endif // MANAGERFACTORY_H

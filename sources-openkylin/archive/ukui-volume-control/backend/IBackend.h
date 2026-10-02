#ifndef BACKEND_H
#define BACKEND_H

#include <memory>
#include "AudioManager.h"

namespace UkuiAudioFramwork
{
class IBackend
{
public:
    IBackend() = default;
    virtual ~IBackend() = default;

public:
    virtual void init() = 0;

    virtual bool detect() = 0;

    //    virtual auto getManager() -> std::unique_ptr<AudioManager> = 0;

private:
    IBackend(const IBackend&) = delete;
    IBackend(IBackend&&) = delete;
    void operator=(const IBackend&) = delete;
    void operator=(IBackend&&) = delete;
};
}
#endif // IBACKEND_H

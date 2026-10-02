#ifndef PULSEAUDIOBACKEND_H
#define PULSEAUDIOBACKEND_H

#include "IBackend.h"
#include "PulseaudioManager.h"

namespace UkuiAudioFramwork
{
class PulseaudioBackend : public IBackend
{
public:
    PulseaudioBackend();
    ~PulseaudioBackend() = default;

public:
    virtual void init();

    virtual bool detect();
//    virtual auto getManager() -> std::unique_ptr<AudioManager>;

private:
};
}
#endif // PULSEAUDIOBACKEND_H

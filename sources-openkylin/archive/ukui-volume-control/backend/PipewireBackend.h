#ifndef PIPEWIREBACKEND_H
#define PIPEWIREBACKEND_H

#include "IBackend.h"

namespace UkuiAudioFramwork
{
class PipewireBackend : public IBackend
{
public:
    PipewireBackend();
    ~PipewireBackend() = default;

public:
    virtual void init();

    virtual bool detect();
};
}
#endif // PIPEWIREBACKEND_H

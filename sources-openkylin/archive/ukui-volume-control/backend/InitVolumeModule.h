#ifndef INITVOLUMEMODULE_H
#define INITVOLUMEMODULE_H

#include "CustomAudioVolumeJson.h"

namespace UkuiAudioFramwork
{
class InitVolumeModule
{
public:
    static InitVolumeModule& getInstance();

    bool isNeedInitVolume(const std::string&, const std::string&);

    void insert(const std::string&, const std::string&, uint32_t);

private:
    InitVolumeModule() = default;
    InitVolumeModule(const InitVolumeModule&) = delete;
    InitVolumeModule(InitVolumeModule&&) = delete;
    InitVolumeModule operator=(const InitVolumeModule&) = delete;
    InitVolumeModule operator=(InitVolumeModule&&) = delete;
    virtual ~InitVolumeModule() = default;

private:
    CustomAudioVolumeJson m_json;
};
}
#endif // INITVOLUMEMODULE_H

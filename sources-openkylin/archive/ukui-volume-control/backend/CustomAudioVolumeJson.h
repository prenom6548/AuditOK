#ifndef CUSTOMAUDIOVOLUMEJSON_H
#define CUSTOMAUDIOVOLUMEJSON_H

#include <fstream>
#include <iomanip>

#include "IJson.h"

namespace UkuiAudioFramwork
{
#define CUSTOM_AUDIO_VOLUME_JSON  "/.config/ukui-volume-control/custom-audio-volume.json"

typedef struct AudioVolumeInfo
{
    std::string portName;
    std::string cardName;
    uint32_t volume;
}AudioVolumeInfo;

class CustomAudioVolumeJson : public IJson<AudioVolumeInfo>
{
public:
    CustomAudioVolumeJson();

public:
    virtual bool isContain(const AudioVolumeInfo&) const override;

    virtual void insert(const AudioVolumeInfo&) override;
};
}
#endif // CUSTOMAUDIOVOLUMEJSON_H

#ifndef MEDIAAUTOPAUSEMODULE_H
#define MEDIAAUTOPAUSEMODULE_H

#include "RestoreVolumeJson.h"

namespace UkuiAudioFramwork
{
class MediaAutoPauseModule
{
public:
    static MediaAutoPauseModule& getInstance();

    void setJson(std::shared_ptr<RestoreVolumeJson>);

    bool getAutoPauseStatus();

    void setAutoPauseStatus(bool);

private:
    MediaAutoPauseModule() = default;
    MediaAutoPauseModule(const MediaAutoPauseModule&) = delete;
    MediaAutoPauseModule(MediaAutoPauseModule&&) = delete;
    MediaAutoPauseModule operator=(const MediaAutoPauseModule&) = delete;
    MediaAutoPauseModule operator=(MediaAutoPauseModule&&) = delete;
    virtual ~MediaAutoPauseModule() = default;

private:
    std::shared_ptr<RestoreVolumeJson> m_pJson = nullptr;
};
}
#endif // MEDIAAUTOPAUSEMODULE_H

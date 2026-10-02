#ifndef DEVICEMANAGERMODULE_H
#define DEVICEMANAGERMODULE_H

#include "Util.h"
#include "AudioManager.h"
#include "RestoreVolumeJson.h"

namespace UkuiAudioFramwork
{
class AudioManager;

class DeviceManagerModule
{
public:
    static DeviceManagerModule& getInstance();

    void setManager(std::shared_ptr<AudioManager>);

    void setJson(std::shared_ptr<RestoreVolumeJson>);

    void setEnabled(const std::string&, const std::string&, bool);

    bool isEnabled(const std::string&, const std::string&) const;

private:
    DeviceManagerModule() = default;
    DeviceManagerModule(const DeviceManagerModule&) = delete;
    DeviceManagerModule(DeviceManagerModule&&) = delete;
    DeviceManagerModule operator=(const DeviceManagerModule&) = delete;
    DeviceManagerModule operator=(DeviceManagerModule&&) = delete;
    virtual ~DeviceManagerModule() = default;

private:
    std::shared_ptr<RestoreVolumeJson> m_pJson {};
    std::shared_ptr<AudioManager> m_pAudioManager {};
};
}
#endif // DEVICEMANAGERMODULE_H

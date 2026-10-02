#ifndef VOLUMECONTROL_H
#define VOLUMECONTROL_H

#include "BackendFactory.h"
#include "BaseType.h"
#include "PulseaudioBackendFactory.h"
#include "PipewireBackendFactory.h"
#include "ManagerFactory.h"
#include "PulseaudioManagerFactory.h"
#include "PipewireManagerFactory.h"
#include "AudioManager.h"
#include "AudioMethod.h"
#include "JackDetect.h"

namespace UkuiAudioFramwork
{
class VolumeControl
{
public:
    static VolumeControl& getInstance();

public:
    void init();


private:
    VolumeControl() = default;
    VolumeControl(const VolumeControl&) = delete;
    VolumeControl(VolumeControl&&) = delete;
    VolumeControl operator=(const VolumeControl&) = delete;
    VolumeControl operator=(VolumeControl&&) = delete;
    virtual ~VolumeControl() = default;

private:
    BackendType m_type = BackendType::PULSEAUDIO;
    std::shared_ptr<BackendFactory> m_pBackendFactory;
    std::shared_ptr<IBackend> m_pBackend;
    std::shared_ptr<ManagerFactory> m_pManagerFactory;
    std::shared_ptr<AudioManager> m_pAudioManager;

    std::unordered_map<ModuleType, std::vector<std::string>> m_settingsMap = {
        {ModuleType::MODULE_REMAP_SINK, {MONO_AUDIO_KEY, "module-remap-sink", "sink_name=mono channels=1 channel_map=mono"}},
        {ModuleType::MODULE_ECHO_CANCEL, {NOISE_REDUCTION_KEY, "module-echo-cancel", "use_master_format=1 aec_method=webrtc aec_args=analog_gain_control=0 source_name=noiseReduceSource"}},
        {ModuleType::MODULE_LOOPBACK ,{LOOPBACK_KEY, "module-loopback", ""}},
        {ModuleType::MODULE_SWITCH_ON_CONNECT, {SWITCH_ON_CONNECT_KEY, "module-switch-on-connect", ""}}
    };

    std::unordered_map<std::string, std::function<bool()>> m_statusFunctions = {
        {MONO_AUDIO_KEY, [this]() { return AudioMethod::getInstance().getMonoStatus(); }},
        {NOISE_REDUCTION_KEY, [this]() { return AudioMethod::getInstance().getEchoCancelStatus(); }},
        {LOOPBACK_KEY, [this]() { return AudioMethod::getInstance().getLoopbackStatus(); }},
        {SWITCH_ON_CONNECT_KEY, []() {return true;}}
    };
};
}
#endif // VOLUMECONTROL_H

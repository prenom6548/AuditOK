#ifndef AUDIOMETHOD_H
#define AUDIOMETHOD_H

#include <memory>
#include <optional>
#include <string_view>
#include <vector>

#include "BaseType.h"
#include "ISetting.h"
#include "AudioSetting.h"
#include "GlobalThemeSettings.h"
#include "MediaAutoPauseModule.h"
#include "SoundThemeModule.h"
#include "SoundThemePlayerSetting.h"
#include "SessionManagerSetting.h"
#include "AudioManager.h"
#include "DbusServer.h"
#include "MultiAudioCombineJson.h"

namespace UkuiAudioFramwork
{
class AudioDBusServer;

class AudioMethod
{
public:
    static AudioMethod& getInstance();

    void setManager(std::shared_ptr<AudioManager>);

    void init();

    void run();

    void showOsd(const std::string&);

    /**
    * @brief  获取当前桌面环境版本信息
    *
    * @return 返回当前桌面环境版本信息
    */
    DesktopEnvironmentVersion getDesktopEnvironmentVersion();

    /**
    * @brief  获取音频流的描述
    *
    * @param  name 对应的desktop名称
    * @return 音频流的描述
    */
    std::string getStreamDescription(const std::string&);

    /**
    * @brief  初始化音量
    *
    * @param  type      设备类型
    *         portName  端口名称
    *         cardName  声卡名称
    * @return 无
    */
    void initVolume(NodeType, const std::string&, const std::string&);

    // Interface
    /**
    * @brief  加载模块
    *
    * @param  name 模块名称，例module-echo-cancel
    *         param 模块参数
    * @return 无
    */
    void loadModule(const std::string&, const std::string&);

    /**
    * @brief  卸载模块
    *
    * @param  name 模块名称，例module-echo-cancel
    * @return 无
    */
    void unloadModule(const std::string&, bool restoreMasterDevice = true);

    // Method
    /**
    * @brief  获取音频后端名称
    *
    * @return 返回后端名
    */
    std::string getBackend() const;

    /**
    * @brief  设置音频后端
    *
    * @param  type 后端类型
    * @return 无
    */
    void setBackend(const BackendType&);

    /**
    * @brief  设置音量
    *
    * @param  type 设置音量的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         name 对应的名称（如果是设置INPUT和OUTPUT音量，该参数为需要获取端口的名称，例如analog-output-speaker；
    *              如果是设置SINK_INPUT和SOURCE_OUTPUT的音量，则将该参数设置成当前应用的名称）
    *         dev  设备名称（如果是设置INPUT和OUTPUT音量，该参数为对应端口的声卡名称；
    *              如果是设置SINK_INPUT和SOURCE_OUTPUT的音量，则该值可设置为任意值）
    *         volume 设置的音量值
    * @return 无
    */
    void setDetailVolume(const NodeType&, const std::string&, const std::string&, int);

    /**
    * @brief  获取音量
    *
    * @param  type 获取音量的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         name 对应的名称（如果是获取INPUT和OUTPUT音量，该参数为需要获取端口的名称，例如analog-output-speaker；
    *              如果是获取SINK_INPUT和SOURCE_OUTPUT的音量，则将该参数设置成当前应用的名称）
    *         dev  设备名称（如果是获取INPUT和OUTPUT音量，该参数为对应端口的声卡名称；
    *              如果是获取SINK_INPUT和SOURCE_OUTPUT的音量，则该值可设置为任意值）
    *         volume 设置的音量值
    * @return 无
    */
    int getDetailVolume(const NodeType&, const std::string&, const std::string&) const;

    /**
    * @brief  设置音量
    *
    * @param  type   设置音量的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         index  音频流的索引（如果是设置INPUT和OUTPUT音量，则该值可设置为任意值；
    *                如果是设置SINK_INPUT和SOURCE_OUTPUT的音量，则该值可设置为应用的索引）
    *         volume 设置的音量值
    * @return 无
    */
    void setVolume(const NodeType&, int, int);

    /**
    * @brief  获取音量
    *
    * @param  type   获取音量的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         index  音频流的索引（如果是设置INPUT和OUTPUT音量，则该值可设置为任意值；
    *                如果是获取SINK_INPUT和SOURCE_OUTPUT的音量，则将该参数设置成应用的索引）
    * @return 返回获取的音量
    */
    int getVolume(const NodeType&, int = -1) const;

    /**
    * @brief  设置平衡音量
    *
    * @param  type 设置音量的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         volume 设置的音量值
    * @return 无
    */
    void setBalance(const NodeType&, double);

    /**
    * @brief  获取平衡音量
    *
    * @param  type 获取音量的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    * @return 返回平衡音量
    */
    double getBalance(const NodeType&) const;

    /**
    * @brief  设置静音状态
    *
    * @param  type 设置静音状态的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         name 对应的名称（如果是设置INPUT和OUTPUT的静音状态，该参数可为任意值；
    *              如果是设置SINK_INPUT和SOURCE_OUTPUT的静音状态，则将该参数设置成当前应用的名称）
    *         dev  设备名称（如果是设置INPUT和OUTPUT的静音状态，该参数为对应端口的声卡名称；
    *              如果是设置SINK_INPUT和SOURCE_OUTPUT的静音状态，则该值可设置为任意值）
    *         mute 设置的静音状态
    * @return 无
    */
    void setDetailMute(const NodeType&, const std::string&, const std::string&, bool);

    /**
    * @brief  获取静音状态
    *
    * @param  type 获取静音的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         name 对应的名称（如果是获取INPUT和OUTPUT的静音状态，该参数为需要获取端口的名称，例如analog-output-speaker；
    *              如果是获取SINK_INPUT和SOURCE_OUTPUT的静音状态，则将该参数设置成当前应用的名称）
    *         dev  设备名称（如果是获取INPUT和OUTPUT的静音状态，该参数为对应端口的声卡名称；
    *              如果是获取SINK_INPUT和SOURCE_OUTPUT的静音状态，则该值可设置为任意值）
    * @return 返回对应的静音状态
    */
    bool getDetailMute(const NodeType&, const std::string&, const std::string&) const;

    /**
    * @brief  设置默认设备或应用的静音状态
    *
    * @param  type 设置静音状态的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         name 音频流的索引（如果是设置INPUT和OUTPUT音量，则该值可设置为任意值；
    *              如果是设置SINK_INPUT和SOURCE_OUTPUT的静音状态，则将该参数设置成当前应用的索引）
    *         mute 设置的静音状态
    * @return 无
    */
    void setMute(const NodeType&, int, bool);

    /**
    * @brief  获取静音状态
    *
    * @param  type  获取静音的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         index 音频流的索引（如果是设置INPUT和OUTPUT音量，则该值可设置为任意值；
    *               如果是获取SINK_INPUT和SOURCE_OUTPUT的静音状态，则将该参数设置成当前应用的索引）
    * @return 返回对应的静音状态
    */
    bool getMute(const NodeType&, int = -1) const;

    /**
    * @brief  获取默认的设备
    *
    * @param  type  获取设备的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         index 音频流的索引（如果是设置INPUT和OUTPUT的默认设备，则该值可设置成-1
    *               如果是设置SINK_INPUT和SOURCE_OUTPUT的默认设备，则该值可设置为应用的索引）
    * @return 返回获取的默认设备名称
    */
    DefaultDeviceInfo getDefaultDevice(const NodeType&, int) const;

    /**
    * @brief  设置默认的设备
    *
    * @param  type 设置默认设备的类型，类型包括INPUT， OUTPUT， SINK_INPUT, SOURCE_OUTPUT
    *         idx  音频流的索引（如果是设置INPUT和OUTPUT的默认设备，则该值可设置成-1
    *              如果是设置SINK_INPUT和SOURCE_OUTPUT的默认设备，则该值可设置为应用的索引）
    *         name 端口名称（该参数为需要设置端口的名称，例如analog-output-speaker）
    *         dev  声卡名称（该参数为对应端口的声卡名称）
    * @return 无
    */
    void setDefaultDevice(const NodeType&, int idx, const std::string&, const std::string&);

    std::vector<std::string> getDeviceList(const NodeType&) const;

    std::list<DefaultDeviceInfo> getSinkList() const;

    std::list<DefaultDeviceInfo> getSourceList() const;

    std::list<std::shared_ptr<DeviceInfo>> getAvailablePortList(const NodeType&) const;

    std::list<StreamMediaInfo> getSinkInputList() const;

    std::list<StreamMediaInfo> getSourceOutputList() const;

    std::list<StreamMediaIntegrationInfo> getStreamMediaList() const;

    bool isValidDevice(const NodeType&) const;

    // settings method
    /**
    * @brief  获取音量增强状态
    *
    * @return 返回音量增强的开关状态
    */
    bool getVolumeBoostStatus() const;

    /**
    * @brief  设置音量增强的状态
    *
    * @param  status 设置的状态
    *
    * @return 无
    */
    void setVolumeBoostStatus(bool);

    /**
    * @brief  获取开启音量增强状态下的最大音量值
    *
    * @return 返回开启音量增强状态的音量值
    */
    int getVolumeBoostValue() const;

    /**
    * @brief  设置音量增强状态下的最大音量值
    *
    * @param  value 设置的最大音量
    *
    * @return 无
    */
    void setVolumeBoostValue(int);

    /**
    * @brief  获取单声道音频功能的开启状态
    *
    * @return 返回单声道音频的开启状态
    */
    bool getMonoStatus() const;

    /**
    * @brief  设置单声道音频功能的开启状态
    *
    * @param  status 设置的状态
    *
    * @return 无
    */
    void setMonoStatus(bool);

    /**
    * @brief  获取智能降噪功能的开启状态
    *
    * @return 返回智能降噪功能的开启状态
    */
    bool getEchoCancelStatus() const;

    /**
    * @brief  设置智能降噪功能的开启状态
    *
    * @param  status 设置的状态
    *
    * @return 无
    */
    void setEchoCancelStatus(bool);

    /**
    * @brief  获取侦听功能的开启状态
    *
    * @return 返回侦听功能的开启状态
    */
    bool getLoopbackStatus() const;

    /**
    * @brief  设置侦听功能的开启状态
    *
    * @param  status 设置的状态
    *
    * @return 无
    */
    void setLoopbackStatus(bool);

    /**
    * @brief  获取当前音效主题名称
    *
    * @return 返回当前音效主题名称
    */
    std::string getSoundThemeName() const;

    /**
    * @brief  设置当前音效主题名称
    *
    * @param  当前音效主题名称
    *
    * @return 无
    */
    void setSoundThemeName(const std::string&);

    /**
    * @brief  获取自定义音效主题的状态
    *
    * @return 返回自定义音效主题的状态
    */
    bool getCustomThemeStatus() const;

    /**
    * @brief  设置自定义音效主题的状态
    *
    * @param  自定义音效主题的状态
    *
    * @return 无
    */
    void setCustomThemeStatus(bool);

    /**
    * @brief  获取通知音效的名称
    *
    * @return 返回通知音效的名称
    */
    std::string getNotifyGeneralName() const;

    /**
    * @brief  设置通知音效的名称
    *
    * @param  通知音效的名称
    *
    * @return 无
    */
    void setNotifyGeneralName(const std::string&);

    /**
    * @brief  获取音量调节的名称
    *
    * @return 返回音量调节的名称
    */
    std::string getVolumeChangedName() const;

    /**
    * @brief  设置音量调节的名称
    *
    * @param  音量调节的名称
    *
    * @return 无
    */
    void setVolumeChangedName(const std::string&);

    /**
    * @brief  获取提示音功能的开启状态
    *
    * @return 返回提示音功能的开启状态
    */
    bool getAlertStatus() const;

    /**
    * @brief  获取提示音音量
    * @return int 音量值 (0-100)
    */
    int getAlertVolume() const;

    /**
    * @brief  设置提示音音量
    * @param volume 音量值 (0-100)
    */
    void setAlertVolume(int volume);

    /**
    * @brief  设置提示音功能的开启状态
    *
    * @param  status 设置的状态
    *
    * @return 无
    */
    void setAlertStatus(bool);

    bool getStartupStatus() const;

    void setStartupStatus(bool);

    bool getPoweroffStatus() const;

    void setPoweroffStatus(bool);

    bool getLogoutStatus() const;

    void setLogoutStatus(bool);

    bool getWakeupStatus() const;

    void setWakeupStatus(bool);

    std::string getGlobalThemeName() const;

    void setGlobalThemeName(const std::string&);

    // sound theme method
    /**
    * @brief  获取系统音效主题列表
    *
    * @return 返回音效主题列表
    */
    std::list<SoundThemeInfo> getSoundThemeList();

    /**
    * @brief  获取指定音效主题的音效文件列表
    *
    * @return 返回音效文件列表
    */
    std::list<SoundEffectFileInfo> getSoundEffectFileList(const std::string&);

    /**
    * @brief  播放提示音
    *
    * @param  soundEvent 声音事件名称
    * @return 播放是否成功
    */
    bool playAlertSound(const std::string& soundEvent);

    void setEnabled(const std::string&, const std::string&, bool);

    bool isEnabled(const std::string&, const std::string&) const;

    bool getAutoPauseStatus() const;

    void setAutoPauseStatus(bool);

    bool getMultiAudioCombineStatus(const NodeType&) const;

    void setMultiAudioCombine(const NodeType&, bool, const std::vector<std::string>&);

    bool isLoaded(const ModuleType&);

    // signal
    void volumeChanged(const NodeType&, int, int);

    void muteChanged(const NodeType&, int, bool);

    void deviceChanged(const NodeType&, const std::string&, const std::string&, bool = false);

    void deviceAdjust(const NodeType&);

    void portChanged(const NodeType&, const std::string&);

    void cardRemoved(const std::string&);

    void settingsChanged(const char*, GVariant*);

    void addStream(int, int, const std::string&, const std::string&);

    void removeStream(int);

    void addStream(int, int, int, bool, const std::string&, const std::string&);

    void removeStream(int, int);


private:
    void initJson();

    SystemVersionInfo getSystemVersionInfo();

    std::optional<std::string> getFieldValue(const std::string&, const std::string_view&);

    DesktopEnvironmentVersion getTypeFromSystemVersionInfo(const SystemVersionInfo&) const;

    std::string extractThemeName(const std::string&);

    std::string replaceThemeName(const std::string&, const std::string&);

    template <typename Container>
    auto findByName(const Container& container, const std::string& name) {
        return std::find_if(container.begin(), container.end(), [&name](const SoundEffectFileInfo& info) {
            return info.name.find(name) != std::string::npos;
        });
    }

private:
    AudioMethod() = default;
    AudioMethod(const AudioMethod&) = delete;
    AudioMethod(AudioMethod&&) = delete;
    AudioMethod operator=(const AudioMethod&) = delete;
    AudioMethod operator=(AudioMethod&&) = delete;
    virtual ~AudioMethod() = default;

private:
    AudioDBusServer m_dbus;
    std::shared_ptr<AudioManager> m_pAudioManager {};
    std::shared_ptr<RestoreVolumeJson> m_pRestoreVolumeJson {};
    std::shared_ptr<MultiAudioCombineJson> m_pMultiAudioJson {};

    std::unordered_map<std::string, std::pair<std::string, std::string>> m_settingsMap = {
        {MONO_AUDIO_KEY, {"module-remap-sink", "sink_name=mono channels=1 channel_map=mono"}},
        {NOISE_REDUCTION_KEY, {"module-echo-cancel", "use_master_format=1 aec_method=webrtc aec_args=analog_gain_control=0 source_name=noiseReduceSource"}},
        {LOOPBACK_KEY, {"module-loopback", ""}},
        {SWITCH_ON_CONNECT_KEY, {"module-switch-on-connect", ""}}
    };

    std::unordered_map<SettingType, std::shared_ptr<ISetting>> m_keys = {
        {SettingType::AUDIO_SETTING, std::shared_ptr<ISetting>()},
        {SettingType::GLOBALTHEME_SETTING, std::shared_ptr<ISetting>()},
        {SettingType::SESSIONMANAGER_SETTING, std::shared_ptr<ISetting>()},
        {SettingType::SOUDNTHEMEPLAYER_SETTING, std::shared_ptr<ISetting>()}
    };

    std::unordered_map<DevicePortType, uint32_t> m_outputVolumeMap = {
        {DevicePortType::PORT_TYPE_BLUETOOTH, 17},
        {DevicePortType::PORT_TYPE_USB, 67},
        {DevicePortType::PORT_TYPE_HEADSET, 17},
        {DevicePortType::PORT_TYPE_BUILTIN, 67},
        {DevicePortType::PORT_TYPE_LINEIO, 17},
        {DevicePortType::PORT_TYPE_HDMI, 100},
        {DevicePortType::PORT_TYPE_MULTICHANNEL, 67},
        {DevicePortType::PORT_TYPE_UNKNOWN, 67}
    };

    std::unordered_map<std::string, std::pair<std::string, std::string>> m_defaultThemeMap = {
        {"HeYin", {"/usr/share/sounds/HeYin/stereo/audio-volume-change.ogg",
                   "/usr/share/sounds/HeYin/stereo/notification-general.ogg"}},
        {"Light-Seeking", {"/usr/share/sounds/Light-Seeking/stereo/audio-volume-change.ogg",
                           "/usr/share/sounds/Light-Seeking/stereo/notification-general.ogg"}},
        {"Classic", {"/usr/share/sounds/Classic/stereo/audio-volume-change.ogg",
                     "/usr/share/sounds/Classic/stereo/notification-general.ogg"}},
        {"DawnLight", {"/usr/share/sounds/DawnLight/stereo/audio-volume-change.ogg",
                           "/usr/share/sounds/DawnLight/stereo/notification-general.ogg"}}
    };

    std::unordered_map<std::string, std::string> m_adapterStreamNameMap = {
        {"qaxbrowser", "qaxbrowser-safe"}
    };

};
}
#endif // AUDIOMETHOD_H

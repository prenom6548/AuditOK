#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>
#include "BaseType.h"

namespace UkuiAudioFramwork
{
// DBus info
#define AUDIO_OBJECT_SERVICE "org.ukui.volume.control"
#define AUDIO_OBJECT_PATH    "/org/ukui/volume/control"
#define AUDIO_INTERFACE      "org.ukui.volume.control"
#define SETTINGS_INTERFACE   "org.ukui.audio.settings"
#define SOUNDTHEME_INTERFACE "org.ukui.soundTheme"

// settings info
#define UKUI_SOUND_SCHEMA           "org.ukui.sound"
#define UKUI_SOUND_PATH             "/org/ukui/sound/"
#define EVENT_SOUND_KEY             "event-sounds"
#define THEME_NAME_KEY              "theme-name"
#define CUSTOM_THEME_KEY            "custom-theme"
#define MONO_AUDIO_KEY              "mono-audio"
#define NOISE_REDUCTION_KEY         "dns-noise-reduction"
#define LOOPBACK_KEY                "loopback"
#define SWITCH_ON_CONNECT_KEY       "switch-on-connect"
#define VOLUME_BOOST_KEY            "volume-increase"
#define VOLUME_BOOST_VOLUME_KEY     "volume-increase-value"
#define STARTUP_MUSIC_KEY           "startup-music"
#define NOTIFY_GENERAL_KEY          "notification-general"
#define VOLUME_CHANGED_KEY          "audio-volume-change"
#define ALERT_VOLUME_KEY            "alert-volume"

//  UKUI Session
#define SESSION_MANAGER_SCHEMA  "org.ukui.session"
#define SESSION_MANAGER_PATH    "/org/ukui/desktop/session/"
#define STARTUP_MUSIC_KEY       "startup-music"
#define POWEROFF_MUSIC_KEY      "poweroff-music"
#define LOGOUT_MUSIC_KEY        "logout-music"
#define WAKEUP_MUSIC_KEY        "weakup-music"

#define DEFAULT_VOLUME_BOOST_VALUE 125

// 与 PulseAudio 哨兵值对齐，避免 Util.h 依赖 libpulse 头文件
// PA_INVALID_INDEX  == ((uint32_t) -1)
// PA_VOLUME_INVALID == ((pa_volume_t) UINT32_MAX)
constexpr uint32_t kInvalidDeviceIndex = static_cast<uint32_t>(-1);
constexpr uint32_t kInvalidVolume = UINT32_MAX;

/**
 * @brief 设备端口的类型
 */
enum class DevicePortType
{
    PORT_TYPE_BLUETOOTH,        // 蓝牙音频  声卡名包含bluez
    PORT_TYPE_USB,              // USB  声卡名称包含usb
    PORT_TYPE_HEADSET,          // 3.5mm 耳麦 端口名包含rear-mic, front-mic,headset,headphone
    PORT_TYPE_BUILTIN,          // 内置扬声器和话筒 端口名包含speaker, input-mic
    PORT_TYPE_LINEIO,           // 线缆输入输出  端口名称包含linein或lineout
    PORT_TYPE_HDMI,             // HDMI  端口名称包含hdmi
    PORT_TYPE_MULTICHANNEL,     // 多声道   端口名称包含multichannel
    PORT_TYPE_UNKNOWN           // 其他类型
};

enum class ModuleType
{
    MODULE_ECHO_CANCEL,      // 智能降噪模块
    MODULE_REMAP_SINK,       // 单声道模块
    MODULE_LOOPBACK,         // 侦听模块
    MODULE_SWITCH_ON_CONNECT // 连接时切换模块
};

/**
 * @brief 设备端口信息
 */
typedef struct DeviceInfo
{
    DeviceInfo(std::string name, std::string desc, std::string pName, std::string pLabel, int prio, int dire, int ava, bool enable, DevicePortType type)
        : cardName(name), cardDesc(desc), portName(pName), portLabel(pLabel), priority(prio), direction(dire), available(ava), enabled(enable), type(type){}
    DeviceInfo() {}

    bool operator==(const DeviceInfo& other) const
    {
        /**
         *  When the port status is "Unknown", playing audio will cause the status to change to "yes", which will then be judged as inconsistent device information
         *  by operator==, and subsequently trigger the deviceAdjust signal.
         *  However, "Unknown" and "yes" are essentially both available states and should not be regarded as a change in the device. The correct judgment logic
         *  should be: Only when the availability switches between available (yes/Unknown) and unavailable (no), is it considered that the device has changed and
         *  the signal is triggered.
         */
        auto sameAvailability = [this, &other]() -> bool {
            return (available == other.available) ||
                   (available != 1 && other.available != 1);
        };

	return cardName == other.cardName &&
                cardDesc == other.cardDesc &&
                portName == other.portName &&
                portLabel == other.portLabel &&
                priority == other.priority &&
                direction == other.direction &&
                sameAvailability() &&
                enabled == other.enabled &&
                type == other.type;
    }

    bool operator!=(const DeviceInfo& other) const
    {
        return !(*this == other);
    }

    std::string toString() const;

    std::string cardName;  // 声卡名称
    std::string cardDesc;  // 声卡描述
    std::string portName;  // 设备端口名称
    std::string portLabel; // 设备端口描述
    uint32_t priority;     // 端口优先级
    uint32_t direction;    // 端口方向 input or output
    uint32_t available;    // 端口可用状态
    bool enabled = true;   // 端口是否启用
    DevicePortType type;   // 端口的类型
} DeviceInfo;

/**
 * @brief 默认设备信息
 */
typedef struct DefaultDeviceInfo
{
    uint32_t idx = kInvalidDeviceIndex;  // 默认设备的索引
    uint32_t volume = kInvalidVolume;    // 默认设备的音量
    std::string name;            // 默认设备的名称
    std::string cardName;        // 默认设备所在的声卡名称
//    std::string cardDesc;        // 默认设备所在的声卡描述
    std::string activePortName;  // 默认设备的活跃端口名称，可能为空
    std::string activePortLabel; // 默认设备的活跃端口标签

    std::string toString() const;

} DefaultDeviceInfo;

/**
 * @brief 流媒体信息
 */
typedef struct StreamMediaInfo
{
    std::string name;      // 流媒体名称
    uint32_t direction;    // 流媒体的direction, input or output
    uint32_t index;        // 流媒体索引
    uint32_t volume;       // 流媒体音量
    std::string mediaRole; // 流媒体角色
    std::string iconName;  // 流媒体应用图标名称
    std::string binary;    // 流媒体对应的二进制名称

    std::string toString() const;
} StreamMediaInfo;

typedef struct MediaInfo
{
    uint32_t direction;    // 流媒体的direction, input or output
    uint32_t index;        // 流媒体索引
    uint32_t volume;       // 流媒体音量
    bool mute;             // 流媒体静音状态
    std::string name;      // 流媒体名称
} MediaInfo;

typedef struct StreamMediaIntegrationInfo
{
    std::string mediaRole;              // 流媒体角色
    std::string iconName;               // 流媒体应用图标名称
    std::string binary;                 // 流媒体对应的二进制名称
    std::list<MediaInfo> mediaInfoList; // 流媒体信息

    std::string toString() const;
} StreamMediaIntegrationInfo;


/**
 * @brief 系统版本信息
 */
typedef struct SystemVersionInfo
{
    std::optional<std::string> prettyName; // 系统版本名称
    std::optional<std::string> releaseId;  // 发布版本Id
} SystemVersionInfo;

/**
 * @brief 系统版本类型
 */
enum class DesktopEnvironmentVersion
{
    DESKTOP_ENVIRONMENT_VERSION_UKUI2,  // ukui2 对应商业版本2107,
    DESKTOP_ENVIRONMENT_VERSION_UKUI3,  // ukui3 对应商业版本2203、2303、 2403、2503？ 社区版本yangtze nile
    DESKTOP_ENVIRONMENT_VERSION_UKUI4,  // ukui4 对应商业版本2603？， 社区版本openkylin huanghe
    DESKTOP_ENVIRONMENT_VERSION_UKUI5,  // 后续的版本
    DESKTOP_ENVIRONMENT_VERSION_UNKNOWN
};

template<typename EnumType, typename = typename std::enable_if<std::is_enum<EnumType>::value>::type>
inline constexpr auto EnumToInt(EnumType t)
{
    return static_cast<int>(t);
}

template <typename EnumType, typename = typename std::enable_if<std::is_enum<EnumType>::value>::type>
inline EnumType IntToEnum(int value)
{
    return static_cast<EnumType>(value);
}

#define CASE(_code) \
    case _code: { \
        std::string name(#_code); \
        return name.substr(name.find_last_of(':') + 1);\
    }

std::string getModuleName(const Module&);
std::string getBackendName(const BackendType&);
std::string getNodeTypeName(const NodeType&);
DevicePortType getDevicePortType(const std::string&, const std::string&);
int calaPriorityByName(const std::string&);
bool hasKeyword(const std::vector<std::string>&, const std::string&);
std::string resolveStreamAppIconName(
    const char* iconName,
    const char* processBinary,
    const char* streamName,
    const std::unordered_map<std::string, std::string>& adapterMap);
}
#endif // UTIL_H

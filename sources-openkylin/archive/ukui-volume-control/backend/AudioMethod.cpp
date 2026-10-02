/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "AudioMethod.h"
#include "SoundThemePlayer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>
#include <system_error>


#include "InitVolumeModule.h"
#include "AudioContext.h"
#include "MediaControler.h"
#include "MediaAutoPauseModule.h"
#include "MultiAudioCombineModule.h"
#include "DeviceManagerModule.h"
#include "Util.h"
#include <algorithm>

namespace UkuiAudioFramwork
{
AudioMethod& AudioMethod::getInstance()
{
    static AudioMethod instance;
    return instance;
}

void AudioMethod::setManager(std::shared_ptr<AudioManager> manager)
{
    m_pAudioManager = manager;
}

void AudioMethod::init()
{
    // init dbus
    m_dbus.initialize();

    // init settings
    for (auto& [k, v] : m_keys) {
        switch (k) {
        case SettingType::AUDIO_SETTING: {
            v = std::make_shared<AudioSetting>();
            break;
        }
        case SettingType::GLOBALTHEME_SETTING: {
            v = std::make_shared<GlobalThemeSettings>();
            break;
        }
        case SettingType::SESSIONMANAGER_SETTING: {
            v = std::make_shared<SessionManagerSetting>();
            break;
        }
        case SettingType::SOUDNTHEMEPLAYER_SETTING: {
            v = std::make_shared<SoundThemePlayerSetting>();
            break;
        }
        default:
            break;
        }
    }

    // 初始化音量
    auto initSystemVolume = [this]() {
        auto inputDev = getDefaultDevice(NodeType::INPUT, -1);
        initVolume(NodeType::INPUT, inputDev.activePortName, inputDev.cardName);

        auto outputDev = getDefaultDevice(NodeType::OUTPUT, -1);
        initVolume(NodeType::OUTPUT, outputDev.activePortName, outputDev.cardName);
    };

    std::thread(initSystemVolume).detach();

//    auto initModule = [this]() {
//        for (const auto& [key, value] : m_settingsMap) {
//            const auto& [module, param] = value;
//            bool status = m_statusFunctions[key]();
//            if (status) {
//                loadModule(module, param);
//            }
//        }
//    };
//    std::thread(initModule).detach();
    initJson();

    DeviceManagerModule::getInstance().setManager(m_pAudioManager);
    DeviceManagerModule::getInstance().setJson(m_pRestoreVolumeJson);
    MediaAutoPauseModule::getInstance().setJson(m_pRestoreVolumeJson);
    MultiAudioCombineModule::getInstance().setJson(m_pMultiAudioJson);
}

void AudioMethod::run()
{
    m_dbus.run();
}

void AudioMethod::showOsd(const std::string& portName)
{
    std::string iconName;

    if (std::string::npos != portName.find("headphone")) {
        iconName = "audio-headphones-symbolic";
    } else if (std::string::npos != portName.find("headset")) {
        iconName = "audio-headset-symbolic";
    } else {
        iconName = "audio-volume-high-symbolic";
    }

    m_dbus.sendSignal("org.ukui.Framework", "/org/ukui/Framework/UI/OsdServer", "org.ukui.Framework.UI.OsdServer",
                       "ShowCustomOSD", g_variant_new("(s)", iconName.c_str()));
}

std::optional<std::string> AudioMethod::getFieldValue(const std::string& line, const std::string_view& fieldName)
{
    if (line.find(fieldName) != std::string::npos) {
        auto start = line.find('=') + 1;
        if (line[start] == '"') {
            ++start;
            auto end = line.find('"', start);
            if (end != std::string::npos) {
                return line.substr(start, end - start);
            }
        }
    }
    return std::nullopt;
}

void AudioMethod::initJson()
{
    m_pRestoreVolumeJson = std::make_shared<RestoreVolumeJson>();
    m_pMultiAudioJson = std::make_shared<MultiAudioCombineJson>();
}

SystemVersionInfo AudioMethod::getSystemVersionInfo()
{
    SystemVersionInfo info;
    const auto filePath = std::filesystem::path("/etc/os-release");

    try {
        if (std::ifstream osReleaseFile(filePath); osReleaseFile.is_open()) {
            std::string line;
            while (std::getline(osReleaseFile, line)) {
                if (!info.prettyName) {
                    info.prettyName = getFieldValue(line, "PRETTY_NAME");
                }
                if (info.prettyName && !info.releaseId) {
                    info.releaseId = getFieldValue(line, "KYLIN_RELEASE_ID");
                }
            }
        } else {
            std::cerr << "can not open " << filePath << std::endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "file system error: " << e.what() << std::endl;
    }

    return info;
}

DesktopEnvironmentVersion AudioMethod::getTypeFromSystemVersionInfo(const SystemVersionInfo& info) const
{
    if (auto prettyName = info.prettyName; prettyName) {
        if (prettyName.value().find("openKylin") != std::string::npos) {
            return DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI4;
        } else if (prettyName.value().find("Kylin") != std::string::npos) {
            if (auto releaseId = info.releaseId; releaseId) {
                if (releaseId.value().find("2107") != std::string_view::npos) {
                    return DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI2;
                } else if (releaseId.value().find("2203") != std::string_view::npos ||
                           releaseId.value().find("2303") != std::string_view::npos ||
                           releaseId.value().find("2403") != std::string_view::npos) {
                    return DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UKUI3;
                }
            }
        }
    }
    return DesktopEnvironmentVersion::DESKTOP_ENVIRONMENT_VERSION_UNKNOWN;
}

DesktopEnvironmentVersion AudioMethod::getDesktopEnvironmentVersion()
{
    return getTypeFromSystemVersionInfo(getSystemVersionInfo());
}

std::string AudioMethod::getStreamDescription(const std::string& streamName)
{
    if ("" == streamName)
        return {};

    AUDIO_DEBUG("getStreamDescription streamName: {} .",streamName);
    GError** error = nullptr;
    GKeyFileFlags flags = G_KEY_FILE_NONE;
    GKeyFile* keyfile = g_key_file_new();
    std::string path = "/usr/share/applications/";
    std::string kaimingPath = "/opt/kaiming/share/applications/";
    std::string karePath = "/opt/kare/usr/share/applications/";

    std::string description = "";

    //start adapter application
    auto it = m_adapterStreamNameMap.find(streamName);
    if (it != m_adapterStreamNameMap.end()) {
        path += it->second;
        kaimingPath += it->second;
        karePath += it->second;
    } else {
        path += streamName;
        kaimingPath += streamName;
        karePath += streamName;
    }
    //end

    path += ".desktop";
    kaimingPath += ".desktop";
    karePath += ".desktop";

    gboolean ret = g_key_file_load_from_file(keyfile, path.c_str(), flags, error);
    AUDIO_DEBUG("getStreamDescription ret: {} , path: {} .",ret , path);
    if (!ret) {
        ret = g_key_file_load_from_file(keyfile, kaimingPath.c_str(), flags, error);
        AUDIO_DEBUG("getStreamDescription ret: {} , kaimingPath: {} .",ret , kaimingPath);
        if (!ret) {
            ret = g_key_file_load_from_file(keyfile, karePath.c_str(), flags, error);
            AUDIO_DEBUG("getStreamDescription ret: {} , karePath: {} .",ret , karePath);
        }
    }

    char* name = g_key_file_get_locale_string(keyfile, "Desktop Entry", "Name", nullptr, nullptr);

    description = name ? name : streamName;

    g_key_file_free(keyfile);

    return description;
}

void AudioMethod::initVolume(NodeType type, const std::string& portName, const std::string& cardName)
{
    if (InitVolumeModule::getInstance().isNeedInitVolume(portName, cardName)) {
        if (cardName.starts_with("bluez")) {
            m_pAudioManager->waitBluezReady(3000);
            m_pAudioManager->setBluezReadyStatus(false);
        }

        auto volume = 17;
        if (NodeType::INPUT == type) {
            volume = 100;
        }
        else if (NodeType::OUTPUT == type) {
            auto portType = getDevicePortType(cardName, portName);
            volume = m_outputVolumeMap[portType];
        }

        AUDIO_DEBUG("When the sound card: {} port: {} is connected for the first time, the initial volume is {}.", cardName, portName, volume);
        // PipeWire 异步下先 mute 再 unmute 可能乱序，导致设备卡住静音；仅 unmute + 设音量。
        // 其它后端仍 mute→unmute，绕过 setMute 状态相同早退，确保 device-volumes.tdb 写入 mute 记录。
        setMute(type, -1, false);
        setVolume(type, -1, volume);
        InitVolumeModule::getInstance().insert(portName, cardName, volume);
    }
}

void AudioMethod::loadModule(const std::string& name, const std::string& param)
{
    AUDIO_DEBUG("load module: {} with param: {}", name, param);
    m_pAudioManager->loadModule(name, param);
}

void AudioMethod::unloadModule(const std::string& name, bool restoreMasterDevice)
{
    AUDIO_DEBUG("unload module: {}", name);
    m_pAudioManager->unloadModule(name, restoreMasterDevice);
}

std::string AudioMethod::getBackend() const
{
//    return getBackendName(BackendType::PULSEAUDIO);
    return m_pAudioManager->getBackend();
}

void AudioMethod::setBackend(const BackendType& type)
{
    m_pAudioManager->setBackend(type);
}

void AudioMethod::setDetailVolume(const NodeType& type, const std::string& name, const std::string& dev, int volume)
{
    AUDIO_DEBUG("Set {} detail volume, card: {} port: {} valume changed to {}.", getNodeTypeName(type), dev, name, volume);
    m_pAudioManager->setDetailVolume(type, name, dev, volume);
}

int AudioMethod::getDetailVolume(const NodeType& type, const std::string& name, const std::string& dev) const
{
    auto volume = m_pAudioManager->getDetailVolume(type, name, dev);

    AUDIO_DEBUG("Get {} detail volume, card: {} port: {} volume: {}", getNodeTypeName(type), dev, name, volume);
    return volume;
}

void AudioMethod::setVolume(const NodeType& type, int idx, int volume)
{
    AUDIO_DEBUG("Set {} volume, {} volume changed to {}.", getNodeTypeName(type), idx, volume);
    m_pAudioManager->setVolume(type, idx, volume);
}

int AudioMethod::getVolume(const NodeType& type, int idx) const
{
    auto volume = m_pAudioManager->getVolume(type, idx);

    AUDIO_DEBUG("Get {} volume, {} volume: {}.", getNodeTypeName(type), idx, volume);
    return volume;
}

void AudioMethod::setBalance(const NodeType& type, double v)
{
    AUDIO_DEBUG("Set {} balance value to {}.", getNodeTypeName(type), v);
    m_pAudioManager->setBalance(type, v);
}

double AudioMethod::getBalance(const NodeType& type) const
{
    auto balance = m_pAudioManager->getBalance(type);

    AUDIO_DEBUG("Get {} balance, balance {}.", getNodeTypeName(type), balance);
    return balance;
}

void AudioMethod::setDetailMute(const NodeType& type, const std::string& name, const std::string& dev, bool mute)
{
    AUDIO_DEBUG("Set {} detail mute, card: {} port: {} mute changed to {}.", getNodeTypeName(type), dev, name, mute);
    m_pAudioManager->setDetailMute(type, name, dev, mute);
}

bool AudioMethod::getDetailMute(const NodeType& type, const std::string& name, const std::string& dev) const
{
    auto mute = m_pAudioManager->getDetailMute(type, name, dev);

    AUDIO_DEBUG("Get {} detail mute, card: {} port: {} mute: {}", getNodeTypeName(type), dev, name, mute);
    return mute;
}

void AudioMethod::setMute(const NodeType& type, int idx, bool mute)
{
    AUDIO_DEBUG("Set {} mute, {} mute changed to {}.", getNodeTypeName(type), idx, mute);
    m_pAudioManager->setMute(type, idx, mute);
}

bool AudioMethod::getMute(const NodeType& type, int idx) const
{
    auto mute = m_pAudioManager->getMute(type, idx);

    AUDIO_DEBUG("Get {} mute, {} mute: {}.", getNodeTypeName(type), idx, mute);
    return mute;
}

DefaultDeviceInfo AudioMethod::getDefaultDevice(const NodeType& type, int idx) const
{
    auto defaultDevice = m_pAudioManager->getDefaultDevice(type, idx);

    AUDIO_DEBUG("Get {} default device, idx: {}, default device: {}", getNodeTypeName(type), idx, defaultDevice.name);
    return defaultDevice;
}

void AudioMethod::setDefaultDevice(const NodeType& type, int idx, const std::string& name, const std::string& dev)
{
    AUDIO_DEBUG("Set {} default device, default device change to card: {} port: {}.", getNodeTypeName(type), dev, name);

    if (NodeType::OUTPUT == type && getMonoStatus() &&
        m_pAudioManager->isLoaded(ModuleType::MODULE_REMAP_SINK)) {
        const auto currentOutput = getDefaultDevice(NodeType::OUTPUT, -1);
        const bool switchingOutput = currentOutput.cardName != dev || currentOutput.activePortName != name;

        // if (switchingOutput) {
        //     AUDIO_DEBUG("Switching output device with mono enabled, unload module-remap-sink before device switch. "
        //                 "current card: {} port: {}, target card: {} port: {}.",
        //                 currentOutput.cardName, currentOutput.activePortName, dev, name);
        //     unloadModule("module-remap-sink", false);
        // }
    }

    m_pAudioManager->setDefaultDevice(type, idx, name, dev);
}

std::vector<std::string> AudioMethod::getDeviceList(const NodeType& type) const
{
    auto list = m_pAudioManager->getDeviceList(type);

    AUDIO_DEBUG("Get {} device list.", getNodeTypeName(type));
    return list;
}

std::list<DefaultDeviceInfo> AudioMethod::getSinkList() const
{
    auto list = m_pAudioManager->getSinkList();

    std::string sl = "[";
    for (const auto& s : list) {
        sl += "(";
        sl += s.toString();
        sl += "),";
    }
    sl += "]";

    AUDIO_DEBUG("Get sink list, sinks: {}.", sl);
    return list;
}

std::list<DefaultDeviceInfo> AudioMethod::getSourceList() const
{
    auto list = m_pAudioManager->getSourceList();

    std::string sl = "[";
    for (const auto& s : list) {
        sl += "(";
        sl += s.toString();
        sl += "),";
    }
    sl += "]";

    AUDIO_DEBUG("Get source list, sources: {}.", sl);
    return list;
}

std::list<std::shared_ptr<DeviceInfo>> AudioMethod::getAvailablePortList(const NodeType& type) const
{
    auto list = m_pAudioManager->getAvailablePortList(type);

    std::string pl = "[";
    for (const auto& p : list) {
        pl += "(";
        pl += p->toString();
        pl += "),";
    }
    pl += "]";

    AUDIO_DEBUG("Get {} available port, ports: {}.", getNodeTypeName(type), pl);
    return list;
}

std::list<StreamMediaInfo> AudioMethod::getSinkInputList() const
{
    auto list = m_pAudioManager->getSinkInputList();

    std::string sil = "[";
    for (const auto& si : list) {
        sil += "(";
        sil += si.toString();
        sil += "),";
    }
    sil += "]";

    AUDIO_DEBUG("Get sink inputs, inputs: {}.", sil);
    return list;
}

std::list<StreamMediaInfo> AudioMethod::getSourceOutputList() const
{
    auto list = m_pAudioManager->getSourceOutputList();

    std::string sol = "[";
    for (const auto& so : list) {
        sol += "(";
        sol += so.toString();
        sol += "),";
    }
    sol += "]";

    AUDIO_DEBUG("Get source outputs, outputs: {}.", sol);
    return list;
}

std::list<StreamMediaIntegrationInfo> AudioMethod::getStreamMediaList() const
{
    return m_pAudioManager->getStreamMediaList();
}

bool AudioMethod::isValidDevice(const NodeType& type) const
{
    auto ret = m_pAudioManager->isValidDevice(type);
    AUDIO_DEBUG("{} has {} device.", getNodeTypeName(type), ret ? "vaild" : "invaild");
    return ret;
}

bool AudioMethod::getVolumeBoostStatus() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(VOLUME_BOOST_KEY));

        AUDIO_DEBUG("Get volume boost status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get volume boost status failed.");
    return false;
}

void AudioMethod::setVolumeBoostStatus(bool status)
{
    AUDIO_DEBUG("Set volume boost status to {}.", status);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(VOLUME_BOOST_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set volume boost status failed.");

    if (!status) {
        auto sinkList = getSinkList();
        std::for_each(sinkList.begin(), sinkList.end(), [this](const DefaultDeviceInfo& dev) {
            if (dev.volume > PA_VOLUME_NORMAL) {
                AUDIO_DEBUG("When Volume-Increase is turned off, device: {} needs to be set volume: {} to 65536", dev.name, dev.volume);
                setVolume(NodeType::OUTPUT, dev.idx, UKMEDIA_VOLUME_NORMAL);
            }
        });
    }
}

int AudioMethod::getVolumeBoostValue() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        auto value = g_variant_get_int32(key->second->getValue(VOLUME_BOOST_VOLUME_KEY));

        AUDIO_DEBUG("Get volume boost value, value: {}.", value);
        return value;
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get volume boost value failed.");
    return DEFAULT_VOLUME_BOOST_VALUE;
}

void AudioMethod::setVolumeBoostValue(int value)
{
    AUDIO_DEBUG("Set volume boost value to {}.", value);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(VOLUME_BOOST_VOLUME_KEY, g_variant_new("i", value));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set volume boost value failed.");
}

bool AudioMethod::getMonoStatus() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(MONO_AUDIO_KEY));

        AUDIO_DEBUG("Get mono status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get mono status failed.");
    return false;
}

void AudioMethod::setMonoStatus(bool status)
{
    AUDIO_DEBUG("Set mono status to {}.", status);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(MONO_AUDIO_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set mono status failed.");
}

bool AudioMethod::getEchoCancelStatus() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(NOISE_REDUCTION_KEY));

        AUDIO_DEBUG("Get echo cancel status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get noise resduction status failed.");
    return false;
}

void AudioMethod::setEchoCancelStatus(bool status)
{
    AUDIO_DEBUG("Set echo cancel status to {}.", status);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(NOISE_REDUCTION_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set noise resduction status failed.");
}

bool AudioMethod::getLoopbackStatus() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(LOOPBACK_KEY));

        AUDIO_DEBUG("Get loopback status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get loopback status failed.");
    return false;
}

void AudioMethod::setLoopbackStatus(bool status)
{
    AUDIO_DEBUG("Set loopback status to {}.", status);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(LOOPBACK_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set loopback status failed.");
}

std::string AudioMethod::getSoundThemeName() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        gsize length;
        auto name = g_variant_get_string(key->second->getValue(THEME_NAME_KEY), &length);

        AUDIO_DEBUG("Get sound theme name, name: {}.", name);
        return name;
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get theme name failed.");
    return {};
}

void AudioMethod::setSoundThemeName(const std::string& name)
{
    AUDIO_DEBUG("Set sound theme name to {}.", name);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(THEME_NAME_KEY, g_variant_new("s", name.c_str()));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set theme name failed.");
}

bool AudioMethod::getCustomThemeStatus() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(CUSTOM_THEME_KEY));

        AUDIO_DEBUG("Get custom theme status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get custom theme status failed.");
    return {};
}

void AudioMethod::setCustomThemeStatus(bool status)
{
    AUDIO_DEBUG("Set custom theme status to {}.", status);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(CUSTOM_THEME_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set custom theme status failed.");
}

std::string AudioMethod::getNotifyGeneralName() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        gsize length;
        auto name = g_variant_get_string(key->second->getValue(NOTIFY_GENERAL_KEY), &length);

        AUDIO_DEBUG("Get notify general name, name: {}.", name);
        return name ? name : "";
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get notify general failed.");
    return {};
}

void AudioMethod::setNotifyGeneralName(const std::string& name)
{
    AUDIO_DEBUG("Set notify geberal name to {}.", name);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(NOTIFY_GENERAL_KEY, g_variant_new("s", name.c_str()));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set notify general failed.");
}

std::string AudioMethod::getVolumeChangedName() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        gsize length;
        auto name = g_variant_get_string(key->second->getValue(VOLUME_CHANGED_KEY), &length);

        AUDIO_DEBUG("Get volume changed name, name: {}.", name);
        return name ? name : "";
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get volume changed failed.");
    return {};
}

void AudioMethod::setVolumeChangedName(const std::string& name)
{
    AUDIO_DEBUG("Set volume changed name to {}.", name);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(VOLUME_CHANGED_KEY, g_variant_new("s", name.c_str()));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set volume changed failed.");
}

bool AudioMethod::getAlertStatus() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(EVENT_SOUND_KEY));

        AUDIO_DEBUG("Get alert status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get alert status failed.");
    return false;
}

void AudioMethod::setAlertStatus(bool status)
{
    AUDIO_DEBUG("Set alert status to {}.", status);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(EVENT_SOUND_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set alert status failed.");
}

int AudioMethod::getAlertVolume() const
{
    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
        auto volume = g_variant_get_int32(key->second->getValue(ALERT_VOLUME_KEY));

        AUDIO_DEBUG("Get alert volume, volume: {}.", volume);
        return volume;
    }

    AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, get alert volume failed.");
    return 100;  // Default volume
}

void AudioMethod::setAlertVolume(int volume)
{
    AUDIO_DEBUG("Set alert volume to {}.", volume);

    if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end())
        key->second->setValue(ALERT_VOLUME_KEY, g_variant_new("i", volume));
    else
        AUDIO_ERROR("Not found SettingType::AUDIO_SETTING keys, set alert volume failed.");
}


bool AudioMethod::getStartupStatus() const
{
    if (auto key = m_keys.find(SettingType::SESSIONMANAGER_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(STARTUP_MUSIC_KEY));

        AUDIO_DEBUG("Get startup status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::SESSIONMANAGER_SETTING keys, get startup status failed.");
    return false;
}

void AudioMethod::setStartupStatus(bool status)
{
    AUDIO_DEBUG("Set startup status to {}.", status);

    if (auto key = m_keys.find(SettingType::SESSIONMANAGER_SETTING); key != m_keys.end())
        key->second->setValue(STARTUP_MUSIC_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::SESSIONMANAGER_SETTING keys, set startup status failed.");
}

bool AudioMethod::getPoweroffStatus() const
{
    if (auto key = m_keys.find(SettingType::SESSIONMANAGER_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(POWEROFF_MUSIC_KEY));

        AUDIO_DEBUG("Get poweroff status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::SESSIONMANAGER_SETTING keys, get poweroff status failed.");
    return false;
}

void AudioMethod::setPoweroffStatus(bool status)
{
    AUDIO_DEBUG("Set poweroff status to {}.", status);

    if (auto key = m_keys.find(SettingType::SESSIONMANAGER_SETTING); key != m_keys.end())
        key->second->setValue(POWEROFF_MUSIC_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::SESSIONMANAGER_SETTING keys, set poweroff status failed.");
}

bool AudioMethod::getLogoutStatus() const
{
    if (auto key = m_keys.find(SettingType::SESSIONMANAGER_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(LOGOUT_MUSIC_KEY));

        AUDIO_DEBUG("Get logout status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::SESSIONMANAGER_SETTING keys, get logout status failed.");
    return false;
}

void AudioMethod::setLogoutStatus(bool status)
{
    AUDIO_DEBUG("Set logout status to {}.", status);

    if (auto key = m_keys.find(SettingType::SESSIONMANAGER_SETTING); key != m_keys.end())
        key->second->setValue(LOGOUT_MUSIC_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::SESSIONMANAGER_SETTING keys, set logout status failed.");
}

bool AudioMethod::getWakeupStatus() const
{
    if (auto key = m_keys.find(SettingType::SESSIONMANAGER_SETTING); key != m_keys.end()) {
        auto status = g_variant_get_boolean(key->second->getValue(WAKEUP_MUSIC_KEY));

        AUDIO_DEBUG("Get wakeup status, status: {}.", status);
        return status;
    }

    AUDIO_ERROR("Not found SettingType::SESSIONMANAGER_SETTING keys, get wakeup status failed.");
    return false;
}

void AudioMethod::setWakeupStatus(bool status)
{
    AUDIO_DEBUG("Set wakeup status to {}.", status);

    if (auto key = m_keys.find(SettingType::SESSIONMANAGER_SETTING); key != m_keys.end())
        key->second->setValue(WAKEUP_MUSIC_KEY, g_variant_new("b", status));
    else
        AUDIO_ERROR("Not found SettingType::SESSIONMANAGER_SETTING keys, set wakeup status failed.");
}

std::string AudioMethod::getGlobalThemeName() const
{
    if (auto key = m_keys.find(SettingType::GLOBALTHEME_SETTING); key != m_keys.end()) {
        gsize length;
        auto name = g_variant_get_string(key->second->getValue(GLOBAL_THEME_NAME_KEY), &length);

        AUDIO_DEBUG("Get global theme name, name: {}.", name);
        return name;
    }

    AUDIO_ERROR("Not found SettingType::GLOBALTHEME_SETTING keys, get global theme name failed.");
    return {};
}

void AudioMethod::setGlobalThemeName(const std::string& name)
{
    AUDIO_DEBUG("Set global theme name to {}.", name);

    if (auto key = m_keys.find(SettingType::GLOBALTHEME_SETTING); key != m_keys.end())
        key->second->setValue(GLOBAL_THEME_NAME_KEY, g_variant_new("s", name.c_str()));
    else
        AUDIO_ERROR("Not found SettingType::GLOBALTHEME_SETTING keys, set global theme name failed.");
}

std::list<SoundThemeInfo> AudioMethod::getSoundThemeList()
{
    auto list = SoundThemeModule::getInstance().getSoundThemeList();

    std::string stl = "[";
    for (const auto& s : list) {
        stl += "(";
        stl += s.toString();
        stl += "),";
    }
    stl += "]";

    AUDIO_DEBUG("Get sound theme list, theme list: {}.", stl);
    return list;
}

std::list<SoundEffectFileInfo> AudioMethod::getSoundEffectFileList(const std::string& themeName)
{
    auto list = SoundThemeModule::getInstance().getSoundEffectFileList(themeName);

    std::string sefl = "[";
    for (const auto& f : list) {
        sefl += "(";
        sefl += f.toString();
        sefl += "),";
    }
    sefl += "]";

    AUDIO_DEBUG("Get {} theme sound effect file list, list: {}.", themeName, sefl);
    return list;
}

bool AudioMethod::playAlertSound(const std::string& soundEvent)
{
    AUDIO_DEBUG("Play alert sound: {}", soundEvent);
    return SoundThemePlayer::getInstance().playAlertSound(soundEvent);
}

void AudioMethod::setEnabled(const std::string& card, const std::string& port, bool enabled)
{
    AUDIO_DEBUG("Set card: {} port: {} enabled to {}.", card, port, enabled);
    DeviceManagerModule::getInstance().setEnabled(card, port, enabled);
}

bool AudioMethod::isEnabled(const std::string& card, const std::string& port) const
{
    auto enable = m_pAudioManager->isEnabled(card, port);

    AUDIO_DEBUG("Get card: {} port: {} enable status, enable: {}.", card, port, enable);
    return enable;
}

bool AudioMethod::getAutoPauseStatus() const
{
    auto status = MediaAutoPauseModule::getInstance().getAutoPauseStatus();
    AUDIO_DEBUG("Get auto pause status, status: {}", status);
    return status;
}

void AudioMethod::setAutoPauseStatus(bool status)
{
    AUDIO_DEBUG("Set auto pause status to {}.", status);
    MediaAutoPauseModule::getInstance().setAutoPauseStatus(status);
}

bool AudioMethod::getMultiAudioCombineStatus(const NodeType& type) const
{
    auto status = MultiAudioCombineModule::getInstance().getMultiAudioCombineStatus(type);

    AUDIO_DEBUG("Get {} multi audio combine status, status: {}.", getNodeTypeName(type), status);
    return status;
}

void AudioMethod::setMultiAudioCombine(const NodeType& type, bool status, const std::vector<std::string>& devList)
{
    std::string deviceList = "[";
    for (const auto& d : deviceList) {
        deviceList += d;
        deviceList += ", ";
    }
    deviceList += "]";

    AUDIO_DEBUG("Set multi audio combine status to {}, combine devices: {}", status, deviceList);
    MultiAudioCombineModule::getInstance().setMultiAudioCombine(type, status, devList);
}

void AudioMethod::volumeChanged(const NodeType& type, int index, int volume)
{
    AUDIO_DEBUG("{} volume changed to {}.", getNodeTypeName(type), volume);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "volumeChanged", g_variant_new("(iiv)", type, index, g_variant_new("i", volume)));

    // 只在 OUTPUT 类型音量变化时播放提示音
    if (type == NodeType::OUTPUT) {
        // 检查是否启用了事件声音
        bool eventSoundsEnabled = false;
        if (auto key = m_keys.find(SettingType::AUDIO_SETTING); key != m_keys.end()) {
            auto value = key->second->getValue(EVENT_SOUND_KEY);
            if (value) {
                eventSoundsEnabled = g_variant_get_boolean(value);
            }
        }

        if (eventSoundsEnabled) {
            playAlertSound("audio-volume-change");
        }
    }
}

void AudioMethod::muteChanged(const NodeType& type, int index, bool mute)
{
    AUDIO_DEBUG("{} type mute changed to {}", EnumToInt(type), mute);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "muteChanged", g_variant_new("(iib)", type, index, mute));
}

void AudioMethod::deviceChanged(const NodeType& type, const std::string& portName, const std::string& cardName, bool autoPause)
{
    // 设备更改是可能需要初始化音量
    std::thread([type, portName, cardName, this]() {
        initVolume(type, portName, cardName);
    }).join();

    // 当默认输出设备发生更改时满足以下条件需要暂停播放
    if (getAutoPauseStatus() && NodeType::OUTPUT == type && autoPause) {
        std::thread([]() {
            MediaControler::getInstance().pauseAllPlayers();
        }).detach();
    }

    // 默认设备更改是可能存在模块需要加载或卸载的操作
    if (NodeType::INPUT == type) {
        auto status = getEchoCancelStatus();
        auto defaultInput = getDefaultDevice(NodeType::INPUT, -1).name;
        auto isLoaded = m_pAudioManager->isLoaded(ModuleType::MODULE_ECHO_CANCEL);
        AUDIO_DEBUG("device changed getEchoCancelStatus:{} defaultInput:{} isLoaded:{} .", status, defaultInput, isLoaded);
        if (status) {
            if (isLoaded) {
                if (std::string::npos != defaultInput.find("bluez_input") ||
                    std::string::npos != defaultInput.find("bluez_source") ||
                    std::string::npos != defaultInput.find("input.usb") ||
                    std::string::npos != defaultInput.find(".monitor")) {
                    AUDIO_DEBUG("The default input device is {} unload the module-echo-cancel.", defaultInput);
                    unloadModule("module-echo-cancel", false);
                }
            }
            else {
                if ((std::string::npos != defaultInput.find("alsa_input") &&
                     std::string::npos == defaultInput.find("input.usb"))) {
                    auto it = m_settingsMap.find(NOISE_REDUCTION_KEY);
                    if (it != m_settingsMap.end()) {
                        const auto& [module, param] = it->second;
                        loadModule(module, param);
                    }
                }
                else {
                    AUDIO_DEBUG("The default input device is {} and there is no need to "
                                "load the module-echo-cancel module.", defaultInput);
                }
            }
        }

        auto loopbackStatus = getLoopbackStatus();
        auto devInfo = getDefaultDevice(NodeType::INPUT, -1);
        const bool isNullorInternalDev =
            (std::string::npos != devInfo.name.find("auto_null")) ||
            (std::string::npos != devInfo.name.find(".monitor")) ||
            (std::string::npos != devInfo.activePortName.find("internal")) ||
            (std::string::npos != devInfo.activePortLabel.find("Digital"));
        AUDIO_DEBUG("device changed loopbackStatus:{} isNullorInternalDev:{}.", loopbackStatus, isNullorInternalDev);
        if (loopbackStatus && m_pAudioManager->isLoaded(ModuleType::MODULE_LOOPBACK)) {
            AUDIO_DEBUG("device changed is loaded MODULE_LOOPBACK.");
            if (isNullorInternalDev) {
                unloadModule("module-loopback", false);
            }
        }
        else if (loopbackStatus && !m_pAudioManager->isLoaded(ModuleType::MODULE_LOOPBACK)) {
            AUDIO_DEBUG("device changed unloaded MODULE_LOOPBACK.");
            if (!isNullorInternalDev) {
                auto it = m_settingsMap.find(LOOPBACK_KEY);
                if (it != m_settingsMap.end()) {
                    const auto& [module, param] = it->second;
                    loadModule(module, param);
                }
            }
        }
    }
    else if (NodeType::OUTPUT == type) {
        auto status = getMonoStatus();
        auto defaultOutput = getDefaultDevice(NodeType::OUTPUT, -1).name;
        auto isLoaded = m_pAudioManager->isLoaded(ModuleType::MODULE_REMAP_SINK);
        const bool isNullDevice = std::string::npos != defaultOutput.find("auto_null");
        const bool isBluetoothDevice =
            DevicePortType::PORT_TYPE_BLUETOOTH == getDevicePortType(cardName, portName);
        const bool isCombineSinkDevice =
            cardName == "combine-device" || portName == "output-combine";

        if (status) {
            if (isLoaded) {
                if (isNullDevice || isBluetoothDevice || isCombineSinkDevice) {
                    AUDIO_DEBUG("The default output device is {} unload the module-remap-sink.", defaultOutput);
                    unloadModule("module-remap-sink", false);
                }
            }
            else {
                AUDIO_DEBUG("The default output device is {} load the module-remap-sink.", defaultOutput);
                if (!isNullDevice && !isBluetoothDevice && !isCombineSinkDevice) {
                    auto it = m_settingsMap.find(MONO_AUDIO_KEY);
                    if (it != m_settingsMap.end()) {
                        const auto& [module, param] = it->second;
                        loadModule(module, param);
                    }
                }
            }
        }

    }

    AUDIO_DEBUG("{} device changed to card: {} port: {}.", getNodeTypeName(type), cardName, portName);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "deviceChanged", g_variant_new("(iss)", type, portName.c_str(), cardName.c_str()));
}

void AudioMethod::deviceAdjust(const NodeType& type)
{
    AUDIO_DEBUG("Device adjust.");
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "deviceAdjust", g_variant_new("(i)", type));
}

void AudioMethod::portChanged(const NodeType& type, const std::string& name)
{
    AUDIO_DEBUG("{} prot change to {}", getNodeTypeName(type), name);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "portChanged", g_variant_new("(is)", type, name.c_str()));
}

void AudioMethod::cardRemoved(const std::string& name)
{
    AUDIO_DEBUG("Card: {} removed", name);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "cardRemoved", g_variant_new("(s)", name.c_str()));
}

void AudioMethod::settingsChanged(const char* key, GVariant* v)
{
    gsize length;

    if (const auto& it = m_settingsMap.find(key); it != m_settingsMap.end()) {
        const auto& [module, param] = it->second;
        auto status = g_variant_get_boolean(v);

        if (status) {
            loadModule(module, param);
        } else {
            unloadModule(module);
        }
    }

    AUDIO_DEBUG("Setting key : {} .", key);
    if (0 == strcmp(key, GLOBAL_THEME_NAME_KEY)) {
        gsize length;
        auto name = g_variant_get_string(v, &length);
        if (strcmp(name, "custom")) {
            setSoundThemeName(name);
        }
    }
    else if (0 == strcmp(key, THEME_NAME_KEY)) {
        const std::string theme = g_variant_get_string(v, &length);
        AUDIO_DEBUG("Setting theme : {} .", theme);

        // 音效主题更改时需要更新音量更改和通知的音效
        if (strcmp(theme.c_str(), "custom")) {
            setCustomThemeStatus(false);

            auto list = SoundThemeModule::getInstance().getSoundEffectFileList(theme);
            AUDIO_DEBUG("Setting theme list: {} .", list.size());
            if (auto it = findByName(list, "audio-volume-change"); it != list.end()) {
                auto avcName = replaceThemeName(it->name, theme);
                setVolumeChangedName(avcName);
            }

            if (auto it = findByName(list, "notification-general"); it != list.end()) {
                auto ntfName = replaceThemeName(it->name, theme);
                setNotifyGeneralName(ntfName);
            }
        }

        auto globalName = getGlobalThemeName();
        AUDIO_DEBUG("Setting globalName : {} .", globalName);

        // 切换音效主题时，将系统全局主题切换成自定义
        if (g_variant_get_string(v, &length) != globalName) {
            setGlobalThemeName("custom");
        }
    }
    else if (0 == strcmp(key, NOTIFY_GENERAL_KEY) || 0 == strcmp(key, VOLUME_CHANGED_KEY)) {
        gsize lg;
        const gchar* vName = g_variant_get_string(v, &lg);
        std::string themeName = getSoundThemeName();
        auto it = m_defaultThemeMap.find(themeName);
        if (it != m_defaultThemeMap.end()) {
            const auto [avcThemeName, ngThemeName] = it->second;
            if ((0 == strcmp(key, NOTIFY_GENERAL_KEY) && 0 == strcmp(vName, ngThemeName.c_str()))
                || (0 == strcmp(key, VOLUME_CHANGED_KEY) && 0 == strcmp(vName, avcThemeName.c_str()))) {
                setCustomThemeStatus(false);
            } else {
                setCustomThemeStatus(true);
                setSoundThemeName("custom");
            }
        } else {
            setCustomThemeStatus(true);
            setSoundThemeName("custom");
        }
    }

    AUDIO_DEBUG("Setting : {} change to {}.", key, v);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, SETTINGS_INTERFACE, "changed", g_variant_new("(sv)", key, v));
}

void AudioMethod::addStream(int idx, int value, const std::string& iconName, const std::string& descName)
{
    AUDIO_DEBUG("Stream {} added, index: {}, value: {}, iconName: {}.", descName, idx, value, iconName);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "addStream", g_variant_new("(iiss)", idx, value, iconName.c_str(), descName.c_str()));
}

void AudioMethod::removeStream(int idx)
{
    AUDIO_DEBUG("Stream {} removed.", idx);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "removeStream", g_variant_new("(i)", idx));
}

void AudioMethod::addStream(int dir, int idx, int value, bool mute, const std::string& iconName, const std::string& descName)
{
    AUDIO_DEBUG("Stream {} added, index: {}, value: {}, iconName: {}.", descName, idx, value, iconName);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "addStream", g_variant_new("(iiibss)", dir, idx, value, mute, iconName.c_str(), descName.c_str()));
}

void AudioMethod::removeStream(int dire, int idx)
{
    AUDIO_DEBUG("Stream {} removed.", idx);
    m_dbus.sendSignal(AUDIO_OBJECT_PATH, AUDIO_INTERFACE, "removeStream", g_variant_new("(ii)", dire, idx));
}

std::string AudioMethod::extractThemeName(const std::string& name)
{
    size_t soundsPos = name.find("/sounds/");
    if (soundsPos == std::string::npos) {
        return "";
    }
    size_t startThemePos = soundsPos + 8;
    size_t endThemePos = name.find("/", startThemePos);
    if (endThemePos == std::string::npos) {
        endThemePos = name.length();
    }
    return name.substr(startThemePos, endThemePos - startThemePos);
}

std::string AudioMethod::replaceThemeName(const std::string& name, const std::string& themeName)
{
    if (themeName.empty()) {
        return name;
    }
    std::string currentThemeName = extractThemeName(name);
    if (currentThemeName != themeName) {
        size_t soundsPos = name.find("/sounds/");
        if (soundsPos == std::string::npos) {
            return name;
        }
        size_t startThemePos = soundsPos + 8;
        size_t endThemePos = name.find("/", startThemePos);
        if (endThemePos == std::string::npos) {
            endThemePos = name.length();
        }
        std::string prefix = name.substr(0, startThemePos);
        std::string suffix = name.substr(endThemePos);
        return prefix + themeName + suffix;
    }
    return name;
}

bool AudioMethod::isLoaded(const ModuleType& type)
{
    return m_pAudioManager->isLoaded(type);
}

}

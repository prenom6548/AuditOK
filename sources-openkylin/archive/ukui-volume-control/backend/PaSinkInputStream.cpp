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

#include "PaSinkInputStream.h"
#include "PaSinkStream.h"
#include "AudioMethod.h"
#include "Util.h"
#include <algorithm>


namespace UkuiAudioFramwork
{
void PaSinkInputStream::copy(const pa_sink_input_info& other)
{
    if (PA_INVALID_INDEX != index) {
        if (!pa_cvolume_equal(&volume, &other.volume) && index == other.index) {
            volumeChanged(paCvolumeToValue(other.volume));
        }

        if (mute != other.mute && index == other.index) {
            muteChanged(!mute);
        }
    }

    index = other.index;
    name = other.name;
    owner_module = other.owner_module;
    client = other.client;
    sink = other.sink;
    sample_spec = other.sample_spec;
    channel_map = other.channel_map;
    volume = other.volume;
    buffer_usec = other.buffer_usec;
    sink_usec = other.sink_usec;
    resample_method = other.resample_method ? other.resample_method : "";
    driver = other.driver;
    mute = other.mute;
    proplist = other.proplist;
    corked = other.corked;
    has_volume = other.has_volume;
    volume_writable = other.volume_writable;
    format = other.format;

    auto mediaRole = pa_proplist_gets(proplist, PA_PROP_MEDIA_ROLE);
    role = mediaRole ? mediaRole : "Playback Stream";
    auto iconName = pa_proplist_gets(proplist, PA_PROP_APPLICATION_ICON_NAME);
    auto binary = pa_proplist_gets(proplist, PA_PROP_APPLICATION_PROCESS_BINARY);
    m_processBinary = binary ? binary : name;
    m_appIconName = resolveStreamAppIconName(iconName, binary, name.c_str(), m_adapterStreamNameMap);
}

std::string PaSinkInputStream::getMediaRole() const
{
    return role;
}

std::string PaSinkInputStream::getAppIconName() const
{
    return m_appIconName;
}

std::string PaSinkInputStream::getProcessBinary() const
{
    return m_processBinary;
}

std::string PaSinkInputStream::getDriver() const
{
    return driver;
}

uint32_t PaSinkInputStream::getSink() const
{
    return sink;
}

std::string PaSinkInputStream::toString() const
{
    std::string vol = "[";
    for (int i = 0; i < volume.channels; ++i) {
        vol += std::to_string(volume.values[i]);
        vol += ", ";
    }
    vol += "]";

    return FORMAT("name={}, index={}, owner_module={}, sink={}, driver={}, volume={}, mute={}",
                  name, index, owner_module, sink, driver, vol, mute);
}

NodeType PaSinkInputStream::getNodeType() const
{
    return NodeType::SINK_INPUT;
}

uint32_t PaSinkInputStream::getIndex() const
{
    return index;
}

std::string PaSinkInputStream::getName() const
{
    return name;
}

pa_proplist* PaSinkInputStream::getProplist() const
{
    return proplist;
}

uint32_t PaSinkInputStream::getCard() const
{
    // TODO
    return {};
}

void PaSinkInputStream::setVolume(int vol)
{
    auto v = volume;

    for (int i = 0; i < v.channels; ++i) {
        v.values[i] = vol;
    }

    if (pa_cvolume_equal(&v, &volume))
        return;

    pa_operation_unref(pa_context_set_sink_input_volume(m_pContext, index, &v, nullptr, nullptr));
}

const pa_cvolume PaSinkInputStream::getVolume() const
{
    return volume;
}

void PaSinkInputStream::setBalance(double v)
{
    // TODO
}

double PaSinkInputStream::getBalance() const
{
    // TODO
    return {};
}

void PaSinkInputStream::setMute(bool mute)
{
    pa_operation_unref(pa_context_set_sink_input_mute(m_pContext, index, mute, nullptr, nullptr));
}

bool PaSinkInputStream::getMute() const
{
    return mute;
}

DefaultDeviceInfo PaSinkInputStream::getDefaultDevice() const
{
    DefaultDeviceInfo info;
    std::shared_ptr<IStream> s = nullptr;
    for (const auto& c : m_info.cardInfoList) {
        auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [this](const std::shared_ptr<IStream>& stream) {
            return sink == stream->getIndex();
        });
        if (it != c->sinks.end()) {
            s = *it;
            break;
        }
    }

    if (s) {
        info.idx = s->getIndex();
        info.name = s->getName();
        info.cardName = std::dynamic_pointer_cast<PaSinkStream>(s)->getCardName();
        info.activePortName = std::dynamic_pointer_cast<PaSinkStream>(s)->getActivePortName();
        info.activePortLabel = std::dynamic_pointer_cast<PaSinkStream>(s)->getActivePortDescription();
    }

    return info;
}

/**
 * @brief 设置音频流（sink-input）的默认输出设备
 *
 * 该函数用于将当前音频流移动到指定的输出设备（sink）。
 *
 * @param port 目标端口名称，例如 "[Out] Speaker" 或 "[Out] HDMI1"
 * @param card 目标声卡名称，例如 "alsa_card.pci-0000_00_1f.3-platform-skl_hda_dsp_generic"
 *
 * @details 选择 sink 的逻辑：
 * 1. 优先匹配策略：优先选择 activePortName 等于目标端口名的 sink
 *    - 这确保选择当前正在使用该端口的 sink，避免错误地连接到其他 sink
 *    - 例如：当设置到 "[Out] Speaker" 时，优先选择 activePortName 为 "[Out] Speaker" 的 sink
 *
 * 2. 备用匹配策略：如果找不到 activePortName 匹配的 sink，则使用端口列表中包含目标端口的第一个 sink
 *    - 这处理某些特殊情况下端口存在但未激活的场景
 *
 * @note 当同一张声卡上有多个 sink（如 Speaker、HDMI1、HDMI2、HDMI3）时，仅通过端口名称匹配
 *       可能无法准确找到正确的 sink，因此优先使用 activePortName 匹配策略。
 *
 * @note 找到目标 sink 后，会调用 pa_context_move_sink_input_by_index 将音频流移动到该 sink
 */
void PaSinkInputStream::setDefaultDevice(const std::string& port, const std::string& card)
{    
    std::shared_ptr<IStream> sink = nullptr;
    std::shared_ptr<IStream> fallbackSink = nullptr;

    for (const auto& c : m_info.cardInfoList) {
        if (strcmp(card.c_str(), c->card.name.c_str()))
            continue;

        for(const auto& s : c->sinks) {
            auto sinkStream = std::dynamic_pointer_cast<PaSinkStream>(s);
            auto ports = sinkStream->getPorts();

            if (sinkStream->getActivePortName() == port) {
                sink = s;
                break;
            }

            auto it = std::find_if(ports.begin(), ports.end(), [port](const std::shared_ptr<PaSinkPortInfo>& info) {
                return info->name == port;
            });
            if (it != ports.end() && !fallbackSink) {
                fallbackSink = s;
            }
        }

        if (sink)
            break;
    }

    if (!sink && fallbackSink) {
        sink = fallbackSink;
    }

    if (!sink) {
        AUDIO_ERROR("Set {} default device to card: {}, port: {} failed, not found such device.", name, card, port);
        return;
    }

    this->sink = sink->getIndex();

    AUDIO_DEBUG("Set {} default device to {}.", name, sink->getName());

    pa_operation_unref(pa_context_move_sink_input_by_index(m_pContext, index, sink->getIndex(), nullptr, nullptr));
}

std::vector<std::string> PaSinkInputStream::getDeviceList() const
{
    return {};
}

std::list<std::shared_ptr<DeviceInfo>> PaSinkInputStream::getAvailablePortList() const
{
    return {};
}

void PaSinkInputStream::volumeChanged(int volume)
{
    AudioMethod::getInstance().volumeChanged(NodeType::SINK_INPUT, index, volume);
}

void PaSinkInputStream::muteChanged(bool mute)
{
    AudioMethod::getInstance().muteChanged(NodeType::SINK_INPUT, index, mute);
}

void PaSinkInputStream::portChanged(const std::string& name)
{
    AudioMethod::getInstance().portChanged(NodeType::SINK_INPUT, name);
}

void PaSinkInputStream::deviceChanged(bool autoPause)
{
//    AudioMethod::getInstance().deviceChanged(NodeType::SINK_INPUT, name);
}
}

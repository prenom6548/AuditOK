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

#include "PaSourceOutputStream.h"
#include "PaSourceStream.h"
#include "AudioMethod.h"
#include "Util.h"
#include <algorithm>


namespace UkuiAudioFramwork
{
void PaSourceOutputStream::copy(const pa_source_output_info& other)
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
    source = other.source;
    sample_spec = other.sample_spec;
    channel_map = other.channel_map;
    volume = other.volume;
    buffer_usec = other.buffer_usec;
    source_usec = other.source_usec;
    resample_method = other.resample_method ? other.resample_method : "";
    driver = other.driver;
    proplist = other.proplist;
    corked = other.corked;
    mute = other.mute;
    has_volume = other.has_volume;
    volume_writable = other.volume_writable;
    format = other.format;

    auto mediaRole = pa_proplist_gets(proplist, PA_PROP_MEDIA_ROLE);
    role = mediaRole ? mediaRole : "Record Stream";
    auto appIconName = pa_proplist_gets(proplist, PA_PROP_APPLICATION_ICON_NAME);
    auto binary = pa_proplist_gets(proplist, PA_PROP_APPLICATION_PROCESS_BINARY);
    m_processBinary = binary ? binary : name;
    m_appIconName = resolveStreamAppIconName(appIconName, binary, name.c_str(), m_adapterStreamNameMap);
}

std::string PaSourceOutputStream::getMediaRole() const
{
    // 在这里使用pa_proplist_gets获取role会出错，暂时先使用这种方式获取media.role
    return role;
}

std::string PaSourceOutputStream::getAppIconName() const
{
    return m_appIconName;
}

std::string PaSourceOutputStream::getProcessBinary() const
{
    return m_processBinary;
}

std::string PaSourceOutputStream::getDriver() const
{
    return driver;
}

uint32_t PaSourceOutputStream::getSource() const
{
    return source;
}

std::string PaSourceOutputStream::toString() const
{
    std::string vol = "[";
    for (int i = 0; i < volume.channels; ++i) {
        vol += std::to_string(volume.values[i]);
        vol += ", ";
    }
    vol += "]";

    return FORMAT("name={}, index={}, owner_module={}, source={}, driver={}, volume={}, mute={}",
                  name, index, owner_module, source, driver, vol, mute);
}

NodeType PaSourceOutputStream::getNodeType() const
{
    return NodeType::SOURCE_OUTPUT;
}

uint32_t PaSourceOutputStream::getIndex() const
{
    return index;
}

std::string PaSourceOutputStream::getName() const
{
    return name;
}

pa_proplist* PaSourceOutputStream::getProplist() const
{
    return proplist;
}

uint32_t PaSourceOutputStream::getCard() const
{
    return {};
}

void PaSourceOutputStream::setVolume(int vol)
{
    auto v = volume;
    for (int i = 0; i < v.channels; ++i) {
        v.values[i] = vol;
    }

    if (pa_cvolume_equal(&v, &volume))
        return;

    pa_operation_unref(pa_context_set_source_output_volume(m_pContext, index, &v, nullptr, nullptr));
}

const pa_cvolume PaSourceOutputStream::getVolume() const
{
    return volume;
}

void PaSourceOutputStream::setBalance(double v)
{
    // TODO
}

double PaSourceOutputStream::getBalance() const
{
    // TODO
    return {};
}

void PaSourceOutputStream::setMute(bool mute)
{
    pa_operation_unref(pa_context_set_source_mute_by_index(m_pContext, index, mute, nullptr, nullptr));
}

bool PaSourceOutputStream::getMute() const
{
    return mute;
}

DefaultDeviceInfo PaSourceOutputStream::getDefaultDevice() const
{
    DefaultDeviceInfo info;
    std::shared_ptr<IStream> s = nullptr;
    for (const auto& c : m_info.cardInfoList) {
        auto it = std::find_if(c->sources.begin(), c->sources.end(), [this](const std::shared_ptr<IStream>& stream) {
            return source == stream->getIndex();
        });
        if (it != c->sources.end()) {
            s = *it;
            break;
        }
    }

    if (s) {
        info.idx = s->getIndex();
        info.name = s->getName();
        info.cardName = std::dynamic_pointer_cast<PaSourceStream>(s)->getCardName();
        info.activePortName = std::dynamic_pointer_cast<PaSourceStream>(s)->getActivePortName();
        info.activePortLabel = std::dynamic_pointer_cast<PaSourceStream>(s)->getActivePortDescription();
    }

    return info;
}

void PaSourceOutputStream::setDefaultDevice(const std::string& port, const std::string& card)
{
    std::shared_ptr<IStream> source = nullptr;
    for (const auto& c : m_info.cardInfoList) {
        if (strcmp(card.c_str(), c->card.name.c_str()))
            continue;

        for(const auto& s : c->sources) {
            auto ports = std::dynamic_pointer_cast<PaSourceStream>(s)->getPorts();
            auto it = std::find_if(ports.begin(), ports.end(), [s, port](const std::shared_ptr<PaSourcePortInfo> p) {
                return p->name == port;
            });
            if (it != ports.end()) {
                source = s;
                break;
            }
        }
    }

    if (!source) {
        AUDIO_ERROR("Set {} device to card: {}, port: {} error, not found such device.", name, card, port);
        return;
    }

    this->source = source->getIndex();
    AUDIO_DEBUG("Set {} default device to {}", name, source->getName());
    pa_operation_unref(pa_context_move_source_output_by_index(m_pContext, index, source->getIndex(), nullptr, nullptr));
}

std::vector<std::string> PaSourceOutputStream::getDeviceList() const
{
    return {};
}

std::list<std::shared_ptr<DeviceInfo>> PaSourceOutputStream::getAvailablePortList() const
{
    return {};
}

void PaSourceOutputStream::volumeChanged(int volume)
{
    AudioMethod::getInstance().volumeChanged(NodeType::SOURCE_OUTPUT, index, volume);
}

void PaSourceOutputStream::muteChanged(bool mute)
{
    AudioMethod::getInstance().muteChanged(NodeType::SOURCE_OUTPUT, index, mute);
}

void PaSourceOutputStream::portChanged(const std::string& name)
{
    AudioMethod::getInstance().portChanged(NodeType::SOURCE_OUTPUT, name);
}

void PaSourceOutputStream::deviceChanged(bool autoPause)
{

}
}

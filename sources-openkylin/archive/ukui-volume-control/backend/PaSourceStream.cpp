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

#include "PaSourceStream.h"

#include <algorithm>
#include "AudioMethod.h"
#include "PaSourceOutputStream.h"


namespace UkuiAudioFramwork
{
std::shared_ptr<PaSourcePortInfo> PaSourceStream::getActivePort() const
{
    return active_port;
}

std::string PaSourceStream::getActivePortName() const
{
    return (this && active_port) ? active_port->name : "";
}

std::string PaSourceStream::getActivePortDescription() const
{
    return (this && active_port) ? active_port->description : "";
}

std::string PaSourceStream::getCardName() const
{
    return cardName;
}

std::string PaSourceStream::getDeviceClass() const
{
    return m_deviceClass;
}

void PaSourceStream::setMasterDevice(const std::string& master)
{
    AUDIO_DEBUG("Set {} master device to {}.", name, master);

    m_masterDevice = master;
}

std::string PaSourceStream::getMasterDevice() const
{
    AUDIO_DEBUG("Get {} master device: {}.", name, m_masterDevice);
    return m_masterDevice;
}

uint32_t PaSourceStream::getNPorts() const
{
    return n_ports;
}

uint32_t PaSourceStream::getMasterDeviceIndex() const
{
    return m_masterDeviceIndex;
}

void PaSourceStream::copy(const pa_source_info& other)
{
    auto it = std::find_if(m_info.cardInfoList.begin(), m_info.cardInfoList.end(),
                           [other](const std::shared_ptr<CardInfo> c) {
        return c->card.index == other.card;
    });

    if (it != m_info.cardInfoList.end()) {
        cardName = (*it)->card.name;
    }
    else {
        cardName = "";
    }

//    AUDIO_DEBUG("PaSourceStream::copy, name:{} other.name:{} index:{} other.index:{}."
//                , name, other.name, index, other.index);
//    AUDIO_DEBUG("PaSourceStream::copy, volume:{} other.volume:{} mute:{} other.mute:{}."
//                , volume.values[0], other.volume.values[0], mute, other.mute);
//    if (active_port && other.active_port) {
//        AUDIO_DEBUG("PaSourceStream::copy, active_port->name:{} other.active_port->name:{}."
//                    , active_port->name, other.active_port->name);
//    }
    if (PA_INVALID_INDEX != index) {
        if (!pa_cvolume_equal(&volume, &other.volume) && index == other.index) {
            volumeChanged(paCvolumeToValue(other.volume));
        }
        int idx = PA_INVALID_INDEX;
        auto deviceClass = pa_proplist_gets(other.proplist, PA_PROP_DEVICE_CLASS);
        if (deviceClass && strcmp(deviceClass, "filter") == 0) {
            auto it = std::find_if(m_info.sourceOutputList.begin(), m_info.sourceOutputList.end(), [&idx](const std::shared_ptr<IStream>& stream) {
                return std::dynamic_pointer_cast<PaSourceOutputStream>(stream)->getDriver() == "module-echo-cancel.c";
            });

            if (it != m_info.sourceOutputList.end()) {
                idx = std::dynamic_pointer_cast<PaSourceOutputStream>(*it)->getSource();
            }
        }

        if (mute != other.mute && (index == other.index || index == idx)) {
            muteChanged(!mute);
        }

        if (active_port && other.active_port && g_strcmp0(active_port->name.c_str(), other.active_port->name)) {
            portChanged(other.active_port->name);
        }
    }

    name = other.name;
    index = other.index;
    description = other.description;
    sample_spec = other.sample_spec;
    channel_map = other.channel_map;
    owner_module = other.owner_module;
    volume = other.volume;
    mute = other.mute;
    monitor_of_sink = other.monitor_of_sink;
    monitor_of_sink_name = other.monitor_of_sink_name ? other.monitor_of_sink_name : "";
    latency = other.latency;
    driver = other.driver;
    flags = other.flags;
    proplist = other.proplist;
    configured_latency = other.configured_latency;
    base_volume = other.base_volume;
    state = other.state;
    n_volume_steps = other.n_volume_steps;
    card = other.card;
    n_ports = other.n_ports;
    n_formats = other.n_formats;
    formats = other.formats;

    //start
    m_tmpMute = mute;
    m_masterDeviceIndex = index;
    //end

    auto deviceClass = pa_proplist_gets(proplist, PA_PROP_DEVICE_CLASS);
    m_deviceClass = deviceClass ? deviceClass : "";
//    AUDIO_DEBUG("copy m_deviceClass: {} name: {} card {}.", m_deviceClass, name, card);
    if (std::string::npos != name.find("noiseReduceSource")) {
        if (m_deviceClass.empty()) {
            m_deviceClass = "filter";
        }

        auto it = std::find_if(m_info.sourceOutputList.begin(), m_info.sourceOutputList.end(), [](const std::shared_ptr<IStream>& s) {
            auto driver = std::dynamic_pointer_cast<PaSourceOutputStream>(s)->getDriver();
            return driver == "module-echo-cancel.c";
        });

        if (it != m_info.sourceOutputList.end()) {
            auto sourceIdx = std::dynamic_pointer_cast<PaSourceOutputStream>(*it)->getSource();
            for (const auto& c : m_info.cardInfoList) {
                auto source = std::find_if(c->sources.begin(), c->sources.end(), [sourceIdx](const std::shared_ptr<IStream>& s) {
                   return sourceIdx == s->getIndex();
                });

                if (source != c->sources.end()) {
                    setMasterDevice((*source)->getName());
                }
            }
        }
    }
    else {
        auto masterDevice = pa_proplist_gets(proplist, PA_PROP_DEVICE_MASTER_DEVICE);
        m_masterDevice = masterDevice ? masterDevice : "";
    }

    ports.clear();
    for (int i = 0; i < n_ports; ++i) {
        ports.push_back(std::make_shared<PaSourcePortInfo>(cardName, *other.ports[i]));
    }
    active_port = other.active_port ? std::make_shared<PaSourcePortInfo>(cardName, *other.active_port) : nullptr;

    //start
    if ("filter" == m_deviceClass) {
        std::string masterDevice = "";
        for (const auto& c : m_info.cardInfoList) {
            auto it = std::find_if(c->sources.begin(), c->sources.end(), [this](const std::shared_ptr<IStream>& s) {
                return this->name == s->getName();
            });

            if (it != c->sources.end()) {
                auto source = std::dynamic_pointer_cast<PaSourceStream>(*it);
                masterDevice = source->getMasterDevice();
                break;
            }
        }

        for (const auto& c : m_info.cardInfoList) {
            auto it = std::find_if(c->sources.begin(), c->sources.end(), [masterDevice](const std::shared_ptr<IStream>& s) {
                return masterDevice == s->getName();
            });

            if (it != c->sources.end()) {
                auto sourceStream = std::dynamic_pointer_cast<PaSourceStream>(*it);
                if (sourceStream) {
                    m_masterDeviceIndex = sourceStream->getIndex();
                    card = sourceStream->getCard();
                    cardName = sourceStream->getCardName();
                    n_ports = sourceStream->getNPorts();
                    ports.clear();
                    ports = sourceStream->getPorts();
                    active_port = sourceStream->getActivePort() ? sourceStream->getActivePort() : nullptr;
                    AUDIO_DEBUG("copy masterDevice sourceStream card: {} cardName: {} n_ports: {}.", card, cardName, n_ports);
                }
                break;
            }
        }
    }
    //end
}


std::vector<std::shared_ptr<PaSourcePortInfo>> PaSourceStream::getPorts() const
{
    return ports;
}

std::string PaSourceStream::toString() const
{
    std::string vol = "[";
    for (int i = 0; i < volume.channels; ++i) {
        vol += std::to_string(volume.values[i]);
        vol += ", ";
    }
    vol += "]";

    std::string activePortName = (active_port) ? active_port->name : "";

    return FORMAT("name={}, index={}, description={}, card={}, n_ports={}, active_port={}, "
                  "volume={}, mute={}, state={}, monitor_of_sink={}, monitor_of_sink_name={}",
                  name, index, description, card, n_ports, activePortName,
                  vol, mute, state, monitor_of_sink, monitor_of_sink_name);
}

NodeType PaSourceStream::getNodeType() const
{
    return NodeType::INPUT;
}

uint32_t PaSourceStream::getIndex() const
{
    return index;
}

std::string PaSourceStream::getName() const
{
    return name;
}

pa_proplist* PaSourceStream::getProplist() const
{
    return proplist;
}

uint32_t PaSourceStream::getCard() const
{
    return card;
}

void PaSourceStream::setVolume(int value)
{
    auto v = volume;
    for (int i = 0; i < v.channels; ++i) {
        v.values[i] = value;
    }

    if (pa_cvolume_equal(&v, &volume))
        return;

    pa_operation_unref(pa_context_set_source_volume_by_index(m_pContext, index, &v, nullptr, nullptr));
}

const pa_cvolume PaSourceStream::getVolume() const
{
    return volume;
}

void PaSourceStream::setBalance(double v)
{
    if (channel_map.channels == 1) {
        AUDIO_ERROR("source {} is not support set balance.", name);
        return;
    }

    pa_cvolume_set_balance(&volume, &channel_map, v);

    pa_operation_unref(pa_context_set_sink_volume_by_index(m_pContext, index, &volume, nullptr, nullptr));
}

double PaSourceStream::getBalance() const
{
    return pa_cvolume_get_balance(&volume, &channel_map);
}

void PaSourceStream::setMute(bool mute)
{
    AUDIO_DEBUG("Set {} mute to {}, this.mute:{} m_tmpMute:{} .", name, mute, this->mute, m_tmpMute);
    //start hand asynchronous time issues
    m_tmpMute = mute;
    //end

    //start synchronous masterdevice mute status
    if (std::string::npos != name.find("noiseReduceSource")) {
        auto it = std::find_if(m_info.sourceOutputList.begin(), m_info.sourceOutputList.end(), [](const std::shared_ptr<IStream>& s) {
            auto driver = std::dynamic_pointer_cast<PaSourceOutputStream>(s)->getDriver();
            return driver == "module-echo-cancel.c";
        });

        if (it != m_info.sourceOutputList.end()) {
            auto sourceIdx = std::dynamic_pointer_cast<PaSourceOutputStream>(*it)->getSource();
            for (const auto& c : m_info.cardInfoList) {
                auto source = std::find_if(c->sources.begin(), c->sources.end(), [sourceIdx](const std::shared_ptr<IStream>& s) {
                    return sourceIdx == s->getIndex();
                });

                if (source != c->sources.end()) {
                    AUDIO_DEBUG("setMute getName:{} .", (*source)->getName());
                    (*source)->setMute(mute);
                }
            }
        }
    }

    //end

    pa_operation_unref(pa_context_set_source_mute_by_index(m_pContext, index, mute, nullptr, nullptr));
}

bool PaSourceStream::getMute() const
{
    //start hand asynchronous time issues
    return m_tmpMute;
    //end
    // return mute;
}

DefaultDeviceInfo PaSourceStream::getDefaultDevice() const
{
    DefaultDeviceInfo info;
    info.idx = m_info.defaultSource->index;
    info.name = m_info.defaultSource->name;

    if ("filter" == m_info.defaultSource->getDeviceClass()) {

        std::string masterDevice = "";
        for (const auto& c : m_info.cardInfoList) {
            auto it = std::find_if(c->sources.begin(), c->sources.end(), [this](const std::shared_ptr<IStream>& s) {
                return m_info.defaultSource->getName() == s->getName();
            });

            if (it != c->sources.end()) {
                auto source = std::dynamic_pointer_cast<PaSourceStream>(*it);
                masterDevice = source->getMasterDevice();

                AUDIO_DEBUG("Get {} master device: {}.", m_info.defaultSource->getName(), masterDevice);
                break;
            }
        }

        for (const auto& c : m_info.cardInfoList) {
            auto it = std::find_if(c->sources.begin(), c->sources.end(), [masterDevice](const std::shared_ptr<IStream>& s) {
                return masterDevice == s->getName();
            });

            if (it != c->sources.end()) {
                info.cardName = c->card.name;
                auto sourceStream = std::dynamic_pointer_cast<PaSourceStream>(*it);
                if (sourceStream) {
                    info.activePortName = sourceStream->getActivePortName();
                    info.activePortLabel = sourceStream->getActivePortDescription();
                }
                break;
            }
        }
    }
    else {
        info.cardName = m_info.defaultSource->cardName;
        info.activePortName = m_info.defaultSource->active_port ? m_info.defaultSource->active_port->name : "";
        info.activePortLabel = m_info.defaultSource->active_port ? m_info.defaultSource->active_port->description : "";
    }

    return info;
}

void PaSourceStream::setDefaultDevice(const std::string& name, const std::string& dev)
{
    bool found = false;
    bool setPort = false;
    bool setProfile = false;
    std::string profileName = "";
    std::string sourceName = "";
    uint32_t sourceIdx = PA_INVALID_INDEX, cardIdx = PA_INVALID_INDEX;

//    AUDIO_DEBUG("Set default source to name: {} dev: {}.", name, dev);
    for (const auto& c : m_info.cardInfoList) {
        if (0 == strcmp(dev.c_str(), c->card.name.c_str())) {
            cardIdx = c->card.index;
//            AUDIO_DEBUG("Set default source cardIdx: {} c->card.name: {}.", cardIdx, c->card.name);

            for (const auto& p : c->card.ports) {
                if (PA_DIRECTION_OUTPUT == p->direction)
                    continue;

                if (strcmp(name.c_str(), p->name.c_str())) {
//                    AUDIO_DEBUG("Set default source continue name: {} p->name: {}.", name, p->name);
                    continue;
                }

//                AUDIO_DEBUG("Set default source name: {} p->name: {}.", name, p->name);
//                AUDIO_DEBUG("Set default source cardIdx: {} m_info.defaultSource->card: {} defaultSource->name: {}."
//                            , cardIdx, m_info.defaultSource->card, m_info.defaultSource->name);

                if (m_info.defaultSource->card == cardIdx
                        && contains(m_info, [name, dev, c](PaDataInfo& data) -> std::vector<std::shared_ptr<PaSourcePortInfo>> {
                                 if (data.defaultSource && c->card.name == dev) {
                                     return data.defaultSource->ports;
                                 }
                                 return {};
                             },name)) {
                    AUDIO_DEBUG("Set default device status1: set port.");
                    setPort = true;
                    if (std::string::npos != m_info.defaultSource->name.find("noiseReduceSource")) {
                        sourceIdx = m_info.defaultSource->getMasterDeviceIndex();
//                        AUDIO_DEBUG("Set default source success getMasterDeviceIndex: {} .", sourceIdx);
                    } else {
                        sourceIdx = m_info.defaultSource->getIndex();
//                        AUDIO_DEBUG("Set default source success sourceIdx: {} .", sourceIdx);
                    }
                    goto success;
                }
                else {
                    if (contains(m_info,
                                 [&p] (PaDataInfo& data) -> std::vector<std::shared_ptr<PaCardProfileInfo2>> {
                                    return p->profiles2;
                                 },
                                 c->card.active_profile->name)) {

                            AUDIO_DEBUG("Set default device status2: set default source.");
                            for (const auto& s : c->sources) {
                                for (const auto &p : std::dynamic_pointer_cast<PaSourceStream>(s)->ports) {
//                                    AUDIO_DEBUG("Set default source port no found name: {} p->name: {} .", name, p->name);
                                    if (p->name == name) {
                                        sourceName = s->getName();
                                        goto success;
                                    }
                                }
                            }
                    }
                    else {
                        AUDIO_DEBUG("Set default device3 status3: set card profile, default source.");
                        int pri = 0;
                        for (const auto& profile : p->profiles2) {
                            if (0 == profile->available)
                                continue;

                            if (pri < profile->priority) {
                                setProfile = true;
                                pri = profile->priority;
                                profileName = profile->name;
//                                AUDIO_DEBUG("Set default source port no found, profiles2 no switch pri {} profileName {}."
//                                            , pri, profileName);
                            }
                        }
                    }
                }
            }
        }
    }

    if (!found)
        AUDIO_DEBUG("Card: {} port: {} not found.", dev, name);
success:
    if (setProfile) {
//        AUDIO_DEBUG("Set default source setProfile cardIdx: {} profileName: {}.", cardIdx, profileName);
       // 勿捕获 this：切 profile 会析构当前 PaSourceStream，导致 UAF 崩溃。
       // detach 线程按值持有 PaDataInfo*，避免局部引用在函数返回后悬空。
       PaDataInfo* info = &m_info;
       pa_context* ctx = m_pContext;
       std::thread([cardIdx, profileName, info, ctx] () {
           std::unique_lock<std::mutex> lock(info->m_mtx);
           info->m_operation = false;
           pa_operation_unref(pa_context_set_card_profile_by_index(ctx, cardIdx, profileName.c_str(), nullptr, nullptr));
       }).join();

       // 设置default source线程，等待主线程cardInfoList更新完毕之后在去设置default source。
       // 保持 detach：若在 GLib/PA 主线程上 join 等待 sourceCb，会与同线程回调死锁。
       std::thread([cardIdx, name, info, ctx] () {
           std::unique_lock<std::mutex> lock(info->m_mtx);
           info->m_cond.wait_for(lock, std::chrono::milliseconds(3000), [info] {
               return info->m_operation;
           });

           if (info->m_operation) {
               info->m_operation = false;

               for (const auto& c: info->cardInfoList) {
                   if (c->card.index == cardIdx) {
                       for (const auto& s : c->sources) {
                           auto source = std::dynamic_pointer_cast<PaSourceStream>(s);
                           if (!source)
                               continue;
                           for (const auto& p : source->ports) {
                               if (p->name == name) {
                                   AUDIO_DEBUG("Set deault source to {} port: {}.", s->getName(), name);
                                   pa_operation_unref(pa_context_set_source_port_by_name(ctx, s->getName().c_str(), name.c_str(), nullptr, nullptr));
                                   pa_operation_unref(pa_context_set_default_source(ctx, s->getName().c_str(), nullptr, nullptr));
                                   return;
                               }
                           }
                       }
                   }
               }
           }
       }).detach();
    }

    if (setPort) {
        AUDIO_DEBUG("Set default source setPort sourceIdx: {} name: {}.", sourceIdx, name);
        pa_operation_unref(pa_context_set_source_port_by_index(m_pContext, sourceIdx, name.c_str(), nullptr, nullptr));
    }

    if ("" != sourceName) {
        AUDIO_DEBUG("Set default source to {}.", sourceName);
        pa_operation_unref(pa_context_set_default_source(m_pContext, sourceName.c_str(), nullptr, nullptr));
    }
}

std::vector<std::string> PaSourceStream::getDeviceList() const
{
    std::vector<std::string> list;
    for (const auto& c : m_info.cardInfoList) {
        if (!c->sources.empty()) {
            list.push_back(c->card.name);
        }
    }
    return list;
}

std::list<std::shared_ptr<DeviceInfo>> PaSourceStream::getAvailablePortList() const
{
    std::list<std::shared_ptr<DeviceInfo>> info;
    for (const auto& c : m_info.cardInfoList) {
        if (c->card.deviceFactor.find("computer") != std::string::npos ||
            c->card.deviceFactor.find("phone") != std::string::npos)
            continue;

        for (const auto& d : c->card.deviceList) {
            if (PA_DIRECTION_INPUT == d->direction && PA_PORT_AVAILABLE_NO != d->available) {
                info.push_back(d);
            }
        }
    }

    return info;
}

void PaSourceStream::volumeChanged(int volume)
{
    AudioMethod::getInstance().volumeChanged(NodeType::INPUT, index, volume);
}

void PaSourceStream::muteChanged(bool mute)
{
    AudioMethod::getInstance().muteChanged(NodeType::INPUT, index, mute);
}

void PaSourceStream::portChanged(const std::string& name)
{
    AudioMethod::getInstance().portChanged(NodeType::INPUT, name);
}

void PaSourceStream::deviceChanged(bool autoPause)
{
    std::string portName, deviceName;
    if ("filter" == m_deviceClass) {
        AUDIO_DEBUG("deviceChanged filter m_masterDevice {}.", m_masterDevice);
        for (const auto& c : m_info.cardInfoList) {
            auto it = std::find_if(c->sources.begin(), c->sources.end(), [this](const std::shared_ptr<IStream>& s) {
                return m_masterDevice == s->getName();
            });

            if (it != c->sources.end()) {
                deviceName = c->card.name;
                auto sourceStream = std::dynamic_pointer_cast<PaSourceStream>(*it);
                if (sourceStream) {
                    portName = sourceStream->getActivePortName();
                }
                AUDIO_DEBUG("deviceChanged filter portName {} deviceName {}.", portName, deviceName);
                break;
            }
        }
    }
    else {
        portName = active_port ? active_port->name : "";
        deviceName = cardName;
        AUDIO_DEBUG("deviceChanged set portName {} deviceName {}.", portName, deviceName);

        if (AudioMethod::getInstance().isLoaded(ModuleType::MODULE_ECHO_CANCEL)) {
            auto it = std::find_if(m_info.sourceOutputList.begin(), m_info.sourceOutputList.end(), [](const std::shared_ptr<IStream>& s) {
                auto driver = std::dynamic_pointer_cast<PaSourceOutputStream>(s)->getDriver();
                return driver == "module-echo-cancel.c";
            });

            if (it != m_info.sourceOutputList.end() && deviceName.find("alsa_card.usb") == std::string::npos
                && deviceName.find("bluez_card") == std::string::npos) {
                AUDIO_DEBUG("prepare set {} default device, card: {}, port: {}.", (*it)->getName(), deviceName, portName);
                (*it)->setDefaultDevice(portName, deviceName);

                // 更新noiseReduceSource的master device
                // std::unique_lock<std::mutex> lock(m_info.m_mtx);
                for (const auto& c : m_info.cardInfoList) {
                    auto source = std::find_if(c->sources.begin(), c->sources.end(), [](const std::shared_ptr<IStream>& s) {
                        return std::string::npos != s->getName().find("noiseReduceSource");
                    });

                    if (source != c->sources.end()) {
                        auto masterDevice = m_info.defaultSource->getName();
                        // 同步default source的音量和静音状态
                        AUDIO_DEBUG("noiseReduceSource masterDevice defaultSource: {} getMute: {}, source getMute: {}."
                                    , masterDevice, m_info.defaultSource->getMute(), (*source)->getMute());
                        (*source)->setMute(m_info.defaultSource->getMute());
                        auto volume = m_info.defaultSource->getVolume();

                        auto value = paCvolumeToPaValue(volume);
                        AUDIO_DEBUG("noiseReduceSource value:{} .", value);
                        (*source)->setVolume(value);

                        std::dynamic_pointer_cast<PaSourceStream>(*source)->setMasterDevice(masterDevice);
                        break;
                    }
                }

                pa_operation_unref(pa_context_set_default_source(m_pContext, "noiseReduceSource", nullptr, nullptr));
            }
        }
    }
    AudioMethod::getInstance().deviceChanged(NodeType::INPUT, portName, deviceName);
}
}

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

#include "PaSinkStream.h"

#include <future>
#include <algorithm>

#include "AudioMethod.h"
#include "PaSinkInputStream.h"


namespace UkuiAudioFramwork
{
std::string PaSinkStream::toString() const
{
    std::string vol = "[";
    for (int i = 0; i < volume.channels; ++i) {
        vol += std::to_string(volume.values[i]);
        vol += ", ";
    }
    vol += "]";

    std::string pln = "[";
    for (int i = 0; i < n_ports; ++i) {
        pln += ports[i]->name;
        pln += ":";
        pln += std::to_string(ports[i]->available);
        pln += ",";
    }
    pln += "]";
    std::string activePortName = (active_port) ? active_port->name : "";

    return FORMAT("name={}, index={}, m_bIsCombine={} description={}, card={}, n_ports={}, active_port={}, "
                  "volume={}, mute={}, state={}, monitor_source={}, monitor_source_name={}, pln = {}",
                  name, index, m_bIsCombine, description, card, n_ports, activePortName,
                  vol, mute, state, monitor_source, monitor_source_name, pln);
}

NodeType PaSinkStream::getNodeType() const
{
    return NodeType::OUTPUT;
}

std::shared_ptr<PaSinkPortInfo> PaSinkStream::getActivePort() const
{
    return active_port;
}

std::string PaSinkStream::getActivePortName() const
{
    return (this && active_port) ? active_port->name : "";
}

std::string PaSinkStream::getActivePortDescription() const
{
    return (this && active_port) ? active_port->description : "";
}

std::string PaSinkStream::getCardName() const
{
    return cardName;
}

std::string PaSinkStream::getDeviceClass() const
{
    return m_deviceClass;
}

void PaSinkStream::setMasterDevice(const std::string& master)
{
    AUDIO_DEBUG("Set {} master device to {}.", name, master);

    m_masterDevice = master;
}

std::string PaSinkStream::getMasterDevice() const
{
    AUDIO_DEBUG("Get {} master device {}.", name, m_masterDevice);

    return m_masterDevice;
}

uint32_t PaSinkStream::getNPorts() const
{
    return n_ports;
}

uint32_t PaSinkStream::getMasterDeviceIndex() const
{
    return m_masterDeviceIndex;
}

void PaSinkStream::copy(const pa_sink_info& other)
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

    if (PA_INVALID_INDEX != index) {
        if (!pa_cvolume_equal(&volume, &other.volume) && other.index == m_info.defaultSink->getIndex()) {
            volumeChanged(paCvolumeToValue(other.volume));
        }

        if (mute != other.mute && index == other.index) {
            muteChanged(!mute);
        }

        if (active_port && other.active_port && g_strcmp0(active_port->name.c_str(), other.active_port->name)) {
            portChanged(other.active_port->name);
        }

        active_port = other.active_port ? std::make_shared<PaSinkPortInfo>(cardName, *other.active_port) : nullptr;
    }

    name = other.name;
    index = other.index;
    description = other.description;
    sample_spec = other.sample_spec;
    channel_map = other.channel_map;
    owner_module = other.owner_module;
    volume = other.volume;
    mute = other.mute;
    monitor_source = other.monitor_source;
    monitor_source_name = other.monitor_source_name;
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

    m_masterDeviceIndex = index;

    auto deviceClass = pa_proplist_gets(proplist, PA_PROP_DEVICE_CLASS);
    m_deviceClass = deviceClass ? deviceClass : "";
    const bool isEchoCancelSink = name == "echo-cancel-sink"
        || (proplist && g_strcmp0(pa_proplist_gets(proplist, "node.name"), "echo-cancel-sink") == 0);
    if (isEchoCancelSink && m_deviceClass.empty()) {
        m_deviceClass = "filter";
    }
    if (name == "mono" && m_deviceClass.empty()) {
        m_deviceClass = "filter";
    }
    if (std::string::npos != name.find("mono")) {
        auto it = std::find_if(m_info.sinkInputList.begin(), m_info.sinkInputList.end(), [](const std::shared_ptr<IStream>& s) {
            const auto input = std::dynamic_pointer_cast<PaSinkInputStream>(s);
            if (!input) {
                return false;
            }
            return input->getDriver() == "module-remap-sink.c"
                || input->getName().find("mono sink output") != std::string::npos;
        });

        if (it != m_info.sinkInputList.end()) {
            auto sinkIdx = std::dynamic_pointer_cast<PaSinkInputStream>(*it)->getSink();
            for (const auto& c : m_info.cardInfoList) {
                auto sink = std::find_if(c->sinks.begin(), c->sinks.end(), [sinkIdx](const std::shared_ptr<IStream>& s) {
                   return sinkIdx == s->getIndex();
                });

                if (sink != c->sinks.end()) {
                    setMasterDevice((*sink)->getName());
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
        ports.push_back(std::make_shared<PaSinkPortInfo>(cardName, *other.ports[i]));
    }
    active_port = other.active_port ? std::make_shared<PaSinkPortInfo>(cardName, *other.active_port) : nullptr;

    //start
    if ("filter" == m_deviceClass) {
        std::string masterDevice = "";
        for (const auto& c : m_info.cardInfoList) {
            auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [this](const std::shared_ptr<IStream>& s) {
                AUDIO_DEBUG("copy s->getName: {} .", s->getName());
                return this->name == s->getName();
            });

            if (it != c->sinks.end()) {
                auto sink = std::dynamic_pointer_cast<PaSinkStream>(*it);
                masterDevice = sink->getMasterDevice();
                break;
            }
        }

        for (const auto& c : m_info.cardInfoList) {
            auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [masterDevice](const std::shared_ptr<IStream>& s) {
                return masterDevice == s->getName();
            });

            if (it != c->sinks.end()) {
                auto sinkStream = std::dynamic_pointer_cast<PaSinkStream>(*it);
                if (sinkStream) {
                    m_masterDeviceIndex = sinkStream->getIndex();
                    card = sinkStream->getCard();
                    cardName = sinkStream->getCardName();
                    n_ports = sinkStream->getNPorts();
                    ports.clear();
                    ports = sinkStream->getPorts();
                    active_port = sinkStream->getActivePort() ? sinkStream->getActivePort() : nullptr;
                    AUDIO_DEBUG("copy masterDevice sinkStream card: {} cardName: {} n_ports: {}.", card, cardName, n_ports);
                }
                break;
            }
        }
    }
    //end
}

std::vector<std::shared_ptr<PaSinkPortInfo>> PaSinkStream::getPorts() const
{
    return ports;
}

bool PaSinkStream::getCombineStatus() const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_bIsCombine;
}

void PaSinkStream::setCombineStatus(bool status)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bIsCombine = status;
}

uint32_t PaSinkStream::getIndex() const
{
    return index;
}

std::string PaSinkStream::getName() const
{
    return name;
}

pa_proplist* PaSinkStream::getProplist() const
{
    return proplist;
}

uint32_t PaSinkStream::getCard() const
{
    return card;
}

void PaSinkStream::setVolume(int value)
{
    auto v = volume;
    auto balance = getBalance();
    for (int i = 0; i < v.channels; ++i) {
        v.values[i] = value;
    }

    pa_cvolume_set_balance(&v, &channel_map, balance);

    if (pa_cvolume_equal(&v, &volume))
        return;

    AUDIO_DEBUG("Set {} volume from {} changed to {}.", name, paCvolumeToPaValue(volume), value);
    pa_operation_unref(pa_context_set_sink_volume_by_index(m_pContext, index, &v, nullptr, nullptr));
}

const pa_cvolume PaSinkStream::getVolume() const
{
    return volume;
}

void PaSinkStream::setBalance(double v)
{
    if (channel_map.channels == 1) {
        AUDIO_ERROR("sink {} is not support set balance.", name);
        return;
    }

    if (v == getBalance())
        return;

    pa_cvolume_set_balance(&volume, &channel_map, v);
    pa_operation_unref(pa_context_set_sink_volume_by_index(m_pContext, index, &volume, nullptr, nullptr));
}

double PaSinkStream::getBalance() const
{
    return pa_cvolume_get_balance(&volume, &channel_map);
}

void PaSinkStream::setMute(bool mute)
{
    if (mute == this->mute)
        return;

    pa_operation_unref(pa_context_set_sink_mute_by_index(m_pContext, index, mute, nullptr, nullptr));
    // 下发后同步本地状态，避免异步回调未到时连续 setMute 被早退跳过
    this->mute = mute;
}

bool PaSinkStream::getMute() const
{
    return mute;
}

DefaultDeviceInfo PaSinkStream::getDefaultDevice() const
{
    DefaultDeviceInfo info;
    info.idx = m_info.defaultSink->index;
    info.name = m_info.defaultSink->name;

    if ("filter" == m_info.defaultSink->getDeviceClass()) {
        if (std::string::npos != info.name.find("combined")) {
            info.activePortName = "output-combine";
            info.cardName = "combine-device";
        }
        else {
            std::string masterDevice = "";
            for (const auto& c : m_info.cardInfoList) {
                auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [this](const std::shared_ptr<IStream>& s) {
                    return m_info.defaultSink->getName() == s->getName();
                });

                if (it != c->sinks.end()) {
                    auto sink = std::dynamic_pointer_cast<PaSinkStream>(*it);
                    masterDevice = sink->getMasterDevice();

                    AUDIO_DEBUG("Get {} master device: {}.", m_info.defaultSink->getName(), masterDevice);
                    break;
                }
            }

            // 存在单声道音频所在的card和Master不在同一个card的情况因此需要重新查找，后续优化
            for (const auto& c : m_info.cardInfoList) {
                auto at = std::find_if(c->sinks.begin(), c->sinks.end(), [masterDevice](const std::shared_ptr<IStream>& s) {
                    return masterDevice == s->getName();
                });

                if (at == c->sinks.end()) {
                    AUDIO_WARN("Can not found {} master device {} on card {}.", m_info.defaultSink->getName(), masterDevice, c->card.name);
                    continue;
                }
                else {
                    info.cardName = c->card.name;
                    auto sink = std::dynamic_pointer_cast<PaSinkStream>(*at);
                    if (sink) {
                        info.activePortName = sink->getActivePortName();
                        info.activePortLabel = sink->getActivePortDescription();
                        AUDIO_DEBUG("Found {} master device {}, card: {}, port: {}, portDesc: {}.",
                                    sink->getName(), masterDevice, info.cardName, info.activePortName, info.activePortLabel);
                    }

                    break;
                }
            }
        }
    }
    else {
        info.cardName = cardName;
        info.activePortName = m_info.defaultSink->active_port ? m_info.defaultSink->active_port->name : "";
        info.activePortLabel = m_info.defaultSink->active_port ? m_info.defaultSink->active_port->description : "";
    }

    return info;
}

/***
 * 1.如果需要切换的端口和当前默认的输出设备在同一个sink上，则只需要切换端口即可（pa_context_set_sink_port_by_index）
 * 2.需要切换的端口在不同的配置文件下，则需要切换一个高优先级的配置文件后，在设置默认的输出设备(pa_context_set_card_profile, pa_context_set_default_sink)
 * 3.需要切换的端口在相同的配置文件下，则只需要设置默认输出设备即可（pa_context_set_default_sink, 例如：HiFi）
 */
void PaSinkStream:: setDefaultDevice(const std::string& name, const std::string& dev)
{
    auto found = false;
    auto setPort = false;
    auto setProfile = false;
    std::string profileName = "";
    std::string sinkName = "";
    auto sinkIdx = PA_INVALID_INDEX, cardIdx = PA_INVALID_INDEX;

    // 直接设置组合输出设备，后续存在多个组合设备时需要优化
    if (std::string::npos != name.find("output-combine")) {
        pa_operation_unref(pa_context_set_default_sink(m_pContext, "combined", nullptr, nullptr));
        return;
    }

    for (const auto& c : m_info.cardInfoList) {
        if (strcmp(dev.c_str(), c->card.name.c_str()))
            continue;

        cardIdx = c->card.index;

        for (const auto& p : c->card.ports) {
            if (PA_DIRECTION_INPUT == p->direction)
                continue;

            if (strcmp(name.c_str(), p->name.c_str()))
                continue;

            // 如果sink列表的可用端口存在需要切换的端口，则只需要进行setSinkPort操作即可
            auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [name](const std::shared_ptr<IStream>& s) {
                auto ports = std::dynamic_pointer_cast<PaSinkStream>(s)->getPorts();
                auto at = std::find_if(ports.begin(), ports.end(), [name](const std::shared_ptr<PaSinkPortInfo>& p) {
                    return p->name == name;
                });

                return (at != ports.end());
            });

            if (it != c->sinks.end())  {
                setPort = true;
                sinkIdx = (*it)->getIndex();
                pa_operation_unref(pa_context_set_default_sink(m_pContext, (*it)->getName().c_str(), nullptr, nullptr));
                goto success;
            }
            else {
                auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [p](const std::shared_ptr<IStream> s) {
                    return std::dynamic_pointer_cast<PaSinkStream>(s)->getActivePortName() == p->name;
                });

                if (it != c->sinks.end()) {
                    sinkName = (*it)->getName();
                }
                else {
                    int pri = 0;
                    for (const auto& profile : p->profiles2) {
                        if (0 == profile->available)
                            continue;

                        if (pri < profile->priority) {
                            setProfile = true;
                            pri = profile->priority;
                            profileName = profile->name;
                        }
                    }
                }
            }
        }

    }

    if (!found)
        AUDIO_DEBUG("Card: {} port: {} not found", dev, name);
success:
    AUDIO_DEBUG("Set default device, setProfile: {}, setPort: {}, sinkName: {}.", setProfile, setPort, sinkName);
    if (setProfile) {
        // 由于pa_context_set_card_profile_by_index接口是异步的，使用detach会出现线程实际还没执行完就释放资源，
        // 会出现配置文件没有切换过去的问题,因此修改成join

        AUDIO_DEBUG("Set card: {} profile to {}.", dev, profileName);
        // 勿捕获 this：切 profile / 卸 mono 会析构当前 PaSinkStream，导致 UAF 崩溃。
        // PaDataInfo / pa_context 随 PulseaudioManager 存活；detach 线程按值持有指针，避免局部引用悬空。
        PaDataInfo* info = &m_info;
        pa_context* ctx = m_pContext;
        std::thread([cardIdx, profileName, info, ctx] () {
            // 设置配置文件需要重设m_operation为false防止设置完配置文件时还没有更新sinks/sources就去设置默认的sink/source
            std::unique_lock<std::mutex> lock(info->m_mtx);
            info->m_operation = false;
            pa_operation* o = pa_context_set_card_profile_by_index(ctx, cardIdx, profileName.c_str(), nullptr, nullptr);

            pa_operation_unref(o);
        }).join();

        // 设置default sink线程，等待主线程cardInfoList更新完毕之后在去设置default sink。
        // 保持 detach：若在 GLib/PA 主线程上 join 等待 sinkCb，会与同线程回调死锁。
        std::thread([cardIdx, name, info, ctx] () {
            std::unique_lock<std::mutex> lock(info->m_mtx);
            info->m_cond.wait_for(lock, std::chrono::milliseconds(3000), [info] {
                return info->m_operation;
            });

            if (info->m_operation) {
                info->m_operation = false;

                for (const auto& c: info->cardInfoList) {
                    if (c->card.index != cardIdx)
                        continue;

                    for (const auto& s : c->sinks) {
                        auto sink = std::dynamic_pointer_cast<PaSinkStream>(s);
                        if (!sink)
                            continue;
                        for (const auto& p : sink->ports) {
                            if (p->name == name) {
                                AUDIO_DEBUG("Set deault sink to {} port: {}.", s->getName(), name);
                                pa_operation_unref(pa_context_set_sink_port_by_name(ctx, s->getName().c_str(), name.c_str(), nullptr, nullptr));
                                pa_operation_unref(pa_context_set_default_sink(ctx, s->getName().c_str(), nullptr, nullptr));
                                return;
                            }
                        }

                    }
                }
            }
        }).detach();
    }

    if (setPort) {
        AUDIO_DEBUG("Set sink port to {}", name);
        pa_operation_unref(pa_context_set_sink_port_by_index(m_pContext, sinkIdx, name.c_str(), nullptr, nullptr));
    }

    if ("" != sinkName) {
        AUDIO_DEBUG("Set default sink to {}", sinkName);
        pa_operation_unref(pa_context_set_default_sink(m_pContext, sinkName.c_str(), nullptr, nullptr));
    }
}

std::vector<std::string> PaSinkStream::getDeviceList() const
{
    std::vector<std::string> list;
    for (const auto& c : m_info.cardInfoList) {
        if (!c->sinks.empty()) {
            list.push_back(c->card.name);
        }
    }
    return list;
}

std::list<std::shared_ptr<DeviceInfo>> PaSinkStream::getAvailablePortList() const
{
    std::list<std::shared_ptr<DeviceInfo>> info;

    for (const auto& c : m_info.cardInfoList) {
        for (const auto& d : c->card.deviceList) {
            if (PA_DIRECTION_OUTPUT == d->direction && PA_PORT_AVAILABLE_NO != d->available) {
                info.push_back(d);
            }
        }
    }

    return info;
}

void PaSinkStream::volumeChanged(int volume)
{
    AudioMethod::getInstance().volumeChanged(NodeType::OUTPUT, index, volume);
}

void PaSinkStream::muteChanged(bool mute)
{
    AudioMethod::getInstance().muteChanged(NodeType::OUTPUT, index, mute);
}

void PaSinkStream::portChanged(const std::string& name)
{
    AudioMethod::getInstance().portChanged(NodeType::OUTPUT, name);
}

void PaSinkStream::deviceChanged(bool autoPause)
{
    std::string portName, deviceName;

    if ("filter" == m_deviceClass) {
        if (name == "combined") {
            portName = "output-combine";
            deviceName = "combine-device";
        }
        else {
            for (const auto& c : m_info.cardInfoList) {
                auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [this](const std::shared_ptr<IStream>& s) {
                    return m_masterDevice == s->getName();
                });

                if (it != c->sinks.end()) {
                    deviceName = c->card.name;
                    auto sink = std::dynamic_pointer_cast<PaSinkStream>(*it);
                    if (sink) {
                        portName = sink->getActivePortName();
                    }
                    break;
                }
            }
        }
    }
    else {
        portName = active_port ? active_port->name : "";
        deviceName = cardName;
    }

    if (!AudioMethod::getInstance().getVolumeBoostStatus()) {
        int v = paCvolumeToPaValue(volume);
        if (v > PA_VOLUME_NORMAL) {
            AUDIO_DEBUG("Device changed. Volume-Increase is turned off, device: {} needs to be set volume: {} to 65536", name, v);
            AudioMethod::getInstance().setVolume(NodeType::OUTPUT, index, UKMEDIA_VOLUME_NORMAL);
        }
    }

    if ("filter" != m_deviceClass && AudioMethod::getInstance().isLoaded(ModuleType::MODULE_REMAP_SINK)) {
        auto it = std::find_if(m_info.sinkInputList.begin(), m_info.sinkInputList.end(), [](const std::shared_ptr<IStream>& s) {
            const auto input = std::dynamic_pointer_cast<PaSinkInputStream>(s);
            if (!input) {
                return false;
            }
            return input->getDriver() == "module-remap-sink.c"
                || input->getName().find("mono sink output") != std::string::npos;
        });

        if (it != m_info.sinkInputList.end()) {
            AUDIO_DEBUG("Device changed. module-remap-sink, portName: {} deviceName: {}. ", portName, deviceName);
            (*it)->setDefaultDevice(portName, deviceName);

            // 更新mono sink的master device
//            std::unique_lock<std::mutex> lock(m_info.m_mtx);
            for (const auto& c : m_info.cardInfoList) {
                auto sink = std::find_if(c->sinks.begin(), c->sinks.end(), [](const std::shared_ptr<IStream>& s) {
                    return std::string::npos != s->getName().find("mono");
                });

                if (sink != c->sinks.end()) {
                    auto masterDevice = m_info.defaultSink->getName();
#if 0
                /* 暂时移除同步单声道音量的设置
                    // 同步default sink的音量和静音状态
                    (*sink)->setMute(m_info.defaultSink->getMute());
                    auto volume = m_info.defaultSink->getVolume();

                    auto value = paCvolumeToPaValue(volume);
                    (*sink)->setVolume(value);
                */
#endif
                    std::dynamic_pointer_cast<PaSinkStream>(*sink)->setMasterDevice(masterDevice);
                    break;
                }
            }
            {
                pa_operation_unref(pa_context_set_default_sink(m_pContext, "mono", nullptr, nullptr));
            }
        }
    }

    AudioMethod::getInstance().deviceChanged(NodeType::OUTPUT, portName, deviceName, autoPause);
}

void PaSinkStream::setCardProfileCb(pa_context *ctx, int success, void *userdata)
{
//    PaSinkStream* stream = static_cast<PaSinkStream*>(userdata);
//    if (!success) {
//        AUDIO_ERROR("Set card profile failed, error: {}.", pa_strerror(pa_context_errno(ctx)));
//        return;
//    }
}
}

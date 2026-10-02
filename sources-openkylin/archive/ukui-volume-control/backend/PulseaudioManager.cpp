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

#include "PulseaudioManager.h"
#include <algorithm>
#include <functional>
#include <optional>
#include <chrono>

#include "PaSinkStream.h"
#include "PaSourceStream.h"


namespace UkuiAudioFramwork
{
pa_mainloop_api* PulseaudioManager::m_pApi = nullptr;
pa_context* PulseaudioManager::m_pContext = nullptr;


template <typename Range>
static std::shared_ptr<IStream> findStream(const Range& rng, const std::string& name)
{
    auto it = std::find_if(rng.begin(), rng.end(), [name](const std::shared_ptr<IStream>& s) {
        return s->getName() == name;
    });
    return it != rng.end() ? *it : nullptr;
}

static bool isEchoCancelCaptureStream(const std::string& driver, const std::string& streamName)
{
    if (driver == "module-echo-cancel.c") {
        return true;
    }

    return std::string::npos != streamName.find("Echo-Cancel Capture");
}

static bool isEchoCancelPlaybackStream(const std::string& driver, const std::string& streamName)
{
    if (driver == "module-echo-cancel.c") {
        return true;
    }

    return std::string::npos != streamName.find("Echo-Cancel Playback");
}

static bool isMonoRemapOutputStream(const std::string& driver, const std::string& streamName)
{
    if (driver == "module-remap-sink.c") {
        return true;
    }

    return std::string::npos != streamName.find("mono sink output");
}

static void updateMonoMaster(PaDataInfo& info, uint32_t masterSinkIndex)
{
    std::shared_ptr<IStream> master;
    for (const auto& card : info.cardInfoList) {
        const auto it = std::find_if(card->sinks.begin(), card->sinks.end(),
                                     [masterSinkIndex](const std::shared_ptr<IStream>& s) {
            return s->getIndex() == masterSinkIndex;
        });
        if (it != card->sinks.end()) {
            master = *it;
            break;
        }
    }
    if (!master) {
        return;
    }

    const std::string masterName = master->getName();
    for (const auto& card : info.cardInfoList) {
        for (const auto& s : card->sinks) {
            if (s->getName() == "mono") {
                std::dynamic_pointer_cast<PaSinkStream>(s)->setMasterDevice(masterName);
            }
        }
    }
    if (info.defaultSink && info.defaultSink->getName() == "mono") {
        std::dynamic_pointer_cast<PaSinkStream>(info.defaultSink)->setMasterDevice(masterName);
    }
}

static void resolveMonoMaster(PaDataInfo& info)
{
    for (const auto& stream : info.sinkInputList) {
        const auto input = std::dynamic_pointer_cast<PaSinkInputStream>(stream);
        if (input && isMonoRemapOutputStream(input->getDriver(), input->getName())) {
            updateMonoMaster(info, input->getSink());
            return;
        }
    }
}

static void updateEchoCancelMaster(PaDataInfo& info, NodeType type,
                                   uint32_t targetIndex, const char* filterName)
{
    std::shared_ptr<IStream> master;
    std::shared_ptr<IStream> filter;
    for (const auto& card : info.cardInfoList) {
        const auto& streams = type == NodeType::INPUT ? card->sources : card->sinks;
        if (!master) {
            const auto it = std::find_if(streams.begin(), streams.end(), [targetIndex](const std::shared_ptr<IStream>& s) {
                return s->getIndex() == targetIndex;
            });
            master = it != streams.end() ? *it : nullptr;
        }
        if (!filter) {
            filter = findStream(streams, filterName);
        }
    }

    if (master && filter) {
        if (type == NodeType::INPUT) {
            std::dynamic_pointer_cast<PaSourceStream>(filter)->setMasterDevice(master->getName());
        } else {
            std::dynamic_pointer_cast<PaSinkStream>(filter)->setMasterDevice(master->getName());
        }
    }
}

static void resolveEchoCancelMasters(PaDataInfo& info)
{
    for (const auto& stream : info.sourceOutputList) {
        const auto output = std::dynamic_pointer_cast<PaSourceOutputStream>(stream);
        if (output && isEchoCancelCaptureStream(output->getDriver(), output->getName())) {
            updateEchoCancelMaster(info, NodeType::INPUT, output->getSource(), "noiseReduceSource");
            break;
        }
    }
    for (const auto& stream : info.sinkInputList) {
        const auto input = std::dynamic_pointer_cast<PaSinkInputStream>(stream);
        if (input && isEchoCancelPlaybackStream(input->getDriver(), input->getName())) {
            updateEchoCancelMaster(info, NodeType::OUTPUT, input->getSink(), "echo-cancel-sink");
            break;
        }
    }
}

static long long nowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

bool PulseaudioManager::shouldShowOsdDedup(const std::string& newDefaultSinkName, long long now)
{
    std::lock_guard<std::mutex> lk(m_osdMtx);
    const long long last = m_lastOsdMs.load(std::memory_order_relaxed);

    // 同一默认设备在短时间内重复触发，视为同一次事件
    if (m_lastOsdDefaultSinkName == newDefaultSinkName && (now - last) < 800) {
        return false;
    }

    m_lastOsdDefaultSinkName = newDefaultSinkName;
    m_lastOsdMs.store(now, std::memory_order_relaxed);
    return true;
}


PulseaudioManager::PulseaudioManager(int times)
    : m_reconnectTime(times)
{

}

std::shared_ptr<PulseaudioManager::ModuleLoadState>
PulseaudioManager::registerModuleLoad(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_moduleStatesMtx);
    if (m_moduleStates.find(name) != m_moduleStates.end()) {
        return nullptr;
    }

    auto state = std::make_shared<ModuleLoadState>();
    m_moduleStates.emplace(name, state);
    return state;
}

std::shared_ptr<PulseaudioManager::ModuleLoadState>
PulseaudioManager::findModuleLoadState(const std::string& name)
{
    std::lock_guard<std::mutex> lock(m_moduleStatesMtx);
    auto it = m_moduleStates.find(name);
    return it != m_moduleStates.end() ? it->second : nullptr;
}

void PulseaudioManager::setModuleLoadIndex(
    const std::shared_ptr<ModuleLoadState>& state,
    uint32_t index)
{
    std::lock_guard<std::mutex> lock(state->mutex);
    if (state->result == ModuleLoadResult::Pending) {
        state->moduleIndex = index;
    }
}

bool PulseaudioManager::completeModuleLoad(const std::string& name, uint32_t ownerModule)
{
    auto state = findModuleLoadState(name);
    if (!state) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(state->mutex);
        if (state->result != ModuleLoadResult::Pending ||
            state->moduleIndex == PA_INVALID_INDEX ||
            state->moduleIndex != ownerModule) {
            return false;
        }
        state->result = ModuleLoadResult::Succeeded;
    }
    state->condition.notify_all();
    return true;
}

void PulseaudioManager::failModuleLoad(const std::shared_ptr<ModuleLoadState>& state)
{
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        if (state->result != ModuleLoadResult::Pending) {
            return;
        }
        state->result = ModuleLoadResult::Failed;
    }
    state->condition.notify_all();
}

void PulseaudioManager::removeModuleLoadState(
    const std::string& name,
    const std::shared_ptr<ModuleLoadState>& state)
{
    std::lock_guard<std::mutex> lock(m_moduleStatesMtx);
    auto it = m_moduleStates.find(name);
    if (it != m_moduleStates.end() && it->second == state) {
        m_moduleStates.erase(it);
    }
}

void PulseaudioManager::cancelModuleLoad(const std::string& name)
{
    auto state = findModuleLoadState(name);
    if (!state) {
        return;
    }
    failModuleLoad(state);
    removeModuleLoadState(name, state);
}

// Interface
void PulseaudioManager::loadModule(const std::string& name, const std::string& param)
{
    auto it = std::find_if(m_info.moduleInfoList.begin(), m_info.moduleInfoList.end(), [name](const PaModuleInfo& info) {
        return info.name == name;
    });
    if (it != m_info.moduleInfoList.end()) {
        AUDIO_WARN("{} has been loaded and does not need to be loaded again.", name);
        return;
    }


    auto moduleState = registerModuleLoad(name);
    if (!moduleState) {
        AUDIO_WARN("{} is already loading.", name);
        return;
    }

    std::thread([name, param, moduleState, this] () {
        {
            std::unique_lock<std::mutex> lock(m_contextMtx);
            if (!m_bContexReady)
                m_contextCond.wait_for(lock, std::chrono::milliseconds(3000));

            if (!m_bContexReady) {
                AUDIO_WARN("Can not load {}, wrong context state.", name);
                failModuleLoad(moduleState);
                return;
            }
        }

        if (!evaluateLoadModulePreconditions(name, param)) {
            failModuleLoad(moduleState);
            return;
        }

        {
            pa_mainloop* mainloop = pa_mainloop_new();
            if (!mainloop) {
                failModuleLoad(moduleState);
                return;
            }

            pa_mainloop_api* api = pa_mainloop_get_api(mainloop);
            pa_context* context = pa_context_new(api, "Module Loader");

            if (!context) {
                pa_mainloop_free(mainloop);
                failModuleLoad(moduleState);
                return;
            }

            struct State {
                bool context_ready = false;
                bool operation_done = false;
                uint32_t module_index = 0;
                bool success = false;
            } state;

            pa_context_set_state_callback(context, [](pa_context* c, void* userdata) {
                auto s = static_cast<State*>(userdata);
                if (pa_context_get_state(c) == PA_CONTEXT_READY) {
                    s->context_ready = true;
                }
            }, &state);

            if (pa_context_connect(context, nullptr, PA_CONTEXT_NOFLAGS, nullptr) < 0) {
                pa_context_unref(context);
                pa_mainloop_free(mainloop);
                failModuleLoad(moduleState);
                return;
            }

            int ret;
            int timeout = 300;
            while (!state.context_ready && timeout-- > 0) {
                pa_mainloop_iterate(mainloop, 0, &ret);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            if (!state.context_ready) {
                AUDIO_ERROR("Timeout connecting to pulseAudio, load module failed.");
                pa_context_unref(context);
                pa_mainloop_free(mainloop);
                failModuleLoad(moduleState);
                return ;
            }

            pa_operation* op = pa_context_load_module(context,
                                                      name.c_str(),
                                                      param.c_str(),
                                                      [](pa_context* c, uint32_t idx, void* userdata) {
                    auto s = static_cast<State*>(userdata);
                    s->operation_done = true;
                    s->module_index = idx;
                    s->success = (idx != PA_INVALID_INDEX);
            }, &state);

            if (!op) {
                AUDIO_ERROR("Failed to create load module operation.");
                pa_context_unref(context);
                pa_mainloop_free(mainloop);
                failModuleLoad(moduleState);
                return;
            }

            if (std::string::npos != name.find("module-loopback")) {
                m_bModuleLoopbackLoading = true;
            }

            timeout = 300;
            while (!state.operation_done && timeout-- > 0) {
                pa_mainloop_iterate(mainloop, 0, &ret);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            if (!state.operation_done) {
                AUDIO_WARN("Load module operation timeout, can not load module {}", name);
                pa_operation_cancel(op);
                failModuleLoad(moduleState);
            } else if (state.success) {
                AUDIO_DEBUG("Module {} loaded successfully with index {}.", name, state.module_index);
                setModuleLoadIndex(moduleState, state.module_index);
                pa_operation_unref(pa_context_get_module_info_list(m_pContext, moduleInfoCb, this));
                // 主动查询目标设备，避免订阅事件先到达而遗漏就绪通知
                pa_operation* infoOp = nullptr;
                if (name == "module-echo-cancel") {
                    infoOp = pa_context_get_source_info_by_name(
                        m_pContext, "noiseReduceSource", sourceCb, this);
                } else if (name == "module-remap-sink") {
                    infoOp = pa_context_get_sink_info_by_name(m_pContext, "mono", sinkCb, this);
                } else if (name == "module-combine-sink") {
                    infoOp = pa_context_get_sink_info_by_name(m_pContext, "combined", sinkCb, this);
                }
                if (infoOp) {
                    pa_operation_unref(infoOp);
                }
            } else {
                failModuleLoad(moduleState);
            }

            pa_operation_unref(op);
            pa_context_unref(context);
            pa_mainloop_free(mainloop);
        }

    }).detach();

    // 等待当前模块的目标设备就绪，再设置默认输入或输出设备
    std::thread([name, param, moduleState, this] () {
        {
            std::unique_lock<std::mutex> lock(moduleState->mutex);
            const bool completed = moduleState->condition.wait_for(
                lock,
                std::chrono::milliseconds(3000),
                [moduleState] {
                    return moduleState->result != ModuleLoadResult::Pending;
                });

            /** If the module does not need to be loaded or the module loading timeout occurs,
             *  no operation is required.
             */
            if (!completed) {
                moduleState->result = ModuleLoadResult::Failed;
            }
            if (moduleState->result != ModuleLoadResult::Succeeded) {
                AUDIO_DEBUG("{} module was not loaded or the loading timed out, no need to configure the default sink/source.", name);
                lock.unlock();
                removeModuleLoadState(name, moduleState);
                return;
            }
        }

        removeModuleLoadState(name, moduleState);

        if ("module-echo-cancel" == name) {
            AUDIO_DEBUG("Loaded module-echo-cancel, set default source to noiseReduceSource.");
            pa_operation_unref(pa_context_set_default_source(m_pContext, "noiseReduceSource", nullptr, nullptr));
        }
        else if ("module-remap-sink" == name) {
            AUDIO_DEBUG("Loaded module-remap-sink, set default sink to mono.");

            // 单声道输出仍经 master 物理 sink，开启前须将左右声道平衡复位，否则偏侧时仅一侧有声
            bool masterBalanceReset = false;
            for (const auto& c : m_info.cardInfoList) {
                auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [](const std::shared_ptr<IStream>& s) {
                    return s->getName() == "mono";
                });

                if (it != c->sinks.end()) {
                    auto master = getMasterStream(it->get(), NodeType::OUTPUT);
                    if (master) {
                        AUDIO_DEBUG("Reset master device {} balance to center before enabling mono.",
                                    master->getName());
                        master->setBalance(0);
                        masterBalanceReset = true;
                    }
                    break;
                }
            }

            if (!masterBalanceReset && m_info.defaultSink) {
                // mono sink 尚未就绪时，回退到当前物理默认输出设备
                auto sink = std::dynamic_pointer_cast<PaSinkStream>(m_info.defaultSink);
                if (sink && sink->getName() != "mono" && sink->getDeviceClass() != "filter") {
                    AUDIO_DEBUG("Reset default output device {} balance to center before enabling mono.",
                                sink->getName());
                    sink->setBalance(0);
                }
            }

            pa_cvolume volume;
            pa_cvolume_set(&volume, 1, PA_VOLUME_NORM);
            pa_operation_unref(pa_context_set_sink_volume_by_name(m_pContext, "mono", &volume, nullptr, nullptr));
            pa_operation_unref(pa_context_set_default_sink(m_pContext, "mono", nullptr, nullptr));
        }
        else if ("module-combine-sink" == name) {
            auto devList = parseSlaves(param);

            for (const auto& dev : devList) {
                for (const auto& c : m_info.cardInfoList) {
                    auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [dev](const std::shared_ptr<IStream>& s){
                        return dev == s->getName();
                    });

                    if (it != c->sinks.end()) {
                        std::dynamic_pointer_cast<PaSinkStream>(*it)->setCombineStatus(true);
                    }
                }
            }

            deviceAdjust(NodeType::OUTPUT);
            AUDIO_DEBUG("Loaded module-combine-sink, set default sink to combined.");
            pa_operation_unref(pa_context_set_default_sink(m_pContext, "combined", nullptr, nullptr));
        }
    }).detach();
}

void PulseaudioManager::unloadModule(const std::string& name, bool restoreMasterDevice)
{
    // 防止卸载过程中仍有线程等待该模块加载完成
    cancelModuleLoad(name);

    uint32_t idx = PA_INVALID_INDEX;
    auto it = std::find_if(m_info.moduleInfoList.begin(),
                           m_info.moduleInfoList.end(),
                           [name](const PaModuleInfo& info) {
        return info.name == name;
    });

    if (it != m_info.moduleInfoList.end()) {
        idx = (*it).index;
        m_info.moduleInfoList.erase(it);
    }

    if (PA_INVALID_INDEX == idx) {
        AUDIO_ERROR("Unload {} module failed, not found such module.", name);
        return;
    }

    /** If the mono module is uninstalled, the default output device needs
     *  to be set to the master device of mono.
     */
    if (name == "module-remap-sink" && restoreMasterDevice) {
        std::string masterDevice;
        // 先尝试从 m_info.defaultSink 获取，或遍历所有 sinks 找到名为 "mono" 的 sink
        if (m_info.defaultSink && m_info.defaultSink->getName() == "mono") {
            masterDevice = std::dynamic_pointer_cast<PaSinkStream>(m_info.defaultSink)->getMasterDevice();
        } else {
            // 遍历所有 sinks 找到名为 "mono" 的 sink
            for (const auto& c : m_info.cardInfoList) {
                auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [](const std::shared_ptr<IStream>& s) {
                    return s->getName() == "mono";
                });
                if (it != c->sinks.end()) {
                    masterDevice = std::dynamic_pointer_cast<PaSinkStream>(*it)->getMasterDevice();
                    break;
                }
            }
        }
        // 如果找到了 master device，就设置它为默认设备
        if (!masterDevice.empty() && m_pContext) {
            AUDIO_DEBUG("Unloading module-remap-sink, setting default sink to master device: {}", masterDevice);
            pa_operation_unref(pa_context_set_default_sink(m_pContext, masterDevice.c_str(), nullptr, nullptr));
        }
    } else if (name == "module-echo-cancel" && restoreMasterDevice) {
        std::string masterDevice;
        // 尝试从 m_info.defaultSource 获取，或遍历所有 sources 找到 filter device
        if (m_info.defaultSource) {
            auto sourceStream = std::dynamic_pointer_cast<PaSourceStream>(m_info.defaultSource);
            if (sourceStream && sourceStream->getDeviceClass() == "filter") {
                masterDevice = sourceStream->getMasterDevice();
            }
        }
        // 如果没找到，遍历所有 sources 来找
        if (masterDevice.empty()) {
            for (const auto& c : m_info.cardInfoList) {
                auto it = std::find_if(c->sources.begin(), c->sources.end(), [](const std::shared_ptr<IStream>& s) {
                    auto sourceStream = std::dynamic_pointer_cast<PaSourceStream>(s);
                    return sourceStream && sourceStream->getDeviceClass() == "filter";
                });
                if (it != c->sources.end()) {
                    masterDevice = std::dynamic_pointer_cast<PaSourceStream>(*it)->getMasterDevice();
                    break;
                }
            }
        }
        // 如果找到了 master device，就设置它为默认设备
        if (!masterDevice.empty() && m_pContext) {
            AUDIO_DEBUG("Unloading module-echo-cancel, setting default source to master device: {}", masterDevice);
            pa_operation_unref(pa_context_set_default_source(m_pContext, masterDevice.c_str(), nullptr, nullptr));
        }
    } else if (name == "module-loopback") {
        m_bModuleLoopbackLoading = false;
    }

    AUDIO_DEBUG("Unload module {}.", name);
    pa_operation_unref(pa_context_unload_module(m_pContext, idx, nullptr, nullptr));
}

// Method
void PulseaudioManager::setDetailVolume(const NodeType& type, const std::string& port, const std::string& card, int volume)
{
    std::shared_ptr<IStream> s = foundInfoByName(type, port, card);
    if (nullptr == s) {
        AUDIO_ERROR("Set {} detail volume failed, not found such port: {} on card: {}.", getNodeTypeName(type), port, card);
        return;
    }

    s->setVolume(valueToPaVolume(volume));
}

int PulseaudioManager::getDetailVolume(const NodeType& type, const std::string& name, const std::string& dev) const
{
    pa_cvolume v {};
    std::shared_ptr<IStream> s = foundInfoByName(type, name, dev);
    if (nullptr == s) {
        AUDIO_ERROR("Get {} volume failed, not found such port: {} on device: {}.", getNodeTypeName(type), name, dev);
        return {};
    }

    v = s->getVolume();

    return paVolumeToValue(paCvolumeToPaValue(v));
}

void PulseaudioManager::setVolume(const NodeType& type, int idx, int volume)
{
    std::shared_ptr<IStream> s = foundInfoByName(type, idx, true);
    if (nullptr == s) {
        AUDIO_ERROR("set {} volume failed, not found such device.", idx);
        return;
    }

    std::string deviceClass = "";
    if (NodeType::OUTPUT == type)
        deviceClass = std::dynamic_pointer_cast<PaSinkStream>(s)->getDeviceClass();
    else if (NodeType::INPUT == type) {
        deviceClass = std::dynamic_pointer_cast<PaSourceStream>(s)->getDeviceClass();
    }

    if (deviceClass == "filter") {
        auto master = getMasterStream(s.get(), type);
        if (master) {
            AudioMethod::getInstance().volumeChanged(type, s->getIndex(), volume);
            s = master;
        }
        else
            AUDIO_ERROR("Filter stream {} has no valid master device, volume not changed.", s->getName());
    }

    s->setMute(false);
    s->setVolume(valueToPaVolume(volume));
}

int PulseaudioManager::getVolume(const NodeType& type, int idx) const
{
    int ret = 0;
    std::shared_ptr<IStream> s = foundInfoByName(type, idx, true);
    if (nullptr == s) {
        AUDIO_ERROR("Get {} volume failed, not found such device.", idx);
        return ret;
    }

    std::string deviceClass = "";
    if (NodeType::OUTPUT == type)
        deviceClass = std::dynamic_pointer_cast<PaSinkStream>(s)->getDeviceClass();
    else if (NodeType::INPUT == type) {
        deviceClass = std::dynamic_pointer_cast<PaSourceStream>(s)->getDeviceClass();
    }

    if (deviceClass == "filter") {
        auto master = getMasterStream(s.get(), type);
        if (master) {
            s = master;
        }
        else {
            AUDIO_ERROR("Filter stream {} has no valid master device .", s->getName());
        }
    }

    auto v = s->getVolume();

    return paVolumeToValue(paCvolumeToPaValue(v));
}

void PulseaudioManager::setDetailMute(const NodeType& type, const std::string& name, const std::string& dev, bool mute)
{
    auto s = foundInfoByName(type, name, dev);
    if (nullptr == s) {
        AUDIO_ERROR("Set {} detail mute failed, not found such port:{} on device: {}.", getNodeTypeName(type), name, dev);
        return;
    }

    s->setMute(mute);
}

bool PulseaudioManager::getDetailMute(const NodeType& type, const std::string& name, const std::string& dev) const
{
    auto s = foundInfoByName(type, name, dev);
    if (nullptr == s) {
        AUDIO_ERROR("get {} mute failed, not found such device on {}.", name, dev);
        return false;
    }

    AUDIO_DEBUG("Get {} {} detail mute.", getNodeTypeName(type), name);
    return s->getMute();
}

void PulseaudioManager::setBalance(const NodeType& type, double v)
{
    auto s = foundInfoByName(type, "", "", true);
    if (nullptr == s) {
        AUDIO_ERROR("Set balance failed, not found such {} type.", getNodeTypeName(type));
        return;
    }

    std::string deviceClass = "";
    if (NodeType::OUTPUT == type)
        deviceClass = std::dynamic_pointer_cast<PaSinkStream>(s)->getDeviceClass();
    else if (NodeType::INPUT == type) {
        deviceClass = std::dynamic_pointer_cast<PaSourceStream>(s)->getDeviceClass();
    }

    if (deviceClass == "filter") {
        // 单声道等 filter sink 无双声道平衡，操作转发至 master 物理设备
        auto master = getMasterStream(s.get(), type);
        if (master) {
            s = master;
        }
        else {
            AUDIO_ERROR("Filter stream {} has no valid master device, balance not changed.", s->getName());
            return;
        }
    }

    s->setBalance(v);
}

double PulseaudioManager::getBalance(const NodeType& type) const
{
    auto s = foundInfoByName(type, "", "", true);
    if (nullptr == s) {
        AUDIO_ERROR("Get balance failed, not found such {} type.", getNodeTypeName(type));
        return {};
    }

    std::string deviceClass = "";
    if (NodeType::OUTPUT == type)
        deviceClass = std::dynamic_pointer_cast<PaSinkStream>(s)->getDeviceClass();
    else if (NodeType::INPUT == type) {
        deviceClass = std::dynamic_pointer_cast<PaSourceStream>(s)->getDeviceClass();
    }

    if (deviceClass == "filter") {
        // 读取平衡值时同样取自 master 物理设备
        auto master = getMasterStream(s.get(), type);
        if (master) {
            s = master;
        }
        else {
            AUDIO_ERROR("Filter stream {} has no valid master device.", s->getName());
        }
    }

    return s->getBalance();
}

void PulseaudioManager::setMute(const NodeType& type, int idx, bool mute)
{
    std::shared_ptr<IStream> s = foundInfoByName(type, idx, true);
    if (nullptr == s) {
        AUDIO_ERROR("Set {} mute failed, not found such device.", idx);
        return;
    }

    std::string deviceClass = "";
    if (NodeType::OUTPUT == type)
        deviceClass = std::dynamic_pointer_cast<PaSinkStream>(s)->getDeviceClass();
    else if (NodeType::INPUT == type) {
        deviceClass = std::dynamic_pointer_cast<PaSourceStream>(s)->getDeviceClass();
    }

    if (deviceClass == "filter") {
        auto master = getMasterStream(s.get(), type);
        if (master) {
            AudioMethod::getInstance().muteChanged(type, s->getIndex(), mute);
            s = master;
        }
    }

    s->setMute(mute);
}

bool PulseaudioManager::getMute(const NodeType& type, int idx) const
{
    std::shared_ptr<IStream> s = foundInfoByName(type, idx, true);
    if (nullptr == s) {
        AUDIO_ERROR("Get {} mute failed, not found such device.", idx);
        return false;
    }

    std::string deviceClass = "";
    if (NodeType::OUTPUT == type)
        deviceClass = std::dynamic_pointer_cast<PaSinkStream>(s)->getDeviceClass();
    else if (NodeType::INPUT == type) {
        deviceClass = std::dynamic_pointer_cast<PaSourceStream>(s)->getDeviceClass();
    }

    if (deviceClass == "filter") {
        auto master = getMasterStream(s.get(), type);
        if (master) {
            s = master;
        }
        else {
            AUDIO_ERROR("Filter stream {} has no valid master device .", s->getName());
        }
    }

    return s->getMute();
}

DefaultDeviceInfo PulseaudioManager::getDefaultDevice(const NodeType& type, int idx) const
{
    {
        std::unique_lock<std::mutex> lock(m_contextMtx);
        if (!m_bContexReady)
            m_contextCond.wait_for(lock, std::chrono::milliseconds(3000));

        if (!m_bContexReady) {
            AUDIO_WARN("Can not get {} default device, wrong context state.", getNodeTypeName(type));
            return {};
        }
    }

    auto s = foundInfoByName(type, idx, true);
    if (nullptr == s) {
        AUDIO_ERROR("Get {} default device failed, not found such device.", getNodeTypeName(type));
        return {};
    }

    return s->getDefaultDevice();
}

void PulseaudioManager::setDefaultDevice(const NodeType& type, int idx, const std::string& name, const std::string& dev)
{
    if (NodeType::OUTPUT == type) {
        m_lastUserSetDefaultOutputMs.store(nowMs(), std::memory_order_relaxed);
    }

    auto s = foundInfoByName(type, idx, true);
    if (nullptr == s) {
        AUDIO_ERROR("set {} default device failed, not found such card: {}, port: {}.", getNodeTypeName(type), dev, name);
        return;
    }

    s->setDefaultDevice(name, dev);
}

std::vector<std::string> PulseaudioManager::getDeviceList(const NodeType& type) const
{
    auto s = foundInfoByName(type, "", "", true);
    if (nullptr == s) {
        AUDIO_ERROR("Get {} device list failed, not found such device.");
        return {};
    }

    return s->getDeviceList();
}

std::list<DefaultDeviceInfo> PulseaudioManager::getSinkList() const
{
    std::list<DefaultDeviceInfo> list;

    std::lock_guard<std::mutex> lock(m_info.m_mtx);
    for (const auto& c : m_info.cardInfoList) {
        for (const auto& s : c->sinks) {
            DefaultDeviceInfo info;
            info.idx = s->getIndex();
            info.name = s->getName();
//            info.cardDesc = c->card.deviceDesc;

            auto v = s->getVolume();
            info.volume = paCvolumeToPaValue(v);

            if (std::string::npos != info.name.find("combined")) {
                info.activePortName = "output-combine";
                info.activePortLabel = "Mulit Audio Output";
                info.cardName = "combine-device";
            }
            else {
                info.cardName = c->card.name;
                info.activePortName = std::dynamic_pointer_cast<PaSinkStream>(s)->getActivePortName();
                info.activePortLabel = std::dynamic_pointer_cast<PaSinkStream>(s)->getActivePortDescription();
            }
            list.push_back(info);
        }
    }
    std::string sl = "[";
    for (const auto& s : list) {
        sl += "(";
        sl += s.toString();
        sl += "),";
    }
    sl += "]";

    AUDIO_DEBUG("Get sink list, list: {}", sl);
    return list;
}

std::list<DefaultDeviceInfo> PulseaudioManager::getSourceList() const
{
    std::list<DefaultDeviceInfo> list;

    for (const auto& c : m_info.cardInfoList) {
        for (const auto& s : c->sources) {
            DefaultDeviceInfo info;
            info.idx = s->getIndex();
            info.name = s->getName();
            info.cardName = c->card.name;
            info.activePortName = std::dynamic_pointer_cast<PaSourceStream>(s)->getActivePortName();
            info.activePortLabel = std::dynamic_pointer_cast<PaSourceStream>(s)->getActivePortDescription();
            list.push_back(info);
        }
    }

    return list;
}

std::list<std::shared_ptr<DeviceInfo>> PulseaudioManager::getAvailablePortList(const NodeType& type) const
{
    auto s = foundInfoByName(type, "", "", true);
    if (nullptr == s) {
        AUDIO_ERROR("get {} device list failed, not found such device.", type);
        return {};
    }

    return s->getAvailablePortList();
}

std::list<StreamMediaInfo> PulseaudioManager::getSinkInputList() const
{
    std::list<StreamMediaInfo> infoList;

    for (const auto& s : m_info.sinkInputList) {
        StreamMediaInfo info;
        info.index = s->getIndex();
        info.volume = paCvolumeToValue(s->getVolume());
        info.binary = std::dynamic_pointer_cast<PaSinkInputStream>(s)->getProcessBinary();
        info.name = AudioMethod::getInstance().getStreamDescription(info.binary);
        info.iconName = std::dynamic_pointer_cast<PaSinkInputStream>(s)->getAppIconName();

        //start adapter application
        auto it = m_adapterStreamNameMap.find(info.iconName);
        if (it != m_adapterStreamNameMap.end()) {
            info.iconName = it->second;
        }
        //end

        info.mediaRole = std::dynamic_pointer_cast<PaSinkInputStream>(s)->getMediaRole();
        infoList.push_back(info);
    }

    return infoList;
}

std::list<StreamMediaInfo> PulseaudioManager::getSourceOutputList() const
{
    std::list<StreamMediaInfo> infoList;

    for (const auto& s : m_info.sourceOutputList) {
        if (s->getName().compare("Peak detect") == 0)
            continue;

        StreamMediaInfo info;
        info.index = s->getIndex();
        info.volume = paCvolumeToValue(s->getVolume());
        info.binary = std::dynamic_pointer_cast<PaSourceOutputStream>(s)->getProcessBinary();
        info.name = AudioMethod::getInstance().getStreamDescription(info.binary);
        info.iconName = std::dynamic_pointer_cast<PaSourceOutputStream>(s)->getAppIconName();
        info.mediaRole = std::dynamic_pointer_cast<PaSourceOutputStream>(s)->getMediaRole();
        infoList.push_back(info);
    }

    return infoList;
}

std::list<StreamMediaIntegrationInfo> PulseaudioManager::getStreamMediaList() const
{
    return m_streamMediaList;
}

std::list<std::string> PulseaudioManager::getModuleList()
{
    std::list<std::string> list;
    {
        std::unique_lock<std::mutex> lock(m_contextMtx);
        if (!m_bContexReady)
            m_contextCond.wait_for(lock, std::chrono::milliseconds(3000));

        if (!m_bContexReady) {
            AUDIO_ERROR("Unable to get module list, wrong context state.");
            return {};
        }
    }


    auto o = pa_context_get_module_info_list(m_pContext, moduleInfoCb, this);
    pa_threaded_mainloop* loop = pa_threaded_mainloop_new();
    while(pa_operation_get_state(o) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(loop);
    }
    pa_threaded_mainloop_free(loop);
    if (o) {
        pa_operation_unref(o);
    }

    for (const auto& l : m_info.moduleInfoList) {
        list.emplace_back(l.name);
    }

    return list;
}

/**
 * 禁用分为以下两种情况
 *  1.该声卡中除了禁用的端口外还存在其他可用的同类型的端口
 *      · 可用的端口在同一个sink/sources上 --> 需要将当前设备静音并记录之前的静音状态，并将该端口切换到其他端口上，将禁用的端口的状态同步到数据库并发出信号，通知ui程序去更新端口列表（典型事例：扬声器和模拟耳机、前后置耳机和线缆输出，内部话筒和话筒）
 *      · 可用的端口在不同的sink/source上  --> 如果另一个端口和当前端口是所在相同的配置文件下，则将当前设备静音并记录之前的静音状态，判断禁用的设备是不是默认的设备，是的话则需要切换到一个可用的优先级最高的设备上
 *                                      --> 如果另一个端口和当前端口是不在相同的配置文件下，只需要切换到另一个设备的最高优先级的配置文件即可，相当于切换设备；将禁用的端口的状态同步到数据库并发出信号，通知ui程序去更新端口列表
 *  2.该声卡中除了禁用的端口外不存在其他可用的同类型的端口
 *	    · 将当前设备静音并记录之前的静音状态，判断禁用的设备是不是默认的设备，是的话则需要切换到一个可用的优先级最高的设备上，如果不是则不需要进行切换操作；
 *		· 将禁用的端口的状态同步到数据库并发出信号，通知ui程序去更新端口列表
 */
//void PulseaudioManager::setEnabled(const std::string& port, const std::string& card, bool enabled)
//{
//    auto cardInfo = findCardInfo(card);
//    if (!cardInfo) {
//        std::cout << "Setting " << card << ":" << port << " status to " << enabled << " Failed, no such card!" << std::endl;
//        return;
//    }

//    auto deviceInfo = findDeviceInfoInCard(*cardInfo, port);
//    if (!deviceInfo) {
//        std::cout << "Setting " << card << ":" << port << " status to " << enabled << " Failed, no such device!" << std::endl;
//        return;
//    }

//    bool found = false;
//    std::shared_ptr<PaCardPortInfo> tmp = nullptr;
//    for (const auto& d : (*cardInfo)->card.ports) {
//        if ((*deviceInfo)->direction == d->direction && (*deviceInfo)->name != d->name) {
//            found = true;
//            if (!tmp || tmp->priority < d->priority) {
//                tmp = d;
//            }
//        }
//    }

//    // 除了禁用的端口外还存在其他同类型的可用的端口
//    if (found && tmp) {
//        // 判断端口是否在相同的sink/source上
//        if (PA_DIRECTION_OUTPUT == tmp->direction) {
//            for(const auto& s : (*cardInfo)->sinks) {

//                auto ports = std::dynamic_pointer_cast<PaSinkStream>(s)->getPorts();
//                auto it = std::find_if(ports.begin(), ports.end(), [port, tmp](const std::shared_ptr<PaSinkPortInfo>& info){
//                    return port == info->name;
//                });

//                auto at = std::find_if(ports.begin(), ports.end(), [port, tmp](const std::shared_ptr<PaSinkPortInfo>& info){
//                    return tmp->name == info->name;
//                });

//                // 都能找到的话说明在同一个sink上
//                if (it != ports.end() && at != ports.end()) {
//                    // 保存当前设备静音状态

//                    // 设置当前设备静音
//                    s->setMute(true);

//                    std::cout << " set sink port:" << tmp->name << std::endl;
//                    // 设置优先级较高的端口
//                    pa_context_set_sink_port_by_index(m_pContext, s->getIndex(), tmp->name.c_str(), nullptr, nullptr);
//                }
//                else {
//                    // 判断两个端口是否存在相同的配置文件
//                    bool found = false;

//                    auto it = std::find_if((*cardInfo)->card.ports.begin(), (*cardInfo)->card.ports.end(), [port](const std::shared_ptr<PaCardPortInfo>& info){
//                        return port == info->name;
//                    });

//                    auto at = std::find_if((*cardInfo)->card.ports.begin(), (*cardInfo)->card.ports.end(), [tmp](const std::shared_ptr<PaCardPortInfo>& info){
//                        return tmp->name == info->name;
//                    });

//                    for (const auto& p1 : (*it)->profiles2) {
//                        for (const auto& p2 : (*at)->profiles2) {
//                            if (p1->name == p2->name) {
//                                found = true;
//                                break;
//                            }
//                        }
//                    }

//                    if (found) {
//                        // 判断禁用的设备是不是默认的设备，是的话则需要切换到一个可用的优先级最高的设备上
//                        // 设置当前设备静音
//                        s->setMute(true);
//                    }
//                    else { // 不在相同的配置文件下，只需要切换到另一个设备的最高优先级的配置文件即可，
//                        int pri = 0;
//                        std::string profileName = "";
//                        for (const auto& profile : tmp->profiles2) {
//                            if (0 == profile->available)
//                                continue;

//                            if (pri < profile->priority) {

//                                pri = profile->priority;
//                                profileName = profile->name;
//                            }
//                        }

//                        std::cout << "std cardprofile:" << profileName << "sinkName:" << s->getName() << std::endl;
//                        if (profileName != "")
//                            pa_context_set_card_profile_by_name(m_pContext, (*cardInfo)->card.name.c_str(), profileName.c_str(), nullptr, nullptr);
//                    }
//                }
//            }
//        }
//        else {
//            for(const auto& s : (*cardInfo)->sources) {

//            }
//        }


//    }
//    else {

//    }

//    (*deviceInfo)->enabled = enabled;
//    std::cout << "Setting " << card << ":" << port << " status to " << enabled << " Successed." << std::endl;
//}

bool PulseaudioManager::isEnabled(const std::string& card, const std::string& port) const
{
    auto cardInfo = findCardByName(card);
    if (!cardInfo) {
        AUDIO_ERROR("Get card: {} port: {} enable failed, no such card.", card, port);
        return true;
    }

    auto deviceInfo = findPortByName(*cardInfo, port);
    if (!deviceInfo) {
        AUDIO_ERROR("Get card: {} port: {} enable failed, no such port.", card, port);
        return true;
    }

    AUDIO_DEBUG("Get card: {} port: {} enabled successed.", card, port);
    return (*deviceInfo)->enabled;
}

bool PulseaudioManager::isLoaded(const ModuleType& type) const
{
    std::string moduleName = "unknown";
    switch (type) {
    case ModuleType::MODULE_ECHO_CANCEL:
        moduleName = "module-echo-cancel";
        break;
    case ModuleType::MODULE_LOOPBACK:
        moduleName = "module-loopback";
        break;
    case ModuleType::MODULE_REMAP_SINK:
        moduleName = "module-remap-sink";
        break;
    default:
        return false;
    }

    auto it = std::find_if(m_info.moduleInfoList.begin(), m_info.moduleInfoList.end(), [moduleName](const PaModuleInfo& info){
        return moduleName == info.name;
    });

    return it != m_info.moduleInfoList.end();
}

bool PulseaudioManager::isValidDevice(const NodeType& type) const
{
    bool ret = false;
    auto portList = getAvailablePortList(type);

    for (const auto& p : portList) {
        if (p->enabled) {
            ret = true;
            break;
        }
    }

    return ret;
}

std::optional<std::shared_ptr<PaCardPortInfo>> PulseaudioManager::findPortByName(const std::shared_ptr<CardInfo>& cardInfo, const std::string& portName) const
{
    for (const auto& d : cardInfo->card.ports) {
        if (d->name == portName) {
            return d;
        }
    }
    return std::nullopt;
}

std::optional<std::shared_ptr<CardInfo>> PulseaudioManager::findCardByName(const std::string& cardName) const
{
    for (const auto& c : m_info.cardInfoList) {
        if (c->card.name == cardName) {
            return c;
        }
    }
    return std::nullopt;
}

bool PulseaudioManager::streamIsExitPort(const std::shared_ptr<IStream>& stream, const std::string& portName) const
{
    auto type = stream->getNodeType();
    if (NodeType::INPUT == type) {
        auto ports = std::dynamic_pointer_cast<PaSourceStream>(stream)->getPorts();
        for (const auto& d : ports) {
            if (d->name == portName) {
                return true;
            }
        }
    }
    else if (NodeType::OUTPUT == type) {
        auto ports = std::dynamic_pointer_cast<PaSinkStream>(stream)->getPorts();
        for (const auto& d : ports) {
            if (d->name == portName) {
                return true;
            }
        }
    }
    else {
        AUDIO_ERROR("{} type does not exit ports.", EnumToInt(type));
        return false;
    }

    return false;
}

std::optional<std::shared_ptr<PaCardProfileInfo2>> PulseaudioManager::findCommonProfile(const std::shared_ptr<PaCardPortInfo>& port1, const std::shared_ptr<PaCardPortInfo>& port2) const {
    for (const auto& p1 : port1->profiles2) {
        for (const auto& p2 : port2->profiles2) {
            if (p1->name == p2->name) {
                return p1;
            }
        }
    }
    return std::nullopt;
}

std::shared_ptr<PaCardProfileInfo2> PulseaudioManager::findHighestPriorityAvailableProfile(const std::vector<std::shared_ptr<PaCardProfileInfo2>>& profiles) const
{
    std::shared_ptr<PaCardProfileInfo2> highestPriorityProfile = nullptr;
    int highestPriority = -1;
    for (const auto& profile : profiles) {
        if (profile->available == 0)
            continue;

        if (!highestPriorityProfile || profile->priority > highestPriority) {
            highestPriorityProfile = profile;
            highestPriority = profile->priority;
        }
    }
    return highestPriorityProfile;
}

/**
 *  禁用分以下情况:
 *  1.该声卡中除了禁用的端口外还存在其他可用的同类型的端口：
 *      如果端口在同一个sink/source上则只需要切换端口接口；
 *      如果不在同一个sink/source上则需要判断是否在统一配置文件下，在同一配置文件下则需要找一个优先级最高的设备，
 *      不在同一配置文件下时则需要切换配置文件；
 *  2.该声卡中除了禁用的端口外不存在其他可用的同类型的端口：
 *      禁用设备之后需要切换到另一个可用的优先级高的设备上
 *
 *  启用:
 *      1. 如果启用的设备默认设备则不需要处理
 *      2. 启用的设备不是默认的设备且当前没有可用的设备时需要将默认设备切换为启用的设备
 */
void PulseaudioManager::setEnabled(std::shared_ptr<RestoreVolumeJson> json, const std::string& card, const std::string& port, bool enabled)
{
    RestoreVolumeInfo info;
    double balance = 0.0; // 记录上一次的平衡音量
    bool mute = false;    // 记录上一次的静音状态
    int value = 0;        // 记录上一次的音量值

    auto cardInfo = findCardByName(card);
    if (!cardInfo) {
        AUDIO_ERROR("Setting card: {} port: {} status to {} failed, no such card!", card, port, enabled);
        return;
    }

    auto deviceInfo = findPortByName(*cardInfo, port);
    if (!deviceInfo) {
        AUDIO_ERROR("Setting card: {} port: {} status to {} failed, no such device!", card, port, enabled);
        return;
    }

    auto s = findStreamByPort(cardInfo, (*deviceInfo)->name, (*deviceInfo)->direction);

    // 禁用时，需先判断sink/source是否存在，如USB声卡当前配置为模拟输出，此时禁用数字输出，无需处理静音及设备切换流程
    if (!enabled) {
        if (s) {
            // 1. 记录当前待禁用设备的音量信息
            balance = s->getBalance();
            mute = s->getMute();
            auto volume = s->getVolume();
            value = paCvolumeToPaValue(volume);

            // 2. 禁用设备为默认sink/source下的端口
            if (isDefaultDevice(s)) {
                AUDIO_DEBUG("Disable device case 1: device is default sink/source.");

                // 2.1 禁用该设备端口时，还存在其他可用类型端口，且为同一sink/source，仅需要切换端口，不处理设置禁音
                auto availablePort = findHighestPriorityAvailablePort(cardInfo, (*deviceInfo)->direction, (*deviceInfo)->name);
                if (availablePort && streamIsExitPort(s, (*availablePort)->name)) {
                    AUDIO_DEBUG("Card: {} has other available port: {}.", card, (*availablePort)->name);
                    AUDIO_DEBUG("Set sink/source port: {}.", (*availablePort)->name);

                    if ((*deviceInfo)->direction == PA_DIRECTION_OUTPUT) {
                        pa_operation_unref(pa_context_set_sink_port_by_index(m_pContext, s->getIndex(), (*availablePort)->name.c_str(), nullptr, nullptr));
                    } else {
                        pa_operation_unref(pa_context_set_source_port_by_index(m_pContext, s->getIndex(), (*availablePort)->name.c_str(), nullptr, nullptr));
                    }
                }

                // 2.2 禁用该设备端口时，存在其他可用端口，但需要切换声卡配置文件产生新的sink(如: USB声卡的模拟输出与数字输出)，需要处理静音
                else if (availablePort && !streamIsExitPort(s, (*availablePort)->name)) {
                    s->setMute(true);
                    auto highestPriorityProfile = findHighestPriorityAvailableProfile((*availablePort)->profiles2);
                    if (highestPriorityProfile) {
                        AUDIO_DEBUG("Set card: {} profile to {}.", (*cardInfo)->card.name, highestPriorityProfile->name);
                        pa_operation_unref(pa_context_set_card_profile_by_name(m_pContext, (*cardInfo)->card.name.c_str(), highestPriorityProfile->name.c_str(), nullptr, nullptr));
                        // 由于设置配置文件可能会导致默认设备发生变化，因此需要重新设置默认输入/输出设备
                        if (NodeType::OUTPUT == s->getNodeType()) {
                            pa_operation_unref(pa_context_set_default_sink(m_pContext, m_info.defaultSink->getName().c_str(), nullptr, nullptr));
                        } else {
                            pa_operation_unref(pa_context_set_default_source(m_pContext, m_info.defaultSource->getName().c_str(), nullptr, nullptr));
                        }
                    }
                    else {
                        AUDIO_ERROR("Can not found available profile for: {} in {}.", (*availablePort)->name, (*cardInfo)->card.name);
                    }
                }

                // 2.3 禁用该设备端口时，sink/source不存在其他可用端口，需要将默认设备切换至别的stream，并将原来的stream静音
                else {
                    s->setMute(true);
                    auto newStream = findHighestPriorityStream(s);
                    if (!newStream) {
                        AUDIO_ERROR("No available {} device to switch to after disabling default device {}.",
                                    getNodeTypeName(s->getNodeType()), s->getName());
                    }
                    else {
                        AUDIO_DEBUG("Disable sink/source: {} Switch to new default sink/source: {}",
                                    s->getName(), newStream->getName());
                        if (NodeType::OUTPUT == newStream->getNodeType()) {
                            pa_operation_unref(pa_context_set_default_sink(m_pContext, newStream->getName().c_str(), nullptr, nullptr));
                            m_info.serverInfo.default_sink_name = newStream->getName();
                            m_info.defaultSink = std::dynamic_pointer_cast<PaSinkStream>(newStream);
                            deviceChanged(NodeType::OUTPUT, "", "");
                        }
                        else if (NodeType::INPUT == newStream->getNodeType()) {
                            pa_operation_unref(pa_context_set_default_source(m_pContext, newStream->getName().c_str(), nullptr, nullptr));
                            m_info.serverInfo.default_source_name = newStream->getName();
                            m_info.defaultSource = std::dynamic_pointer_cast<PaSourceStream>(newStream);
                            deviceChanged(NodeType::INPUT, "", "");
                        }
                    }
                }
            }

            // 3. 禁用的设备为非默认sink/source下的端口
            else {
                AUDIO_DEBUG("Disable device case 2: device is not default sink/source.");
                // 3.1 禁用该设备端口时，还存在其他可用端口，且为同一sink/source，仅需要切换端口，不处理设置禁音
                auto availablePort = findHighestPriorityAvailablePort(cardInfo, (*deviceInfo)->direction, (*deviceInfo)->name);
                if (availablePort && streamIsExitPort(s, (*availablePort)->name)) {
                    AUDIO_DEBUG("Card: {} has other available port: {}.", card, (*availablePort)->name);
                    AUDIO_DEBUG("Set sink/source port: {}.", (*availablePort)->name);

                    if ((*deviceInfo)->direction == PA_DIRECTION_OUTPUT) {
                        pa_operation_unref(pa_context_set_sink_port_by_index(m_pContext, s->getIndex(), (*availablePort)->name.c_str(), nullptr, nullptr));
                    } else {
                        pa_operation_unref(pa_context_set_source_port_by_index(m_pContext, s->getIndex(), (*availablePort)->name.c_str(), nullptr, nullptr));
                    }
                }

                // 3.2 禁用该设备端口时，sink/source不存在其他可用端口，由于该设备非默认sink/source，所以仅设置静音即可
                else {
                    s->setMute(true);
                }
            }
        }
        else {
            AUDIO_DEBUG("待禁用的设备: card:{} port: {} 不在sink/source list中，无需处理", card, port);
        }
    }
    // 启用设备
    else {
        // 启用的是默认设备则不做处理，启用的不是默认设备/启用设备的声卡不存在启用的端口则将默认设备切换到启用的设备
        NodeType type = (*deviceInfo)->direction == PA_DIRECTION_INPUT ? NodeType::INPUT : NodeType::OUTPUT;
        if (!isDefaultDevice(s)) {
            auto it = std::find_if((*cardInfo)->card.ports.begin(), (*cardInfo)->card.ports.end(), [deviceInfo](const std::shared_ptr<PaCardPortInfo>& info){
                AUDIO_DEBUG("ctf ... port: {}, enabled: {}", info->name, info->enabled);
                return (*deviceInfo)->direction == info->direction && info->enabled;
            });

            AUDIO_DEBUG("The enabled device is not the default device, set card: {} port: {} to default.", card, port);
            if (!isExitEnableDevice(type) || it == (*cardInfo)->card.ports.end()) {
                setDefaultDevice(type, -1, port, card);
            }
        }

        if (s) {
            s->setMute(json->getStatus(card, port, "mute"));
        }
    }

    // 更新设备状态
    (*deviceInfo)->enabled = enabled;
    auto portIt = std::find_if((*cardInfo)->card.deviceList.begin(), (*cardInfo)->card.deviceList.end(), [port](const std::shared_ptr<DeviceInfo>& info){
        return port == info->portName;
    });

    if (portIt != (*cardInfo)->card.deviceList.end()) {
        (*portIt)->enabled = enabled;
    }

    AUDIO_DEBUG("{} card: {} port: {} successed, mute: {}.", enabled ? "Enable" : "Disable", card, port, json->getStatus(card, port, "mute"));

    // 写入到数据库文件并发出信号
    info.cardName = card;
    info.portName = port;
    info.volume = value;
    info.priority = (*deviceInfo)->priority;
    info.balance = balance;
    info.enable = enabled;
    info.muted = mute;

    json->insert(info);
    deviceAdjust(NodeType::OUTPUT);
}

std::optional<std::shared_ptr<PaCardPortInfo>> PulseaudioManager::findHighestPriorityAvailablePort(const std::optional<std::shared_ptr<CardInfo>>& cardInfo, int direction, const std::string& excludedPortName) const
{
    std::optional<std::shared_ptr<PaCardPortInfo>> highestPriorityPort = std::nullopt;
    for (const auto& port : (*cardInfo)->card.ports) {
        if (PA_PORT_AVAILABLE_NO == port->available)
            continue;

        if (!port->enabled)
            continue;

        if (port->direction == direction && port->name != excludedPortName) {
            if (!highestPriorityPort || port->priority > (*highestPriorityPort)->priority) {
                highestPriorityPort = port;
            }
        }
    }
    return highestPriorityPort;
}

std::shared_ptr<IStream> PulseaudioManager::findStreamByPort(const std::optional<std::shared_ptr<CardInfo>>& cardInfo, const std::string& portName, int direction) const
{
    if (direction == PA_DIRECTION_OUTPUT) {
        for (const auto& sink : (*cardInfo)->sinks) {
            auto ports = std::dynamic_pointer_cast<PaSinkStream>(sink)->getPorts();
            for (const auto& port : ports) {
                if (port->name == portName) {
                    return sink;
                }
            }
        }
    } else {
        // 对于source的处理
        for (const auto& source : (*cardInfo)->sources) {
            auto ports = std::dynamic_pointer_cast<PaSourceStream>(source)->getPorts();
            for (const auto& port : ports) {
                if (port->name == portName) {
                    return source;
                }
            }
        }
    }
    return nullptr;
}

std::shared_ptr<IStream> PulseaudioManager::findHighestPriorityStream(const std::shared_ptr<IStream>& stream)
{
    std::shared_ptr<IStream> ret = nullptr;
    int priority = -1;

    auto findHighestPriority = [this, stream, &ret, &priority](const std::list<std::shared_ptr<IStream>>& streams, auto getActivePortName) {
        for (const auto& s : streams) {
            if (s->getName() == stream->getName())
                continue;

            std::string activePortName = getActivePortName(s);
            auto card = s->getCard();
            auto it = std::find_if(m_info.cardInfoList.begin(), m_info.cardInfoList.end(), [card, activePortName](const std::shared_ptr<CardInfo>& c) {
                return card == c->card.index && std::any_of(c->card.ports.begin(), c->card.ports.end(), [activePortName](const std::shared_ptr<PaCardPortInfo>& info) {
                    return info->enabled && activePortName == info->name && info->available != PA_PORT_AVAILABLE_NO;
                });
            });

            if (it == m_info.cardInfoList.end())
                continue;

            int pri = calaPriorityByName(s->getName());
            if (!ret || pri > priority) {
                ret = s;
                priority = pri;
            }
        }
    };

    for (const auto& c : m_info.cardInfoList) {
        if (NodeType::INPUT == stream->getNodeType()) {
            findHighestPriority(c->sources, [](const std::shared_ptr<IStream>& s) {
                return std::dynamic_pointer_cast<PaSourceStream>(s)->getActivePortName();
            });
        } else if (NodeType::OUTPUT == stream->getNodeType()) {
            findHighestPriority(c->sinks, [](const std::shared_ptr<IStream>& s) {
                return std::dynamic_pointer_cast<PaSinkStream>(s)->getActivePortName();
            });
        }
    }

    return ret;
}

bool PulseaudioManager::isDefaultDevice(const std::shared_ptr<IStream>& s)
{
    if (!s)
        return false;

    auto type = s->getNodeType();

    if (NodeType::INPUT == type) {
        if (!m_info.defaultSource)
            return false;

        /* The default source may be a filter stream (e.g. noiseReduceSource),
         * in which case its master device must be used for comparison against
         * the physical stream; otherwise every physical source is incorrectly
         * identified as not the default device. */
        if (m_info.defaultSource->getDeviceClass() == "filter") {
            auto masterSource = getMasterStream(m_info.defaultSource.get(), NodeType::INPUT);
            if (masterSource) {
                return s->getName() == masterSource->getName();
            }
        }
        return s->getName() == m_info.defaultSource->getName();
    }
    else if (NodeType::OUTPUT == type) {
        if (!m_info.defaultSink)
            return false;

        /* The default sink may be a filter stream (e.g. mono),
         * in which case its master device must be used for comparison
         * against the physical stream. */
        if (m_info.defaultSink->getDeviceClass() == "filter") {
            auto masterSink = getMasterStream(m_info.defaultSink.get(), NodeType::OUTPUT);
            if (masterSink) {
                return s->getName() == masterSink->getName();
            }
        }
        return s->getName() == m_info.defaultSink->getName();
    }

    AUDIO_ERROR("Invaild {} type, unable to determine if device: {} is the default device.", getNodeTypeName(type), s->getName());
    return false;
}

// signal
void PulseaudioManager::deviceChanged(const NodeType& type, const std::string& portName, const std::string& cardName)
{
    auto s = foundInfoByName(type, "", "", true);
    if (nullptr == s) {
        AUDIO_ERROR("send {} deviceChanged failed, not found such device.", type);
        return;
    }

    s->deviceChanged();
}

void PulseaudioManager::portChanged(const NodeType& type, const std::string& name)
{

}

void PulseaudioManager::cardRemoved(const std::string& name)
{

}

void PulseaudioManager::deviceAdjust(const NodeType& type)
{
    AudioMethod::getInstance().deviceAdjust(type);
}

void PulseaudioManager::addStream(int idx, int value, const std::string& iconName, const std::string& descName)
{
    AudioMethod::getInstance().addStream(idx, value, iconName, descName);
}

void PulseaudioManager::removeStream(int idx)
{
    AudioMethod::getInstance().removeStream(idx);
}

void PulseaudioManager::addStream(int dire, int idx, int value, bool mute, const std::string& iconName, const std::string& descName)
{
    AudioMethod::getInstance().addStream(dire, idx, value, mute, iconName, descName);
}

void PulseaudioManager::removeStream(int dire, int idx)
{
    AudioMethod::getInstance().removeStream(dire, idx);
}

std::shared_ptr<IStream> PulseaudioManager::foundStreamByPortName(const std::string& card, const std::string& port) const
{
    auto cardInfo = findCardByName(card);
    if (!cardInfo) {
        return nullptr;
    }

    if (auto sink = findStreamByPort(cardInfo, port, PA_DIRECTION_OUTPUT)) {
        return sink;
    }
    return findStreamByPort(cardInfo, port, PA_DIRECTION_INPUT);
}

std::shared_ptr<IStream> PulseaudioManager::foundInfoByName(const NodeType& type, const std::string& name, const std::string& dev, bool general) const
{
    switch (type) {
    case NodeType::INPUT: {
        if (general)
            return m_info.defaultSource;

        for (const auto& c : m_info.cardInfoList) {
            if (!general && strcmp(dev.c_str(), c->card.name.c_str()))
                continue;

            for(const auto& s : c->sources) {

                auto it = std::find_if(std::dynamic_pointer_cast<PaSourceStream>(s)->getPorts().begin(),
                                       std::dynamic_pointer_cast<PaSourceStream>(s)->getPorts().end(),
                                       [name](const std::shared_ptr<PaSourcePortInfo> s) {
                                        return s->name == name;
                });
                if (it != std::dynamic_pointer_cast<PaSourceStream>(s)->getPorts().end()) {
                    return s;
                }  
            }
        }
        return m_info.defaultSource;
    }
    case NodeType::OUTPUT: {
        if (general)
            return m_info.defaultSink;

        for (const auto& c : m_info.cardInfoList) {
            if (!general && strcmp(dev.c_str(), c->card.name.c_str()))
                continue;

            for(const auto& s : c->sinks) {

                auto it = std::find_if(std::dynamic_pointer_cast<PaSinkStream>(s)->getPorts().begin(),
                                       std::dynamic_pointer_cast<PaSinkStream>(s)->getPorts().end(),
                                       [name](const std::shared_ptr<PaSinkPortInfo> s) {
                    return s->name == name;
                });
                if (general || it != std::dynamic_pointer_cast<PaSinkStream>(s)->getPorts().end()) {
                    return s;
                }
            }
        }

        return m_info.defaultSink;
    }
    case NodeType::SINK_INPUT: {
        for(const auto& si : m_info.sinkInputList) {
            if (general || 0 == strcmp(si->getName().c_str(), name.c_str())) {
                return si;
            }
        }
        break;
    }
    case NodeType::SOURCE_OUTPUT: {
        for(const auto& so : m_info.sourceOutputList) {
            if (general || 0 == strcmp(so->getName().c_str(), name.c_str())) {
                return so;
            }
        }
        break;
    }
    default:
        break;
    }
    return nullptr;
}

std::shared_ptr<IStream> PulseaudioManager::foundInfoByName(const NodeType& type, int idx, bool general) const
{
    switch (type) {
    case NodeType::INPUT: {
        if (general)
            return m_info.defaultSource;

        for (const auto& c : m_info.cardInfoList) {
            auto it = std::find_if(c->sources.begin(), c->sources.end(), [idx](const std::shared_ptr<IStream>& s) {
                return idx == s->getIndex();
            });

            if (it != c->sources.end())
                return *it;
        }
        return m_info.defaultSource;
    }
    case NodeType::OUTPUT: {
        if (general)
            return m_info.defaultSink;

        for (const auto& c : m_info.cardInfoList) {
            auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [idx](const std::shared_ptr<IStream>& s) {
                return idx == s->getIndex();
            });

            if (it != c->sources.end())
                return *it;
        }

        return m_info.defaultSink;
    }
    case NodeType::SINK_INPUT: {
        for(const auto& si : m_info.sinkInputList) {
            if (idx == si->getIndex()) {
                return si;
            }
        }
        break;
    }
    case NodeType::SOURCE_OUTPUT: {
        for(const auto& so : m_info.sourceOutputList) {
            if (idx == so->getIndex()) {
                return so;
            }
        }
        break;
    }
    default:
        break;
    }
    return nullptr;
}

bool PulseaudioManager::deviceInfosIsEqual(const std::list<std::shared_ptr<DeviceInfo>>& list1, const std::list<std::shared_ptr<DeviceInfo>>& list2)
{
    if (list1.size() != list2.size()) {
        return false;
    }

    auto it1 = list1.begin();
    auto it2 = list2.begin();

    for (; it1 != list1.end(); ++it1, ++it2)
    {
        if (!(*it1) || !(*it2)) {
            return false;
        }

        if (**it1 != **it2) {
            return false;
        }
    }

    return true;
}


void PulseaudioManager::init()
{
    if (m_pContext) {
        AUDIO_DEBUG("pulseAudio is connected");
        return;
    }

    pa_glib_mainloop* m = pa_glib_mainloop_new(g_main_context_default());
    m_pApi = pa_glib_mainloop_get_api(m);
    pa_proplist* proplist = pa_proplist_new();
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_NAME, "Ukui Media Volume Control");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ID, "org.ukui.media.vucontrol");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_ICON_NAME, "audio-card");
    pa_proplist_sets(proplist, PA_PROP_APPLICATION_VERSION, "PACKAGE_VERSION");
    m_pContext = pa_context_new_with_proplist(m_pApi, nullptr, proplist);
    g_assert(m_pContext);

    pa_proplist_free(proplist);
}

bool PulseaudioManager::connect()
{
    pa_context_set_state_callback(m_pContext, contextStateCallback, this);

    if (pa_context_connect(m_pContext, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        if (pa_context_errno(m_pContext) == PA_ERR_INVALID) {
            /*w->setConnectingMessage(QObject::tr("Connection to PulseAudio failed. Automatic retry in 5s\n\n"
                "In this case this is likely because PULSE_SERVER in the Environment/X11 Root Window Properties\n"
                "or default-server in client.conf is misconfigured.\n"
                "This situation can also arrise when PulseAudio crashed and left stale details in the X11 Root Window.\n"
                "If this is the case, then PulseAudio should autospawn again, or if this is not configured you should\n"
                "run start-pulseaudio-x11 manually.").toUtf8().constData());*/
            AUDIO_DEBUG("connect pulseaudio failed");
        }
        else {
            m_reconnectTime--;
            if (m_reconnectTime <= 0) {
                AUDIO_ERROR("reconnect pulseaudio Three times failed");
                return false;
            }

//            g_timeout_add_seconds(5, connect(), nullptr);
        }
    }

    return false;
}

void PulseaudioManager::cardCb(pa_context* ctx, const pa_card_info* i, int eol, void* userdata) {
    PulseaudioManager* manager = static_cast<PulseaudioManager*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(ctx) == PA_ERR_NOENTITY)
            return;

        AUDIO_ERROR("Card callback failure");
        return;
    }

    if (eol > 0) {
//        decOutstanding(w);
        return;
    }

    auto info = std::make_shared<CardInfo>();
    info->card = *i;

    bool deviceListChanged = false;
    for (const auto& c : manager->m_info.cardInfoList) {
        if (c->card.index != i->index)
            continue;

        if (!manager->deviceInfosIsEqual(info->card.deviceList, c->card.deviceList)) {
            deviceListChanged = true;
            manager->deviceAdjust(NodeType::OUTPUT);
        }
    }

    auto it = std::find_if(manager->m_info.cardInfoList.begin(), manager->m_info.cardInfoList.end(), [info](const std::shared_ptr<CardInfo> i) {
       return info->card.index == i->card.index;
    });

    if (it == manager->m_info.cardInfoList.end()) {
        deviceListChanged = true;
        manager->deviceAdjust(NodeType::OUTPUT);
    }

    AUDIO_DEBUG("card cb, updateData info: {}.", std::dynamic_pointer_cast<CardInfo>(info)->toString());
    updateData(manager->m_info, info,
        [](PaDataInfo& data) -> std::optional<std::reference_wrapper<std::list<std::shared_ptr<CardInfo>>>> {
            return data.cardInfoList;
        },
        [](const std::shared_ptr<CardInfo> existing, const std::shared_ptr<CardInfo> newCard) {
            return existing->card.index == newCard->card.index;
        }
    );

    // 端口插拔导致可用设备变化后，若当前默认仍落在已禁用/不可用端口上，则切到最高优先级的已启用端口
    if (deviceListChanged) {
        manager->ensureDefaultOnEnabledDevice(NodeType::OUTPUT);
        manager->ensureDefaultOnEnabledDevice(NodeType::INPUT);
    }

    AUDIO_DEBUG("card cb, card->name={}, card->index={}, info={}", info->card.name, info->card.index, manager->m_info.toString());
}

#if HAVE_EXT_DEVICE_RESTORE_API
static void ext_device_restore_subscribeCb(pa_context* ctx, pa_device_type_t type, uint32_t idx, void* userdata);
#endif

void PulseaudioManager::sinkCb(pa_context* ctx, const pa_sink_info* i, int eol, void* userdata)
{
    auto manager = static_cast<PulseaudioManager*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(ctx) == PA_ERR_NOENTITY) {
            return;
        }

        AUDIO_ERROR("Sink callback failure");
        return;
    }

    if (eol > 0) {
        return;
    }

    std::shared_ptr<IStream> info = std::make_shared<PaSinkStream>(m_pContext, manager->m_info);
    std::dynamic_pointer_cast<PaSinkStream>(info)->copy(*i);

    for (const auto& c : manager->m_info.cardInfoList) {
//        if (c->card.index != i->card)
//            continue;

        auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [info](const std::shared_ptr<IStream>& s){
            return info->getName() == s->getName();
        });

        if (it != c->sinks.end()) {
            auto sink = std::dynamic_pointer_cast<PaSinkStream>(*it);
            auto status = sink->getCombineStatus();
#if 0
            /* 移除同步单声道的master音量的设置逻辑 */
            auto master = std::dynamic_pointer_cast<PaSinkStream>(info)->getMasterDevice();

            // 重设单声道的master device
            if (std::string::npos != info->getName().find("mono") && (manager->m_info.serverInfo.default_sink_name == "mono" ||
                manager->m_info.serverInfo.default_sink_name == master)) {
                // 同步设置maser音量
                int volume = 0;
                bool mute = false;
                for (const auto& c : manager->m_info.cardInfoList) {
                    auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [master](const std::shared_ptr<IStream>& s){
                        return master == s->getName();
                    });

                    if (it != c->sinks.end()) {
                        volume = paCvolumeToPaValue(i->volume);
                        auto v = paCvolumeToPaValue((*it)->getVolume());

                        AUDIO_DEBUG("Device: {} master device is: {}, update mute: {} and volume from {} change to {}.",
                                    sink->getName(), master, mute, v, volume);
                        (*it)->setMute(i->mute);
                        (*it)->setBalance(0);
                        (*it)->setVolume(volume);
                        break;
                    }
                }
            }

#endif
            std::dynamic_pointer_cast<PaSinkStream>(info)->setCombineStatus(status);
            break;
        }
    }

    {
        auto getSinkist = [&info](PaDataInfo& data) -> std::optional<std::reference_wrapper<std::list<std::shared_ptr<IStream>>>> {
            std::shared_ptr<CardInfo> cardInfo = nullptr;
            // 如果cardInfoList为空，返回空值
            if (data.cardInfoList.empty()) {
                AUDIO_DEBUG("Card info list is empty.");
                return std::nullopt;
            }

            for (auto& c : data.cardInfoList) {
                if (PA_INVALID_INDEX == info->getCard()) {
                    cardInfo = c;
                }
                auto prop = pa_proplist_gets(info->getProplist(), PA_PROP_DEVICE_CLASS);
                // 如果是filer,则返回当前所在的声卡
                auto found = false;
                if (!prop || 0 == strcmp(prop, "filter")) {
                    for (const auto& s : c->sinks) {

                        if (!cardInfo || 0 == strcmp(data.serverInfo.default_sink_name.c_str(), s->getName().c_str())) {
                            cardInfo = c;
                        }

                        if (0 == strcmp(info->getName().c_str(), s->getName().c_str())) {
                            found = true;
                            break;
                        }
                    }
                }

                if (c->card.index == info->getCard() || found) {
                    AUDIO_DEBUG("Found sinks, info->name: {}, info->card: {}, c->card: {}.", info->getName(), info->getCard(), c->card.index);
                    return c->sinks;
                }
            }
            AUDIO_DEBUG("Can not found sink, return default sinks, info->name: {}, info->card: {}.", info->getName(), info->getCard());
            return cardInfo->sinks;
        };

        if (std::string::npos != info->getName().find("mono")) {
            /* Delete the mono information stored in the database to prevent it from being stored on multiple sound cards. */
            for(const auto& c : manager->m_info.cardInfoList) {
                c->sinks.erase(std::remove_if(c->sinks.begin(), c->sinks.end(), [](const std::shared_ptr<IStream>& s) {
                    return s->getName() == "mono";
                }), c->sinks.end());
            }
        }

        auto isSameSink = [](std::shared_ptr<IStream> existing, std::shared_ptr<IStream> newSink) {
            return  existing->getIndex() == newSink->getIndex();
        };

        std::unique_lock<std::mutex> lock(manager->m_info.m_mtx);
        auto isNew = updateData(manager->m_info, info, getSinkist, isSameSink);
        if (isNew) {
            manager->m_info.m_operation = true;
            manager->m_info.m_cond.notify_one();
        }
    }

    // 设备名称和 owner_module 均匹配时，完成对应模块的加载
    if (i->name && strcmp(i->name, "mono") == 0) {
        manager->completeModuleLoad("module-remap-sink", i->owner_module);
    } else if (i->name && strcmp(i->name, "combined") == 0) {
        manager->completeModuleLoad("module-combine-sink", i->owner_module);
    }

    std::shared_ptr<IStream> masterSink = nullptr;
    if (manager->m_info.defaultSink) {
        masterSink = manager->getMasterStream(manager->m_info.defaultSink.get(), NodeType::OUTPUT);
        if (nullptr == masterSink) {
            masterSink = manager->m_info.defaultSink;
        }
    }

//    if (i->name) {
//        AUDIO_DEBUG("Sink cb, g_strcmp0 i->name: {} serverInfo.default_sink_name {}."
//                    , i->name, manager->m_info.serverInfo.default_sink_name.c_str());
//    }
//    if (masterSink) {
//        AUDIO_DEBUG("Sink cb, masterSink:{} .", masterSink->getName().c_str());
//    }

    if (0 == g_strcmp0(i->name, manager->m_info.serverInfo.default_sink_name.c_str()) ||
            (masterSink && 0 == g_strcmp0(i->name, masterSink->getName().c_str()))) {
        if (nullptr == manager->m_info.defaultSink) {
            manager->m_info.defaultSink = std::make_shared<PaSinkStream>(m_pContext, manager->m_info);
            manager->m_info.defaultSink->copy(*i);
        }

//        if (i->name && manager->m_info.defaultSink) {
//            AUDIO_DEBUG("Sink cb, i->name: {} defaultSink->getName {}."
//                        , i->name, manager->m_info.defaultSink->getName());
//        }
//        if (i->active_port && manager->m_info.defaultSink) {
//            AUDIO_DEBUG("Sink cb, i->active_port->name: {} defaultSink->getActivePortName {}."
//                        , i->active_port->name, manager->m_info.defaultSink->getActivePortName());
//        }

        if (manager->m_info.defaultSink && (i->name != manager->m_info.defaultSink->getName() ||
            (i->active_port && i->active_port->name != manager->m_info.defaultSink->getActivePortName()))) {
            const long long lastUserMs = manager->m_lastUserSetDefaultOutputMs.load(std::memory_order_relaxed);
            const bool recentUserSwitch = (nowMs() - lastUserMs) < 1500;

            bool found = false;
            for (const auto& c : manager->m_info.cardInfoList) {
                auto it = std::find_if(c->sinks.begin(), c->sinks.end(),
                                       [masterSink](const std::shared_ptr<IStream>& s) {
                    return masterSink->getName() == s->getName();
                });

                if (it != c->sinks.end()) {
                    found = true;
                    auto ports = std::dynamic_pointer_cast<PaSinkStream>(*it)->getPorts();

                    AUDIO_DEBUG("Found sink {} on database .", manager->m_info.defaultSink->getName());
                    auto at = std::find_if(ports.begin(), ports.end(), [manager](const std::shared_ptr<PaSinkPortInfo>& p){
                        if (manager->m_info.defaultSink->getActivePort()) {
                            return manager->m_info.defaultSink->getActivePort()->name == p->name;
                        }
                        return false;
                    });

                    if (at != ports.end()) {
                        AUDIO_DEBUG("Found sink (*at)->available {} .", (*at)->available);
                        if (PA_PORT_AVAILABLE_NO == (*at)->available /*&& (*at)->priority < i->active_port->priority*/) {
                            manager->m_autoPause = true;
                            AUDIO_DEBUG("The default output device port is changed to {}, and multimedia applications need "
                                        "to be paused to play.", (*at)->name);
                        }
                    }
                    else {
                        AUDIO_DEBUG("The {} port cannot be found in the default output device {}. The multimedia application being "
                                    "played does not need to be paused.", manager->m_info.defaultSink->getActivePort() ? manager->m_info.defaultSink->getActivePort()->name : "", (*it)->getName());
                        manager->m_autoPause = false;
                    }
                    break;
                }

            }

            /** To find the corresponding default sink, you need to determine whether the port has been removed.
             *  If it is not found, it means that the device has been removed.
             */
            if (!found) {
                AUDIO_DEBUG("The output device {} has been removed, and the multimedia application that is already"
                            " playing needs to be paused.", manager->m_info.defaultSink->getName());
                manager->m_autoPause = true;
            }

            manager->m_info.defaultSink->copy(*i);
            auto defaultSink = std::dynamic_pointer_cast<PaSinkStream>(manager->m_info.defaultSink);
            if (defaultSink) {
                // 连接设备导致的默认输出/端口变化：显示OSD；用户手动切换不显示OSD
                bool showOsd = !recentUserSwitch;
                if (showOsd) {
                    showOsd = manager->shouldShowOsdDedup(defaultSink->getName(), nowMs());
                }

                if (showOsd && i->active_port) {
                    AudioMethod::getInstance().showOsd(i->active_port->name);
                }

                defaultSink->deviceChanged(manager->m_autoPause);
            }

            if (manager->m_info.defaultSink->getName().starts_with("bluez") &&
                manager->isBluetoothDeviceReady(manager->m_info.defaultSink->getCardName())) {
                manager->setBluezReadyStatus(true);
            }

            if (manager->m_autoPause)
                manager->m_autoPause = false;
        }
        else {
            manager->m_info.defaultSink->copy(*i);
        }
    }
    resolveEchoCancelMasters(manager->m_info);
    resolveMonoMaster(manager->m_info);
    AUDIO_DEBUG("sink cb, i: {}", std::dynamic_pointer_cast<PaSinkStream>(info)->toString());
}

void PulseaudioManager::sourceCb(pa_context* ctx, const pa_source_info* i, int eol, void* userdata)
{
    auto manager = static_cast<PulseaudioManager*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(ctx) == PA_ERR_NOENTITY) {
            return;
        }

        AUDIO_ERROR("Source callback failure.");
        return;
    }

    if (eol > 0) {
//        decOutstanding(w);
        return;
    }

    std::shared_ptr<IStream> masterSource = nullptr;
    if (manager->m_info.defaultSource) {
        masterSource = manager->getMasterStream(manager->m_info.defaultSource.get(), NodeType::INPUT);
        if (nullptr == masterSource) {
            masterSource = manager->m_info.defaultSource;
        }
    }

//    AUDIO_DEBUG("Source cb, i->name: {} serverInfo.default_source_name:{} .", i->name, manager->m_info.serverInfo.default_source_name.c_str());
//    if (masterSource) {
//        AUDIO_DEBUG("Source cb, masterSource:{} .", masterSource->getName().c_str());
//    }
    if (0 == g_strcmp0(i->name, manager->m_info.serverInfo.default_source_name.c_str()) ||
        (masterSource && 0 == g_strcmp0(i->name, masterSource->getName().c_str()))) {
        if (nullptr == manager->m_info.defaultSource) {
            manager->m_info.defaultSource = std::make_shared<PaSourceStream>(m_pContext, manager->m_info);
            manager->m_info.defaultSource->copy(*i);

            std::unique_lock<std::mutex> lock(manager->m_contextMtx);
            manager->m_bContexReady = true;
            manager->m_contextCond.notify_all();
        }

        const auto& defaultSource = manager->m_info.defaultSource;
        const auto& expectedName = manager->m_info.serverInfo.default_source_name;

        const bool matchedName = (i->name == expectedName || (masterSource && i->name == masterSource->getName()));
        const char* deviceClass = pa_proplist_gets(i->proplist, PA_PROP_DEVICE_CLASS);
        const bool isFilterSource = (deviceClass && 0 == strcmp(deviceClass, "filter"))
                || (i->name && std::string::npos != std::string(i->name).find("noiseReduceSource"));
        const bool nameOutOfSync = matchedName && (masterSource && i->name != masterSource->getName())
                && !isFilterSource;
        const bool portOutOfSync = matchedName && i->active_port && (i->active_port->name != defaultSource->getActivePortName() ||
            (masterSource && i->active_port->name != std::dynamic_pointer_cast<PaSourceStream>(masterSource)->getActivePortName()));

//        if (defaultSource) {
//            AUDIO_DEBUG("Source cb, defaultSource->getName():{} expectedName:{} .", defaultSource->getName(), expectedName);
//        }
//        AUDIO_DEBUG("Source cb, matchedName:{} nameOutOfSync:{} portOutOfSync:{}.", matchedName, nameOutOfSync, portOutOfSync);

        if (defaultSource && (nameOutOfSync || portOutOfSync)) {
//            AUDIO_DEBUG("Source cb, i->name: {} serverInfo.default_source_name:{} ."
//                        , i->name, manager->m_info.serverInfo.default_source_name.c_str());
//            if (i->active_port) {
//                AUDIO_DEBUG("Source cb, i->active_port->name:{} m_info.defaultSource->getActivePortName:{} ."
//                            , i->active_port->name, manager->m_info.defaultSource->getActivePortName());
//            }

            manager->m_info.defaultSource->copy(*i);
            manager->m_info.defaultSource->deviceChanged();
            if (i->name == manager->m_info.serverInfo.default_source_name) {

                //start 连接usb耳机，usb输入设备的sourcecb在serverinfocb后更新，没能调用devicechange，侦听也没加载
//                if (defaultSource && nameOutOfSync) {
//                    AUDIO_DEBUG("default source changed, input device changed to {}.", i->name);
//                    manager->m_info.defaultSource->deviceChanged();
//                }
                //end
//                manager->m_info.defaultSource->deviceChanged();
            }
            else {
                AUDIO_DEBUG("source {} port changed.", i->name);
//                manager->m_info.defaultSource->deviceChanged();
            }
        }
        else {
            manager->m_info.defaultSource->copy(*i);
        }
    }

    // 暂时不处理.monitor source
    /** 2025/10/31 取消过滤.monitor设备信息
      * 当默认输入从input设备切换至.monitor设备时
      * 会因c->sources找不到设备从而导致无法出发deviceChanged信号
     **/
//    if (strstr(i->name, ".monitor")) {
//        return;
//    }

    std::shared_ptr<IStream> info = std::make_shared<PaSourceStream>(m_pContext, manager->m_info);
    std::dynamic_pointer_cast<PaSourceStream>(info)->copy(*i);

    {
        auto getSourceList = [&info](PaDataInfo& data) -> std::optional<std::reference_wrapper<std::list<std::shared_ptr<IStream>>>> {
            std::shared_ptr<CardInfo> cardInfo = nullptr;
            for (auto& c : data.cardInfoList) {
                auto prop = pa_proplist_gets(info->getProplist(), PA_PROP_DEVICE_CLASS);
                // if filter,则找到对应maser_device
                auto found = false;
                if (!prop || 0 == strcmp(prop, "filter") || 0 == strcmp(prop, "monitor")) {
                    auto masterDevice = pa_proplist_gets(info->getProplist(), PA_PROP_DEVICE_MASTER_DEVICE);
                    for (const auto& s : c->sources) {

                        if (!cardInfo || 0 == strcmp(data.serverInfo.default_source_name.c_str(), s->getName().c_str())) {
                            cardInfo = c;
                        }

                        if (masterDevice && 0 == strcmp(masterDevice, s->getName().c_str())) {
                            found = true;
                            break;
                        }

                    }
                }

                if (c->card.index == info->getCard() || found) {
                    return c->sources;
                }
            }

            if (!cardInfo) {
                cardInfo = std::make_shared<CardInfo>();
            }
            return cardInfo->sources;
        };

        auto isSameSource = [](std::shared_ptr<IStream> existing, std::shared_ptr<IStream> newSource) {
            return existing->getIndex() == newSource->getIndex();
        };

        std::unique_lock<std::mutex> lock(manager->m_info.m_mtx);
        AUDIO_DEBUG("Source cb, updateData info: {}.", std::dynamic_pointer_cast<PaSourceStream>(info)->toString());
        auto isNew = updateData(manager->m_info, info, getSourceList, isSameSource);

        if (isNew) {
            manager->m_info.m_operation = true;
            manager->m_info.m_cond.notify_one();
        }
    }


    // 确认降噪 source 属于本次加载的回声消除模块
    if (i->name && strcmp(i->name, "noiseReduceSource") == 0) {
        manager->completeModuleLoad("module-echo-cancel", i->owner_module);
    }

    /**  If the default input device is noise reduction, and the active port status of the noise
     *   reduction master device is unavailable, you need to switch to a high-priority input device.
     */
    manager->resetDefaultSource("noiseReduceSource", info);

    resolveEchoCancelMasters(manager->m_info);
    AUDIO_DEBUG("Source cb, i: {}.", std::dynamic_pointer_cast<PaSourceStream>(info)->toString());
}

void PulseaudioManager::sinkInputCb(pa_context* ctx, const pa_sink_input_info* i, int eol, void* userdata) {
    auto manager = static_cast<PulseaudioManager*>(userdata);
    if (eol < 0) {
        if (pa_context_errno(ctx) == PA_ERR_NOENTITY)
            return;

        AUDIO_ERROR("Sink input callback failure");
        return;
    }

    if (eol > 0) {
        return;
    }

    std::shared_ptr<IStream> info = std::make_shared<PaSinkInputStream>(m_pContext, manager->m_info);
    std::dynamic_pointer_cast<PaSinkInputStream>(info)->copy(*i);

    auto it = std::find_if(manager->m_info.sinkInputList.begin(),
                           manager->m_info.sinkInputList.end(),
                           [i](const std::shared_ptr<IStream> s) {
        return s->getIndex() == i->index;
    });

    // 没找到，说明是新产生的sinkinput
    if (it == manager->m_info.sinkInputList.end()) {
        StreamMediaIntegrationInfo streamInfo;
        MediaInfo mInfo;
        streamInfo.mediaRole = std::dynamic_pointer_cast<PaSinkInputStream>(info)->getMediaRole();
        streamInfo.iconName = std::dynamic_pointer_cast<PaSinkInputStream>(info)->getAppIconName();
        streamInfo.binary = std::dynamic_pointer_cast<PaSinkInputStream>(info)->getProcessBinary();
        auto description = AudioMethod::getInstance().getStreamDescription(streamInfo.binary);

        mInfo.direction = 1;
        mInfo.index = i->index;
        mInfo.mute = i->mute;
        mInfo.volume = paCvolumeToValue(info->getVolume());
        mInfo.name = description != "" ? description : info->getName();

        auto it = std::find_if(manager->m_streamMediaList.begin(),
                               manager->m_streamMediaList.end(),
                               [streamInfo](const StreamMediaIntegrationInfo& i) {
            return streamInfo.binary == i.binary;
        });

        /** If the same binary audio stream is found, it means that they are generated
         *  by the same binary and need to be stored in mediaInfoList
         */
        if (it != manager->m_streamMediaList.end()) {
            (*it).mediaInfoList.emplace_back(mInfo);
        }
        else {
            streamInfo.mediaInfoList.emplace_back(mInfo);
            manager->m_streamMediaList.emplace_back(streamInfo);
        }

        manager->addStream(info->getIndex(), mInfo.volume, streamInfo.iconName, mInfo.name);
        manager->addStream(mInfo.direction, mInfo.index, mInfo.volume, mInfo.mute, streamInfo.iconName, mInfo.name);
    }
    else {
        auto v = (*it)->getVolume();
        if (!pa_cvolume_equal(&i->volume, &v)) {
            (*it)->volumeChanged(paCvolumeToValue(i->volume));
        }

        auto mute = (*it)->getMute();
        if (mute != i->mute) {
            (*it)->muteChanged(i->mute);
        }

        //start update mediaInfoList
        StreamMediaIntegrationInfo streamInfo;
        streamInfo.binary = std::dynamic_pointer_cast<PaSinkInputStream>(info)->getProcessBinary();
        AUDIO_DEBUG("Sink input cb, streamInfo.binary: {}.", streamInfo.binary);
        auto it = std::find_if(manager->m_streamMediaList.begin(),
                               manager->m_streamMediaList.end(),
                               [streamInfo](const StreamMediaIntegrationInfo& i) {
            return streamInfo.binary == i.binary;
        });
        if (it != manager->m_streamMediaList.end()) {
            AUDIO_DEBUG("Sink input cb, i->index: {}.", i->index);
            auto mediaInfoIt = std::find_if((*it).mediaInfoList.begin(),
                                    (*it).mediaInfoList.end(),
                                   [i](const MediaInfo& mediaInfo) {
                return i->index == mediaInfo.index;
            });
            if (mediaInfoIt != (*it).mediaInfoList.end()) {
                AUDIO_DEBUG("Sink input cb, mediaInfoIt.index: {}.", mediaInfoIt->index);
                mediaInfoIt->volume = paCvolumeToValue(i->volume);
                mediaInfoIt->mute = i->mute;
            }
        }
        //end
    }

    AUDIO_DEBUG("Sink input cb, updateData info: {}.", std::dynamic_pointer_cast<PaSinkInputStream>(info)->toString());
    updateData(manager->m_info, info,
        [](PaDataInfo& data) -> std::optional<std::reference_wrapper<std::list<std::shared_ptr<IStream>>>> {
            return data.sinkInputList;
        },
        [](std::shared_ptr<IStream> oldSi, std::shared_ptr<IStream> newSi) {
            return oldSi->getIndex() == newSi->getIndex();
        }
    );

    if (i->driver && i->name
        && isEchoCancelPlaybackStream(i->driver, i->name)) {
        updateEchoCancelMaster(manager->m_info, NodeType::OUTPUT, i->sink, "echo-cancel-sink");
    } else {
        resolveEchoCancelMasters(manager->m_info);
    }

    if (i->driver && i->name && isMonoRemapOutputStream(i->driver, i->name)) {
        updateMonoMaster(manager->m_info, i->sink);
    } else {
        resolveMonoMaster(manager->m_info);
    }

    AUDIO_DEBUG("Sink input cb, i: {}.", std::dynamic_pointer_cast<PaSinkInputStream>(info)->toString());
}

void PulseaudioManager::sourceOutputCb(pa_context* ctx, const pa_source_output_info* i, int eol, void* userdata) {
    auto manager = static_cast<PulseaudioManager*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(ctx) == PA_ERR_NOENTITY)
            return;

        AUDIO_ERROR("Source output callback failure");
        return;
    }

    if (eol > 0)  {
        return;
    }

    if (i->source == PA_ID_INVALID)
        return;

    std::shared_ptr<IStream> info = std::make_shared<PaSourceOutputStream>(m_pContext, manager->m_info);
    std::dynamic_pointer_cast<PaSourceOutputStream>(info)->copy(*i);

    auto it = std::find_if(manager->m_info.sourceOutputList.begin(),
                           manager->m_info.sourceOutputList.end(),
                           [i](const std::shared_ptr<IStream> s) {
        return s->getIndex() == i->index;
    });

    // 没找到，说明是新产生的sinkinput
    if (it == manager->m_info.sourceOutputList.end()) {
        StreamMediaIntegrationInfo streamInfo;
        MediaInfo mInfo;
        streamInfo.mediaRole = std::dynamic_pointer_cast<PaSourceOutputStream>(info)->getMediaRole();
        streamInfo.iconName = std::dynamic_pointer_cast<PaSourceOutputStream>(info)->getAppIconName();
        streamInfo.binary = std::dynamic_pointer_cast<PaSourceOutputStream>(info)->getProcessBinary();
        auto description = AudioMethod::getInstance().getStreamDescription(streamInfo.binary);

        mInfo.direction = 0;
        mInfo.index = i->index;
        mInfo.volume = paCvolumeToValue(info->getVolume());
        mInfo.mute = i->mute;
        mInfo.name = description != "" ? description : info->getName();

        auto it = std::find_if(manager->m_streamMediaList.begin(),
                               manager->m_streamMediaList.end(),
                               [streamInfo](const StreamMediaIntegrationInfo& i) {
            return streamInfo.binary == i.binary;
        });

        /** If the same binary audio stream is found, it means that they are generated
         *  by the same binary and need to be stored in mediaInfoList
         */
        if (it != manager->m_streamMediaList.end()) {
            (*it).mediaInfoList.emplace_back(mInfo);
        }
        else {
            streamInfo.mediaInfoList.emplace_back(mInfo);
            manager->m_streamMediaList.emplace_back(streamInfo);
        }

        manager->addStream(mInfo.direction, mInfo.index, mInfo.volume, mInfo.mute, streamInfo.iconName, mInfo.name);
    }
    else {
        auto v = (*it)->getVolume();
        if (!pa_cvolume_equal(&i->volume, &v)) {
            (*it)->volumeChanged(paCvolumeToValue(i->volume));
        }

        auto mute = (*it)->getMute();
        if (mute != i->mute) {
            (*it)->muteChanged(i->mute);
        }
    }

    AUDIO_DEBUG("Source output cb, info: {}.", std::dynamic_pointer_cast<PaSourceOutputStream>(info)->toString());
    updateData(manager->m_info, info,
        [](PaDataInfo& data) -> std::optional<std::reference_wrapper<std::list<std::shared_ptr<IStream>>>> {
            return data.sourceOutputList;
        },
        [](std::shared_ptr<IStream> oldValue, std::shared_ptr<IStream> newVaule) {
            return oldValue->getIndex() == newVaule->getIndex();
        }
        );

    if (i->driver && i->name
        && isEchoCancelCaptureStream(i->driver, i->name)) {
        updateEchoCancelMaster(manager->m_info, NodeType::INPUT, i->source, "noiseReduceSource");
    } else {
        resolveEchoCancelMasters(manager->m_info);
    }

    AUDIO_DEBUG("Source output cb, i: {}.", std::dynamic_pointer_cast<PaSourceOutputStream>(info)->toString());
}

void PulseaudioManager::clientCb(pa_context* ctx, const pa_client_info* i, int eol, void* userdata) {
    auto manager = static_cast<PulseaudioManager*>(userdata);

    if (eol < 0) {
        if (pa_context_errno(ctx) == PA_ERR_NOENTITY)
            return;

        AUDIO_ERROR("Client callback failure");
        return;
    }

    if (eol > 0) {
//        decOutstanding(w);
        return;
    }
    AUDIO_DEBUG("clientCb, client->name: {}", i->name);
//    manager->updateClient(*i);
}

void PulseaudioManager::serverInfoCb(pa_context* ctx, const pa_server_info* i, void* userdata) {
    auto manager = static_cast<PulseaudioManager*>(userdata);

    if (!i) {
        AUDIO_ERROR("Server info callback failure");
        return;
    }

    std::shared_ptr<IStream> masterSink = nullptr;
    std::string deviceClass = "";
    if (manager->m_info.defaultSink) {
        deviceClass = static_cast<const PaSinkStream*>(manager->m_info.defaultSink.get())->getDeviceClass();
        if (deviceClass == "filter") {
            masterSink = manager->getMasterStream(manager->m_info.defaultSink.get(), NodeType::OUTPUT);

        } else {
            masterSink = manager->m_info.defaultSink;
        }
    }

    if (masterSink) {
        AUDIO_DEBUG("serverInfoCb, masterSink:{} .", masterSink->getName());
    }

    // 如果只是做sink切换，是不会触发sink回调的，此时我们需要更新default sink以及发出设备更改的信号
    if (manager->m_info.defaultSink &&
        manager->m_info.serverInfo.default_sink_name != i->default_sink_name) {
        const long long lastUserMs = manager->m_lastUserSetDefaultOutputMs.load(std::memory_order_relaxed);
        const bool recentUserSwitch = (nowMs() - lastUserMs) < 1500;

        bool found = false;
        for (const auto&c : manager->m_info.cardInfoList) {
            auto at = std::find_if(c->sinks.begin(), c->sinks.end(), [manager](const std::shared_ptr<IStream> s) {
                return manager->m_info.defaultSink->getName() == s->getName();
            });

            /**
             * If it is not found, it means that the default output device has been removed
             * and the multimedia application needs to be paused. If it is found, it means
             * that the default device has been manually switched and no pause is needed.
             */
            if (at != c->sinks.end()) {
                AUDIO_DEBUG("Find the last default output device without pausing multimedia applications.");
                manager->m_autoPause = false;
                break;
            }
            else {
                /**
                 *  An empty masterSink indicates that the device has been removed and needs to be paused.
                 */
                if ((std::string::npos == manager->m_info.defaultSink->getName().find("mono")) || !masterSink) {
                    manager->m_autoPause = true;
                }
            }
        }

        if (manager->m_autoPause)
            AUDIO_DEBUG("The last default output device was removed, and the multimedia application needs to be paused.");

        for (const auto& c : manager->m_info.cardInfoList) {

            auto it = std::find_if(c->sinks.begin(), c->sinks.end(), [i](const std::shared_ptr<IStream> s) {
                return s->getName() == i->default_sink_name;
            });

            // 更新default sink
            if (it != c->sinks.end()) {
                found = true;
                manager->m_info.defaultSink = std::dynamic_pointer_cast<PaSinkStream>(*it);
                if (manager) {
                    AUDIO_DEBUG("serverInfoCb,  serverInfo.default_sink_name {}."
                                , manager->m_info.defaultSink->getName());
                }
                break;
            }
        }

        if (manager->m_info.defaultSink && "mono" != manager->m_info.defaultSink->getName()) {
            for (const auto& c : manager->m_info.cardInfoList) {
                auto mute = manager->m_info.defaultSink->getMute();
                auto volume = manager->m_info.defaultSink->getVolume();

                // 当设备发生变更且存在单声道设备时，同步当前设备音量到单声道音频
                auto at = std::find_if(c->sinks.begin(), c->sinks.end(), [i](const std::shared_ptr<IStream> s) {
                    return s->getName() == "mono";
                });

                if (at != c->sinks.end()) {

                    int value = paCvolumeToPaValue(volume);

                    AUDIO_DEBUG("Default device changed to: {}, set {} volume to {}.", manager->m_info.defaultSink->getName(),
                                (*at)->getName(), value);
//                    (*at)->setMute(mute);
//                    (*at)->setVolume(value);
                    break;
                }
            }
        }

        if (found && strcmp(i->default_sink_name, "mono")) {
            AUDIO_DEBUG("serverinfo cb, mono to deviceChanged");
            auto defaultSink = std::dynamic_pointer_cast<PaSinkStream>(manager->m_info.defaultSink);
            if (defaultSink) {
                // serverInfoCb：
                // - 手动切换默认输出：不显示OSD
                // - 自动切换默认输出(如USB/蓝牙/耳机插入自动设为默认)：显示OSD
                // - 拔掉默认输出设备导致回退到其它有效设备：显示OSD
                bool showOsd = !recentUserSwitch;
                if (showOsd) {
                    showOsd = manager->shouldShowOsdDedup(defaultSink->getName(), nowMs());
                }
                if (showOsd) {
                    AudioMethod::getInstance().showOsd(defaultSink->getActivePortName());
                }
                defaultSink->deviceChanged(manager->m_autoPause);
            }
        }

        if (manager->m_autoPause)
            manager->m_autoPause = false;

        AUDIO_DEBUG("Default sink from {} change to {}.", manager->m_info.serverInfo.default_sink_name, i->default_sink_name);
    }

    AUDIO_DEBUG("serverInfoCb, serverInfo.default_source_name: {} i->default_source_name:{} ."
                , manager->m_info.serverInfo.default_source_name, i->default_source_name);
    if (manager->m_info.serverInfo.default_source_name != i->default_source_name) {
        for (const auto& c : manager->m_info.cardInfoList) {

            auto it = std::find_if(c->sources.begin(), c->sources.end(),
                                   [i](const std::shared_ptr<IStream> s) {
                AUDIO_DEBUG("serverInfoCb, s->getName(): {} i->default_source_name:{} ."
                            , s->getName(), i->default_source_name);
                return s->getName() == i->default_source_name;
            });

            // 更新default source
            if (it != c->sources.end()) {
                manager->m_info.defaultSource = std::dynamic_pointer_cast<PaSourceStream>(*it);
                AUDIO_DEBUG("serverinfo cb, defaultSource to deviceChanged defaultSource->getActivePortName:{}"
                            , manager->m_info.defaultSource->getActivePortName());
                manager->m_info.defaultSource->deviceChanged();
                break;
            }
        }

        AUDIO_DEBUG("Default source from {} change to {}.", manager->m_info.serverInfo.default_source_name, i->default_source_name);
    }

    manager->m_info.serverInfo = *i;


    AUDIO_DEBUG("serverInfoCb, default sink name: {}, default source name: {}", i->default_sink_name, i->default_source_name);
}

//void PulseaudioManager::getBatteryLevel(std::string dev)
//{
//    pa_operation *o;
//    if (!(o = pa_context_get_card_info_by_name(getContext(), dev.toLatin1().data(), batteryLevelCb, this))) {
//        std::cout << "pa_context_get_card_info_by_index() failed";
//        return ;
//    }
//    pa_operation_unref(o);
//}

void PulseaudioManager::extStreamRestoreReadCb(
        pa_context* ctx,
        const pa_ext_stream_restore_info* i,
        int eol,
        void* userdata) {

    auto manager = static_cast<PulseaudioManager*>(userdata);

    if (eol < 0) {
//        decOutstanding(w);
        AUDIO_ERROR("Failed to initialize stream_restore extension ={}", pa_strerror(pa_context_errno(ctx)));
        return;
    }

    if (eol > 0) {
//        decOutstanding(w);
        return;
    }
}

void PulseaudioManager::extStreamRestoreSubscribeCb(pa_context* ctx, void* userdata) {
    auto manager = static_cast<PulseaudioManager*>(userdata);
    pa_operation* o;

    if (!(o = pa_ext_stream_restore_read(ctx, extStreamRestoreReadCb, manager))) {
        AUDIO_ERROR("pa_ext_stream_restore_read() failed");
        return;
    }
    AUDIO_DEBUG("extStreamRestoreSubscribeCb");
    pa_operation_unref(o);
}

#if HAVE_EXT_DEVICE_RESTORE_API
void ext_device_restore_read_cb(
        pa_context *ctx,
        const pa_ext_device_restore_info *i,
        int eol,
        void *userdata) {

    PulseaudioManager* manager = static_cast<PulseaudioManager*>(userdata);

    if (eol < 0) {
        manager->decOutstanding(manager);
        AUDIO_ERROR("Failed to initialize device restore extension: ={}", pa_strerror(pa_context_errno(ctx));
        return;
    }

    if (eol > 0) {
        return;
    }

    /* Do something with a widget when this part is written */
    manager->updateDeviceInfo(*i);
}

static void ext_device_restore_subscribeCb(pa_context* ctx, pa_device_type_t type, uint32_t idx, void* userdata) {
    PulseaudioManager* manager = static_cast<PulseaudioManager*>(userdata);
    pa_operation* o;

    if (type != PA_DEVICE_TYPE_SINK)
        return;

    if (!(o = pa_ext_device_restore_read_formats(ctx, type, idx, ext_device_restore_read_cb, manager))) {
        AUDIO_ERROR("pa_ext_device_restore_read_sink_formats() failed"));
        return;
    }

    pa_operation_unref(o);
}
#endif

void PulseaudioManager::extDeviceManagerReadCb(
        pa_context* ctx,
        const pa_ext_device_manager_info*,
        int eol,
        void* userdata) {

    auto manager = static_cast<PulseaudioManager*>(userdata);

    if (eol < 0) {
//        decOutstanding(w);
        AUDIO_ERROR("Failed to initialize device manager extension: ={}", pa_strerror(pa_context_errno(ctx)));
        return;
    }

//    w->canRenameDevices = true;

    if (eol > 0) {
//        decOutstanding(w);
        return;
    }
    AUDIO_DEBUG("extDeviceManagerReadCb");
    /* Do something with a widget when this part is written */
}

void PulseaudioManager::extDeviceManagerSubscribeCb(pa_context* ctx, void *userdata) {
    auto manager = static_cast<PulseaudioManager*>(userdata);
    pa_operation* o;

    if (!(o = pa_ext_device_manager_read(ctx, extDeviceManagerReadCb, manager))) {
        AUDIO_ERROR("pa_ext_device_manager_read() failed");
        return;
    }
    AUDIO_DEBUG("extDeviceManagerSubscribeCb");
    pa_operation_unref(o);
}

void PulseaudioManager::subscribeCb(pa_context* ctx, pa_subscription_event_type_t t, uint32_t index, void *userdata) {
    auto manager = static_cast<PulseaudioManager*>(userdata);

    switch (t & PA_SUBSCRIPTION_EVENT_FACILITY_MASK) {
    case PA_SUBSCRIPTION_EVENT_SINK: {
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE){
            AUDIO_DEBUG("Sink removed index: {}.", index);
            for (const auto& c : manager->m_info.cardInfoList) {

                auto it = std::remove_if(c->sinks.begin(), c->sinks.end(), [index, manager](const std::shared_ptr<IStream>& s) {
                    if (s->getIndex() == index) {
                        AUDIO_DEBUG("Remove {} sink, name : {}.", index, s->getName());
                        // 如果是组合设备被移除，则需要卸载模块
                        if (std::dynamic_pointer_cast<PaSinkStream>(s)->getCombineStatus()) {
                            AudioMethod::getInstance().unloadModule("module-combine-sink");
                            manager->deviceAdjust(NodeType::OUTPUT);
                        }

                        if (s->getName().compare("mono") == 0) {
                            manager->removeModuleInfoByName("module-remap-sink");
                        }
                        return true;
                    }
                    return false;
                });
                c->sinks.erase(it, c->sinks.end());
                if (it == c->sinks.end()) {
                    AUDIO_ERROR("Card {} not found {} index sink.", c->card.index, index);
                }
            }
        }
        else {
            pa_operation* o;
            if (!(o = pa_context_get_sink_info_by_index(ctx, index, sinkCb, manager))) {
                AUDIO_ERROR("pa_context_get_sink_info_by_index() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    }
    case PA_SUBSCRIPTION_EVENT_SOURCE: {
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            for (const auto& c : manager->m_info.cardInfoList) {
                auto it = std::find_if(c->sources.begin(), c->sources.end(), [index](const std::shared_ptr<IStream> s) {
                    return s->getIndex() == index;
                });

                if (it != c->sources.end()) {
                    AUDIO_DEBUG("Remove {} source, name : {}.", index, (*it)->getName());
                    c->sources.erase(it);
//                    break;
                }
                else {
                    AUDIO_ERROR("Card {} not found {} index source.", c->card.index, index);
                }
            }
        }
        else {
            pa_operation* o;
            if (!(o = pa_context_get_source_info_by_index(ctx, index, sourceCb, manager))) {
                AUDIO_ERROR("pa_context_get_source_info_by_index() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    }
    case PA_SUBSCRIPTION_EVENT_SINK_INPUT: {
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            auto it = std::find_if(manager->m_info.sinkInputList.begin(), manager->m_info.sinkInputList.end(),
                                   [index](const std::shared_ptr<IStream> s) {
                return s->getIndex() == index;
            });

            if (it != manager->m_info.sinkInputList.end()) {
                auto binary = std::dynamic_pointer_cast<PaSinkInputStream>(*it)->getProcessBinary();
                manager->removeStreamMedia(binary, 1, index);
                manager->m_info.sinkInputList.erase(it);
                AUDIO_DEBUG("Sink input {} removed.", index);
            }
            else {
                AUDIO_ERROR("Can not found {} index sink input.", index);
            }
	    manager->removeStream(index);
            manager->removeStream(1, index);
        }
        else {
            pa_operation* o;
            if (!(o = pa_context_get_sink_input_info(ctx, index, sinkInputCb, manager))) {
                AUDIO_ERROR("pa_context_get_sink_input_info() failed.");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    }
    case PA_SUBSCRIPTION_EVENT_SOURCE_OUTPUT: {
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            auto it = std::find_if(manager->m_info.sourceOutputList.begin(), manager->m_info.sourceOutputList.end(),
                                   [index](const std::shared_ptr<IStream> s) {
                return s->getIndex() == index;
            });

            if (it != manager->m_info.sourceOutputList.end()) {
                auto binary = std::dynamic_pointer_cast<PaSourceOutputStream>(*it)->getProcessBinary();
                manager->removeStreamMedia(binary, 0, index);
                manager->m_info.sourceOutputList.erase(it);
                AUDIO_DEBUG("Source output {} removed.", index);
            }
            else {
                AUDIO_ERROR("Can not found {} index source output.", index);
            }

            manager->removeStream(0, index);
        }
        else {
            pa_operation* o;
            if (!(o = pa_context_get_source_output_info(ctx, index, sourceOutputCb, manager))) {
                AUDIO_ERROR("pa_context_get_sink_input_info() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    }
    case PA_SUBSCRIPTION_EVENT_CLIENT: {
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            //                w->removeClient(index);
        }
        else {
            pa_operation* o;
            if (!(o = pa_context_get_client_info(ctx, index, clientCb, manager))) {
                AUDIO_ERROR("pa_context_get_client_info() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    }
    case PA_SUBSCRIPTION_EVENT_SERVER: {
        pa_operation* o;
        if (!(o = pa_context_get_server_info(ctx, serverInfoCb, manager))) {
            AUDIO_ERROR("pa_context_get_server_info() failed");
            return;
        }
        pa_operation_unref(o);

        break;
    }
    case PA_SUBSCRIPTION_EVENT_CARD: {
        if ((t & PA_SUBSCRIPTION_EVENT_TYPE_MASK) == PA_SUBSCRIPTION_EVENT_REMOVE) {
            auto it = std::find_if(manager->m_info.cardInfoList.begin(), manager->m_info.cardInfoList.end(),
                                   [index](const std::shared_ptr<CardInfo> c) {
                return c->card.index == index;
            });

            if (it != manager->m_info.cardInfoList.end()) {
                manager->m_info.cardInfoList.erase(it);
            }
            else {
                AUDIO_ERROR("Can not found {} index card.", index);
            }

            manager->deviceAdjust(NodeType::OUTPUT);
        }
        else {
            pa_operation* o;
            if (!(o = pa_context_get_card_info_by_index(ctx, index, cardCb, manager))) {
                AUDIO_ERROR("pa_context_get_card_info_by_index() failed");
                return;
            }
            pa_operation_unref(o);
        }
        break;
    }
    default:
        break;
    }
}

void PulseaudioManager::contextStateCallback(pa_context* ctx, void* userdata) {
    auto manager = static_cast<PulseaudioManager*>(userdata);

    g_assert(ctx);
    const pa_context_state_t contextState = pa_context_get_state(ctx);
    switch (contextState) {
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY: {
            pa_operation* o;
            AUDIO_DEBUG("pa context state change to ={}", pa_context_get_state(ctx));
            manager->m_reconnectTime = 3;
            pa_context_set_subscribe_callback(ctx, subscribeCb, manager);

            if (!(o = pa_context_subscribe(ctx, (pa_subscription_mask_t)
                                           (PA_SUBSCRIPTION_MASK_SINK|
                                            PA_SUBSCRIPTION_MASK_SOURCE|
                                            PA_SUBSCRIPTION_MASK_SINK_INPUT|
                                            PA_SUBSCRIPTION_MASK_SOURCE_OUTPUT|
                                            PA_SUBSCRIPTION_MASK_CLIENT|
                                            PA_SUBSCRIPTION_MASK_SERVER|
                                            PA_SUBSCRIPTION_MASK_CARD), nullptr, nullptr))) {
                AUDIO_ERROR("pa_context_subscribe() failed.");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_server_info(ctx, serverInfoCb, manager))) {
                AUDIO_ERROR("pa_context_get_server_info() failed.");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_client_info_list(ctx, clientCb, manager))) {
                AUDIO_ERROR("pa_context_client_info_list() failed.");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_card_info_list(ctx, cardCb, manager))) {
                AUDIO_ERROR("pa_context_get_card_info_list() failed.");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_sink_info_list(ctx, sinkCb, manager))) {
                AUDIO_ERROR("pa_context_get_sink_info_list() failed.");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_source_info_list(ctx, sourceCb, manager))) {
                AUDIO_ERROR("pa_context_get_source_info_list() failed.");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_sink_input_info_list(ctx, sinkInputCb, manager))) {
                AUDIO_ERROR("pa_context_get_sink_input_info_list() failed.");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_source_output_info_list(ctx, sourceOutputCb, manager))) {
                AUDIO_ERROR("pa_context_get_source_output_info_list() failed.");
                return;
            }
            pa_operation_unref(o);

            if (!(o = pa_context_get_module_info_list(ctx, moduleInfoCb, manager))) {
                AUDIO_ERROR("pa_context_get_module_info_list() failed.");
                return;
            }
            pa_operation_unref(o);
            break;
        }

        case PA_CONTEXT_FAILED:
            pa_context_unref(manager->m_pContext);
            manager->m_pContext = nullptr;

            AUDIO_ERROR("Connection failed, attempting reconnect.");
            manager->m_reconnectTime--;
            if (manager->m_reconnectTime <= 0) {
                AUDIO_DEBUG("reconnect pulseaudio Three times failed.");
                return;
            }

//            g_timeout_add_seconds(5, connectToPulse, w);
            return;

        case PA_CONTEXT_TERMINATED:
        default:
            return;
    }
}

void PulseaudioManager::moduleInfoCb(pa_context* ctx, const pa_module_info* i, int eol, void *userdata)
{
    if (!i)
        return;

    if (eol < 0) {
        if (pa_context_errno(ctx) == PA_ERR_NOENTITY)
            return;

        AUDIO_ERROR("moduleInfoCb callback failure.");
        return;
    }

    auto manager = static_cast<PulseaudioManager*>(userdata);
    PaModuleInfo info;
    info = *i;
    updateData(manager->m_info, info,
        [](PaDataInfo& data) -> std::optional<std::reference_wrapper<std::list<PaModuleInfo>>> {
            return data.moduleInfoList;
        },
        [](const PaModuleInfo& existing, const PaModuleInfo& newCard) {
            return existing.index == newCard.index;
        }
    );

    AUDIO_DEBUG("Module info cb, info: {}.", info.toString());
}

std::vector<std::string> PulseaudioManager::parseSlaves(const std::string& config)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(config);

    while (std::getline(tokenStream, token, '=')) {
        if (token == "slaves") {
            std::getline(tokenStream, token, ','); // 读取第一个sink
            token.erase(0, token.find_first_not_of(" \t\n\r\f\v")); // 去除前导空格
            token.erase(token.find_last_not_of(" \t\n\r\f\v") + 1); // 去除尾随空格
            if (!token.empty()) {
                tokens.push_back(token);
            }

            // 继续读取剩余的sink，直到没有更多的分隔符
            while (std::getline(tokenStream, token, ',')) {
                token.erase(0, token.find_first_not_of(" \t\n\r\f\v")); // 去除前导空格
                token.erase(token.find_last_not_of(" \t\n\r\f\v") + 1); // 去除尾随空格
                if (!token.empty()) {
                    tokens.push_back(token);
                }
            }

            // 如果最后一个sink后面没有逗号，也会被正确处理
            if (tokenStream) {
                std::getline(tokenStream, token);
                token.erase(0, token.find_first_not_of(" \t\n\r\f\v")); // 去除前导空格
                token.erase(token.find_last_not_of(" \t\n\r\f\v") + 1); // 去除尾随空格
                if (!token.empty()) {
                    tokens.push_back(token);
                }
            }
        }
    }

    return tokens;
}

bool PulseaudioManager::isExitEnableDevice(const NodeType& type)
{
    auto ports = getAvailablePortList(type);

    auto it = std::find_if(ports.begin(), ports.end(), [](const std::shared_ptr<DeviceInfo>& info){
        return info->enabled;
    });

    return it != ports.end();
}

void PulseaudioManager::ensureDefaultOnEnabledDevice(const NodeType& type)
{
    const DefaultDeviceInfo current = getDefaultDevice(type, -1);
    const auto ports = getAvailablePortList(type);

    const bool currentValid = std::any_of(ports.begin(), ports.end(), [&current](const std::shared_ptr<DeviceInfo>& port) {
        return port && port->enabled
            && port->cardName == current.cardName
            && port->portName == current.activePortName;
    });

    if (currentValid) {
        return;
    }

    std::shared_ptr<DeviceInfo> bestPort;
    for (const auto& port : ports) {
        if (!port || !port->enabled) {
            continue;
        }
        if (!bestPort || port->priority > bestPort->priority) {
            bestPort = port;
        }
    }

    if (!bestPort) {
        AUDIO_DEBUG("No enabled {} device to switch default to after port availability change.",
                    getNodeTypeName(type));
        return;
    }

    AUDIO_DEBUG("Default {} device card: {} port: {} is disabled or unavailable, switch to card: {} port: {}.",
                getNodeTypeName(type), current.cardName, current.activePortName,
                bestPort->cardName, bestPort->portName);
    setDefaultDevice(type, -1, bestPort->portName, bestPort->cardName);
}

bool PulseaudioManager::evaluateLoadModulePreconditions(const std::string& name, const std::string& param)
{
    (void)param;
    if (std::string::npos != name.find("module-echo-cancel")) {
        if (!m_info.defaultSource ||
            std::string::npos == m_info.defaultSource->getName().find("alsa_input") ||
            std::string::npos != m_info.defaultSource->getName().find("input.usb")) {
            if (m_info.defaultSource) {
                AUDIO_WARN("Can not load module-echo-cancel, default_source: {}.", m_info.defaultSource->getName());
            }
            return false;
        }
    } else if (std::string::npos != name.find("module-remap-sink")) {
        if (!m_info.defaultSink || std::string::npos != m_info.defaultSink->getName().find("auto_null")) {
            if (m_info.defaultSink) {
                AUDIO_WARN("module-remap-sink cannot be loaded with {} sink.", m_info.defaultSink->getName());
            }
            return false;
        }
    } else if (std::string::npos != name.find("module-loopback")) {
        if (m_bModuleLoopbackLoading) {
            AUDIO_WARN("module-loopback is already loading, can not load again.");
            return false;
        }
        const DefaultDeviceInfo devInfo = getDefaultDevice(NodeType::INPUT, -1);
        const bool isNullorInternalDev =
            (std::string::npos != devInfo.name.find("auto_null")) ||
            (std::string::npos != devInfo.name.find(".monitor")) ||
            (std::string::npos != devInfo.activePortName.find("internal")) ||
            (std::string::npos != devInfo.activePortLabel.find("Digital"));
        AUDIO_DEBUG("module-loopback isNullorInternalDev:{}.", isNullorInternalDev);
        if (isNullorInternalDev) {
            return false;
        }
    }
    return true;
}

void PulseaudioManager::resetDefaultSource(const std::string& name, const std::shared_ptr<IStream>& info)
{
    if (m_info.serverInfo.default_source_name == name) {
        std::shared_ptr<IStream> source = nullptr;

        for (const auto& c : m_info.cardInfoList) {
            auto s = std::find_if(c->sources.begin(), c->sources.end(), [this](const std::shared_ptr<IStream>& s) {
                return s->getName() == m_info.serverInfo.default_source_name;
            });

            if (s != c->sources.end()) {
                auto masterDevice = std::dynamic_pointer_cast<PaSourceStream>(*s)->getMasterDevice();

                if (masterDevice != info->getName())
                    break;

                auto it = std::find_if(c->sources.begin(), c->sources.end(), [masterDevice](const std::shared_ptr<IStream>& s) {
                    return masterDevice == s->getName();
                });

                if (it != c->sources.end()) {
                    source = *it;
                }

                break;
            }
        }

        if (source) {
            auto port = std::dynamic_pointer_cast<PaSourceStream>(source)->getActivePort();
            if (port && port->available == PA_PORT_AVAILABLE_NO) {
                auto defaultSouce = findHighestPriorityStream(source);
                if (defaultSouce) {
                    AUDIO_DEBUG("noiseReduceSource master device {} active port become not availabe, set default source to {}.", info->getName(), defaultSouce->getName());
                    pa_operation_unref(pa_context_set_default_source(m_pContext, defaultSouce->getName().c_str(), nullptr, nullptr));
                }
            }

        }
    }
}

void PulseaudioManager::removeStreamMedia(const std::string& binary, int direction, int idx)
{
    auto it = std::find_if(m_streamMediaList.begin(),
                           m_streamMediaList.end(),
                           [binary](const StreamMediaIntegrationInfo& i) {
        return binary == i.binary;
    });

    if (it != m_streamMediaList.end()) {
        if ((*it).mediaInfoList.size() > 1) {
            std::erase_if((*it).mediaInfoList, [direction, idx](const MediaInfo& i) {
                return direction == i.direction && idx == i.index;
            });
        }
        else {
            m_streamMediaList.erase(it);
        }
    }
}

void PulseaudioManager::removeModuleInfoByName(const std::string& name)
{
    auto it = std::find_if(m_info.moduleInfoList.begin(), m_info.moduleInfoList.end(),
        [name](const PaModuleInfo& info) { return name == info.name; });

    if (it != m_info.moduleInfoList.end()) {
        AUDIO_DEBUG("Module {} has removed.", name);
        m_info.moduleInfoList.erase(it);
    }
    else {
        AUDIO_ERROR("Module {} not found in module list.", name);
    }
}

std::shared_ptr<IStream> PulseaudioManager::getMasterStream(const IStream* filter, NodeType type) const
{
    const std::string masterName = (type == NodeType::INPUT)
                                    ? static_cast<const PaSourceStream*>(filter)->getMasterDevice()
                                    : static_cast<const PaSinkStream*>(filter)->getMasterDevice();
    if (masterName.empty()) return nullptr;

    for (const auto& card : m_info.cardInfoList) {
        const auto& candidates = (type == NodeType::INPUT) ? card->sources : card->sinks;
        if (auto m = findStream(candidates, masterName)) return m;
    }
    return nullptr;
}

std::string PulseaudioManager::getBlueMediaTransportPath(GDBusConnection* connection , const std::string& deviceMac)
{
    std::string transportPath = "";
    GError* error = nullptr;
    GVariant* result = nullptr;

    // 1. 调用ObjectManager的GetManagedObjects方法
    result = g_dbus_connection_call_sync(connection,
                                         "org.bluez",
                                         "/",
                                         "org.freedesktop.DBus.ObjectManager",
                                         "GetManagedObjects",
                                         nullptr,
                                         G_VARIANT_TYPE("(a{oa{sa{sv}}})"),
                                         G_DBUS_CALL_FLAGS_NONE,
                                         -1,
                                         nullptr,
                                         &error);

    if (!result) {
        AUDIO_WARN("GetManagedObjects failed in bluez bus, error: {}.",
                   error ? error->message : "unknown");
        if (error) g_error_free(error);
        return "";
    }

    // 2. 解析返回的对象数组
    GVariant* objectsDict;
    g_variant_get(result, "(@a{oa{sa{sv}}})", &objectsDict);

    GVariantIter iter;
    g_variant_iter_init(&iter, objectsDict);
    char* objectPath;
    GVariant* interfacesDict;

    // 3. 遍历所有对象路径和接口
    while (g_variant_iter_next(&iter, "{&o@a{sa{sv}}}", &objectPath, &interfacesDict)) {
        GVariantIter interfacesIter;
        g_variant_iter_init(&interfacesIter, interfacesDict);
        char* interfaceName;
        bool found = false;

        // 4. 检查每个对象的接口
        while (g_variant_iter_next(&interfacesIter, "{&s@a{sv}}", &interfaceName, nullptr)) {
            // 5. 寻找org.bluez.MediaTransport1接口
            if (std::string(interfaceName) == "org.bluez.MediaTransport1") {
                // 6. 检查对象路径是否包含目标设备MAC地址
                if (std::string(objectPath).find(deviceMac) != std::string::npos) {
                    transportPath = objectPath;
                    found = true;
                    break;
                }
            }
        }

        if (found)
            break;
    }

    g_variant_unref(objectsDict);
    g_variant_unref(interfacesDict);
    g_variant_unref(result);

    return transportPath;
}

bool PulseaudioManager::isBluetoothDeviceReady(const std::string& cardName)
{
    if (!cardName.starts_with("bluez")) {
        AUDIO_ERROR("Device '{}' is not a Bluetooth device.", cardName);
        return false;
    }

    size_t dotPos = cardName.find ('.');
    if (dotPos == std::string::npos || dotPos + 1 >= cardName.size ()) {
        AUDIO_ERROR ("Invalid bluetooth cardName format: {}.", cardName);
        return false;
    }
    std::string targetMac = cardName.substr (dotPos + 1);

    bool isReady = false;

    GError* error = nullptr;
    GVariant* result = nullptr;

    GDBusConnection* connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
    if (!connection) {
        AUDIO_ERROR("Can't not connect to D-Bus, Error: {}.",
                    error ? error->message : "unknown");
        if (error) g_error_free(error);
        return false;
    }

    std::string transportPath = getBlueMediaTransportPath(connection, targetMac);
    if (transportPath.empty()) {
        AUDIO_ERROR("Please check device {} is support AVRCP.", targetMac);
        g_object_unref(connection);
        return false;
    }
    {
        result = g_dbus_connection_call_sync(connection,
                                             "org.bluez",
                                             transportPath.c_str(),
                                             "org.freedesktop.DBus.Properties",
                                             "Get",
                                             g_variant_new("(ss)", "org.bluez.MediaTransport1", "Volume"),
                                             G_VARIANT_TYPE("(v)"),
                                             G_DBUS_CALL_FLAGS_NONE,
                                             -1,
                                             nullptr,
                                             &error);

        if (!result) {
            // 获取属性失败，可能是该属性不存在（例如设备未连接或不支持）
            AUDIO_ERROR("Get device volume property failed, Error: {}.",
                        error ? error->message : "unknown");
            if (error) g_error_free(error);
            return false;
        } else {
            isReady = true;
        }

        GVariant* volumeVariant;
        g_variant_get(result, "(v)", &volumeVariant);

        if (g_variant_is_of_type(volumeVariant, G_VARIANT_TYPE_UINT16)) {
            guint16 volume = g_variant_get_uint16(volumeVariant);
            AUDIO_DEBUG("Device {} a2dp volume is {}.", cardName, volume);
        } else {
            gchar* valueStr = g_variant_print(volumeVariant, TRUE);
            AUDIO_DEBUG("Device {} a2dp volume is {}.", cardName, valueStr);
            g_free(valueStr);
        }

        g_variant_unref(volumeVariant);
        g_variant_unref(result);
    }

    return isReady;
}


}

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

#include "MultiAudioCombineModule.h"

#include "AudioMethod.h"
#include "AudioContext.h"
#include "BaseType.h"

namespace
{
constexpr const char* kPulseCombineModule = "module-combine-sink";
constexpr const char* kPipewireCombineModule = "libpipewire-module-combine-stream";
constexpr const char* kCombinedSinkName = "combined";

bool isEchoCancelDevice(const std::string& name)
{
    return std::string::npos != name.find("echo-cancel");
}

bool isBluetoothA2dpSink(const std::string& name)
{
    if (isEchoCancelDevice(name))
        return false;

    // PulseAudio: bluez_sink.xx.a2dp_sink
    if (std::string::npos != name.find("a2dp_sink"))
        return true;

    // PipeWire: bluez_output.xx.a2dp-sink
    return std::string::npos != name.find("a2dp-sink");
}

bool isPipewireBackend()
{
    return UkuiAudioFramwork::getBackendName(UkuiAudioFramwork::BackendType::PIPEWIRE) ==
           UkuiAudioFramwork::AudioMethod::getInstance().getBackend();
}

const char* getCombineModuleName()
{
    return isPipewireBackend() ? kPipewireCombineModule : kPulseCombineModule;
}

std::string buildCombineModuleParam(const std::vector<std::string>& combineDevList)
{
    if (!isPipewireBackend()) {
        std::string param = "slaves=";
        param.append(combineDevList.at(0));
        param.append(",");
        param.append(combineDevList.at(1));
        return param;
    }

    std::string param = "{\n"
                        " combine.mode = sink\n"
                        " node.name = \"";
    param.append(kCombinedSinkName);
    param.append("\"\n"
                 " node.description = \"Multi Audio Output\"\n"
                 " combine.latency-compensate = false\n"
                 " combine.props = { audio.position = [ FL FR ] }\n"
                 " stream.rules = [\n");

    for (size_t i = 0; i < combineDevList.size(); ++i) {
        param.append("  { matches = [ { media.class = \"Audio/Sink\" node.name = \"");
        param.append(combineDevList.at(i));
        param.append("\" } ] actions = { create-stream = {} } }");
        if (i + 1 < combineDevList.size())
            param.append(",");
        param.append("\n");
    }

    param.append(" ]\n}");
    return param;
}
}

namespace UkuiAudioFramwork
{
MultiAudioCombineModule& MultiAudioCombineModule::getInstance()
{
    static MultiAudioCombineModule instance;
    return instance;
}

void MultiAudioCombineModule::setJson(std::shared_ptr<MultiAudioCombineJson> json)
{
    m_pJson = json;
}


bool MultiAudioCombineModule::getMultiAudioCombineStatus(const NodeType& type) const
{
    auto str = EnumToInt(type) ? "output-combine" : "input-combine";
    auto status = m_pJson->getValue(str, "status");

    if (status)
        return *status;

    AUDIO_ERROR("Get {} type multi audio combine status failed.", EnumToInt(type));
    return false;
}

void MultiAudioCombineModule::setMultiAudioCombine(const NodeType& type, bool status, const std::vector<std::string>& list)
{
    if (NodeType::OUTPUT != type) {
        AUDIO_ERROR("Devices with combined {} types are not currently supported.", EnumToInt(type));
        return;
    }

    MultiAudioCombineInfo info;
    info.type = EnumToInt(type);
    info.status = status;
    m_pJson->setValue("output-combine", "status", status);

    if (status) {
        auto devList = AudioMethod::getInstance().getSinkList();
        std::vector<std::string> combineDevList {};

        for (const auto& dev : devList) {
            if (isBluetoothA2dpSink(dev.name))
                combineDevList.push_back(dev.name);
        }

        /**
         * When setting the Bluetooth combined output device, when waking up from sleep or actively
         * reconnecting Bluetooth, since Bluetooth is not sure when to connect to the Bluetooth audio,
         * a reset operation is performed here. If the setting fails to take effect after more than 3 times,
         * it means that the number of connected Bluetooth audios is less than 2 and cannot be combined output.
         */
        if (combineDevList.size() < 2) {
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                if (m_retryTimes >= 15) {
                    AUDIO_ERROR("Set the number of combinations to more than {} times, Unable to combine Bluetooth devices with device number {}, "
                                "Please check the number of Bluetooth devices connected.", m_retryTimes, combineDevList.size());
                    m_retryTimes = 0;
                    return;
                }
            }

            std::thread([type, status, list, combineDevList, this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                {
                    std::lock_guard<std::mutex> lock(m_mtx);
                    ++m_retryTimes;
                }

                AUDIO_WARN("Failed to set the combined output device. The currently connected device is less than 2. Trying to "
                           "reset the combined output device for the {} time", m_retryTimes);
                setMultiAudioCombine(type, status, list);
            }).detach();
            return;
        }

        const auto param = buildCombineModuleParam(combineDevList);
        const char* moduleName = getCombineModuleName();

        {
            std::lock_guard<std::mutex> lock(m_mtx);
            m_retryTimes = 0;
        }
        AUDIO_DEBUG("Turn on the combined device function with module {} and param {}.", moduleName, param);
        AudioMethod::getInstance().loadModule(moduleName, param);

        info.deviceList = combineDevList;
        m_pJson->insert(info);
    }
    else {
        // 卸载组合输出模块
        m_pJson->insert(info);
        AUDIO_DEBUG("Turn off the combined device function.");
        AudioMethod::getInstance().unloadModule(getCombineModuleName());
        AudioMethod::getInstance().deviceAdjust(NodeType::OUTPUT);
    }
}
}

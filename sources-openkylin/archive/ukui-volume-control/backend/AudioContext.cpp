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

#include "AudioContext.h"

namespace UkuiAudioFramwork
{
AudioContext::AudioContext()
    : m_log(getLogPath(), "audio-service.log", EnumToInt(simple_logger::OutputType::Console) | EnumToInt(simple_logger::OutputType::LogFile))
{
    // 开启日志
    std::string name = getModuleName(getModule());
    m_log.AddModule(EnumToInt(getModule()), name);
    m_log.SetLogSwitchOn(simple_logger::LogLevel::Debug);
    m_log.SetOutputTypeOn(simple_logger::OutputType::LogFile);
    m_log.SetDetailMode(true);
}

AudioContext& AudioContext::getInstance()
{
    static AudioContext context;
    return context;
}

std::string AudioContext::getLogPath()
{
    const char* homeDir = getenv("HOME");
    if (homeDir == nullptr) {
        return std::string("ukui-volume-control/log/");
    }
    return std::string(homeDir) + "/.log/ukui-volume-control/log/";
}

simple_logger::Log& AudioContext::getLogger()
{
    return m_log;
}

Module AudioContext::getModule() const
{
    return m_module;
}
}

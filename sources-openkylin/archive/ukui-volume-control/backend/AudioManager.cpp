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

#include "AudioManager.h"


namespace UkuiAudioFramwork
{
AudioManager::AudioManager()
{
    if (checkAudioService("pipewire"))
        m_type = BackendType::PIPEWIRE;
    else if (checkAudioService("pulseaudio"))
        m_type = BackendType::PULSEAUDIO;
    else
        m_type = BackendType::UNKNOWN;
}

std::string AudioManager::getBackend() const
{
    return getBackendName(m_type);
}

void AudioManager::setBackend(BackendType type)
{
    m_type = type;
}

void AudioManager::waitBluezReady(int timeout)
{
    std::unique_lock<std::mutex> lock(m_blueMtx);
    if (!m_bBlueReady)
        m_blueCond.wait_for(lock, std::chrono::milliseconds(timeout));
}

void AudioManager::setBluezReadyStatus(bool status)
{
    {
        std::unique_lock<std::mutex> lock(m_blueMtx);
        m_bBlueReady = status;
    }

    if (status)
        m_blueCond.notify_all();
}

bool AudioManager::getBluezReady() const
{
    std::lock_guard<std::mutex> lock(m_blueMtx);
    return m_bBlueReady;
}

bool AudioManager::checkAudioService(const std::string& name)
{
//    std::ifstream psOutput("/bin/ps");
    std::ostringstream cmd;
    cmd << "/bin/ps -e | grep " << name;
    std::string command = cmd.str();

    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return false;

    char buffer[128];
    std::string result = "";
    while (!feof(pipe)) {
        if (fgets(buffer, 128, pipe) != nullptr) result += buffer;
    }
    pclose(pipe);

    return !result.empty();
}
}

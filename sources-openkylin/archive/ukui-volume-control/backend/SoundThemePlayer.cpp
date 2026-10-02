/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "SoundThemePlayer.h"
#include "AudioContext.h"
#include "AudioMethod.h"  // 使用 AudioMethod 获取设置
#include "Util.h"  // 使用 Util.h 中定义的宏
#include <sys/stat.h>
#include <cstring>

namespace UkuiAudioFramwork
{

SoundThemePlayer& SoundThemePlayer::getInstance()
{
    static SoundThemePlayer instance;
    return instance;
}

SoundThemePlayer::SoundThemePlayer()
    : m_caContext(nullptr)
    , m_dBValue(nullptr)
{
    createCaContext();
    addSoundFileList();
}

bool SoundThemePlayer::playAlertSound(const std::string& soundEvent)
{
    // 检查是否启用了事件声音
    bool soundSwitch = AudioMethod::getInstance().getAlertStatus();

    if (!soundSwitch) {
        AUDIO_DEBUG("Event sounds are disabled");
        return false;
    }

    // 获取提示音音量
    int alertVolume = AudioMethod::getInstance().getAlertVolume();
    m_dBValue = alertVolumeTodB(alertVolume);

    gint retval;
    const gchar *eventId;
    const gchar *desc = "Sound Theme Player";
    std::string eventStr = soundEvent;

    // Check if it's a known sound event
    bool found = false;
    for (const auto& s : m_pSoundFileList) {
        if (s == soundEvent) {
            found = true;
            break;
        }
    }

    if (!found) {
        // Try to find in theme
        std::string theme = AudioMethod::getInstance().getSoundThemeName();
        if (theme.empty()) {
            theme = "ukui";  // Default theme
        }

        if (!isFileExist(theme, soundEvent)) {
            eventStr = NOTIFY_GENERAL_KEY;
        }
    }

    eventId = eventStr.c_str();

    // Handle custom sounds
    bool customTheme = AudioMethod::getInstance().getCustomThemeStatus();

    if (eventStr == VOLUME_CHANGED_KEY && customTheme) {
        std::string soundPath = AudioMethod::getInstance().getVolumeChangedName();
        if (!soundPath.empty()) {
            return playCustomAlertSound(soundPath);
        }
    }
    else if (eventStr == NOTIFY_GENERAL_KEY && customTheme) {
        std::string soundPath = AudioMethod::getInstance().getNotifyGeneralName();
        if (!soundPath.empty()) {
            return playCustomAlertSound(soundPath);
        }
    }

    // Play using libcanberra
    if (createCaContext()) {
        retval = ca_context_play(m_caContext, 0,
                                 CA_PROP_EVENT_ID, eventId,
                                 CA_PROP_EVENT_DESCRIPTION, desc,
                                 CA_PROP_CANBERRA_VOLUME, m_dBValue ? m_dBValue : "1.0",
                                 NULL);
        AUDIO_DEBUG("{} {} {}", __func__, soundEvent, retval);
        return (retval == 0);
    }

    return false;
}

bool SoundThemePlayer::createCaContext()
{
    if (nullptr != m_caContext) {
        ca_context_cancel(m_caContext, 0);
        ca_context_destroy(m_caContext);
    }

    return (ca_context_create(&m_caContext) == 0);
}

bool SoundThemePlayer::playCustomAlertSound(const std::string& soundPath)
{
    int retval = 1;
    if (createCaContext()) {
        retval = ca_context_play(m_caContext, 0,
                                      CA_PROP_APPLICATION_NAME, "Sound Theme Player",
                                      CA_PROP_MEDIA_FILENAME, soundPath.c_str(),
                                      CA_PROP_EVENT_DESCRIPTION, "Testing event sound",
                                      CA_PROP_CANBERRA_CACHE_CONTROL, "never",
                                      CA_PROP_APPLICATION_ID, "org.ukui.VolumeControl",
                                      CA_PROP_CANBERRA_VOLUME, m_dBValue ? m_dBValue : "1.0",
                                  #ifdef CA_PROP_CANBERRA_ENABLE
                                      CA_PROP_CANBERRA_ENABLE, "1",
                                  #endif
                                      NULL);
        AUDIO_DEBUG("{} {} {}", __func__, soundPath, retval);
    }

    return (retval == 0);
}

bool SoundThemePlayer::isFileExist(const std::string& theme, const std::string& soundType)
{
    std::string path = SOUND_FILE_PATH;
    size_t pos = path.find("xxxTheme");
    if (pos != std::string::npos) {
        path.replace(pos, 8, theme);
    }
    pos = path.find("xxxFile");
    if (pos != std::string::npos) {
        path.replace(pos, 7, soundType);
    }

    // Check .ogg file
    struct stat buffer;
    if (stat(path.c_str(), &buffer) == 0) {
        return true;
    }

    // Check .oga file
    std::string pathOga = path;
    pos = pathOga.find(".ogg");
    if (pos != std::string::npos) {
        pathOga.replace(pos, 4, ".oga");
        if (stat(pathOga.c_str(), &buffer) == 0) {
            return true;
        }
    }

    return false;
}

void SoundThemePlayer::addSoundFileList()
{
    m_pSoundFileList.push_back("audio-volume-change");
    m_pSoundFileList.push_back("battery-low");
    m_pSoundFileList.push_back("complete");
    m_pSoundFileList.push_back("device-added-failed");
    m_pSoundFileList.push_back("device-added");
    m_pSoundFileList.push_back("device-removed");
    m_pSoundFileList.push_back("dialog-error");
    m_pSoundFileList.push_back("dialog-warning");
    m_pSoundFileList.push_back("notification-general");
    m_pSoundFileList.push_back("notification-special");
    m_pSoundFileList.push_back("operation-file");
    m_pSoundFileList.push_back("operation-unsupported");
    m_pSoundFileList.push_back("power-plug");
    m_pSoundFileList.push_back("screen-capture");
    m_pSoundFileList.push_back("screen-record");
    m_pSoundFileList.push_back("start-up");
    m_pSoundFileList.push_back("trash-empty");
}

char* SoundThemePlayer::alertVolumeTodB(int value)
{
    if (value < 0 || value > 100)
        value = 100;

    pa_volume_t volume = (pa_volume_t)(value / 100.0 * 65536.0);
    double dBValue = pa_sw_volume_to_dB(volume);

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.2f", dBValue);

    if (m_dBValue) {
        delete[] m_dBValue;
        m_dBValue = nullptr;
    }

    m_dBValue = new char[sizeof(buffer)];
    snprintf(m_dBValue, sizeof(buffer), "%s", buffer);
    return m_dBValue;
}

SoundThemePlayer::~SoundThemePlayer()
{
    if (m_caContext) {
        ca_context_destroy(m_caContext);
        m_caContext = nullptr;
    }

    if (m_dBValue) {
        delete[] m_dBValue;
        m_dBValue = nullptr;
    }
}

} // namespace UkuiAudioFramwork

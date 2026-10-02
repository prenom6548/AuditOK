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
#ifndef SOUND_THEME_PLAYER_H
#define SOUND_THEME_PLAYER_H

#include <string>
#include <vector>
#include <pulse/pulseaudio.h>

extern "C" {
#include <canberra.h>
}

namespace UkuiAudioFramwork
{
#define SOUND_FILE_PATH         "/usr/share/sounds/xxxTheme/stereo/xxxFile.ogg"

class SoundThemePlayer
{
public:
    static SoundThemePlayer& getInstance();

    SoundThemePlayer(const SoundThemePlayer&) = delete;
    SoundThemePlayer(SoundThemePlayer&&) = delete;
    SoundThemePlayer& operator=(const SoundThemePlayer&) = delete;
    SoundThemePlayer& operator=(SoundThemePlayer&&) = delete;

    bool playAlertSound(const std::string& soundEvent);

private:
    SoundThemePlayer();
    ~SoundThemePlayer();

    bool createCaContext();
    bool playCustomAlertSound(const std::string& soundPath);
    bool isFileExist(const std::string& theme, const std::string& soundType);
    void addSoundFileList();
    char* alertVolumeTodB(int value);

private:
    ca_context *m_caContext = nullptr;
    std::vector<std::string> m_pSoundFileList;
    char *m_dBValue = nullptr;
};

} // namespace UkuiAudioFramwork

#endif // SOUND_THEME_PLAYER_H

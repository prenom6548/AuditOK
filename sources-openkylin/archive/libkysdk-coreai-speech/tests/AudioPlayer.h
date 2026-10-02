/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef AUDIO_PLAYER_H
#define AUDIO_PLAYER_H

#include <pulse/error.h>
#include <pulse/simple.h>

#include <vector>

#include "logger.h"

class AudioPlayer {
public:
    AudioPlayer();

    ~AudioPlayer();

    void playing(const void* data, size_t size);

    void playing(const std::vector<uint8_t>& audioData);

private:
    bool open();

    void close();

private:
    pa_simple* simple_{nullptr};
    std::vector<uint8_t> audioDataCache_;
    std::mutex mutex_;
};

#endif  // AUDIO_PLAYER_H

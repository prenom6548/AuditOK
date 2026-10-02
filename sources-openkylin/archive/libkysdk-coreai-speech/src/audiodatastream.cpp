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

#include "audiodatastream.h"

#include <cstring>

#include "_audiodatastream.h"

AudioDataStream *audio_data_stream_create() {
    auto *ptr = new _AudioDataStream();
    ptr->audio_data_to_stream_write_func = nullptr;
    ptr->p_user_data = nullptr;
    return ptr;
}

void audio_data_stream_destroy(AudioDataStream **stream) {
    if (stream == nullptr) {
        return;
    }
    delete *stream;
    *stream = nullptr;
}

int audio_data_stream_write(AudioDataStream *stream, const uint8_t *data, uint32_t data_length) {
    if (stream == nullptr) {
        return -1;
    }
    if (data == nullptr) {
        return -1;
    }
    if (data_length == 0) {
        return -1;
    }
    if (stream->audio_data_to_stream_write_func == nullptr) {
        return -1;
    }
    return stream->audio_data_to_stream_write_func(data, data_length, stream->p_user_data);
}
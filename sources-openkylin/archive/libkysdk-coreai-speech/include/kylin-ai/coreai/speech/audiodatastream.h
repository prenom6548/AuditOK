/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef __KYLIN_AI_AUDIO_DATA_STREAM_H__
#define __KYLIN_AI_AUDIO_DATA_STREAM_H__

/**
 * @file audiodatastream.h
 * @brief Audio data stream for streaming audio input.
 *
 * This header provides functions to create and manage audio data streams for feeding audio data to the speech
 * recognition engine in a streaming manner.
 */

#include <common/macros.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing an audio data stream.
 *
 * AudioDataStream is used to feed audio data to the speech recognition engine in a streaming fashion. Data can be
 * written to the stream incrementally as it becomes available.
 */
typedef struct _AudioDataStream AudioDataStream;

/**
 * @brief Create a new audio data stream.
 *
 * Creates an empty audio data stream that can receive audio data through audio_data_stream_write().
 *
 * @return Pointer to the created AudioDataStream, or NULL on failure.
 *         The caller is responsible for destroying the stream using audio_data_stream_destroy().
 */
AISDK_EXTERN
AudioDataStream *audio_data_stream_create();

/**
 * @brief Destroy an audio data stream and release resources.
 *
 * Frees all resources associated with the audio data stream and sets the pointer to NULL.
 *
 * @param stream Pointer to the AudioDataStream pointer to destroy.
 *               After this call, *stream will be set to NULL.
 */
AISDK_EXTERN
void audio_data_stream_destroy(AudioDataStream **stream);

/**
 * @brief Write audio data to the stream.
 *
 * Appends audio data to the stream for processing by the speech recognition engine. This function can be called
 * multiple times to feed audio data incrementally.
 *
 * @param stream Pointer to the AudioDataStream to write to.
 * @param data Pointer to the audio data buffer.
 * @param data_length Length of the audio data in bytes.
 * @return 0 on success, non-zero error code on failure.
 */
AISDK_EXTERN
int audio_data_stream_write(AudioDataStream *stream, const uint8_t *data, uint32_t data_length);

#ifdef __cplusplus
}
#endif

#endif

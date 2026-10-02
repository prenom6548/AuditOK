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

#ifndef __KYLIN_AI_AUDIO_CONFIG_H__
#define __KYLIN_AI_AUDIO_CONFIG_H__

/**
 * @file audioconfig.h
 * @brief Audio configuration for speech recognition and synthesis.
 *
 * This header provides functions to create and configure audio input/output settings for both streaming and
 * non-streaming speech processing operations.
 */

#include <common/macros.h>
#include <stdint.h>

#include "audiodatastream.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing audio configuration.
 */
typedef struct _AudioConfig AudioConfig;

/******************************************************************************
 * Streaming Interface Configuration
 *****************************************************************************/

/**
 * @brief Create audio input configuration using the default microphone.
 *
 * Creates an audio configuration that captures continuous audio input from the system's default microphone device for
 * streaming recognition.
 *
 * @return Pointer to the created AudioConfig, or NULL on failure.
 *         The caller is responsible for destroying the config using audio_config_destroy().
 */
AISDK_EXTERN
AudioConfig *audio_config_create_continuous_audio_input_from_default_microphone();

/**
 * @brief Create audio input configuration from an audio data stream.
 *
 * Creates an audio configuration that reads continuous audio input from the specified audio data stream for streaming
 * recognition.
 *
 * @param audio_data_stream Pointer to the AudioDataStream to read from.
 *                          Must not be NULL.
 * @return Pointer to the created AudioConfig, or NULL on failure.
 *         The caller is responsible for destroying the config using audio_config_destroy().
 */
AISDK_EXTERN
AudioConfig *audio_config_create_continuous_audio_input_from_audio_data_stream(AudioDataStream *audio_data_stream);

/******************************************************************************
 * Non-Streaming Interface Configuration
 *****************************************************************************/

/**
 * @brief Create audio input configuration from raw PCM data.
 *
 * Creates an audio configuration that uses a block of raw PCM audio data as input for one-shot recognition.
 *
 * @param audio_data Pointer to the raw PCM audio data buffer.
 * @param data_length Length of the audio data in bytes.
 * @return Pointer to the created AudioConfig, or NULL on failure.
 *         The caller is responsible for destroying the config using audio_config_destroy().
 */
AISDK_EXTERN
AudioConfig *audio_config_create_once_audio_input_from_pcm_data(const uint8_t *audio_data, uint32_t data_length);

/**
 * @brief Create audio input configuration from a PCM file.
 *
 * Creates an audio configuration that reads audio data from a PCM file for one-shot recognition.
 *
 * @param pcm_file Path to the PCM audio file.
 * @return Pointer to the created AudioConfig, or NULL on failure.
 *         The caller is responsible for destroying the config using audio_config_destroy().
 */
AISDK_EXTERN
AudioConfig *audio_config_create_once_audio_input_from_pcm_file(const char *pcm_file);

/**
 * @brief Create audio output configuration for raw PCM data.
 *
 * Creates an audio configuration that outputs synthesized audio as raw PCM data in memory.
 *
 * @return Pointer to the created AudioConfig, or NULL on failure.
 *         The caller is responsible for destroying the config using audio_config_destroy().
 */
AISDK_EXTERN
AudioConfig *audio_config_create_audio_output_from_pcm_data();

/**
 * @brief Create audio output configuration using the default speaker.
 *
 * Creates an audio configuration that outputs synthesized audio to the system's default speaker device.
 *
 * @return Pointer to the created AudioConfig, or NULL on failure.
 *         The caller is responsible for destroying the config using audio_config_destroy().
 */
AISDK_EXTERN
AudioConfig *audio_config_create_audio_output_from_default_speaker();

/**
 * @brief Create audio output configuration to a PCM file.
 *
 * Creates an audio configuration that writes synthesized audio to a PCM file.
 *
 * @param pcm_file Path to the output PCM file.
 * @return Pointer to the created AudioConfig, or NULL on failure.
 *         The caller is responsible for destroying the config using audio_config_destroy().
 */
AISDK_EXTERN
AudioConfig *audio_config_create_audio_output_from_pcm_file_name(const char *pcm_file);

/**
 * @brief Destroy an audio configuration and release resources.
 *
 * Frees all resources associated with the audio configuration and sets the pointer to NULL.
 *
 * @param config Pointer to the AudioConfig pointer to destroy.
 *               After this call, *config will be set to NULL.
 */
AISDK_EXTERN
void audio_config_destroy(AudioConfig **config);

/**
 * @brief Set the sample rate for input audio.
 *
 * Configures the sample rate (in Hz) for the input audio stream.
 *
 * @param config Pointer to the AudioConfig to configure.
 * @param rate Sample rate in Hz (e.g., 16000, 44100).
 */
AISDK_EXTERN
void audio_config_set_input_audio_rate(AudioConfig *config, int rate);

/**
 * @brief Set the speech speed for output audio.
 *
 * Configures the speech speed for synthesized audio output.
 *
 * @param config Pointer to the AudioConfig to configure.
 * @param speed Speech speed value. Valid range: [0-100], default is 50.
 */
AISDK_EXTERN
void audio_config_set_output_audio_speed(AudioConfig *config, int speed);

/**
 * @brief Set the volume for output audio.
 *
 * Configures the volume level for synthesized audio output.
 *
 * @param config Pointer to the AudioConfig to configure.
 * @param volume Volume level. Valid range: [0-100], default is 50.
 */
AISDK_EXTERN
void audio_config_set_output_audio_volume(AudioConfig *config, int volume);

/**
 * @brief Set the pitch for output audio.
 *
 * Configures the pitch level for synthesized audio output.
 *
 * @param config Pointer to the AudioConfig to configure.
 * @param pitch Pitch level. Valid range: [0-100], default is 50.
 */
AISDK_EXTERN
void audio_config_set_output_audio_pitch(AudioConfig *config, int pitch);

#ifdef __cplusplus
}
#endif

#endif

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

#ifndef __SPEECH_SYNTHESIZER_H__
#define __SPEECH_SYNTHESIZER_H__

/**
 * @file synthesizer.h
 * @brief Speech synthesis (text-to-speech) session management.
 *
 * This header provides functions to create, configure, and control
 * speech synthesis sessions for converting text to audio output.
 */

#include <common/macros.h>
#include <stdint.h>

#include "audioconfig.h"
#include "config.h"
#include "result.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing a speech synthesis session.
 */
typedef struct _SpeechSynthesizerSession SpeechSynthesizerSession;

/**
 * @brief Create a new speech synthesis session.
 *
 * Creates an uninitialized speech synthesis session that must be configured and initialized before use.
 *
 * @return Pointer to the created SpeechSynthesizerSession, or NULL on failure.
 *         The caller is responsible for destroying the session using speech_synthesizer_destroy_session().
 */
AISDK_EXTERN
SpeechSynthesizerSession *speech_synthesizer_create_session();

/**
 * @brief Destroy a speech synthesis session and release resources.
 *
 * Stops any ongoing synthesis and frees all resources associated with the session. Sets the pointer to NULL after
 * destruction.
 *
 * @param session Pointer to the SpeechSynthesizerSession pointer to destroy.
 *                After this call, *session will be set to NULL.
 */
AISDK_EXTERN
void speech_synthesizer_destroy_session(SpeechSynthesizerSession **session);

/**
 * @brief Initialize a speech synthesis session.
 *
 * Initializes the session with the configured settings. This must be called after setting the model config and audio
 * config, and before starting synthesis.
 *
 * @param session Pointer to the SpeechSynthesizerSession to initialize.
 * @return 0 on success, non-zero error code on failure.
 */
AISDK_EXTERN
int speech_synthesizer_init_session(SpeechSynthesizerSession *session);

/**
 * @brief Set the audio output configuration.
 *
 * Configures the audio destination for synthesized speech output.
 *
 * @param session Pointer to the SpeechSynthesizerSession.
 * @param audio_config Pointer to the AudioConfig specifying the audio output.
 */
AISDK_EXTERN
void speech_synthesizer_set_audio_config(SpeechSynthesizerSession *session, AudioConfig *audio_config);

/**
 * @brief Set the speech model configuration.
 *
 * Configures which speech model to use for synthesis.
 *
 * @param session Pointer to the SpeechSynthesizerSession.
 * @param config Pointer to the SpeechModelConfig to apply.
 */
AISDK_EXTERN
void speech_synthesizer_set_model_config(SpeechSynthesizerSession *session, SpeechModelConfig *config);

/**
 * @brief Set the callback function for synthesis results.
 *
 * Registers a callback function that will be invoked when synthesis results (audio data) are available.
 *
 * @param session Pointer to the SpeechSynthesizerSession.
 * @param callback The callback function to invoke with results.
 * @param user_data User-defined data pointer passed to the callback.
 */
AISDK_EXTERN
void speech_synthesizer_result_set_callback(SpeechSynthesizerSession *session, SpeechSynthesisResultCallback callback,
                                            void *user_data);

/**
 * @brief Synthesize text to speech asynchronously.
 *
 * Converts the specified text to speech audio. Results are delivered through the registered callback as audio data
 * becomes available.
 *
 * @param session Pointer to the SpeechSynthesizerSession.
 * @param text The text to synthesize.
 * @param text_length Length of the text in bytes.
 */
AISDK_EXTERN
void speech_synthesizer_synthesize_text_async(SpeechSynthesizerSession *session, const char *text,
                                              uint32_t text_length);

/**
 * @brief Synthesize text to speech with a request ID.
 *
 * Converts the specified text to speech audio with a user-specified request ID for tracking purposes. The request ID
 * will be included in the result callback, allowing correlation of results with specific requests.
 *
 * @param session Pointer to the SpeechSynthesizerSession.
 * @param request_id A unique identifier for this synthesis request.
 * @param text The text to synthesize.
 * @param text_length Length of the text in bytes.
 * @return 0 on success, non-zero error code on failure.
 */
AISDK_EXTERN
int speech_synthesizer_synthesize_text_async_with_request_id(SpeechSynthesizerSession *session, const char *request_id,
                                                             const char *text, uint32_t text_length);

/**
 * @brief Stop ongoing speech synthesis.
 *
 * Stops any currently active synthesis operation. Any remaining audio data may not be delivered.
 *
 * @param session Pointer to the SpeechSynthesizerSession.
 * @return 0 on success, non-zero error code on failure.
 */
AISDK_EXTERN
int speech_synthesizer_stop_speaking(SpeechSynthesizerSession *session);

/**
 * @brief Enable or disable the internal event loop.
 *
 * When enabled, the session manages its own event loop for processing synthesis events. When disabled, the caller is
 * responsible for event processing.
 *
 * @param session Pointer to the SpeechSynthesizerSession.
 * @param enable True to enable the internal event loop, false to disable.
 */
AISDK_EXTERN
void speech_synthesizer_enable_internal_event_loop(SpeechSynthesizerSession *session, bool enable);

#ifdef __cplusplus
}
#endif

#endif

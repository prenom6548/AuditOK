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

#ifndef __SPEECH_RECOGNIZER_H__
#define __SPEECH_RECOGNIZER_H__

/**
 * @file recognizer.h
 * @brief Speech recognition session management.
 *
 * This header provides functions to create, configure, and control speech recognition sessions for both continuous
 * (streaming) and one-shot recognition modes.
 */

#include "audioconfig.h"
#include "common/macros.h"
#include "config.h"
#include "result.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing a speech recognition session.
 */
typedef struct _SpeechRecognitionSession SpeechRecognitionSession;

/**
 * @brief Create a new speech recognition session.
 *
 * Creates an uninitialized speech recognition session that must be configured and initialized before use.
 *
 * @return Pointer to the created SpeechRecognitionSession, or NULL on failure.
 *         The caller is responsible for destroying the session using speech_recognizer_destroy_session().
 */
AISDK_EXTERN
SpeechRecognitionSession *speech_recognizer_create_session();

/**
 * @brief Destroy a speech recognition session and release resources.
 *
 * Stops any ongoing recognition and frees all resources associated with the session. Sets the pointer to NULL after
 * destruction.
 *
 * @param session Pointer to the SpeechRecognitionSession pointer to destroy.
 *                After this call, *session will be set to NULL.
 */
AISDK_EXTERN
void speech_recognizer_destroy_session(SpeechRecognitionSession **session);

/**
 * @brief Initialize a speech recognition session.
 *
 * Initializes the session with the configured settings. This must be called after setting the model config and audio
 * config, and before starting recognition.
 *
 * @param session Pointer to the SpeechRecognitionSession to initialize.
 * @return 0 on success, non-zero error code on failure.
 */
AISDK_EXTERN
int speech_recognizer_init_session(SpeechRecognitionSession *session);

/**
 * @brief Set the callback function for recognition results.
 *
 * Registers a callback function that will be invoked when recognition results are available. The callback receives both
 * intermediate and final recognition results.
 *
 * @param session Pointer to the SpeechRecognitionSession.
 * @param callback The callback function to invoke with results.
 * @param user_data User-defined data pointer passed to the callback.
 */
AISDK_EXTERN
void speech_recognizer_result_set_callback(SpeechRecognitionSession *session, SpeechRecognitionResultCallback callback,
                                           void *user_data);

/**
 * @brief Set the speech model configuration.
 *
 * Configures which speech model to use for recognition.
 *
 * @param session Pointer to the SpeechRecognitionSession.
 * @param config Pointer to the SpeechModelConfig to apply.
 */
AISDK_EXTERN
void speech_recognizer_set_model_config(SpeechRecognitionSession *session, SpeechModelConfig *config);

/**
 * @brief Set the audio input configuration.
 *
 * Configures the audio source for speech recognition.
 *
 * @param session Pointer to the SpeechRecognitionSession.
 * @param audio_config Pointer to the AudioConfig specifying the audio source.
 */
AISDK_EXTERN
void speech_recognizer_set_audio_config(SpeechRecognitionSession *session, AudioConfig *audio_config);

/**
 * @brief Start continuous speech recognition asynchronously.
 *
 * Begins continuous speech recognition from the configured audio source.
 * Recognition results are delivered through the registered callback.
 * Use speech_recognizer_stop_continuous_recognition_async() to stop.
 *
 * @param session Pointer to the SpeechRecognitionSession.
 */
AISDK_EXTERN
void speech_recognizer_start_continuous_recognition_async(SpeechRecognitionSession *session);

/**
 * @brief Stop continuous speech recognition asynchronously.
 *
 * Stops an ongoing continuous speech recognition operation.
 *
 * @param session Pointer to the SpeechRecognitionSession.
 */
AISDK_EXTERN
void speech_recognizer_stop_continuous_recognition_async(SpeechRecognitionSession *session);

/**
 * @brief Perform one-shot speech recognition asynchronously.
 *
 * Performs a single recognition operation on the configured audio source  (e.g., an audio file or memory buffer).
 * Results are delivered through the registered callback.
 *
 * @param session Pointer to the SpeechRecognitionSession.
 */
AISDK_EXTERN
void speech_recognizer_recognize_once_async(SpeechRecognitionSession *session);

/**
 * @brief Enable or disable the internal event loop.
 *
 * When enabled, the session manages its own event loop for processing recognition events. When disabled, the caller is
 * responsible for event processing.
 *
 * @param session Pointer to the SpeechRecognitionSession.
 * @param enable True to enable the internal event loop, false to disable.
 */
AISDK_EXTERN
void speech_recognizer_enable_internal_event_loop(SpeechRecognitionSession *session, bool enable);

/**
 * @brief Perform one-shot speech recognition with a request ID.
 *
 * Performs a single recognition operation with a user-specified request ID for tracking purposes. The request ID will
 * be included in the result callback, allowing correlation of results with specific requests.
 *
 * @param session Pointer to the SpeechRecognitionSession.
 * @param request_id A unique identifier for this recognition request.
 * @return 0 on success, non-zero error code on failure.
 */
AISDK_EXTERN
int speech_recognizer_recognize_once_async_with_request_id(SpeechRecognitionSession *session, const char *request_id);

#ifdef __cplusplus
}
#endif

#endif

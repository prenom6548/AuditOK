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

#ifndef __KYLIN_AI_SPEECH_RESULT_H__
#define __KYLIN_AI_SPEECH_RESULT_H__

/**
 * @file result.h
 * @brief Speech recognition and synthesis result structures and callbacks.
 *
 * This header defines the result structures, status codes, and callback types for speech recognition and synthesis
 * operations.
 */

#include <common/macros.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of speech operation result reasons/states.
 *
 * These values indicate the current state or outcome of a speech recognition or synthesis operation.
 */
typedef enum {
    /** @brief An error occurred during the operation. */
    SPEECH_ERROR_OCCURRED = 1,

    /** @brief Speech recognition has started. */
    SPEECH_RECOGNITION_STARTED = 2,

    /** @brief Speech recognition is in progress; intermediate result available. */
    SPEECH_RECOGNIZING = 3,

    /** @brief Final recognition result available (corrected/finalized text). */
    SPEECH_RECOGNIZED = 4,

    /** @brief Speech recognition operation has completed. */
    SPEECH_RECOGNITION_COMPLETED = 5,

    /** @brief Speech synthesis has started. */
    SPEECH_SYNTHESIS_STARTED = 6,

    /** @brief Speech synthesis is in progress; audio data available. */
    SPEECH_SYNTHESIZING = 7,

    /** @brief Speech synthesis operation has completed. */
    SPEECH_SYNTHESIS_COMPLETED = 8,
} SpeechResultReason;

/******************************************************************************
 * Speech Recognition Result
 *****************************************************************************/

/**
 * @brief Opaque structure representing a speech recognition result.
 */
typedef struct _SpeechRecognitionResult SpeechRecognitionResult;

/**
 * @brief Get the request ID associated with a recognition result.
 *
 * Returns the request ID that was specified when starting the recognition, or a system-generated ID if none was
 * provided.
 *
 * @param result Pointer to the SpeechRecognitionResult.
 * @return The request ID string, or NULL if not available.
 *         The returned string is owned by the result object.
 */
AISDK_EXTERN
const char *speech_recognition_result_get_request_id(SpeechRecognitionResult *result);

/**
 * @brief Get the reason/state of a recognition result.
 *
 * Returns the current state of the recognition operation.
 *
 * @param result Pointer to the SpeechRecognitionResult.
 * @return The SpeechResultReason indicating the result state.
 */
AISDK_EXTERN
SpeechResultReason speech_recognition_result_get_reason(SpeechRecognitionResult *result);

/**
 * @brief Get the recognized text from a recognition result.
 *
 * Returns the recognized text. For intermediate results (SPEECH_RECOGNIZING), this may change. For final results
 * (SPEECH_RECOGNIZED), this is the corrected/finalized transcription.
 *
 * @param result Pointer to the SpeechRecognitionResult.
 * @return The recognized text string, or NULL if not available.
 *         The returned string is owned by the result object.
 */
AISDK_EXTERN
const char *speech_recognition_result_get_text(SpeechRecognitionResult *result);

/**
 * @brief Get the speaker ID from a recognition result.
 *
 * Returns the identified speaker ID if speaker diarization is enabled.
 *
 * @param result Pointer to the SpeechRecognitionResult.
 * @return The speaker ID, or -1 if speaker identification is not available.
 */
AISDK_EXTERN
int speech_recognition_result_get_speaker_id(SpeechRecognitionResult *result);

/**
 * @brief Get the error code from a recognition result.
 *
 * Returns the error code if the result reason is SPEECH_ERROR_OCCURRED.
 *
 * @param result Pointer to the SpeechRecognitionResult.
 * @return The error code, or 0 if no error occurred.
 */
AISDK_EXTERN
int speech_recognition_result_get_error_code(SpeechRecognitionResult *result);

/**
 * @brief Get the error message from a recognition result.
 *
 * Returns a human-readable error message if the result reason is SPEECH_ERROR_OCCURRED.
 *
 * @param result Pointer to the SpeechRecognitionResult.
 * @return The error message string, or NULL if no error occurred.
 *         The returned string is owned by the result object.
 */
AISDK_EXTERN
const char *speech_recognition_result_get_error_message(SpeechRecognitionResult *result);

/******************************************************************************
 * Speech Synthesis Result
 *****************************************************************************/

/**
 * @brief Opaque structure representing a speech synthesis result.
 */
typedef struct _SpeechSynthesisResult SpeechSynthesisResult;

/**
 * @brief Get the request ID associated with a synthesis result.
 *
 * Returns the request ID that was specified when starting the synthesis, or a system-generated ID if none was provided.
 *
 * @param result Pointer to the SpeechSynthesisResult.
 * @return The request ID string, or NULL if not available.
 *         The returned string is owned by the result object.
 */
AISDK_EXTERN
const char *speech_synthesis_result_get_request_id(SpeechSynthesisResult *result);

/**
 * @brief Get the reason/state of a synthesis result.
 *
 * Returns the current state of the synthesis operation.
 *
 * @param result Pointer to the SpeechSynthesisResult.
 * @return The SpeechResultReason indicating the result state.
 */
AISDK_EXTERN
SpeechResultReason speech_synthesis_result_get_reason(SpeechSynthesisResult *result);

/**
 * @brief Get the audio format of the synthesized audio.
 *
 * Returns the format string describing the synthesized audio format (e.g., "pcm", "wav").
 *
 * @param result Pointer to the SpeechSynthesisResult.
 * @return The audio format string, or NULL if not available.
 *         The returned string is owned by the result object.
 */
AISDK_EXTERN
const char *speech_synthesis_result_get_audio_format(SpeechSynthesisResult *result);

/**
 * @brief Get the sample rate of the synthesized audio.
 *
 * Returns the sample rate in Hz of the synthesized audio.
 *
 * @param result Pointer to the SpeechSynthesisResult.
 * @return The sample rate in Hz (e.g., 16000, 22050).
 */
AISDK_EXTERN
int speech_synthesis_result_get_audio_rate(SpeechSynthesisResult *result);

/**
 * @brief Get the number of audio channels.
 *
 * Returns the number of channels in the synthesized audio (1 for mono, 2 for stereo).
 *
 * @param result Pointer to the SpeechSynthesisResult.
 * @return The number of audio channels.
 */
AISDK_EXTERN
int speech_synthesis_result_get_audio_channel(SpeechSynthesisResult *result);

/**
 * @brief Get the synthesized audio data.
 *
 * Returns a pointer to the synthesized audio data buffer and its length.
 * This is typically called when the result reason is SPEECH_SYNTHESIZING.
 *
 * @param result Pointer to the SpeechSynthesisResult.
 * @param data_length Pointer to receive the length of the audio data in bytes.
 * @return Pointer to the audio data buffer, or NULL if not available.
 *         The returned buffer is owned by the result object.
 */
AISDK_EXTERN
const uint8_t *speech_synthesis_result_get_data(SpeechSynthesisResult *result, uint32_t *data_length);

/**
 * @brief Get the error code from a synthesis result.
 *
 * Returns the error code if the result reason is SPEECH_ERROR_OCCURRED.
 *
 * @param result Pointer to the SpeechSynthesisResult.
 * @return The error code, or 0 if no error occurred.
 */
AISDK_EXTERN
int speech_synthesis_result_get_error_code(SpeechSynthesisResult *result);

/**
 * @brief Get the error message from a synthesis result.
 *
 * Returns a human-readable error message if the result reason is SPEECH_ERROR_OCCURRED.
 *
 * @param result Pointer to the SpeechSynthesisResult.
 * @return The error message string, or NULL if no error occurred.
 *         The returned string is owned by the result object.
 */
AISDK_EXTERN
const char *speech_synthesis_result_get_error_message(SpeechSynthesisResult *result);

/******************************************************************************
 * Callback Types
 *****************************************************************************/

/**
 * @brief Callback function type for speech recognition results.
 *
 * This callback is invoked when recognition results are available.
 * It may be called multiple times during recognition: once for each intermediate result and once for the final result.
 *
 * @param result Pointer to the SpeechRecognitionResult containing the result.
 *               The result object is only valid during the callback invocation.
 * @param user_data The user-defined data pointer passed during callback registration.
 */
typedef void (*SpeechRecognitionResultCallback)(SpeechRecognitionResult *result, void *user_data);

/**
 * @brief Callback function type for speech synthesis results.
 *
 * This callback is invoked when synthesis results are available.
 * It may be called multiple times during synthesis as audio data becomes available.
 *
 * @param result Pointer to the SpeechSynthesisResult containing the result.
 *               The result object is only valid during the callback invocation.
 * @param user_data The user-defined data pointer passed during callback registration.
 */
typedef void (*SpeechSynthesisResultCallback)(SpeechSynthesisResult *result, void *user_data);

#ifdef __cplusplus
}
#endif

#endif

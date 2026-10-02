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

#ifndef __KYLIN_AI_SPEECH_ERROR_H__
#define __KYLIN_AI_SPEECH_ERROR_H__

/**
 * @file error.h
 * @brief Speech-specific error codes.
 *
 * @deprecated This header is deprecated and will be removed in a future release.
 *             Please use <kylin-ai/common/error.h> instead.
 *
 * This header defines error codes specific to speech recognition and synthesis operations.
 */

#include <kylin-ai/common/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Speech-specific error codes.
 *
 * @deprecated This enum is deprecated and will be removed in a future release.
 *             Please use error codes from <kylin-ai/common/error.h> instead.
 *
 * These error codes are returned by speech recognition and synthesis functions to indicate specific failure conditions.
 */
typedef enum {
    /**
     * @brief Audio data size is invalid for recognition.
     *
     * The provided audio data is either too short or too long for the speech recognition engine to process.
     */
    SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID = 100,

    /**
     * @brief Text length is invalid for synthesis.
     *
     * The provided text is either empty or exceeds the maximum length supported by the speech synthesis engine.
     */
    SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID,

    /**
     * @brief Invalid parameter provided.
     *
     * One or more parameters passed to the function are invalid (e.g., NULL pointers where not allowed, out-of-range
     * values).
     */
    SPEECH_PARAM_INVALID,

    /**
     * @brief Default microphone is unavailable or invalid.
     *
     * The system's default microphone could not be accessed, possibly because no microphone is connected or permissions
     * are not granted.
     */
    SPEECH_DEFAULT_MICROPHONE_INVALID,

    /**
     * @brief An unknown error occurred.
     *
     * An unexpected error occurred during speech processing.
     */
    SPEECH_UNKNOWN_ERROR,

    /**
     * @brief The specified language is not supported.
     *
     * The requested language is not supported by the speech recognition or synthesis model.
     */
    SPEECH_UNSUPPORTED_LANGUAGE
} SpeechErrorCode;

#ifdef __cplusplus
}
#endif

#endif

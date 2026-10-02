/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef __AISDK_ERROR_H__
#define __AISDK_ERROR_H__

typedef enum {
    AISDK_NO_ERROR = 0,               // No error
    AISDK_NET_ERROR,                  // Network error (e.g., connection failure, DNS resolution)
    AISDK_AUTHENTICATION_FAILURE,     // Authentication failed (invalid/expired token or credentials)
    AISDK_RUNTIME_ERROR,              // Runtime error (uncategorized execution exception)
    AISDK_TOO_MANY_REQUESTS,          // Too many requests, rate limited
    AISDK_SERVICE_ERROR,              // Server internal error
    AISDK_SERVICE_TIMEOUT,            // Server processing timeout
    AISDK_BAD_REQUEST,                // Invalid request parameters or malformed request
    AISDK_MODEL_RUN_FAILED,           // Model inference failed
    AISDK_MODEL_RUN_TIME_OUT,         // Model inference timeout
    AISDK_MODEL_NOT_FOUND,            // Model not found or not deployed
    AISDK_INVALID_SESSION,            // Invalid or expired session
    AISDK_SSL_CERTIFICATE_ERROR = 12, // SSL certificate error (validation failed or untrusted)

    AI_COMMON_SUCCESS = AISDK_NO_ERROR,                              // Alias: generic success
    AI_COMMON_NET_ERROR = AISDK_NET_ERROR,                           // Alias: generic network error
    AI_COMMON_AUTHENTICATION_FAILURE = AISDK_AUTHENTICATION_FAILURE, // Alias: generic authentication failure
    AI_COMMON_RUNTIME_ERROR = AISDK_RUNTIME_ERROR,                   // Alias: generic runtime error
    AI_COMMON_TOO_MANY_REQUESTS = AISDK_TOO_MANY_REQUESTS,           // Alias: generic rate limiting
    AI_COMMON_SERVICE_ERROR = AISDK_SERVICE_ERROR,                   // Alias: generic service error
    AI_COMMON_SERVICE_TIMEOUT = AISDK_SERVICE_TIMEOUT,               // Alias: generic service timeout
    AI_COMMON_BAD_REQUEST = AISDK_BAD_REQUEST,                       // Alias: generic invalid request
    AI_COMMON_MODEL_RUN_FAILED = AISDK_MODEL_RUN_FAILED,             // Alias: generic model execution failure
    AI_COMMON_MODEL_RUN_TIME_OUT = AISDK_MODEL_RUN_TIME_OUT,         // Alias: generic model timeout
    AI_COMMON_MODEL_NOT_FOUND = AISDK_MODEL_NOT_FOUND,               // Alias: generic model not found
    AI_COMMON_INVALID_SESSION = AISDK_INVALID_SESSION,               // Alias: generic invalid session
    AI_COMMON_SSL_CERTIFICATE_ERROR = AISDK_SSL_CERTIFICATE_ERROR,   // Alias: generic SSL certificate error
    AI_COMMON_TASK_CANCELLED,                                        // Task cancelled

    // Text recognition
    AI_COMMON_OCR_IMAGE_ERROR = 100,  // Image data error
    AI_COMMON_OCR_PARAM_INVALID,      // Parameter invalid

    // Speech recognition and synthesis
    AI_COMMON_SPEECH_RECOGNITION_AUDIO_DATA_SIZE_INVALID = 100,  // Audio data size invalid
    AI_COMMON_SPEECH_SYNTHESIS_TEXT_LENGTH_INVALID,              // Text length invalid
    AI_COMMON_SPEECH_PARAM_INVALID,                              // Parameter invalid
    AI_COMMON_SPEECH_DEFAULT_MICROPHONE_INVALID,                 // Default microphone invalid
    AI_COMMON_SPEECH_UNKNOWN_ERROR,                              // Unknown error
    AI_COMMON_SPEECH_UNSUPPORTED_LANGUAGE,                       // Unsupported language

    // Text generation
    AI_COMMON_NLP_INPUT_INVALID = 100,  // Input text invalid
    AI_COMMON_NLP_PARAM_ERROR,          // Parameter error

    // Image generation
    AI_COMMON_VISION_INPUT_TEXT_LENGTH_INVALID = 100,  // Input text length invalid
    AI_COMMON_VISION_IMAGE_STYLE_INVALID,              // Unsupported style
    AI_COMMON_VISION_IMAGE_SIZE_INVALID,               // Unsupported image size
    AI_COMMON_VISION_IMAGE_NUMBER_INVALID,             // Unsupported image number
    AI_COMMON_VISION_IMAGE_GENERATION_BLOCKED,         // Image generation blocked
    AI_COMMON_VISION_IMAGE_GENERATION_FAILED,          // Image generation failed

    // Embedding
    AI_COMMON_EMBEDDING_SUCCESS = 0,            // Embedding success
    AI_COMMON_EMBEDDING_INPUT_ERROR,            // Input parameter error
    AI_COMMON_EMBEDDING_INIT_ERROR,             // Init error
    AI_COMMON_EMBEDDING_CONNECTION_ERROR,       // Connect runtime error
    AI_COMMON_EMBEDDING_RUNTIME_ERROR,          // Runtime error
    AI_COMMON_EMBEDDING_ERROR_UNKNOWN = 99,     // Embedding unknown error

    // Network-related error codes: 200 - 299
    AI_COMMON_NETWORK_TIMEOUT = 200,      // Network timeout
    AI_COMMON_NETWORK_CONNECTION_FAILED,  // Network connection failed

    // Cloud service error codes: 300 - 399

    // General error codes: 400 - 499
    AI_COMMON_INVALID_PARAMETER = 400,          // Invalid parameter
    AI_COMMON_PERMISSION_DENIED,                // Permission denied
    AI_COMMON_FILE_NOT_FOUND,                   // File not found
    AI_COMMON_RUNTIME_OUTDATED,                 // Runtime outdated
    AI_COMMON_MODEL_UNSUPPORTED_TOOL_CHOICE,    // Unsupported tool choice
    AI_COMMON_FREE_ALLOCATED_QUOTA_EXCEEDED,    // Free allocated quota exceeded

    // Service-related error codes: 500 - 599
    AI_COMMON_SERVER_CONNECTION_FAILED = 500,  // Service connection failed
    AI_COMMON_SERVER_INITIALIZATION_FAILED,    // Init error

    // Model-related error codes: 600 - 699

    // Reserved for future expansion
    // New error codes start from 10000. Each capability occupies a numerical range of 1000 error codes, where:

    // Text recognition and image processing error codes: 10000 - 10999
    AI_COMMON_IMAGE_DATA_INVALID = 10000,   // Image data invalid
    AI_COMMON_IMAGE_COORD_OUT_OF_BOUNDS,    // Coord out of bounds
    AI_COMMON_IMAGE_DECODE_WITHOUT_ENCODE,  // Decode without encode

    // Speech recognition and synthesis error codes: 11000 - 11999
    AI_COMMON_SPEECH_DEFAULT_MICROPHONE_MUTE = 11000,  // Default microphone mute

    // Text generation error codes: 12000 - 12999
    AI_COMMON_NLP_TOKEN_LIMITED = 12000,     // Token limited

    // Image generation error codes: 13000 - 13999
    AI_COMMON_VISION_IMAGE_GENERATION_INSUFFICIENT_LICENSE = 13000,  // Insufficient license

    // Vectorization error codes: 14000 - 14999

    // Knowledge Base error codes: 15000 - 15999

    AI_COMMON_UNKNOWN_ERROR = 1000000 // Unknown error (fallback code)
} AiSdkCommonErrorCode;

#endif

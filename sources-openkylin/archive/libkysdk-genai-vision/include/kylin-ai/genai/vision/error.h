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

#ifndef __KYLIN_AI_GEN_AI_VISION_ERROR_H__
#define __KYLIN_AI_GEN_AI_VISION_ERROR_H__

#include <kylin-ai/common/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes for GenAI Vision.
 * @deprecated These error codes will be deprecated in the future.
 *             Please use error codes defined in <kylin-ai/common/error.h> instead.
 */
typedef enum {
    VISION_INPUT_TEXT_LENGTH_INVALID = 100, /**< Input text length is invalid (too long) */
    VISION_IMAGE_STYLE_INVALID,             /**< Unsupported image style */
    VISION_IMAGE_SIZE_INVALID,              /**< Unsupported image size */
    VISION_IMAGE_NUMBER_INVALID,            /**< Unsupported number of images */
    VISION_IMAGE_GENERATION_BLOCKED,        /**< Image generation blocked (e.g., safety filter) */
    VISION_IMAGE_GENERATION_FAILED,         /**< Image generation failed */
} GenAiVisionErrorCode;

#ifdef __cplusplus
}
#endif

#endif

/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

/**
 * @file error.h
 * @brief Error code definitions for the Vision SDK.
 *
 * This header provides error codes for vision-related operations such as
 * text recognition and image segmentation. The common error codes from
 * kylin-ai/common/error.h should be used for general errors.
 *
 * @note The OcrErrorCode enumeration is deprecated and will be removed in a
 * future release. Please use the error codes defined in kylin-ai/common/error.h
 * instead.
 *
 * @see kylin-ai/common/error.h for the recommended error codes.
 */

#ifndef __KYLIN_AI_CORE_AI_VISION_ERROR_H__
#define __KYLIN_AI_CORE_AI_VISION_ERROR_H__

#include <kylin-ai/common/error.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief OCR specific error codes.
 *
 * @deprecated This enumeration is deprecated and will be removed in a future
 * release. Please use the error codes defined in kylin-ai/common/error.h
 * instead:
 * - Use AI_COMMON_OCR_IMAGE_ERROR instead of OCR_IMAGE_ERROR
 * - Use AI_COMMON_OCR_PARAM_INVALID instead of OCR_PARAM_INVALID
 */
typedef enum {
    OCR_IMAGE_ERROR = 100, /**< @deprecated Image processing error. Use AI_COMMON_OCR_IMAGE_ERROR instead. */
    OCR_PARAM_INVALID,     /**< @deprecated Invalid parameter provided. Use AI_COMMON_OCR_PARAM_INVALID instead. */
} OcrErrorCode;

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_CORE_AI_VISION_ERROR_H__
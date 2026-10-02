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
 * @file textrecognitionresult.h
 * @brief Result handling interface for text recognition (OCR).
 *
 * This header defines the data structures and accessor functions for retrieving
 * text recognition results. Results include:
 * - Overall recognized text content
 * - Individual text lines with bounding box coordinates
 * - Request ID for tracing
 * - Error information when operations fail
 *
 * @note Result objects are owned by the SDK and are only valid during the
 * callback invocation. Do not store pointers or attempt to free them manually.
 *
 * @see textrecognition.h for the main text recognition API.
 * @see types.h for PixelPoint structure definition.
 */

#ifndef __KYLIN_AI_CORE_AI_VISION_TEXT_RECOGNITION_RESULT_H__
#define __KYLIN_AI_CORE_AI_VISION_TEXT_RECOGNITION_RESULT_H__

#include <kylin-ai/common/macros.h>

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for text recognition result.
 */
typedef struct _TextRecognitionResult TextRecognitionResult;

/**
 * @brief Structure representing a line of text in an image.
 */
typedef struct _TextLine TextLine;

/**
 * @brief Get the text content of a text line.
 *
 * @param text_line [in] Handle to the text line.
 * @return Pointer to the null-terminated text string, or NULL if unavailable.
 *
 * @note The returned pointer is managed by the text line object. Do not free it
 * manually.
 */
AISDK_EXTERN
const char *text_line_get_value(TextLine *text_line);

/**
 * @brief Get the corner points of a text line.
 *
 * @param text_line    [in] Handle to the text line.
 * @param point_number [out] Pointer to store the number of corner points.
 * @return Pointer to the array of corner points, or NULL if unavailable.
 *
 * @note The returned pointer is managed by the text line object. Do not free it
 * manually. For most cases, point_number will be 4 (four corners of the text
 * box).
 */
AISDK_EXTERN
PixelPoint *text_line_get_corner_points(TextLine *text_line, int *point_number);

/**
 * @brief Get the request id of the text recognition result.
 *
 * @param result [in] Handle to the text recognition result.
 * @return Pointer to the request id string, or NULL if unavailable.
 *
 * @note The returned pointer is managed by the result object. Do not free it
 * manually.
 */
AISDK_EXTERN
const char *text_recognition_result_get_request_id(TextRecognitionResult *result);

/**
 * @brief Get the overall text content from the recognition result.
 *
 * @param result [in] Handle to the text recognition result.
 * @return Pointer to the null-terminated text string, or NULL if unavailable.
 *
 * @note The returned pointer is managed by the result object. Do not free it
 * manually. This function returns all recognized text without formatting.
 */
AISDK_EXTERN
const char *text_recognition_result_get_value(TextRecognitionResult *result);

/**
 * @brief Get all text lines from the recognition result.
 *
 * @param result    [in] Handle to the text recognition result.
 * @param line_count [out] Pointer to store the number of text lines.
 * @return Pointer to the array of TextLine pointers, or NULL if unavailable.
 *
 * @note The returned pointer is managed by the result object. Do not free it
 * manually.
 */
AISDK_EXTERN
TextLine **text_recognition_result_get_text_lines(TextRecognitionResult *result, int *line_count);

/**
 * @brief Get the error code of the text recognition result.
 *
 * @param result [in] Handle to the text recognition result.
 * @return Error code. 0 means no error, non-zero means failure.
 */
AISDK_EXTERN
int text_recognition_result_get_error_code(TextRecognitionResult *result);

/**
 * @brief Get the error message of the text recognition result.
 *
 * @param result [in] Handle to the text recognition result.
 * @return Pointer to the error message string, or NULL if no error.
 *
 * @note The returned pointer is managed by the result object. Do not free it
 * manually.
 */
AISDK_EXTERN
const char *text_recognition_result_get_error_message(TextRecognitionResult *result);

#ifdef __cplusplus
}
#endif

#endif

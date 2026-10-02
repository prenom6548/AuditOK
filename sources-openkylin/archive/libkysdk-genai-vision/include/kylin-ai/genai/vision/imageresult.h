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

#ifndef __KYLIN_AI_GEN_AI_VISION_IMAGE_RESULT_H__
#define __KYLIN_AI_GEN_AI_VISION_IMAGE_RESULT_H__

#include <kylin-ai/common/macros.h>
#include <stdint.h>

#include "imageconfig.h"
#include "imageformat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _VisionImageResult VisionImageResult;

/**
 * @brief Retrieves the format of the generated image.
 *
 * @param image_result The image result object.
 * @return The format of the image (e.g., JPEG, PNG).
 */
AISDK_EXTERN
ImageFormat image_result_get_format(VisionImageResult *image_result);

/**
 * @brief Retrieves the size dimensions of the generated image.
 *
 * @param image_result The image result object.
 * @return The size of the image (width and height).
 */
AISDK_EXTERN
ImageSize image_result_get_size(VisionImageResult *image_result);

/**
 * @brief Retrieves the total number of images generated in the request.
 *
 * @param image_result The image result object.
 * @return The total number of images.
 */
AISDK_EXTERN
int image_result_get_total(VisionImageResult *image_result);

/**
 * @brief Retrieves the index of the current image in the generated sequence.
 *
 * @param image_result The image result object.
 * @return The zero-based index of the image.
 */
AISDK_EXTERN
int image_result_get_index(VisionImageResult *image_result);

/**
 * @brief Retrieves the raw data of the generated image.
 *
 * @param image_result The image result object.
 * @param data_length A pointer to an integer where the length of the data in bytes will be stored.
 * @return A pointer to the raw image data.
 */
AISDK_EXTERN
const uint8_t *image_result_get_data(VisionImageResult *image_result, int *data_length);
/**
 * @brief Retrieves the error code associated with the image generation result, if any.
 *
 * @param image_result The image result object.
 * @return The error code (0 typically indicates success).
 */
AISDK_EXTERN
int image_result_get_error_code(VisionImageResult *image_result);

/**
 * @brief Retrieves the error message associated with the image generation result, if any.
 *
 * @param image_result The image result object.
 * @return A string containing the error message.
 */
AISDK_EXTERN
const char *image_result_get_error_message(VisionImageResult *image_result);

/**
 * @brief Retrieves the request ID associated with the image generation result.
 *
 * @param image_result The image result object.
 * @return A string containing the request ID.
 */
AISDK_EXTERN
const char *image_result_get_request_id(VisionImageResult *image_result);

/**
 * @brief Callback function type for receiving image generation results.
 *
 * @param result The image result object containing data and metadata.
 * @param user_data User-defined data passed to the callback.
 */
typedef void (*VisionImageResultCallback)(VisionImageResult *result, void *user_data);

#ifdef __cplusplus
}
#endif

#endif

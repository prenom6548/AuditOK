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

#ifndef __KYLIN_AI_GEN_AI_VISION_IMAGE_CONFIG_H__
#define __KYLIN_AI_GEN_AI_VISION_IMAGE_CONFIG_H__

#include <kylin-ai/common/macros.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Structure representing the dimensions of an image.
 */
typedef struct _ImageSize {
    int width;  /**< Width of the image in pixels */
    int height; /**< Height of the image in pixels */
} ImageSize;

typedef struct _ImageConfig ImageConfig;

/**
 * @brief Creates a new image configuration object.
 *
 * @return A pointer to the newly created ImageConfig object, or NULL if creation failed.
 */
AISDK_EXTERN
ImageConfig *image_config_create();

/**
 * @brief Destroys an image configuration object and frees associated resources.
 *
 * @param config A double pointer to the configuration object to be destroyed.
 * The pointer will be set to NULL after destruction.
 */
AISDK_EXTERN
void image_config_destroy(ImageConfig **config);

/**
 * @brief Sets the number of images to generate.
 *
 * @param config The configuration object.
 * @param number The number of images to generate.
 */
AISDK_EXTERN
void image_config_set_generation_number(ImageConfig *config, int number);

/**
 * @brief Sets the style of the generated image.
 *
 * @param config The configuration object.
 * @param style The name of the style to use (e.g., "realistic", "anime").
 */
AISDK_EXTERN
void image_config_set_style(ImageConfig *config, const char *style);

/**
 * @brief Sets the size (dimensions) of the generated image.
 *
 * @param config The configuration object.
 * @param image_size The desired width and height of the image.
 */
AISDK_EXTERN
void image_config_set_size(ImageConfig *config, ImageSize image_size);

/**
 * @brief Choose the serving model by model name.
 * @param config Configuration object.
 * @param model_name UTF - 8 model identifier, or NULL to clear it.
 * @return Error code value, 0 indicates success.
 */
AISDK_EXTERN int image_config_set_model_name(ImageConfig *config, const char *model_name);

#ifdef __cplusplus
}
#endif

#endif

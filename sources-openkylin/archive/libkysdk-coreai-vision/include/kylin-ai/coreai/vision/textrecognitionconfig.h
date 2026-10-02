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
 * @file textrecognitionconfig.h
 * @brief Configuration interface for text recognition (OCR).
 *
 * This header provides the configuration types and functions for text
 * recognition operations. It allows users to configure model parameters
 * such as the model name and deployment type.
 *
 * @see textrecognition.h for the main text recognition API.
 */

#ifndef __KYLIN_AI_CORE_AI_VISION_TEXT_RECOGNITION_CONFIG_H__
#define __KYLIN_AI_CORE_AI_VISION_TEXT_RECOGNITION_CONFIG_H__

#include <kylin-ai/common/enums.h>
#include <kylin-ai/common/macros.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle for text recognition model configuration.
 *
 * This structure holds configuration parameters for text recognition models.
 * It must be created via text_recognition_model_config_create() and destroyed
 * via text_recognition_model_config_destroy().
 */
typedef struct _TextRecognitionModelConfig TextRecognitionModelConfig;

/**
 * @brief Create a new text recognition model configuration instance.
 *
 * Allocates and initializes a default configuration object.
 *
 * @return Pointer to a new TextRecognitionModelConfig instance, or NULL on
 * failure.
 *
 * @code
 * TextRecognitionModelConfig *config = text_recognition_model_config_create();
 * if (!config) {
 *     // Handle allocation failure
 * }
 * @endcode
 */
AISDK_EXTERN
TextRecognitionModelConfig *text_recognition_model_config_create();

/**
 * @brief Destroy a text recognition model configuration instance.
 *
 * Frees all resources associated with the configuration object and sets the
 * pointer to NULL.
 *
 * @param config [in,out] Address of the configuration handle pointer. After
 * destruction, the pointed-to pointer is set to NULL.
 *
 * @note Passing NULL or a pointer to NULL is safe and has no effect.
 *
 * @code
 * text_recognition_model_config_destroy(&config);
 * // config is now NULL
 * @endcode
 */
AISDK_EXTERN
void text_recognition_model_config_destroy(TextRecognitionModelConfig **config);

/**
 * @brief Set the model name in the configuration.
 *
 * @param config [in] The configuration handle. Must not be NULL.
 * @param name   [in] The model name string.
 *
 * @note The string is copied internally, so the caller can free the input
 * after calling this function.
 */
AISDK_EXTERN
void text_recognition_model_config_set_name(TextRecognitionModelConfig *config, const char *name);

/**
 * @brief Set the model deployment type in the configuration.
 *
 * @param config [in] The configuration handle. Must not be NULL.
 * @param type   [in] The model deployment type.
 */
AISDK_EXTERN
void text_recognition_model_config_set_deploy_type(TextRecognitionModelConfig *config, ModelDeployType type);

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_CORE_AI_VISION_TEXT_RECOGNITION_CONFIG_H__

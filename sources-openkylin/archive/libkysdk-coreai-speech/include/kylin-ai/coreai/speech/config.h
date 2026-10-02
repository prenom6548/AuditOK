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

#ifndef _KYLIN_AI_CORE_AI_SPEECH_CONFIG_H
#define _KYLIN_AI_CORE_AI_SPEECH_CONFIG_H

/**
 * @file config.h
 * @brief Speech model configuration.
 *
 * This header provides functions to create and configure speech model settings for speech recognition and synthesis
 * operations.
 */

#include "kylin-ai/common/enums.h"
#include "kylin-ai/common/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque structure representing speech model configuration.
 */
typedef struct _SpeechModelConfig SpeechModelConfig;

/**
 * @brief Create a new speech model configuration.
 *
 * Creates an empty speech model configuration that can be customized using the setter functions.
 *
 * @return Pointer to the created SpeechModelConfig, or NULL on failure.
 *         The caller is responsible for destroying the config using speech_model_config_destroy().
 */
AISDK_EXTERN
SpeechModelConfig *speech_model_config_create();

/**
 * @brief Destroy a speech model configuration and release resources.
 *
 * Frees all resources associated with the speech model configuration and sets the pointer to NULL.
 *
 * @param config Pointer to the SpeechModelConfig pointer to destroy.
 *               After this call, *config will be set to NULL.
 */
AISDK_EXTERN
void speech_model_config_destroy(SpeechModelConfig **config);

/**
 * @brief Set the model name.
 *
 * Specifies the name of the speech model to use for recognition or synthesis.
 *
 * @param config Pointer to the SpeechModelConfig to configure.
 * @param name The model name string.
 */
AISDK_EXTERN
void speech_model_config_set_name(SpeechModelConfig *config, const char *name);

/**
 * @brief Set the model deployment type.
 *
 * Specifies whether to use a local or cloud-based model deployment.
 *
 * @param config Pointer to the SpeechModelConfig to configure.
 * @param type The deployment type (local or cloud).
 */
AISDK_EXTERN
void speech_model_config_set_deploy_type(SpeechModelConfig *config, ModelDeployType type);

#ifdef __cplusplus
}
#endif

#endif  //_KYLIN_AI_CORE_AI_SPEECH_CONFIG_H

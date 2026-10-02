/*
 * Copyright 2025 KylinSoft Co., Ltd.
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
#ifndef __KYLIN_AI_GEN_AI_TEXT_MODEL_INFO_H__
#define __KYLIN_AI_GEN_AI_TEXT_MODEL_INFO_H__

#include <common/enums.h>
#include <common/macros.h>

/**
 * @file modelinfo.h
 * @brief Inspect the list of text models and their exposed capabilities.
 *
 * Example:
 * @code
 * const TextModelList *list = genai_text_get_model_list(session, NULL);
 * int count = genai_text_model_list_get_count(list);
 * for (int i = 0; i < count; ++i) {
 *     const TextModelInfo *info = genai_text_model_list_get_model(list, i, NULL);
 *     printf("model: %s (%s)\\n",
 *            genai_text_model_info_get_model_name(info, NULL),
 *            genai_text_model_info_get_display_name(info, NULL));
 * }
 * @endcode
 *
 * The SDK owns both @ref TextModelList and @ref TextModelInfo. Their storage is shared across sessions and remains
 * valid for the lifetime of the process, so applications must treat the structures as read-only and never attempt to
 * free them.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declarations owned entirely by the SDK.
 */
typedef struct _TextModelInfo TextModelInfo;
typedef struct _TextModelList TextModelList;

/**
 * @brief Return the number of entries inside the list.
 * @param[in] model_list List pointer.
 * @return Count of models, or 0 if the list is NULL.
 */
AISDK_EXTERN
int genai_text_model_list_get_count(const TextModelList *model_list);

/**
 * @brief Retrieve the model info located at the given index.
 * @param[in]  model_list List pointer.
 * @param[in]  index      Zero-based index.
 * @param[out] error_code Optional output parameter receiving 0 on success.
 * @return Pointer to the model info, or NULL when invalid.
 */
AISDK_EXTERN
const TextModelInfo *genai_text_model_list_get_model_info(const TextModelList *model_list, int index, int *error_code);

/**
 * @brief Obtain the canonical model identifier.
 * @param[in]  model_info Model info pointer.
 * @param[out] error_code Optional pointer for additional error diagnostics.
 * @return UTF-8 model ID or NULL when unavailable.
 */
AISDK_EXTERN
const char *genai_text_model_info_get_model_name(const TextModelInfo *model_info, int *error_code);

/**
 * @brief Retrieve a human-readable display name.
 * @param[in]  model_info Model info pointer.
 * @param[out] error_code Optional pointer.
 * @return Display name string or NULL.
 */
AISDK_EXTERN
const char *genai_text_model_info_get_display_name(const TextModelInfo *model_info, int *error_code);

/**
 * @brief Determine how the model is deployed.
 * @param[in]  model_info Model info pointer.
 * @param[out] error_code Optional pointer.
 * @return Deployment type, defaulting to MODEL_DEPLOY_DEFAULT.
 */
AISDK_EXTERN
ModelDeployType genai_text_model_info_get_deploy_type(const TextModelInfo *model_info, int *error_code);

/**
 * @brief Check whether the model can access web-search capabilities.
 * @param[in]  model_info Model info pointer.
 * @param[out] error_code Optional pointer.
 * @return true if supported; false otherwise.
 */
AISDK_EXTERN
bool genai_text_model_info_supports_web_search(const TextModelInfo *model_info, int *error_code);

/**
 * @brief Check whether the model exposes a reasoning / thinking trace.
 * @param[in]  model_info Model info pointer.
 * @param[out] error_code Optional pointer.
 * @return true if supported; false otherwise.
 */
AISDK_EXTERN
bool genai_text_model_info_supports_thinking(const TextModelInfo *model_info, int *error_code);

/**
 * @brief Check whether the reasoning mode can be toggled per request.
 * @param[in]  model_info Model info pointer.
 * @param[out] error_code Optional pointer.
 * @return true if supported; false otherwise.
 */
AISDK_EXTERN
bool genai_text_model_info_supports_thinking_toggle(const TextModelInfo *model_info, int *error_code);

/**
 * @brief Retrieve the brand information of the model.
 * @param[in]  model_info Model info pointer.
 * @param[out] error_code Optional pointer.
 * @return brand string or NULL when unavailable.
 */
AISDK_EXTERN
const char *genai_text_model_info_get_brand(const TextModelInfo *model_info, int *error_code);

/**
 * @brief Check whether the model supports tool selection functionality.
 * @param[in]  model_info Pointer to the model information.
 * @param[out] error_code Optional output parameter for error diagnostics; 0 on success.
 * @return true if supported, false otherwise.
 */
AISDK_EXTERN
bool genai_text_model_info_supports_tool_choice(const TextModelInfo *model_info, int *error_code);

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_GEN_AI_TEXT_MODEL_INFO_H__

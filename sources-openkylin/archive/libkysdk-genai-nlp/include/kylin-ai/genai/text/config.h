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

#ifndef __KYLIN_AI_GEN_AI_TEXT_CONFIG_H__
#define __KYLIN_AI_GEN_AI_TEXT_CONFIG_H__

#include <common/enums.h>
#include <common/macros.h>

/**
 * @file config.h
 * @brief APIs for configuring the text model and sampling strategy.
 *
 * Example:
 * @code
 * ChatModelConfig *config = chat_model_config_create();
 * chat_model_config_set_name(config, "kylin-plus");
 * chat_model_config_set_top_k(config, 4);
 * genai_text_set_model_config(session, config);
 * chat_model_config_destroy(&config);
 * @endcode
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Forward declaration of the configuration object.
 */
typedef struct _ChatModelConfig ChatModelConfig;

/**
 * @brief Create a configuration object with default values.
 *
 * @return A new instance that must be destroyed via @ref chat_model_config_destroy.
 */
AISDK_EXTERN
ChatModelConfig *chat_model_config_create();

/**
 * @brief Destroy a configuration object.
 *
 * @param config Address of the pointer. The pointer is nulled after destruction.
 */
AISDK_EXTERN
void chat_model_config_destroy(ChatModelConfig **config);

/**
 * @brief Choose the serving model by name.
 *
 * Names must match those returned by @ref genai_text_get_model_list.
 *
 * @param config Configuration object.
 * @param name   UTF-8 model identifier, or NULL to clear it.
 */
AISDK_EXTERN
void chat_model_config_set_name(ChatModelConfig *config, const char *name);

/**
 * @brief Configure Top-K sampling.
 *
 * Larger values increase diversity; smaller values improve determinism.
 * Non-positive inputs reset to server defaults.
 *
 * @param config Configuration object.
 * @param top_k  Maximum number of candidates to consider.
 */
AISDK_EXTERN
void chat_model_config_set_top_k(ChatModelConfig *config, int top_k);

/**
 * @brief Specify where the model is hosted.
 *
 * Use this to distinguish local vs. cloud deployments.
 *
 * @param config Configuration object.
 * @param type   See @ref ModelDeployType.
 */
AISDK_EXTERN
void chat_model_config_set_deploy_type(ChatModelConfig *config, ModelDeployType type);

/**
 * @brief Configure Temperature.
 *
 * @param config       Configuration object.
 * @param temperature  Possible values: >= 0 and <= 2   Default value: 1
 *      What sampling temperature to use, between 0 and 2.
 *      Higher values like 0.8 will make the output more random, while lower values like 0.2 will make it more focused
 *      and deterministic.
 *      We generally recommend altering this or top_p but not both.
 *      If input other value, clear temperature.
 * @return Error code value, 0 indicates success.
 */
AISDK_EXTERN
int chat_model_config_set_temperature(ChatModelConfig *config, double temperature);

/**
 * @brief Configure Top-P sampling.
 *
 * @param config Configuration object.
 * @param top_p  Possible values: >= 0 and <= 1  Default value: 1
 *      An alternative to sampling with temperature, called nucleus sampling, where the model considers the results of
 *      the tokens with top_p probability mass.
 *      So 0.1 means only the tokens comprising the top 10% probability mass are considered. We generally recommend
 *      altering this or temperature but not both.
 *      If input other value, clear top_p.
 * @return Error code value, 0 indicates success.
 */
AISDK_EXTERN
int chat_model_config_set_top_p(ChatModelConfig *config, double top_p);

/**
 * @brief Configure Max-Completion-Tokens.
 *
 * @param config     Configuration object.
 * @param max_tokens The maximum number of tokens that can be generated in the chat completion.
 *      The total length of input tokens and generated tokens is limited by the model's context length. *      For the
 * value range and default value, please refer to the documentation. If input <=0, clear max_completion_tokens.
 * @return Error code value, 0 indicates success.
 */
AISDK_EXTERN
int chat_model_config_set_max_completion_tokens(ChatModelConfig *config, int max_tokens);

/**
 * @brief Configure Presence-Penalty.
 *
 * @param config            Configuration object.
 * @param presence_penalty  Possible values: >= -2 and <= 2  Default value: 0
 *      Number between -2.0 and 2.0. Positive values penalize new tokens based on whether they appear in the text so
 *      far, increasing the model's likelihood to talk about new topics.
 *      If input other value, clear presence_penalty.
 * @return Error code value, 0 indicates success.
 */
AISDK_EXTERN
int chat_model_config_set_presence_penalty(ChatModelConfig *config, double presence_penalty);

/**
 * @brief Enable or disable streaming responses.
 *
 * @param config Configuration object.
 * @param stream true for streaming, false for buffered responses.
 * @return 0 on success; otherwise an error code.
 */
AISDK_EXTERN
void chat_model_config_set_stream(ChatModelConfig *config, bool stream);

/**
 * @brief Enable or disable the web search feature for the chat model.
 *
 * @param config Configuration object.
 * @param enabled Set to true to enable web search, false to disable it.
 * @param error_code Optional output parameter receiving 0 on success.
 */
AISDK_EXTERN
void chat_model_config_set_web_search_enabled(ChatModelConfig *config, bool enabled, int *error_code);

/**
 * @brief Enable or disable the web search feature for the chat model.
 *
 * @param config Configuration object.
 * @param enabled Set to true to enable deep thinking, false to disable it.
 * @param error_code Optional output parameter receiving 0 on success.
 */
AISDK_EXTERN
void chat_model_config_set_thinking_enabled(ChatModelConfig *config, bool enabled, int *error_code);

/**
 * @brief Configure `n`.
 *
 * @param config Configuration object.
 * @param n      Possible values: >= 1 and <= 4  Default value: 1
 *               Number of candidates generated at once.
 * @return Error code value, 0 indicates success.
 */
AISDK_EXTERN int chat_model_config_set_n(ChatModelConfig *config, int n);

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_GEN_AI_TEXT_CONFIG_H__

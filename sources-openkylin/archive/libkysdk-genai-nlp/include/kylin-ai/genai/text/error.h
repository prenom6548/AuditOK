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

#ifndef __KYLIN_AI_GEN_AI_TEXT_ERROR_H__
#define __KYLIN_AI_GEN_AI_TEXT_ERROR_H__

#include <kylin-ai/common/error.h>

/**
 * @file error.h
 * @brief Error codes specific to the text generation module.
 *
 * These values are surfaced through @ref chat_result_get_error_code and may be used alongside the common @ref
 * AisdkErrorCode set.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Text-generation specific error codes.
 */
typedef enum {
    /**< The input payload is empty or invalid. */
    NLP_INPUT_INVALID = 100,
    /**< Configuration error, e.g. missing mandatory model info. */
    NLP_PARAM_ERROR,
} GenAiTextErrorCode;

#ifdef __cplusplus
}
#endif

#endif  // __KYLIN_AI_GEN_AI_TEXT_ERROR_H__

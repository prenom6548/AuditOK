/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#ifndef EMBEDDINGERROR_H
#define EMBEDDINGERROR_H

#include <common/error.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef enum : int {
    /*< @deprecated 请使用
       COREAI_EMBEDDING_SUCCESS。此拼写错误将在未来版本移除。 */
    COREAI_EMBEDDING_SUCESS = 0,
    /*< @brief 成功 */
    COREAI_EMBEDDING_SUCCESS = 0,
    COREAI_EMBEDDING_INPUT_ERROR,
    COREAI_EMBEDDING_INIT_ERROR,
    COREAI_EMBEDDING_CONNECTION_ERROR,
    COREAI_EMBEDDING_RUNTIME_ERROR,

    COREAI_EMBEDDING_ERROR_UNKNOWN = 99,
} CoreAiEmbeddingErrorCode;

#ifdef __cplusplus
}
#endif

#endif
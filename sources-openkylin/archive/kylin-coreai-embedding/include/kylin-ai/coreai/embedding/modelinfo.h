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

#ifndef MODELINFO_H
#define MODELINFO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _EmbeddingModelInfo EmbeddingModelInfo;
typedef struct _EmbeddingModelList EmbeddingModelList;

/// @brief 获取模型列表中的模型数量
/// @param model_list[入参] 通过 get_model_list 接口获得
/// @param error_code[出参] 获取成功时返回0，否则返回具体的错误码
/// @return 模型数量
int embedding_model_list_get_count(const EmbeddingModelList *model_list,
                                   int *error_code);

/// @brief 根据模型列表获取索引值对应的模型信息
/// @param model_list[入参] 通过 get_model_list 接口获得
/// @param index[入参] 获取索引对应的模型信息
/// @param error_code[出参] 获取成功时返回0，否则返回具体的错误码
/// @return 模型信息 生命周期和 model_list 绑定
const EmbeddingModelInfo *embedding_model_list_get_model(
    const EmbeddingModelList *model_list, int index, int *error_code);

/// @brief 根据模型信息获取模型的名称
/// @param model_info[入参] 通过 embedding_model_list_get_model 接口获得
/// @param error_code[出参] 获取成功时返回0，否则返回具体的错误码
/// @return 模型名称 生命周期和 model_info 绑定
const char *embedding_model_info_get_model_name(
    const EmbeddingModelInfo *model_info, int *error_code);

/// @brief 根据模型信息获取模型的输出维度
/// @param model_info[入参] 通过 embedding_model_list_get_model 接口获得
/// @param error_code[出参] 获取成功时返回0，否则返回具体的错误码
/// @return 模型输出维度
int embedding_model_info_get_model_dim(const EmbeddingModelInfo *model_info,
                                       int *error_code);

#ifdef __cplusplus
}
#endif

#endif
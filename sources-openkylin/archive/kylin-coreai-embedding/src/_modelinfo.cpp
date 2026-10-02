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

#include "_modelinfo.h"

#include "error.h"

namespace {
void setErrorCode(int *pErrorCode, int errorCode) {
    if (pErrorCode != nullptr) {
        *pErrorCode = errorCode;
    }
}

}  // namespace
int embedding_model_list_get_count(const _EmbeddingModelList *model_list,
                                   int *error_code) {
    // 获取模型的数量
    if (!model_list) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return -1;
    }

    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_list->size();
}

const _EmbeddingModelInfo *embedding_model_list_get_model(
    const _EmbeddingModelList *model_list, int index, int *error_code) {
    // 根据模型列表和索引值，获取模型信息 index为0的是目前适配最好的模型
    if (!model_list) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }

    const _EmbeddingModelInfo *info = model_list->getModelInfo(index);
    if (!info) {
        setErrorCode(error_code, AI_COMMON_MODEL_NOT_FOUND);
    } else {
        setErrorCode(error_code, AI_COMMON_SUCCESS);
    }

    return info;
}

const char *embedding_model_info_get_model_name(
    const _EmbeddingModelInfo *model_info, int *error_code) {
    // 根据模型信息获取模型的名字
    if (!model_info) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return nullptr;
    }

    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->getName().c_str();
}

int embedding_model_info_get_model_dim(const _EmbeddingModelInfo *model_info,
                                       int *error_code) {
    // 根据模型信息获取模型的输出维度
    if (!model_info) {
        setErrorCode(error_code, AI_COMMON_INVALID_PARAMETER);
        return 0;
    }

    setErrorCode(error_code, AI_COMMON_SUCCESS);
    return model_info->getDim();
}

/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "textembeddingproxy.h"

#include <kylin-ai/coreai/embedding/embedding.h>
#include <kylin-ai/coreai/embedding/error.h>
#include <unistd.h>

#include "log.h"
using namespace scene;

TextEmbeddingProxy::~TextEmbeddingProxy() { destory(); }
void TextEmbeddingProxy::destory() {
    if (session_) {
        text_embedding_destroy_session(&session_);
        session_ = nullptr;
    }
}
bool TextEmbeddingProxy::init() {
    PRINT_INFO("Start create text embedding session\n");
    session_ = text_embedding_create_session();
    if (!session_) {
        PRINT_ERROR("Create text embedding session error\n");
        return false;
    }
    PRINT_INFO("Start init text embedding session\n");

    if (text_embedding_init_session(session_) != COREAI_EMBEDDING_SUCESS) {
        PRINT_WARN("Init text embedding session error\n");

        for (int i = 1; i <= 3; i++) {
            PRINT_INFO("Wait 5s, then start reconnecting to kylin-ai-runtime.\n");
            sleep(5);
            PRINT_INFO("Start reconnect kylin-ai-runtime, retrynum: %d\n", i);
            if (text_embedding_init_session(session_) == COREAI_EMBEDDING_SUCESS) {
                PRINT_INFO("Reconnect kylin-ai-runtime Sucess\n");
                return true;
            }
        }
        PRINT_ERROR("Init text embedding session error\n");
        return false;
    }

    // todo 先写死了 25w版本要把模型信息的相关接口开放出去
    const char* modelName = "ensemble-embd_gte-base_uint8-text";
    if (text_embedding_init_model(session_, modelName) != AI_COMMON_SUCCESS) {
        PRINT_ERROR("Init text embedding model:%s ,error\n", modelName);
        return false;
    }

    return true;
}

std::vector<float> TextEmbeddingProxy::embeddingText(const std::string& text) {
    if (!session_) {
        PRINT_ERROR("Session don't init, can't embedding text\n");
        return std::vector<float>();
    }

    EmbeddingResult* result = nullptr;
    bool ret = text_embedding(session_, text.c_str(), &result);

    if (!ret && result && embedding_result_get_error_code(result) == COREAI_EMBEDDING_CONNECTION_ERROR) {
        // 防止初始化没问题，但在后续中runtime崩溃重启了，此时需要创建新的session, 10为连接错误
        destory();
        PRINT_WARN(
            "Text embedding sdk is attempting to reconnect to kylin-ai-runtime due to a crash or unresponsiveness in "
            "kylin-ai-runtime\n");
        if (!init()) {
            PRINT_ERROR("Text embedding sdk reconnection kylin-ai-runtime error\n");
            return std::vector<float>();
        }
        embedding_result_destroy(&result);
        ret = text_embedding(session_, text.c_str(), &result);
        if (ret) {
            PRINT_INFO("Text embedding sdk reconnection kylin-ai-runtime success\n");
        }
    }

    if (!ret) {
        PRINT_ERROR("Text embedding error\n");
        return std::vector<float>();
    }
    if (!result) {
        PRINT_ERROR("Text embedding result is nullptr\n");
        return std::vector<float>();
    }
    if (embedding_result_get_error_code(result) != COREAI_EMBEDDING_SUCESS) {
        PRINT_ERROR("Text embedding result error:%s.\n", embedding_result_get_error_message(result));
        embedding_result_destroy(&result);
        return std::vector<float>();
    }
    int vectorLength = embedding_result_get_vector_length(result);
    float* vectorData = embedding_result_get_vector_data(result);
    std::vector<float> res = std::vector<float>(vectorData, vectorData + vectorLength);
    embedding_result_destroy(&result);

    return res;
}

std::string TextEmbeddingProxy::modelInfo() {
    if (!session_) {
        PRINT_ERROR("Session don't init, can't embedding text\n");
        return "";
    }
    char* info = nullptr;
    if (!text_embedding_get_model_info(session_, &info)) {
        PRINT_WARN("Get text embedding model error\n");
        destory();
        PRINT_WARN(
            "Text embedding sdk is attempting to reconnect to kylin-ai-runtime due to a crash or unresponsiveness in "
            "kylin-ai-runtime\n");
        if (!init()) {
            PRINT_ERROR("Text embedding sdk reconnection kylin-ai-runtime error\n");
            return "";
        }
        text_embedding_get_model_info(session_, &info);
        PRINT_INFO("Text embedding sdk reconnection kylin-ai-runtime sucess\n");
    }
    if (!info) {
        PRINT_ERROR("Get text embedding model result is empty\n");
        return "";
    }
    std::string res(info);
    embedding_model_info_destroy(info);
    return res;
}

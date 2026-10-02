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

#include "imageembeddingproxy.h"

#include <coreai/embedding/embedding.h>
#include <kylin-ai/coreai/embedding/error.h>
#include <unistd.h>

#include "log.h"

using namespace scene;

ImageEmbeddingProxy::~ImageEmbeddingProxy() { destory(); }

void ImageEmbeddingProxy::destory() {
    if (session_) {
        image_embedding_destroy_session(&session_);
        session_ = nullptr;
    }
}

bool ImageEmbeddingProxy::init() {
    PRINT_INFO("Start create image embedding session\n");
    session_ = image_embedding_create_session();
    if (!session_) {
        PRINT_ERROR("Create image embedding session error\n");
        return false;
    }
    PRINT_INFO("Start init image embedding session\n");

    if (image_embedding_init_session(session_) != COREAI_EMBEDDING_SUCESS) {
        PRINT_WARN("Init image embedding session error\n");

        for (int i = 1; i <= 3; i++) {
            PRINT_INFO("Wait 5s, then start reconnecting to kylin-ai-runtime.\n");
            sleep(5);
            PRINT_INFO("Start reconnect kylin-ai-runtime, retrynum: %d.\n", i);
            if (image_embedding_init_session(session_) == COREAI_EMBEDDING_SUCESS) {
                PRINT_INFO("Reconnect kylin-ai-runtime Sucess.\n");
                return true;
            }
        }
        PRINT_ERROR("Init image embedding session error\n");
        return false;
    }

    // todo 先写死了 25w版本要把模型信息的相关接口开放出去
    const char* modelName = "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    if (image_embedding_init_model(session_, modelName) != AI_COMMON_SUCCESS) {
        PRINT_ERROR("Init image embedding model:%s ,error\n", modelName);
        return false;
    }

    return true;
}

std::vector<float> ImageEmbeddingProxy::embeddingText(const std::string& text) {
    if (!session_) {
        PRINT_ERROR("Session don't init, can't embedding text\n");
        return std::vector<float>();
    }

    EmbeddingResult* result = nullptr;
    if (!text_embedding_by_image_model(session_, text.c_str(), &result)) {
        PRINT_ERROR("Text embedding by image model error!\n");
        return std::vector<float>();
    }

    if (embedding_result_get_error_code(result) == COREAI_EMBEDDING_CONNECTION_ERROR) {
        // 说明runtime崩溃重启了，此时需要创建新的session 10为连接错误
        destory();
        PRINT_WARN(
            "Text embedding by image model sdk is attempting to reconnect to kylin-ai-runtime due to a crash or "
            "unresponsiveness in "
            "kylin-ai-runtime.\n");
        if (!init()) {
            PRINT_ERROR("Text embedding by image model sdk reconnection kylin-ai-runtime error\n");
            return std::vector<float>();
        }
        embedding_result_destroy(&result);
        text_embedding_by_image_model(session_, text.c_str(), &result);
        PRINT_INFO("Text embedding by image model sdk reconnection kylin-ai-runtime sucess.\n");
    }

    if (!result) {
        PRINT_ERROR("Text embedding by image model result is nullptr\n");
        return std::vector<float>();
    }
    if (embedding_result_get_error_code(result) != COREAI_EMBEDDING_SUCESS) {
        PRINT_ERROR("Text embedding by image model result error:%s.\n", embedding_result_get_error_message(result));
        embedding_result_destroy(&result);
        return std::vector<float>();
    }
    int vectorLength = embedding_result_get_vector_length(result);
    float* vectorData = embedding_result_get_vector_data(result);
    std::vector<float> res = std::vector<float>(vectorData, vectorData + vectorLength);
    embedding_result_destroy(&result);

    return res;
}

std::vector<float> ImageEmbeddingProxy::embeddingImage(const std::string& imageBase64) {
    if (!session_) {
        PRINT_ERROR("Session don't init, can't embedding imageBase64\n");
        return std::vector<float>();
    }

    EmbeddingResult* result = nullptr;
    if (!image_embedding_by_base64_image_data(session_, (const unsigned char*)imageBase64.c_str(), imageBase64.size(),
                                              &result)) {
        PRINT_ERROR("ImageBase64 embedding error!\n");
        return std::vector<float>();
    }

    if (embedding_result_get_error_code(result) == COREAI_EMBEDDING_CONNECTION_ERROR) {
        // 说明runtime崩溃重启了，此时需要创建新的session 10为连接错误
        destory();
        PRINT_WARN(
            "ImageBase64 embedding sdk is attempting to reconnect to kylin-ai-runtime due to a crash or "
            "unresponsiveness in "
            "kylin-ai-runtime.\n");
        if (!init()) {
            PRINT_ERROR("ImageBase64 embedding sdk reconnection kylin-ai-runtime error.\n");
            return std::vector<float>();
        }
        embedding_result_destroy(&result);
        image_embedding_by_base64_image_data(session_, (const unsigned char*)imageBase64.c_str(), imageBase64.size(),
                                             &result);
        PRINT_INFO("ImageBase64 embedding sdk reconnection kylin-ai-runtime sucess.\n");
    }

    if (!result) {
        PRINT_ERROR("ImageBase64 embedding result is nullptr\n");
        return std::vector<float>();
    }
    if (embedding_result_get_error_code(result) != COREAI_EMBEDDING_SUCESS) {
        PRINT_ERROR("ImageBase64 embedding result error:%s.\n", embedding_result_get_error_message(result));
        embedding_result_destroy(&result);
        return std::vector<float>();
    }
    int vectorLength = embedding_result_get_vector_length(result);
    float* vectorData = embedding_result_get_vector_data(result);
    std::vector<float> res = std::vector<float>(vectorData, vectorData + vectorLength);
    embedding_result_destroy(&result);

    return res;
}

std::vector<float> ImageEmbeddingProxy::embeddingImageFile(const std::string& filepath) {
    if (!session_) {
        PRINT_ERROR("Session don't init, can't embedding imageFile\n");
        return std::vector<float>();
    }

    EmbeddingResult* result = nullptr;
    if (!image_embedding_by_image_file(session_, filepath.c_str(), &result)) {
        PRINT_ERROR("ImageFile embedding error!\n");
        return std::vector<float>();
    }

    if (embedding_result_get_error_code(result) == COREAI_EMBEDDING_CONNECTION_ERROR) {
        // 说明runtime崩溃重启了，此时需要创建新的session 10为连接错误
        destory();
        PRINT_WARN(
            "ImageFile embedding sdk is attempting to reconnect to kylin-ai-runtime due to a crash or "
            "unresponsiveness in "
            "kylin-ai-runtime.\n");
        if (!init()) {
            PRINT_ERROR("ImageFile embedding sdk reconnection kylin-ai-runtime error.\n");
            return std::vector<float>();
        }
        embedding_result_destroy(&result);
        image_embedding_by_image_file(session_, filepath.c_str(), &result);
        PRINT_INFO("ImageFile embedding sdk reconnection kylin-ai-runtime sucess.\n");
    }

    if (!result) {
        PRINT_ERROR("ImageFile embedding result is nullptr\n");
        return std::vector<float>();
    }
    if (embedding_result_get_error_code(result) != COREAI_EMBEDDING_SUCESS) {
        PRINT_ERROR("ImageFile embedding result error:%s.\n", embedding_result_get_error_message(result));
        embedding_result_destroy(&result);
        return std::vector<float>();
    }
    int vectorLength = embedding_result_get_vector_length(result);
    float* vectorData = embedding_result_get_vector_data(result);
    std::vector<float> res = std::vector<float>(vectorData, vectorData + vectorLength);
    embedding_result_destroy(&result);

    return res;
}

std::string ImageEmbeddingProxy::modelInfo() {
    if (!session_) {
        PRINT_ERROR("Session don't init, can't embedding text\n");
        return "";
    }
    char* info = nullptr;
    if (!image_embedding_get_model_info(session_, &info)) {
        PRINT_WARN("Get image embedding model error\n");
        destory();
        PRINT_WARN(
            "Image embedding sdk is attempting to reconnect to kylin-ai-runtime due to a crash or unresponsiveness in "
            "kylin-ai-runtime.\n");
        if (!init()) {
            PRINT_ERROR("Image embedding sdk reconnection kylin-ai-runtime error.\n");
            return "";
        }
        image_embedding_get_model_info(session_, &info);
        PRINT_INFO("Image embedding sdk reconnection kylin-ai-runtime sucess.\n");
    }
    if (!info) {
        PRINT_ERROR("Get image embedding model result is empty\n");
        return "";
    }
    std::string res(info);
    embedding_model_info_destroy(info);
    return res;
}
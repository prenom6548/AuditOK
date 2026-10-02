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

#include "embedding.h"

#include <cstring>
#include <filesystem>

#include "_modelinfo.h"
#include "embeddingcommon.h"
#include "error.h"
#include "imageembeddingsession.h"
#include "logger.h"
#include "textembeddingsession.h"

namespace fs = std::filesystem;

namespace {
/**
 * @brief 验证文件路径是否存在且是普通文件
 *
 * 检查规则：
 * 1. 路径不能为空
 * 2. 文件必须存在
 * 3. 必须是普通文件（不能是目录、符号链接等）
 *
 * @param file_path 待验证的文件路径
 * @return true 如果文件路径有效，false 否则
 */
bool isValidFilePath(const char *file_path) {
    if (!file_path || file_path[0] == '\0') {  // 检查文件路径是否为空
        return false;
    }

    try {
        fs::path path(file_path);

        // 检查文件是否存在
        if (!fs::exists(path)) {
            LOGE("File does not exist: ", file_path);
            return false;
        }

        // 检查是否是普通文件（不是目录）
        if (!fs::is_regular_file(path)) {
            LOGE("Path is not a regular file: ", file_path);
            return false;
        }

        return true;
    } catch (const fs::filesystem_error &e) {
        LOGE("Filesystem error when checking file path: ", file_path,
             ", error: ", e.what());
        return false;
    } catch (const std::exception &e) {
        LOGE("Exception when checking file path: ", file_path,
             ", error: ", e.what());
        return false;
    }
}
}  // namespace

float *embedding_result_get_vector_data(EmbeddingResult *result) {
    if (!result) return nullptr;
    return result->vector_result.data();
}

int embedding_result_get_vector_length(EmbeddingResult *result) {
    if (!result) return 0;
    return result->vector_result.size();
}

int embedding_result_get_error_code(EmbeddingResult *result) {
    if (!result) return COREAI_EMBEDDING_INPUT_ERROR;
    return result->error_code;
}
const char *embedding_result_get_error_message(EmbeddingResult *result) {
    if (!result) return nullptr;
    return result->error_message.c_str();
}

void embedding_result_destroy(EmbeddingResult **result) {
    // 双重检查
    if (!result) return;
    if (*result != nullptr) {
        delete *result;
        *result = nullptr;
    }
}
void embedding_model_info_destroy(char *result) { delete[] result; }

_TextEmbeddingSession *text_embedding_create_session() {
    _TextEmbeddingSession *textEmbeddingSession = new _TextEmbeddingSession();
    return textEmbeddingSession;
}
void text_embedding_destroy_session(_TextEmbeddingSession **session) {
    // 双重检查
    if (!session) return;
    if (*session != nullptr) {
        delete *session;
        *session = nullptr;
    }
}

int text_embedding_init_model(TextEmbeddingSession *session,
                              const char *model_name) {
    // 加载模型
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }
    if (model_name == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }

    return session->initModel(model_name);
}

int text_embedding_init_session(_TextEmbeddingSession *session) {
    if (!session) return COREAI_EMBEDDING_INPUT_ERROR;

    int errCode = session->connect();
    if (errCode != AI_COMMON_SUCCESS) {
        return errCode;
    }
    return session->init();
}

_EmbeddingModelList *text_embedding_get_model_list(
    TextEmbeddingSession *session, int *error_code) {
    if (session == nullptr) {
        if (error_code != nullptr) {
            *error_code = AI_COMMON_INVALID_SESSION;
        }
        return nullptr;
    }

    return session->getModelList(error_code);
}

// 该函数用于集成测试中，不连接runtime，而是直接调用init接口
int text_embedding_init_session_no_connect(_TextEmbeddingSession *session) {
    return session->init();
}

// 该函数用于集成测试中，不连接runtime，而是直接调用init接口
int image_embedding_init_session_no_connect(_ImageEmbeddingSession *session) {
    return session->init();
}

bool text_embedding_get_model_info(_TextEmbeddingSession *session,
                                   char **model_info) {
    if (!session) return false;
    std::string res;
    res = session->getModelInfo();
    if (!res.empty()) {
        size_t length = res.size() + 1;
        *model_info = new char[length];
        std::strcpy(*model_info, res.c_str());
        return true;
    }
    return false;
}

bool text_embedding(TextEmbeddingSession *session, const char *text,
                    EmbeddingResult **result) {
    // 先检查result是否为空
    if (result == nullptr) {
        LOGE("Output parameter 'result' is NULL");
        return false;
    }

    // 先置空，避免传入野指针
    *result = nullptr;
    // 再检查session和text是否为空
    if (!session || !text) {
        LOGE("Incorrect input parameter: session or text is NULL");
        *result = new EmbeddingResult(COREAI_EMBEDDING_INPUT_ERROR,
                                      "Incorrect input parameter");
        return false;
    }

    *result = session->embeddingText(text);
    if (embedding_result_get_error_code(*result) != AI_COMMON_SUCCESS) {
        return false;
    }

    return true;
}

void text_embedding_async(_TextEmbeddingSession *session, const char *text,
                          TextEmbeddingResultCallback callback,
                          void *callback_user_data) {
    if (callback == nullptr) {
        LOGE("Callback function is NULL");
        return;
    }

    if (!session || !text) {
        EmbeddingResult result(COREAI_EMBEDDING_INPUT_ERROR,
                               "Incorrect input parameter");
        callback(&result, callback_user_data);
        return;
    }

    session->embeddingTextAsync(text, callback, callback_user_data);
}

void text_embedding_enable_internal_event_loop(TextEmbeddingSession *session,
                                               bool enable) {
    if (!session) return;
    session->enableInternalEventLoop(enable);
}

_ImageEmbeddingSession *image_embedding_create_session() {
    _ImageEmbeddingSession *imageEmbeddingSession =
        new _ImageEmbeddingSession();
    return imageEmbeddingSession;
}
void image_embedding_destroy_session(_ImageEmbeddingSession **session) {
    if (!session) return;
    if (*session != nullptr) {
        delete *session;
        *session = nullptr;
    }
}
int image_embedding_init_session(_ImageEmbeddingSession *session) {
    if (!session) return COREAI_EMBEDDING_INPUT_ERROR;

    int errCode = session->connect();
    if (errCode != AI_COMMON_SUCCESS) {
        return errCode;
    }
    return session->init();
}
int image_embedding_init_model(ImageEmbeddingSession *session,
                               const char *model_name) {
    // 加载模型
    if (session == nullptr) {
        return AI_COMMON_INVALID_SESSION;
    }

    if (model_name == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }

    return session->initModel(model_name);
}

_EmbeddingModelList *image_embedding_get_model_list(
    ImageEmbeddingSession *session, int *error_code) {
    if (session == nullptr) {
        if (error_code != nullptr) {
            *error_code = AI_COMMON_INVALID_SESSION;
        }
        return nullptr;
    }

    return session->getModelList(error_code);
}

bool image_embedding_get_model_info(_ImageEmbeddingSession *session,
                                    char **model_info) {
    if (!session) return false;
    std::string res;
    res = session->getModelInfo();
    if (!res.empty()) {
        size_t length = res.size() + 1;
        *model_info = new char[length];
        std::strcpy(*model_info, res.c_str());
        return true;
    }
    return false;
}

bool text_embedding_by_image_model(_ImageEmbeddingSession *session,
                                   const char *text, EmbeddingResult **result) {
    if (result == nullptr) {
        LOGE("Output parameter 'result' is NULL");
        return false;
    }

    *result = nullptr;
    if (!session || !text) {
        LOGE("Incorrect input parameter: session or text is NULL");
        *result = new EmbeddingResult(COREAI_EMBEDDING_INPUT_ERROR,
                                      "Incorrect input parameter");
        return false;
    }

    *result = session->embeddingText(text);
    if (embedding_result_get_error_code(*result) != AI_COMMON_SUCCESS) {
        return false;
    }

    return true;
}

bool image_embedding_by_image_file(_ImageEmbeddingSession *session,
                                   const char *image_file,
                                   EmbeddingResult **result) {
    if (result == nullptr) {
        LOGE("Output parameter 'result' is NULL");
        return false;
    }

    *result = nullptr;
    if (!session || !image_file) {
        LOGE("Incorrect input parameter: session or image_file is NULL");
        *result = new EmbeddingResult(COREAI_EMBEDDING_INPUT_ERROR,
                                      "Incorrect input parameter");
        return false;
    }

    // 验证文件路径是否存在且是有效的文件
    if (!isValidFilePath(image_file)) {
        LOGE("Invalid file path: ", image_file);
        *result = new EmbeddingResult(
            COREAI_EMBEDDING_INPUT_ERROR,
            "Invalid file path: file does not exist or is not a regular file");
        return false;
    }

    *result = session->embeddingImage(image_file);
    if (embedding_result_get_error_code(*result) != AI_COMMON_SUCCESS) {
        return false;
    }

    return true;
}

bool image_embedding_by_base64_image_data(_ImageEmbeddingSession *session,
                                          const unsigned char *image_data,
                                          unsigned int image_data_length,
                                          EmbeddingResult **result) {
    if (result == nullptr) {
        LOGE("Output parameter 'result' is NULL");
        return false;
    }

    *result = nullptr;
    if (!session || !image_data) {
        LOGE("Incorrect input parameter: session or image_data is NULL");
        *result = new EmbeddingResult(COREAI_EMBEDDING_INPUT_ERROR,
                                      "Incorrect input parameter");
        return false;
    }

    if (image_data_length <= 0) {
        LOGE("Invalid input parameter: image_data_length is error");
        *result = new EmbeddingResult(
            COREAI_EMBEDDING_INPUT_ERROR,
            "Invalid input parameter: image_data_length is error");
        return false;
    }

    std::string base64_str(reinterpret_cast<const char *>(image_data),
                           image_data_length);
    *result = session->embeddingBase64Image(base64_str);
    if (embedding_result_get_error_code(*result) != AI_COMMON_SUCCESS) {
        return false;
    }

    return true;
}

void text_embedding_by_image_model_async(_ImageEmbeddingSession *session,
                                         const char *text,
                                         ImageEmbeddingResultCallback callback,
                                         void *callback_user_data) {
    if (callback == nullptr) {
        LOGE("Callback function is NULL");
        return;
    }

    if (!session || !text) {
        EmbeddingResult result(COREAI_EMBEDDING_INPUT_ERROR,
                               "Incorrect input parameter");
        callback(&result, callback_user_data);
        return;
    }

    session->embeddingTextAsync(text, callback, callback_user_data);
}
void image_embedding_from_by_file_async(_ImageEmbeddingSession *session,
                                        const char *file_path,
                                        ImageEmbeddingResultCallback callback,
                                        void *callback_user_data) {
    if (callback == nullptr) {
        LOGE("Callback function is NULL");
        return;
    }

    if (!session || !file_path) {
        EmbeddingResult result(COREAI_EMBEDDING_INPUT_ERROR,
                               "Incorrect input parameter");
        callback(&result, callback_user_data);
        return;
    }

    // 验证文件路径是否存在且是有效的文件
    if (!isValidFilePath(file_path)) {
        LOGE("Invalid file path: ", file_path);
        EmbeddingResult result(
            COREAI_EMBEDDING_INPUT_ERROR,
            "Invalid file path: file does not exist or is not a regular file");
        callback(&result, callback_user_data);
        return;
    }

    session->embeddingImageAsync(file_path, callback, callback_user_data);
}

void image_embedding_by_base64_image_data_async(
    _ImageEmbeddingSession *session, const unsigned char *image_data,
    unsigned int image_data_length, ImageEmbeddingResultCallback callback,
    void *callback_user_data) {
    if (callback == nullptr) {
        LOGE("Callback function is NULL");
        return;
    }

    if (!session || !image_data) {
        EmbeddingResult result(COREAI_EMBEDDING_INPUT_ERROR,
                               "Incorrect input parameter");
        callback(&result, callback_user_data);
        return;
    }

    if (image_data_length <= 0) {
        EmbeddingResult result(
            COREAI_EMBEDDING_INPUT_ERROR,
            "Invalid input parameter: image_data_length is error");
        callback(&result, callback_user_data);
        return;
    }

    std::string base64_str(reinterpret_cast<const char *>(image_data),
                           image_data_length);
    session->embeddingBase64ImageAsync(base64_str, callback,
                                       callback_user_data);
}

void image_embedding_enable_internal_event_loop(ImageEmbeddingSession *session,
                                                bool enable) {
    if (!session) return;
    session->enableInternalEventLoop(enable);
}
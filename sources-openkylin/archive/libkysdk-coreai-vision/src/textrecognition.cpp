/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#include "textrecognition.h"

#include "_textrecognitionsession.h"
#include "logger.h"

namespace {
using TextRecognitionSessionPtr = kyai::core::vision::_TextRecognitionSession *;
}

// 创建session
TextRecognitionSession *text_recognition_create_session() {
    auto *session = new kyai::core::vision::_TextRecognitionSession();
    return (TextRecognitionSession *)session;
}

// 销毁session
void text_recognition_destroy_session(TextRecognitionSession **session) {
    if (session == nullptr) {
        return;
    }
    if (*session == nullptr) {
        return;
    }
    ((kyai::core::vision::_TextRecognitionSession *)*session)->destroyTextRecognitionSession();

    delete (kyai::core::vision::_TextRecognitionSession *)*session;
    *session = nullptr;
}

// 初始化session
int text_recognition_init_session(TextRecognitionSession *session) {
    if (session == nullptr) {
        return AISDK_INVALID_SESSION;
    }
    return ((kyai::core::vision::_TextRecognitionSession *)session)->init();
}

// 设置结果回调
void text_recognition_result_set_callback(TextRecognitionSession *session, TextRecognitionResultCallback callback,
                                          void *user_data) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::vision::_TextRecognitionSession *)session)->setRecognitionResultCallback(callback, user_data);
}
// 配置Ocr模型
void text_recognition_set_model_config(TextRecognitionSession *session, TextRecognitionModelConfig *config) {
    if (session == nullptr or config == nullptr) {
        return;
    }
    ((kyai::core::vision::_TextRecognitionSession *)session)->setRecognizeTextModelConfig(*config);
}
// 从图像文件中识别
void text_recognition_recognize_text_from_image_file_async(TextRecognitionSession *session, const char *image_file) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::vision::_TextRecognitionSession *)session)->recognizeTextFromImageFileAsync(image_file);
}

void text_recognition_recognize_text_from_image_file_async_with_request_id(TextRecognitionSession *session,
                                                                           const char *request_id,
                                                                           const char *image_file) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::vision::_TextRecognitionSession *)session)->recognizeTextFromImageFileAsync(request_id, image_file);
}

// 从图像数据中进行识别
void text_recognition_recognize_text_from_image_data_async(TextRecognitionSession *session, const char *image_data,
                                                           unsigned int image_data_length) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::vision::_TextRecognitionSession *)session)
        ->recognizeTextFromImageDataAsync(image_data, image_data_length);
}

void text_recognition_recognize_text_from_image_data_async_with_request_id(TextRecognitionSession *session,
                                                                           const char *request_id,
                                                                           const char *image_data,
                                                                           unsigned int image_data_length) {
    if (session == nullptr) {
        return;
    }
    ((kyai::core::vision::_TextRecognitionSession *)session)
        ->recognizeTextFromImageDataAsync(request_id, image_data, image_data_length);
}

void text_recognition_enable_internal_event_loop(TextRecognitionSession *session, bool enable) {
    if (session == nullptr) {
        LOGE("Session is null.");
        return;
    }
    TextRecognitionSessionPtr(session)->enableInternalEventLoop(enable);
}

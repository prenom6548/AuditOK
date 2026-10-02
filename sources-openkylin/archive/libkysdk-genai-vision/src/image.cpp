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

#include "image.h"

#include "_genaiimagesession.h"

namespace {
using GenAiImageSessionPtr = kylin::genai::vision::_GenAiImageSession *;
}

GenAiImageSession *genai_image_create_session() {
    auto *session = new kylin::genai::vision::_GenAiImageSession();
    return (GenAiImageSession *)session;
}

void genai_image_destroy_session(GenAiImageSession **session) {
    if (session == nullptr) {
        return;
    }
    if (*session == nullptr) {
        return;
    }
    delete GenAiImageSessionPtr(*session);
    *session = nullptr;
}

int genai_image_init_session(GenAiImageSession *session) {
    if (session == nullptr) {
        return AISDK_INVALID_SESSION;
    }
    return ((kylin::genai::vision::_GenAiImageSession *)session)->init();
}

void genai_image_set_config(GenAiImageSession *session, ImageConfig *config) {
    if (session == nullptr) {
        return;
    }
    if (config == nullptr) {
        GenAiImageSessionPtr(session)->setImageConfig(ImageConfig{});
    } else {
        GenAiImageSessionPtr(session)->setImageConfig(*config);
    }
}

void genai_image_result_set_callback(GenAiImageSession *session, ImageResultCallback callback, void *user_data) {
    if (session == nullptr) {
        return;
    }
    ((kylin::genai::vision::_GenAiImageSession *)session)->setImageResultCallback(callback, user_data);
}

const char **genai_image_get_supported_image_style(GenAiImageSession *session, int *number) {
    if (session == nullptr) {
        return nullptr;
    }
    return ((kylin::genai::vision::_GenAiImageSession *)session)->getSupportedImageStyle(number);
}

const ImageSize *genai_image_get_supported_image_size(GenAiImageSession *session, int *number) {
    if (session == nullptr) {
        return nullptr;
    }
    return ((kylin::genai::vision::_GenAiImageSession *)session)->getSupportedImageSize(number);
}

int genai_image_get_supported_image_number(GenAiImageSession *session) {
    if (session == nullptr) {
        return -1;
    }
    return ((kylin::genai::vision::_GenAiImageSession *)session)->getSupportedImageNumber();
}

void genai_image_generate_image_async(GenAiImageSession *session, const char *prompt) {
    if (session == nullptr) {
        return;
    }
    if (prompt == nullptr) {
        prompt = "";
    }
    GenAiImageSessionPtr(session)->generateImageAsync(prompt);
}

int genai_image_generate_image_async_with_request_id(GenAiImageSession *session, const char *prompt,
                                                     const char *request_id) {
    if (session == nullptr) {
        return AISDK_INVALID_SESSION;
    }
    if (prompt == nullptr) {
        return AI_COMMON_INVALID_PARAMETER;
    }
    if (request_id == nullptr) {
        request_id = "";
    }
    return GenAiImageSessionPtr(session)->generateImageAsyncWithRequestId(prompt, request_id);
}

int genai_image_stop_generation(GenAiImageSession *session, const char *request_id) {
    if (session == nullptr) {
        return AISDK_INVALID_SESSION;
    }
    return ((kylin::genai::vision::_GenAiImageSession *)session)->stopGeneration(request_id);
}

void genai_image_enable_internal_event_loop(GenAiImageSession *session, bool enable) {
    if (session == nullptr) {
        return;
    }
    GenAiImageSessionPtr(session)->enableInternalEventLoop(enable);
}

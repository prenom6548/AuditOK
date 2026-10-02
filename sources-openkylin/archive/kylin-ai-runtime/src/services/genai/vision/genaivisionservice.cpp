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

#include "genaivisionservice.h"

#include <kylin-ai/aiengine.h>
#include <kylin-ai/common/error.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <sodium.h>

#include <cstdio>

#include "utils/logger.h"
#include "utils/util.h"

namespace {

constexpr const char* kSdkVersionAnnotation = "SdkVersion";
constexpr const char* kDefaultSdkVersion = "1.0.0";

constexpr const char* objectPath = "/com/kylin/AiRuntime/GenAiVision";
constexpr const char* interface = "com.kylin.AiRuntime.GenAiVision";
constexpr const char* visionResultSignal = "GenAiVisionResult";

}  // namespace

GenAiVisionService::GenAiVisionService(GDBusConnection& connection, EngineSelector& engineSelector)
    : connection_(connection), engineSelector_(engineSelector) {
    exportSkeleton();
    const std::string protocolVersion = getProtocolVersion();
    if (protocolVersion.empty()) {
        LOGW("Protocol version is empty, use default protocol version: {}", kDefaultSdkVersion);
        setRuntimeProtocolVersion(kDefaultSdkVersion);
        LOGI("GenAiVisionService protocol version: {}", kDefaultSdkVersion);
    } else {
        setRuntimeProtocolVersion(protocolVersion);
        LOGI("GenAiVisionService protocol version: {}", protocolVersion);
    }
}

GenAiVisionService::~GenAiVisionService() { unexportSkeleton(); }

void GenAiVisionService::stop() {
    for (const auto& handlerId : handlerIds_) {
        g_signal_handler_disconnect(delegate_, handlerId);
    }
    handlerIds_.clear();
}

void GenAiVisionService::exportSkeleton() {
    delegate_ = ai_runtime_gen_ai_vision_service_skeleton_new();
    connectSignal();

    GError* error = nullptr;

    isExported_ =
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath, &error);
    if (!isExported_) {
        g_printerr("Error creating server at address %s: %s\n", objectPath, error->message);
        g_error_free(error);
    }
}

bool GenAiVisionService::onHandleInit(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                                      gpointer userData) {
    auto* visionService = static_cast<GenAiVisionService*>(userData);
    if (!visionService) {
        LOGE("GenAiVisionService is null.");
        ai_runtime_gen_ai_vision_service_complete_init(service, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr<ImageGenerator> imageGenerator = std::make_shared<ImageGenerator>();
    int sessionId = visionService->generateRandomSessionId();
    imageGenerator->init(sessionId);
    visionService->imageGenerator_[sessionId] = imageGenerator;
    ai_runtime_gen_ai_vision_service_complete_init(service, invocation, sessionId, AI_COMMON_SUCCESS);
    return true;
}

bool GenAiVisionService::onHandleGenerateImage(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                                               gint sessionId, const gchar* prompt, gpointer userData) {
    auto* visionService = static_cast<GenAiVisionService*>(userData);
    if (!visionService) {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED,
                                              "Genai vision service not found.");
        LOGE("GenAiVisionService is null.");
        return false;
    }

    std::shared_ptr sharedNlpService = visionService->getSharedObject();

    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED, "No image generator found.");
        LOGE("No image generator found for session ID: {}", sessionId);
        return false;
    }

    const std::string resultSignalInterfaceName = interface + std::to_string((int)sessionId);
    auto sharedServiceObj = sharedNlpService->getSharedObject();
    genai::ImageGenerationRequest request;
    request.prompt = prompt;
    request.resultCallback = [resultSignalInterfaceName, sharedServiceObj](const genai::ImageGenerationResult& result) {
        sharedServiceObj->emitImageResultSignal(resultSignalInterfaceName, result);
    };
    request.finishedCallback = [service, invocation]() {
        ai_runtime_gen_ai_vision_service_complete_generate_image(service, invocation);
    };
    sharedNlpService->imageGenerator_[sessionId]->generateImageWithoutConfig(request);
    return true;
}

bool GenAiVisionService::onHandleGenerateImageWithRequestId(AiRuntimeGenAiVisionService* service,
                                                            GDBusMethodInvocation* invocation, gint sessionId,
                                                            const gchar* requestId, const gchar* config,
                                                            gpointer userData) {
    auto* visionService = static_cast<GenAiVisionService*>(userData);
    if (!visionService) {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED,
                                              "Genai vision service not found.");
        LOGE("GenAiVisionService is null.");
        return false;
    }

    std::shared_ptr sharedNlpService = visionService->getSharedObject();

    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        g_dbus_method_invocation_return_error(invocation, G_IO_ERROR, G_IO_ERROR_FAILED, "No image generator found.");
        LOGE("No image generator found for session ID: {}", sessionId);
        return false;
    }

    const std::string resultSignalInterfaceName = interface + std::to_string((int)sessionId);
    auto sharedServiceObj = sharedNlpService->getSharedObject();
    std::string requestIdString = requestId == nullptr ? "" : requestId;
    genai::ImageGenerationRequest request{};
    request.fromString(config);
    request.requestId = requestIdString;
    request.resultCallback = [resultSignalInterfaceName, sharedServiceObj](const genai::ImageGenerationResult& result) {
        sharedServiceObj->emitImageResultSignal(resultSignalInterfaceName, result);
    };
    request.finishedCallback = [service, invocation]() {
        ai_runtime_gen_ai_vision_service_complete_generate_image_with_request_id(service, invocation);
    };
    sharedNlpService->imageGenerator_[sessionId]->generateImage(request);
    return true;
}

bool GenAiVisionService::onHandleStopImageGeneration(AiRuntimeGenAiVisionService* service,
                                                     GDBusMethodInvocation* invocation, gint sessionId,
                                                     gpointer userData) {
    auto* visionService = static_cast<GenAiVisionService*>(userData);
    if (!visionService) {
        LOGE("GenAiVisionService is null.");
        ai_runtime_gen_ai_vision_service_complete_stop_image_generation(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = visionService->getSharedObject();

    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        LOGE("No image generator found for session ID: {}", sessionId);
        ai_runtime_gen_ai_vision_service_complete_stop_image_generation(service, invocation);
        return true;
    }

    sharedNlpService->imageGenerator_[sessionId]->stopImageGeneration();
    ai_runtime_gen_ai_vision_service_complete_stop_image_generation(service, invocation);
    return true;
}

bool GenAiVisionService::onHandleStopImageGenerationWithRequestId(AiRuntimeGenAiVisionService* service,
                                                                  GDBusMethodInvocation* invocation, gint sessionId,
                                                                  const gchar* requestId, gpointer userData) {
    auto* visionService = static_cast<GenAiVisionService*>(userData);
    if (!visionService) {
        LOGE("GenAiVisionService is null.");
        ai_runtime_gen_ai_vision_service_complete_stop_image_generation_with_request_id(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = visionService->getSharedObject();

    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        LOGE("No image generator found for session ID: {}", sessionId);
        ai_runtime_gen_ai_vision_service_complete_stop_image_generation_with_request_id(service, invocation);
        return true;
    }

    std::string requestIdString = requestId == nullptr ? "" : requestId;
    sharedNlpService->imageGenerator_[sessionId]->stopImageGeneration(requestIdString);
    ai_runtime_gen_ai_vision_service_complete_stop_image_generation_with_request_id(service, invocation);
    return true;
}

bool GenAiVisionService::onHandleGetSupportedImageNumber(AiRuntimeGenAiVisionService* service,
                                                         GDBusMethodInvocation* invocation, gint sessionId,
                                                         gpointer userData) {
    auto* visionService = static_cast<GenAiVisionService*>(userData);
    if (!visionService) {
        LOGE("GenAiVisionService is null.");
        ai_runtime_gen_ai_vision_service_complete_get_supported_image_number(service, invocation, -1);
        return true;
    }
    std::shared_ptr sharedNlpService = visionService->getSharedObject();

    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        LOGE("No image generator found for session ID: {}", sessionId);
        ai_runtime_gen_ai_vision_service_complete_get_supported_image_number(service, invocation, -1);
        return true;
    }
    int supportedImageNumber = sharedNlpService->imageGenerator_.at(sessionId)->getSupportedImageNumber();
    ai_runtime_gen_ai_vision_service_complete_get_supported_image_number(service, invocation, supportedImageNumber);
    return true;
}

bool GenAiVisionService::onHandleGetSupportedImageSize(AiRuntimeGenAiVisionService* service,
                                                       GDBusMethodInvocation* invocation, gint sessionId,
                                                       gpointer userData) {
    auto* visionService = static_cast<GenAiVisionService*>(userData);
    if (!visionService) {
        LOGE("GenAiVisionService is null.");
        ai_runtime_gen_ai_vision_service_complete_get_supported_image_size(service, invocation, "");
        return true;
    }
    std::shared_ptr sharedNlpService = visionService->getSharedObject();

    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        LOGE("No image generator found for session ID: {}", sessionId);
        ai_runtime_gen_ai_vision_service_complete_get_supported_image_size(service, invocation, "");
        return true;
    }

    std::string supportedImageSize = sharedNlpService->imageGenerator_.at(sessionId)->getSupportedImageSize();

    ai_runtime_gen_ai_vision_service_complete_get_supported_image_size(service, invocation, supportedImageSize.c_str());
    return true;
}

bool GenAiVisionService::onHandleGetSupportedImageStyle(AiRuntimeGenAiVisionService* service,
                                                        GDBusMethodInvocation* invocation, gint sessionId,
                                                        gpointer userData) {
    auto* visionService = static_cast<GenAiVisionService*>(userData);
    if (!visionService) {
        LOGE("GenAiVisionService is null.");
        ai_runtime_gen_ai_vision_service_complete_get_supported_image_style(service, invocation, "");
        return true;
    }
    std::shared_ptr sharedNlpService = visionService->getSharedObject();

    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        LOGE("No image generator found for session ID: {}", sessionId);
        ai_runtime_gen_ai_vision_service_complete_get_supported_image_style(service, invocation, "");
        return true;
    }
    std::string supportedImageStyle = sharedNlpService->imageGenerator_.at(sessionId)->getSupportedImageStyle();

    ai_runtime_gen_ai_vision_service_complete_get_supported_image_style(service, invocation,
                                                                        supportedImageStyle.c_str());
    return true;
}

bool GenAiVisionService::onHandleSetImageConfig(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                                                gint sessionId, const gchar* imageConfig, gpointer userData) {
    auto* visionService = static_cast<GenAiVisionService*>(userData);
    if (!visionService) {
        LOGE("GenAiVisionService is null.");
        ai_runtime_gen_ai_vision_service_complete_set_image_config(service, invocation);
        return true;
    }
    std::shared_ptr sharedNlpService = visionService->getSharedObject();

    if (!sharedNlpService->isSessionIdValid(sessionId)) {
        LOGE("No image generator found for session ID: {}", sessionId);
        ai_runtime_gen_ai_vision_service_complete_set_image_config(service, invocation);
        return true;
    }

    genai::ImageGenerationConfig config{};
    config.fromString(imageConfig);
    sharedNlpService->imageGenerator_.at(sessionId)->setImageConfig(config);
    ai_runtime_gen_ai_vision_service_complete_set_image_config(service, invocation);
    return true;
}

void GenAiVisionService::connectSignal() {
    gulong handlerId{};
    handlerId = g_signal_connect(delegate_, "handle-init", G_CALLBACK(onHandleInit), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-generate-image", G_CALLBACK(onHandleGenerateImage), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-generate-image-with-request-id",
                                 G_CALLBACK(onHandleGenerateImageWithRequestId), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-stop-image-generation", G_CALLBACK(onHandleStopImageGeneration), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-stop-image-generation-with-request-id",
                                 G_CALLBACK(onHandleStopImageGenerationWithRequestId), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle_get_supported_image_number",
                                 G_CALLBACK(onHandleGetSupportedImageNumber), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle_get_supported_image_size", G_CALLBACK(onHandleGetSupportedImageSize), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle_get_supported_image_style",
                                 G_CALLBACK(onHandleGetSupportedImageStyle), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle_set_image_config", G_CALLBACK(onHandleSetImageConfig), this);
    handlerIds_.push_back(handlerId);
}

std::string GenAiVisionService::getProtocolVersion() {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, get protocol version failed.");
        return kDefaultSdkVersion;
    }

    GDBusInterfaceInfo* interfaceInfo = ai_runtime_gen_ai_vision_service_interface_info();
    if (interfaceInfo == nullptr) {
        LOGW("Interface info is null, use default protocol version.");
        return kDefaultSdkVersion;
    }
    if (interfaceInfo->annotations == nullptr) {
        LOGW("Protocol version annotation not found, use default protocol version.");
        return kDefaultSdkVersion;
    }

    for (GDBusAnnotationInfo** annotation = interfaceInfo->annotations; annotation && *annotation; ++annotation) {
        if ((*annotation)->key == nullptr || (*annotation)->value == nullptr) {
            continue;
        }
        if (g_strcmp0((*annotation)->key, kSdkVersionAnnotation) == 0) {
            return (*annotation)->value;
        }
    }
    return kDefaultSdkVersion;
}

void GenAiVisionService::setRuntimeProtocolVersion(const std::string& version) {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, set runtime version failed.");
        return;
    }
    ai_runtime_gen_ai_vision_service_set_runtime_version(delegate_, version.c_str());
}

void GenAiVisionService::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }
    g_object_unref(delegate_);
}

int GenAiVisionService::generateRandomSessionId() {
    uint32_t sessionId;
    randombytes_buf(&sessionId, sizeof(sessionId));
    sessionId = 1 + (sessionId & INT_MAX);
    while (sessionIdSet_.find(sessionId) != sessionIdSet_.end()) {
        randombytes_buf(&sessionId, sizeof(sessionId));
        sessionId = 1 + (sessionId & INT_MAX);
    }
    sessionIdSet_.insert(sessionId);
    return sessionId;
}

bool GenAiVisionService::isSessionIdValid(int sessionId) {
    auto it = imageGenerator_.find(sessionId);
    if (it != imageGenerator_.end()) {
        return true;
    }
    return false;
}

GVariant* GenAiVisionService::buildGVariantFromImageResult(const genai::ImageGenerationResult& result) {
    int errorCode{AI_COMMON_SUCCESS};
    if (result.errorCode == -1) {
        errorCode = AI_COMMON_SUCCESS;
    } else {
        errorCode = result.errorCode;
    }
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE_TUPLE);
    g_variant_builder_add(&builder, "s", result.imageBase64Data.c_str());
    g_variant_builder_add(&builder, "i", result.format);
    g_variant_builder_add(&builder, "i", result.height);
    g_variant_builder_add(&builder, "i", result.width);
    g_variant_builder_add(&builder, "i", result.total);
    g_variant_builder_add(&builder, "i", result.index);
    g_variant_builder_add(&builder, "i", errorCode);
    g_variant_builder_add(&builder, "s", result.errorMessage.c_str());
    g_variant_builder_add(&builder, "s", result.requestId.c_str());
    GVariant* resultVariant = g_variant_builder_end(&builder);
    return resultVariant;
}

void GenAiVisionService::emitImageResultSignal(const std::string& interfaceName,
                                               const genai::ImageGenerationResult& result) {
    GVariant* resultVariant = buildGVariantFromImageResult(result);
    g_dbus_connection_emit_signal(&connection_, interface, objectPath, interfaceName.c_str(), visionResultSignal,
                                  resultVariant, nullptr);
}

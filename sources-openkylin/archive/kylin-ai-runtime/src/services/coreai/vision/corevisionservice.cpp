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

#include "corevisionservice.h"

#include <jsoncpp/json/json.h>
#include <kylin-ai/aiengine.h>
#include <kylin-ai/common/error.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <sodium.h>

#include <cstdio>

#include "utils/logger.h"
#include "utils/util.h"

static const char *objectPath = "/com/kylin/AiRuntime/CoreVision";
static const char *interface = "com.kylin.AiRuntime.CoreVision";

static const char *textRecognitionResultSignal = "TextRecognitionVisionResult";

static const char *segmentationResultSignal = "SegmentationResultSignal";

namespace {
constexpr const char *kSdkVersionAnnotation = "SdkVersion";
constexpr const char *kDefaultSdkVersion = "1.0.0";
namespace {
GVariant *buildGVariantFromTextRecognitionResultInternal(const TextRecognitionResult &result, bool includeRequestId) {
    GVariantBuilder resultBuilder;
    g_variant_builder_init(&resultBuilder, G_VARIANT_TYPE_TUPLE);

    if (includeRequestId) {
        // request_id
        g_variant_builder_add(&resultBuilder, "s", result.requestId.c_str());
    }

    // alltext
    g_variant_builder_add(&resultBuilder, "s", result.allText.c_str());

    // lines
    GVariantBuilder lines_builder;
    g_variant_builder_init(&lines_builder, G_VARIANT_TYPE("a(sa(ii)d)"));

    for (const auto &[text, points, conf] : result.lines) {
        // lines.text
        GVariantBuilder line_builder;
        g_variant_builder_init(&line_builder, G_VARIANT_TYPE("(sa(ii)d)"));
        g_variant_builder_add(&line_builder, "s", text.c_str());

        // lines.points
        GVariantBuilder points_builder;
        g_variant_builder_init(&points_builder, G_VARIANT_TYPE("a(ii)"));
        for (const PixelPoint &point : points) {
            g_variant_builder_add(&points_builder, "(ii)", point.x, point.y);
        }
        g_variant_builder_add_value(&line_builder, g_variant_builder_end(&points_builder));

        // lines.conf
        g_variant_builder_add(&line_builder, "d", static_cast<double>(conf));
        g_variant_builder_add_value(&lines_builder, g_variant_builder_end(&line_builder));
    }
    g_variant_builder_add_value(&resultBuilder, g_variant_builder_end(&lines_builder));

    g_variant_builder_add(&resultBuilder, "i", result.errorCode);
    g_variant_builder_add(&resultBuilder, "s", result.errorMessage.c_str());

    GVariant *resultVariant = g_variant_builder_end(&resultBuilder);

    return resultVariant;
}

}  // namespace

GVariant *buildGVariantFromTextRecognitionResult(const TextRecognitionResult &result) {
    return buildGVariantFromTextRecognitionResultInternal(result, false);
}

GVariant *buildGVariantFromTextRecognitionResultWithRequestId(const TextRecognitionResult &result) {
    return buildGVariantFromTextRecognitionResultInternal(result, true);
}

int convertErrorCode(int errorCode) { return errorCode == -1 ? AI_COMMON_SUCCESS : errorCode; }

GVariant *buildGVariantFromSubjectSegmentationResult(const SubjectSegmentationResult &result) {
    GVariantBuilder resultBuilder;
    g_variant_builder_init(&resultBuilder, G_VARIANT_TYPE("(ssssiisis)"));

    // requestId
    g_variant_builder_add(&resultBuilder, "s", result.requestId.c_str());

    // imageFormat
    g_variant_builder_add(&resultBuilder, "s", result.imageFormat.c_str());

    // imageBase64Data
    g_variant_builder_add(&resultBuilder, "s", result.imageBase64Data.c_str());

    // maskBase64Data
    g_variant_builder_add(&resultBuilder, "s", result.maskBase64Data.c_str());

    // width
    g_variant_builder_add(&resultBuilder, "i", result.width);

    // height
    g_variant_builder_add(&resultBuilder, "i", result.height);

    // edgePoints
    g_variant_builder_add(&resultBuilder, "s", result.edgePoints.c_str());

    g_variant_builder_add(&resultBuilder, "i", convertErrorCode(result.errorCode));
    g_variant_builder_add(&resultBuilder, "s", result.errorMessage.c_str());

    GVariant *resultVariant = g_variant_builder_end(&resultBuilder);

    return resultVariant;
}

GVariant *buildGVariantFromSegmentationResult(const SegmentationSession::SegmentationResult &result) {
    Json::Value maskValue = Json::arrayValue;
    for (const ai_engine::core_ai::vision::RunLengthEncodingRow &row : result.mask) {
        Json::Value rowJson = Json::arrayValue;
        rowJson.append(row.index);
        rowJson.append(row.count);
        maskValue.append(rowJson);
    }
    std::string maskJson = util::jsonToString(maskValue);
    GVariantBuilder builder;
    g_variant_builder_init(&builder, G_VARIANT_TYPE("(suuis)"));
    g_variant_builder_add(&builder, "s", maskJson.c_str());
    g_variant_builder_add(&builder, "u", result.width);
    g_variant_builder_add(&builder, "u", result.height);
    g_variant_builder_add(&builder, "i", convertErrorCode(result.error.getCode()));
    g_variant_builder_add(&builder, "s", result.error.getMessage().c_str());
    return g_variant_builder_end(&builder);
}

}  // namespace

CoreVisionService::CoreVisionService(GDBusConnection &connection, EngineSelector &engineSelector)
    : connection_(connection), engineSelector_(engineSelector) {
    exportSkeleton();
    const std::string protocolVersion = getProtocolVersion();
    if (protocolVersion.empty()) {
        LOGW("Protocol version is empty, use default protocol version: {}", kDefaultSdkVersion);
        setRuntimeProtocolVersion(kDefaultSdkVersion);
        LOGI("CoreVisionService protocol version: {}", kDefaultSdkVersion);
    } else {
        setRuntimeProtocolVersion(protocolVersion);
        LOGI("CoreVisionService protocol version: {}", protocolVersion);
    }
}

CoreVisionService::~CoreVisionService() {
    unexportSkeleton();
    for (auto &pair : subjectSegmentations_) {
        pair.second->destroy();
    }
    for (const auto &[sessionId, segmentationSession] : segmentationSessions_) {
        segmentationSession->destroy();
    }
    LOGI("CoreVisionService::~CoreVisionService");
}

void CoreVisionService::stop() {
    for (const auto &handlerId : handlerIds_) {
        g_signal_handler_disconnect(delegate_, handlerId);
    }
    handlerIds_.clear();
}

void CoreVisionService::exportSkeleton() {
    delegate_ = ai_runtime_core_vision_service_skeleton_new();
    connectSignal();

    GError *error = nullptr;

    isExported_ =
        g_dbus_interface_skeleton_export(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_, objectPath, &error);
    if (!isExported_) {
        g_printerr("Error creating server at address %s: %s\n", objectPath, error->message);
        g_error_free(error);
    }
}

bool CoreVisionService::onHandleInit(AiRuntimeCoreVisionService *service, GDBusMethodInvocation *invocation,
                                     const gchar *modelconfig, gpointer userData) {
    auto *visionService = static_cast<CoreVisionService *>(userData);
    if (!visionService) {
        LOGE("CoreVisionService is null.");
        ai_runtime_core_vision_service_complete_init(service, invocation, -1, AISDK_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    auto visionEngine = sharedVisionService->selectTextRecognitionEngine();

    if (!visionEngine) {
        LOGE("No vision engine found!");
        ai_runtime_core_vision_service_complete_init(service, invocation, -1, AISDK_MODEL_NOT_FOUND);
        return true;
    }

    int sessionId = sharedVisionService->generateRandomSessionId();

    LOGI("init engine {}", visionEngine->engineName(), sessionId);
    auto textRecognition = std::make_shared<TextRecognition>(std::move(visionEngine));

    if (not textRecognition->init()) {
        sharedVisionService->releaseRandomSessionId(sessionId);
        ai_runtime_core_vision_service_complete_init(service, invocation, -1, AISDK_MODEL_RUN_FAILED);
        return true;
    }
    sharedVisionService->textRecognitions_[sessionId] = textRecognition;

    ai_runtime_core_vision_service_complete_init(service, invocation, sessionId, AISDK_NO_ERROR);

    return true;
}

bool CoreVisionService::onHandleInitSubjectSegmentation(AiRuntimeCoreVisionService *service,
                                                        GDBusMethodInvocation *invocation, gpointer userData) {
    auto *visionService = static_cast<CoreVisionService *>(userData);
    if (!visionService) {
        ai_runtime_core_vision_service_complete_init_subject_segmentation(service, invocation, -1, AISDK_RUNTIME_ERROR);
        LOGE("CoreVisionService is null.");
        return true;
    }

    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    auto visionEngine = sharedVisionService->engineSelector_.selectCoreAiImageProcessEngine();
    if (!visionEngine) {
        ai_runtime_core_vision_service_complete_init_subject_segmentation(service, invocation, -1,
                                                                          AISDK_MODEL_NOT_FOUND);
        return true;
    }

    int sessionId = sharedVisionService->generateRandomSessionId();
    LOGI("init engine {}", visionEngine->engineName(), sessionId);
    auto subjectSegmentation = std::make_shared<SubjectSegmentation>(std::move(visionEngine));

    auto errorCode = subjectSegmentation->init();
    if (errorCode != AI_COMMON_SUCCESS) {
        sharedVisionService->releaseRandomSessionId(sessionId);
        LOGE("init subject segmentation failed.{}", errorCode);
        ai_runtime_core_vision_service_complete_init_subject_segmentation(service, invocation, -1, errorCode);
        return true;
    }

    sharedVisionService->subjectSegmentations_[sessionId] = subjectSegmentation;

    ai_runtime_core_vision_service_complete_init_subject_segmentation(service, invocation, sessionId,
                                                                      AI_COMMON_SUCCESS);

    return true;
}

bool CoreVisionService::onHandleDestroyTextRecognition(AiRuntimeCoreVisionService *service,
                                                       GDBusMethodInvocation *invocation, gint sessionId,
                                                       gpointer userData) {
    LOGD("onHandleDestroyTextRecognition");
    auto *visionService = static_cast<CoreVisionService *>(userData);
    if (visionService == nullptr) {
        LOGE("CoreVisionService is null.");
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    if (not sharedVisionService->isTextRecognitionSessionIdValid(sessionId)) {
        LOGE("No text recognition found for session ID: {}", sessionId);
        return true;
    }

    sharedVisionService->textRecognitions_.erase(sessionId);

    ai_runtime_core_vision_service_complete_destroy(service, invocation);
    LOGD("onHandleDestroyTextRecognition complete");
    return true;
}

bool CoreVisionService::onHandleTextRecognitionFromImageData(AiRuntimeCoreVisionService *service,
                                                             GDBusMethodInvocation *invocation, int sessionId,
                                                             const gchar *imageData, unsigned int imageDataLength,
                                                             gpointer userData) {
    // TODO 因为传递的是base64数据，所以不需要imageDataLength
    auto *visionService = static_cast<CoreVisionService *>(userData);
    if (!visionService) {
        TextRecognitionResult result;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No text recognition service found.";
        completeRecognizeTextFromImageData(result, service, invocation);
        LOGE("CoreVisionService is null.");
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    if (!sharedVisionService->isTextRecognitionSessionIdValid(sessionId)) {
        TextRecognitionResult result;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No text recognition found.";
        completeRecognizeTextFromImageData(result, service, invocation);
        LOGE("No text recognition found for session ID: {}", sessionId);
        return true;
    }

    sharedVisionService->textRecognitions_[sessionId]->recognizeTextFromImageData(
        imageData, [service, invocation](const TextRecognitionResult &result) {
            completeRecognizeTextFromImageData(result, service, invocation);
        });

    LOGI("start text recognition.");
    return true;
}

bool CoreVisionService::onHandleTextRecognitionFromImageDataWithRequestId(
    AiRuntimeCoreVisionService *service, GDBusMethodInvocation *invocation, int sessionId, const gchar *requestId,
    const gchar *imageData, unsigned int imageDataLength, gpointer userData) {
    auto *visionService = static_cast<CoreVisionService *>(userData);
    if (!visionService) {
        TextRecognitionResult result;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No text recognition service found.";
        completeRecognizeTextFromImageDataWithRequestId(result, service, invocation);
        LOGE("CoreVisionService is null.");
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    if (!sharedVisionService->isTextRecognitionSessionIdValid(sessionId)) {
        TextRecognitionResult result;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No text recognition found.";
        completeRecognizeTextFromImageDataWithRequestId(result, service, invocation);
        LOGE("No text recognition found for session ID: {}", sessionId);
        return true;
    }

    std::string base64Data = imageData == nullptr ? "" : imageData;
    std::string requestIdString = requestId == nullptr ? "" : requestId;
    sharedVisionService->textRecognitions_[sessionId]->recognizeTextFromImageData(
        base64Data, requestIdString, [service, invocation](const TextRecognitionResult &result) {
            completeRecognizeTextFromImageDataWithRequestId(result, service, invocation);
        });

    LOGI("start text recognition.");
    return true;
}

bool CoreVisionService::onHandleTextRecognitionFromImageFile(AiRuntimeCoreVisionService *service,
                                                             GDBusMethodInvocation *invocation, int sessionId,
                                                             const gchar *imageFilePath, gpointer userData) {
    auto *visionService = static_cast<CoreVisionService *>(userData);
    if (!visionService) {
        TextRecognitionResult result;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No text recognition service found.";
        completeRecognizeTextFromImageFile(result, service, invocation);
        LOGE("CoreVisionService is null.");
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    if (!sharedVisionService->isTextRecognitionSessionIdValid(sessionId)) {
        TextRecognitionResult result;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No text recognition found.";
        completeRecognizeTextFromImageFile(result, service, invocation);
        LOGE("No text recognition found for session ID: {}", sessionId);
        return true;
    }

    sharedVisionService->textRecognitions_[sessionId]->recognizeTextFromImageFile(
        imageFilePath, [service, invocation](const TextRecognitionResult &result) {
            completeRecognizeTextFromImageFile(result, service, invocation);
        });

    return true;
}

bool CoreVisionService::onHandleTextRecognitionFromImageFileWithRequestId(AiRuntimeCoreVisionService *service,
                                                                          GDBusMethodInvocation *invocation,
                                                                          int sessionId, const gchar *requestId,
                                                                          const gchar *imageFilePath,
                                                                          gpointer userData) {
    auto *visionService = static_cast<CoreVisionService *>(userData);
    if (!visionService) {
        TextRecognitionResult result;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No text recognition service found.";
        completeRecognizeTextFromImageFileWithRequestId(result, service, invocation);
        LOGE("CoreVisionService is null.");
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    if (!sharedVisionService->isTextRecognitionSessionIdValid(sessionId)) {
        TextRecognitionResult result;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No text recognition found.";
        completeRecognizeTextFromImageFileWithRequestId(result, service, invocation);
        LOGE("No text recognition found for session ID: {}", sessionId);
        return true;
    }

    std::string requestIdString = requestId == nullptr ? "" : requestId;
    sharedVisionService->textRecognitions_[sessionId]->recognizeTextFromImageFile(
        imageFilePath, requestIdString, [service, invocation](const TextRecognitionResult &result) {
            completeRecognizeTextFromImageFileWithRequestId(result, service, invocation);
        });

    return true;
}

bool CoreVisionService::onHandleSubjectSegmentationFromImageFileAsync(AiRuntimeCoreVisionService *service,
                                                                      GDBusMethodInvocation *invocation, int sessionId,
                                                                      const gchar *requestId,
                                                                      const gchar *imageFilePath, const gchar *config,
                                                                      gpointer userData) {
    auto *visionService = static_cast<CoreVisionService *>(userData);
    LOGI("onHandleSubjectSegmentationFromImageFileAsync sessionId: {}", sessionId);
    if (!visionService) {
        SubjectSegmentationResult result;
        result.requestId = requestId == nullptr ? "" : requestId;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No subject segmentation service found.";
        completeSubjectSegmentationFromImageFile(result, service, invocation);
        LOGE("CoreVisionService is null.");
        return true;
    }

    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    if (!sharedVisionService->isSubjectSegmentationSessionIdValid(sessionId)) {
        SubjectSegmentationResult result;
        result.requestId = requestId == nullptr ? "" : requestId;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No subject segmentation session found.";
        completeSubjectSegmentationFromImageFile(result, service, invocation);
        LOGE("No subject segmentation found for session ID: {}", sessionId);
        return true;
    }
    auto mode = sharedVisionService->parseModeConfig(config);
    if (mode == -1) {
        SubjectSegmentationResult result;
        result.requestId = requestId == nullptr ? "" : requestId;
        result.errorCode = AI_COMMON_INVALID_PARAMETER;
        result.errorMessage = "Invalid mode config. Supported modes are 0 (Portrait) and 1 (General).";
        completeSubjectSegmentationFromImageFile(result, service, invocation);
        LOGE("Invalid mode config provided: {}", config);
        return true;
    }
    std::string requestIdString = requestId == nullptr ? "" : requestId;
    sharedVisionService->subjectSegmentations_[sessionId]->subjectSegmentationFromImageFile(
        requestIdString, imageFilePath, static_cast<SubjectSegmentationMode>(mode),
        [service, invocation](const SubjectSegmentationResult &result) {
            completeSubjectSegmentationFromImageFile(result, service, invocation);
        });
    return true;
}

bool CoreVisionService::onHandleSubjectSegmentationFromImageDataAsync(AiRuntimeCoreVisionService *service,
                                                                      GDBusMethodInvocation *invocation, int sessionId,
                                                                      const gchar *requestId, const gchar *imageData,
                                                                      unsigned int imageDataLength, const gchar *config,
                                                                      gpointer userData) {
    auto *visionService = static_cast<CoreVisionService *>(userData);
    LOGI("onHandleSubjectSegmentationFromImageDataAsync sessionId: {}", sessionId);
    if (!visionService) {
        SubjectSegmentationResult result;
        result.requestId = requestId == nullptr ? "" : requestId;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No subject segmentation service found.";
        completeSubjectSegmentationFromImageData(result, service, invocation);
        LOGE("CoreVisionService is null.");
        return true;
    }

    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    if (!sharedVisionService->isSubjectSegmentationSessionIdValid(sessionId)) {
        SubjectSegmentationResult result;
        result.requestId = requestId == nullptr ? "" : requestId;
        result.errorCode = AISDK_RUNTIME_ERROR;
        result.errorMessage = "No subject segmentation session found.";
        completeSubjectSegmentationFromImageData(result, service, invocation);
        LOGE("No subject segmentation found for session ID: {}", sessionId);
        return true;
    }
    auto mode = sharedVisionService->parseModeConfig(config);
    if (mode == -1) {
        SubjectSegmentationResult result;
        result.requestId = requestId == nullptr ? "" : requestId;
        result.errorCode = AI_COMMON_INVALID_PARAMETER;
        result.errorMessage = "Invalid mode config. Supported modes are 0 (Portrait) and 1 (General).";
        completeSubjectSegmentationFromImageData(result, service, invocation);
        LOGE("Invalid mode config provided: {}", config);
        return true;
    }
    std::string requestIdString = requestId == nullptr ? "" : requestId;
    sharedVisionService->subjectSegmentations_[sessionId]->subjectSegmentationFromImageData(
        requestIdString, imageData, static_cast<SubjectSegmentationMode>(mode),
        [service, invocation](const SubjectSegmentationResult &result) {
            completeSubjectSegmentationFromImageData(result, service, invocation);
        });
    return true;
}

bool CoreVisionService::onHandleDestroySubjectSegmentation(AiRuntimeCoreVisionService *service,
                                                           GDBusMethodInvocation *invocation, gint sessionId,
                                                           gpointer userData) {
    LOGD("onHandleDestroySubjectSegmentation");
    auto *visionService = static_cast<CoreVisionService *>(userData);
    if (visionService == nullptr) {
        LOGE("CoreVisionService is null.");
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();

    if (not sharedVisionService->isSubjectSegmentationSessionIdValid(sessionId)) {
        LOGE("No subject segmentation found for session ID: {}", sessionId);
        return true;
    }

    sharedVisionService->subjectSegmentations_[sessionId]->destroy();
    sharedVisionService->subjectSegmentations_.erase(sessionId);

    ai_runtime_core_vision_service_complete_destroy_subject_segmentation(service, invocation);
    LOGD("onHandleDestroySubjectSegmentation complete");
    return true;
}

bool CoreVisionService::onHandleInitSegmentation(AiRuntimeCoreVisionService *service, GDBusMethodInvocation *invocation,
                                                 gpointer userData) {
    CoreVisionService *visionService = static_cast<CoreVisionService *>(userData);
    if (visionService == nullptr) {
        LOGE("CoreVisionService onHandleInitSegmentation visionService is null.");
        ai_runtime_core_vision_service_complete_init_segmentation(service, invocation, -1, AI_COMMON_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();
    std::unique_ptr engine = sharedVisionService->engineSelector_.selectCoreAiImageProcessEngine();
    if (engine == nullptr) {
        ai_runtime_core_vision_service_complete_init_segmentation(service, invocation, -1, AI_COMMON_MODEL_NOT_FOUND);
        return true;
    }
    int sessionId = sharedVisionService->generateRandomSessionId();
    LOGI("Init engine {} {}", engine->engineName(), sessionId);
    std::shared_ptr session = std::make_shared<SegmentationSession>(std::move(engine));
    auto result = session->init();
    if (!result.success) {
        sharedVisionService->releaseRandomSessionId(sessionId);
        LOGE("Init segmentation failed: {}.", result.error.getCode());
        ai_runtime_core_vision_service_complete_init_segmentation(service, invocation, -1, result.error.getCode());
        return true;
    }
    session->setSegmentationResultCallback(
        [sharedVisionService, sessionId](const SegmentationSession::SegmentationResult &result) {
            sharedVisionService->sendSegmentationResult(sessionId, result);
        });
    sharedVisionService->segmentationSessions_[sessionId] = std::move(session);
    ai_runtime_core_vision_service_complete_init_segmentation(service, invocation, sessionId, AI_COMMON_SUCCESS);
    return true;
}

bool CoreVisionService::onHandleDestroySegmentation(AiRuntimeCoreVisionService *service,
                                                    GDBusMethodInvocation *invocation, gint sessionId,
                                                    gpointer userData) {
    CoreVisionService *visionService = static_cast<CoreVisionService *>(userData);
    if (visionService == nullptr) {
        LOGE("CoreVisionService onHandleDestroySegmentation visionService is null.");
        ai_runtime_core_vision_service_complete_destroy_segmentation(service, invocation);
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();
    if (not sharedVisionService->isSegmentationSessionIdValid(sessionId)) {
        LOGE("No segmentation found for session ID: {}", sessionId);
        ai_runtime_core_vision_service_complete_destroy_segmentation(service, invocation);
        return true;
    }
    sharedVisionService->segmentationSessions_[sessionId]->destroy();
    sharedVisionService->segmentationSessions_.erase(sessionId);
    ai_runtime_core_vision_service_complete_destroy_segmentation(service, invocation);
    return true;
}

bool CoreVisionService::onHandleEncodeSegmentationFromImageFile(AiRuntimeCoreVisionService *service,
                                                                GDBusMethodInvocation *invocation, gint sessionId,
                                                                const gchar *imageFilePath, gpointer userData) {
    CoreVisionService *visionService = static_cast<CoreVisionService *>(userData);
    if (visionService == nullptr) {
        LOGE("CoreVisionService onHandleSegmentationEncodeFromImageFile visionService is null.");
        ai_runtime_core_vision_service_complete_encode_segmentation_from_image_file(service, invocation,
                                                                                    AI_COMMON_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();
    if (not sharedVisionService->isSegmentationSessionIdValid(sessionId)) {
        LOGE("No segmentation found for session ID: {}", sessionId);
        ai_runtime_core_vision_service_complete_encode_segmentation_from_image_file(service, invocation,
                                                                                    AI_COMMON_INVALID_SESSION);
        return true;
    }
    SegmentationSession::SegmentationRequest request{};
    request.type = SegmentationSession::SegmentationRequest::Type::EncodeFromImagePath;
    request.imagePath = imageFilePath;
    request.onFinished = [service, invocation](int errorCode) {
        ai_runtime_core_vision_service_complete_encode_segmentation_from_image_file(service, invocation,
                                                                                    convertErrorCode(errorCode));
    };
    sharedVisionService->segmentationSessions_[sessionId]->submitSegmentationRequest(request);
    return true;
}

bool CoreVisionService::onHandleEncodeSegmentationFromImageData(AiRuntimeCoreVisionService *service,
                                                                GDBusMethodInvocation *invocation, gint sessionId,
                                                                const gchar *base64ImageData, gpointer userData) {
    CoreVisionService *visionService = static_cast<CoreVisionService *>(userData);
    if (visionService == nullptr) {
        LOGE("CoreVisionService onHandleEncodeSegmentationFromImageData visionService is null.");
        ai_runtime_core_vision_service_complete_encode_segmentation_from_base64_image_data(service, invocation,
                                                                                           AI_COMMON_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();
    if (not sharedVisionService->isSegmentationSessionIdValid(sessionId)) {
        LOGE("No segmentation found for session ID: {}", sessionId);
        ai_runtime_core_vision_service_complete_encode_segmentation_from_base64_image_data(service, invocation,
                                                                                           AI_COMMON_INVALID_SESSION);
        return true;
    }
    SegmentationSession::SegmentationRequest request{};
    request.type = SegmentationSession::SegmentationRequest::Type::EncodeFromImageData;
    request.imageData = base64ImageData;
    request.onFinished = [service, invocation](int errorCode) {
        ai_runtime_core_vision_service_complete_encode_segmentation_from_base64_image_data(service, invocation,
                                                                                           convertErrorCode(errorCode));
    };
    sharedVisionService->segmentationSessions_[sessionId]->submitSegmentationRequest(request);
    return true;
}

bool CoreVisionService::onHandleClearSegmentationCache(AiRuntimeCoreVisionService *service,
                                                       GDBusMethodInvocation *invocation, gint sessionId,
                                                       gpointer userData) {
    CoreVisionService *visionService = static_cast<CoreVisionService *>(userData);
    if (visionService == nullptr) {
        LOGE("CoreVisionService onHandleClearSegmentationCache visionService is null.");
        ai_runtime_core_vision_service_complete_clear_segmentation_cache(service, invocation, AI_COMMON_RUNTIME_ERROR);
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();
    if (not sharedVisionService->isSegmentationSessionIdValid(sessionId)) {
        LOGE("No segmentation found for session ID: {}", sessionId);
        ai_runtime_core_vision_service_complete_clear_segmentation_cache(service, invocation,
                                                                         AI_COMMON_INVALID_SESSION);
        return true;
    }
    int errorCode = sharedVisionService->segmentationSessions_[sessionId]->clearCache();
    ai_runtime_core_vision_service_complete_clear_segmentation_cache(service, invocation, errorCode);
    return true;
}

bool CoreVisionService::onHandleRunSegmentation(AiRuntimeCoreVisionService *service, GDBusMethodInvocation *invocation,
                                                gint sessionId, GVariant *pointsVariant, guint32 minArea,
                                                gpointer userData) {
    CoreVisionService *visionService = static_cast<CoreVisionService *>(userData);
    if (visionService == nullptr) {
        LOGE("CoreVisionService onHandleRunSegmentation visionService is null.");
        ai_runtime_core_vision_service_complete_run_segmentation(service, invocation);
        return true;
    }
    std::shared_ptr sharedVisionService = visionService->getSharedObject();
    if (not sharedVisionService->isSegmentationSessionIdValid(sessionId)) {
        LOGE("No segmentation found for session ID: {}", sessionId);
        ai_runtime_core_vision_service_complete_run_segmentation(service, invocation);
        return true;
    }
    SegmentationSession::SegmentationPoints points{};
    GVariantIter iter;
    g_variant_iter_init(&iter, pointsVariant);
    for (uint32_t x, y; g_variant_iter_loop(&iter, "(uu)", &x, &y);) {
        points.push_back({x, y, true});
    }
    SegmentationSession::SegmentationRequest request{};
    request.type = SegmentationSession::SegmentationRequest::Type::RunSegmentation;
    request.points = std::move(points);
    request.minArea = minArea;
    sharedVisionService->segmentationSessions_[sessionId]->submitSegmentationRequest(request);
    ai_runtime_core_vision_service_complete_run_segmentation(service, invocation);
    return true;
}

void CoreVisionService::completeRecognizeTextFromImageFile(const TextRecognitionResult &result,
                                                           AiRuntimeCoreVisionService *service,
                                                           GDBusMethodInvocation *invocation) {
    GVariant *resultVariant = buildGVariantFromTextRecognitionResult(result);
    ai_runtime_core_vision_service_complete_recognize_text_from_image_file(service, invocation, resultVariant);
}

void CoreVisionService::completeRecognizeTextFromImageFileWithRequestId(const TextRecognitionResult &result,
                                                                        AiRuntimeCoreVisionService *service,
                                                                        GDBusMethodInvocation *invocation) {
    GVariant *resultVariant = buildGVariantFromTextRecognitionResultWithRequestId(result);
    ai_runtime_core_vision_service_complete_recognize_text_from_image_file_with_request_id(service, invocation,
                                                                                           resultVariant);
}

void CoreVisionService::completeRecognizeTextFromImageData(const TextRecognitionResult &result,
                                                           AiRuntimeCoreVisionService *service,
                                                           GDBusMethodInvocation *invocation) {
    GVariant *resultVariant = buildGVariantFromTextRecognitionResult(result);
    ai_runtime_core_vision_service_complete_recognize_text_from_image_data(service, invocation, resultVariant);
}

void CoreVisionService::completeRecognizeTextFromImageDataWithRequestId(const TextRecognitionResult &result,
                                                                        AiRuntimeCoreVisionService *service,
                                                                        GDBusMethodInvocation *invocation) {
    GVariant *resultVariant = buildGVariantFromTextRecognitionResultWithRequestId(result);
    ai_runtime_core_vision_service_complete_recognize_text_from_image_data_with_request_id(service, invocation,
                                                                                           resultVariant);
}

void CoreVisionService::completeSubjectSegmentationFromImageFile(const SubjectSegmentationResult &result,
                                                                 AiRuntimeCoreVisionService *service,
                                                                 GDBusMethodInvocation *invocation) {
    GVariant *resultVariant = buildGVariantFromSubjectSegmentationResult(result);
    ai_runtime_core_vision_service_complete_subject_segmentation_from_image_file_async(service, invocation,
                                                                                       resultVariant);
}

void CoreVisionService::completeSubjectSegmentationFromImageData(const SubjectSegmentationResult &result,
                                                                 AiRuntimeCoreVisionService *service,
                                                                 GDBusMethodInvocation *invocation) {
    GVariant *resultVariant = buildGVariantFromSubjectSegmentationResult(result);
    ai_runtime_core_vision_service_complete_subject_segmentation_from_image_data_async(service, invocation,
                                                                                       resultVariant);
}

void CoreVisionService::connectSignal() {
    gulong handlerId{};
    handlerId = g_signal_connect(delegate_, "handle-init", G_CALLBACK(onHandleInit), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-init-subject-segmentation",
                                 G_CALLBACK(onHandleInitSubjectSegmentation), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-destroy", G_CALLBACK(onHandleDestroyTextRecognition), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-recognize-text-from-image-file",
                                 G_CALLBACK(onHandleTextRecognitionFromImageFile), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-recognize-text-from-image-file-with-request-id",
                                 G_CALLBACK(onHandleTextRecognitionFromImageFileWithRequestId), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-recognize-text-from-image-data",
                                 G_CALLBACK(onHandleTextRecognitionFromImageData), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-recognize-text-from-image-data-with-request-id",
                                 G_CALLBACK(onHandleTextRecognitionFromImageDataWithRequestId), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-subject-segmentation-from-image-file-async",
                                 G_CALLBACK(onHandleSubjectSegmentationFromImageFileAsync), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-subject-segmentation-from-image-data-async",
                                 G_CALLBACK(onHandleSubjectSegmentationFromImageDataAsync), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-destroy-subject-segmentation",
                                 G_CALLBACK(onHandleDestroySubjectSegmentation), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-init-segmentation", G_CALLBACK(onHandleInitSegmentation), this);
    handlerIds_.push_back(handlerId);

    handlerId =
        g_signal_connect(delegate_, "handle-destroy-segmentation", G_CALLBACK(onHandleDestroySegmentation), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-encode-segmentation-from-image-file",
                                 G_CALLBACK(onHandleEncodeSegmentationFromImageFile), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-encode-segmentation-from-base64-image-data",
                                 G_CALLBACK(onHandleEncodeSegmentationFromImageData), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-clear-segmentation-cache",
                                 G_CALLBACK(onHandleClearSegmentationCache), this);
    handlerIds_.push_back(handlerId);

    handlerId = g_signal_connect(delegate_, "handle-run-segmentation", G_CALLBACK(onHandleRunSegmentation), this);
    handlerIds_.push_back(handlerId);
}

void CoreVisionService::unexportSkeleton() {
    if (delegate_ == nullptr) {
        return;
    }

    if (isExported_) {
        g_dbus_interface_skeleton_unexport_from_connection(G_DBUS_INTERFACE_SKELETON(delegate_), &connection_);
    }
    g_object_unref(delegate_);
}

std::string CoreVisionService::getProtocolVersion() {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, get protocol version failed.");
        return kDefaultSdkVersion;
    }

    GDBusInterfaceInfo *interfaceInfo = ai_runtime_core_vision_service_interface_info();
    if (interfaceInfo == nullptr) {
        LOGW("Interface info is null, use default protocol version.");
        return kDefaultSdkVersion;
    }
    if (interfaceInfo->annotations == nullptr) {
        LOGW("Protocol version annotation not found, use default protocol version.");
        return kDefaultSdkVersion;
    }

    for (GDBusAnnotationInfo **annotation = interfaceInfo->annotations; annotation && *annotation; ++annotation) {
        if ((*annotation)->key == nullptr || (*annotation)->value == nullptr) {
            continue;
        }
        if (g_strcmp0((*annotation)->key, kSdkVersionAnnotation) == 0) {
            return (*annotation)->value;
        }
    }
    return kDefaultSdkVersion;
}

void CoreVisionService::setRuntimeProtocolVersion(const std::string &version) {
    if (delegate_ == nullptr) {
        LOGE("Delegate is null, set runtime version failed.");
        return;
    }
    ai_runtime_core_vision_service_set_runtime_version(delegate_, version.c_str());
}

int CoreVisionService::generateRandomSessionId() {
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

void CoreVisionService::releaseRandomSessionId(int sessionId) { sessionIdSet_.erase(sessionId); }

bool CoreVisionService::isTextRecognitionSessionIdValid(int sessionId) {
    return textRecognitions_.find(sessionId) != textRecognitions_.end();
}

bool CoreVisionService::isSubjectSegmentationSessionIdValid(int sessionId) {
    return subjectSegmentations_.find(sessionId) != subjectSegmentations_.end();
}

bool CoreVisionService::isSegmentationSessionIdValid(int sessionId) {
    return segmentationSessions_.find(sessionId) != segmentationSessions_.end();
}

int CoreVisionService::parseModeConfig(const std::string &jsonString) {
    if (jsonString.empty()) {
        return -1;
    }

    Json::Value root = util::stringToJson(jsonString);

    if (root.isNull()) {
        return -1;
    }
    if (root.isMember("mode") && root["mode"].isInt()) {
        auto mode = root["mode"].asInt();
        switch (mode) {
            case 0:
                return SUBJECT_SEGMENTATION_MODE_PORTRAIT;
            case 1:
                return SUBJECT_SEGMENTATION_MODE_GENERAL;
            default:
                return -1;
        }
    }
    return -1;
}

DeployType CoreVisionService::parseDeployType(const std::string &typeStr) {
    if (typeStr == "Local") {
        return DeployType::Local;
    }
    if (typeStr == "PrivateCloud") {
        return DeployType::Custom;
    }
    if (typeStr == "PublicCloud") {
        return DeployType::PublicCloud;
    }
    return DeployType::Unknown;  // default
}

const std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine>
CoreVisionService::selectTextRecognitionEngine() {
    return engineSelector_.selectTextRecognitionEngine();
}

void CoreVisionService::sendSegmentationResult(int sessionId, const SegmentationSession::SegmentationResult &result) {
    LOGD("{} {}", result.height, result.width);
    std::string interfaceName = std::string(interface) + std::to_string(sessionId);
    GError *error{nullptr};
    GVariant *variant = buildGVariantFromSegmentationResult(result);
    g_dbus_connection_emit_signal(&connection_, interface, objectPath, interfaceName.c_str(), segmentationResultSignal,
                                  variant, &error);
    if (error != nullptr) {
        g_error_free(error);
    }
}

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

#include "corevisionserviceproxy.h"

#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "_segmentationresult.h"
#include "_subjectsegmentationresult.h"
#include "_textrecognitionconfig.h"
#include "_textrecognitionresult.h"
#include "logger.h"
#include "util.h"

namespace kyai {
namespace core {
namespace vision {
const std::string serverUnixPath =
    "unix:path=/tmp/.kylin-ai-runtime-unix/" + std::to_string(getuid()) + "/core-vision.sock";
const char *objectPath = "/com/kylin/AiRuntime/CoreVision";
const char *interface = "com.kylin.AiRuntime.CoreVision";
const char *segmentationResultSignal = "SegmentationResultSignal";

void dissectRecognitionResultCommon(GVariantIter *iter, TextRecognitionResult *recognitionResult) {
    GVariant *text = g_variant_iter_next_value(iter);
    GVariant *lines = g_variant_iter_next_value(iter);
    GVariant *errorCode = g_variant_iter_next_value(iter);
    GVariant *errorMessage = g_variant_iter_next_value(iter);

    if (!text || !lines || !errorCode || !errorMessage) {
        LOGE("recognition result error: result is nullptr!");
        if (text) g_variant_unref(text);
        if (lines) g_variant_unref(lines);
        if (errorCode) g_variant_unref(errorCode);
        if (errorMessage) g_variant_unref(errorMessage);
        return;
    }

    const gchar *resultText = g_variant_get_string(text, nullptr);
    recognitionResult->allText = std::string(resultText);
    g_variant_unref(text);

    GVariantIter linesIter;
    g_variant_iter_init(&linesIter, lines);

    while (GVariant *lineVariant = g_variant_iter_next_value(&linesIter)) {
        TextLine *line = new TextLine{};
        GVariant *pointsValue;
        double conf;
        gchar *oneTextLine;

        g_variant_get(lineVariant, "(s@a(ii)d)", &oneTextLine, &pointsValue, &conf);
        if (!oneTextLine || !pointsValue) {
            LOGE("recognition result error: result is nullptr!");
            g_variant_unref(lineVariant);
            g_variant_unref(lines);
            delete line;
            return;
        }

        GVariantIter pointsIter;
        g_variant_iter_init(&pointsIter, pointsValue);

        while (GVariant *pointVariant = g_variant_iter_next_value(&pointsIter)) {
            PixelPoint point;
            g_variant_get(pointVariant, "(ii)", &point.x, &point.y);
            if (point.x < 0 || point.y < 0) {
                LOGE("recognition result point error: pixel point < 0!");
                point.x = 0;
                point.y = 0;
            }
            line->points.push_back(point);
            g_variant_unref(pointVariant);
        }
        g_variant_unref(pointsValue);
        line->text = oneTextLine;
        recognitionResult->lines.push_back(line);
        g_free(oneTextLine);
        g_variant_unref(lineVariant);
    }
    g_variant_unref(lines);

    int resultErrorCode = g_variant_get_int32(errorCode);
    recognitionResult->errorCode = resultErrorCode;
    g_variant_unref(errorCode);

    const gchar *resultErrorMessage = g_variant_get_string(errorMessage, nullptr);
    recognitionResult->errorMessage = std::string(resultErrorMessage);
    g_variant_unref(errorMessage);
}

void dissectRecognitionResult(GVariant *parameters, TextRecognitionResult *recognitionResult) {
    if (parameters == nullptr || recognitionResult == nullptr) {
        return;
    }
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);
    dissectRecognitionResultCommon(&iter, recognitionResult);
}

void dissectSubjectSegmentationResult(GVariant *parameters, SubjectSegmentationResult *subjectSegmentationResult) {
    if (!parameters || !subjectSegmentationResult) {
        LOGE(
            "subjectSegmentation result error: parameters or result is "
            "nullptr");
        return;
    }

    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *requestIdVar = g_variant_iter_next_value(&iter);
    GVariant *imageFormatVar = g_variant_iter_next_value(&iter);
    GVariant *imageBase64Var = g_variant_iter_next_value(&iter);
    GVariant *maskBase64Var = g_variant_iter_next_value(&iter);
    GVariant *imageWidthVar = g_variant_iter_next_value(&iter);
    GVariant *imageHeightVar = g_variant_iter_next_value(&iter);
    GVariant *edgePointsVar = g_variant_iter_next_value(&iter);
    GVariant *errorCodeVar = g_variant_iter_next_value(&iter);
    GVariant *errorMessageVar = g_variant_iter_next_value(&iter);

    if (!requestIdVar || !imageFormatVar || !imageBase64Var || !maskBase64Var || !imageWidthVar || !imageHeightVar ||
        !edgePointsVar || !errorCodeVar || !errorMessageVar) {
        LOGE("subjectSegmentation result error: result is nullptr!");
        return;
    }

    const gchar *requestId = g_variant_get_string(requestIdVar, nullptr);
    if (requestId) {
        subjectSegmentationResult->requestId = requestId;
    } else {
        subjectSegmentationResult->requestId.clear();
    }
    g_variant_unref(requestIdVar);

    // 解析掩码格式
    const gchar *format = g_variant_get_string(imageFormatVar, nullptr);
    if (format) {
        subjectSegmentationResult->format = format;
    } else {
        subjectSegmentationResult->format.clear();
    }
    g_variant_unref(imageFormatVar);

    // 解析图片数据
    const gchar *imageData = g_variant_get_string(imageBase64Var, nullptr);
    if (imageData) {
        subjectSegmentationResult->imageData = util::base64Decode(imageData);
    }
    g_variant_unref(imageBase64Var);

    // 解析掩码数据
    const gchar *maskData = g_variant_get_string(maskBase64Var, nullptr);
    if (maskData) {
        auto decodedMaskData = util::base64Decode(maskData);
        auto maskSize = decodedMaskData.size();

        subjectSegmentationResult->maskDataPtr = std::make_unique<SegmentationMaskData>();
        subjectSegmentationResult->maskDataPtr->mask_data = new unsigned char[maskSize];
        std::copy(decodedMaskData.begin(), decodedMaskData.end(), subjectSegmentationResult->maskDataPtr->mask_data);
        subjectSegmentationResult->maskDataPtr->mask_length = maskSize;
    }
    g_variant_unref(maskBase64Var);

    // 解析图片宽高
    subjectSegmentationResult->width = g_variant_get_int32(imageWidthVar);
    subjectSegmentationResult->height = g_variant_get_int32(imageHeightVar);
    subjectSegmentationResult->maskDataPtr->mask_width = subjectSegmentationResult->width;
    subjectSegmentationResult->maskDataPtr->mask_height = subjectSegmentationResult->height;

    g_variant_unref(imageWidthVar);
    g_variant_unref(imageHeightVar);

    // 解析边缘点组
    const gchar *edgePointsJson = g_variant_get_string(edgePointsVar, nullptr);
    if (edgePointsJson) {
        Json::Value root;
        if (util::parseJsonString(edgePointsJson, root) && root.isArray()) {
            for (const auto &groupVal : root) {
                SubjectSegmentationPointGroup group{};
                group.points = nullptr;
                group.point_count = 0;
                if (groupVal.isArray()) {
                    int count = groupVal.size();
                    if (count > 0) {
                        group.points = new PixelPoint[count];
                        group.point_count = count;
                        for (int j = 0; j < count; ++j) {
                            group.points[j].x = groupVal[j]["x"].asInt();
                            group.points[j].y = groupVal[j]["y"].asInt();
                        }
                    }
                }
                subjectSegmentationResult->pointGroups.push_back(group);
            }
        }
    }
    g_variant_unref(edgePointsVar);

    // 错误码和错误信息
    subjectSegmentationResult->errorCode = g_variant_get_int32(errorCodeVar);
    g_variant_unref(errorCodeVar);

    const gchar *errMsg = g_variant_get_string(errorMessageVar, nullptr);
    if (errMsg) {
        subjectSegmentationResult->errorMessage = errMsg;
    }
    g_variant_unref(errorMessageVar);
}

std::optional<Json::Value> stringToJson(const std::string &string) {
    Json::Value value;
    if (Json::Reader reader; reader.parse(string, value)) {
        return value;
    } else {
        return std::nullopt;
    }
}

SegmentationResult dissectSegmentationResult(GVariant *parameters) {
    if (parameters == nullptr) {
        SegmentationResult result{};
        result.errorCode = AI_COMMON_RUNTIME_ERROR;
        result.errorMessage = "Callback parameters is nullptr!";
        return result;
    }
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);
    GVariant *masksVariant = g_variant_iter_next_value(&iter);
    GVariant *widthVariant = g_variant_iter_next_value(&iter);
    GVariant *heightVariant = g_variant_iter_next_value(&iter);
    GVariant *errorCodeVariant = g_variant_iter_next_value(&iter);
    GVariant *errorMessageVariant = g_variant_iter_next_value(&iter);
    if (masksVariant == nullptr || widthVariant == nullptr || heightVariant == nullptr || errorCodeVariant == nullptr ||
        errorMessageVariant == nullptr) {
        SegmentationResult result{};
        result.errorCode = AI_COMMON_RUNTIME_ERROR;
        result.errorMessage = "Callback variant is nullptr!";
        return result;
    }
    std::string masks = g_variant_get_string(masksVariant, nullptr);
    uint32_t width = g_variant_get_uint32(widthVariant);
    uint32_t height = g_variant_get_uint32(heightVariant);
    int errorCode = g_variant_get_int32(errorCodeVariant);
    std::string errorMessage = g_variant_get_string(errorMessageVariant, nullptr);
    SegmentationResult result{};
    auto optional = stringToJson(masks);
    if (!optional.has_value()) {
        if (errorCode == AI_COMMON_SUCCESS) {
            result.errorCode = AI_COMMON_RUNTIME_ERROR;
            result.errorMessage = "Masks is not a json string!";
        }
        return result;
    }
    Json::Value maskJson = optional.value();
    std::vector<uint8_t> data(width * height, 0);
    for (const Json::Value &rowJson : maskJson) {
        uint32_t index = rowJson[0].asUInt();
        uint32_t count = rowJson[1].asUInt();
        for (uint32_t i = index; i < index + count; ++i) {
            data[i] = 255;
        }
    }
    result.data_ = data;
    result.mask_.mask_height = int(height);
    result.mask_.mask_width = int(width);
    result.errorCode = errorCode;
    result.errorMessage = errorMessage;
    return result;
}

void dissectRecognitionResultWithRequestId(GVariant *parameters, TextRecognitionResult *recognitionResult) {
    if (parameters == nullptr || recognitionResult == nullptr) {
        return;
    }
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant *requestIdVar = g_variant_iter_next_value(&iter);
    if (!requestIdVar) {
        LOGE("recognition result error: result is nullptr!");
        if (requestIdVar) g_variant_unref(requestIdVar);
        return;
    }

    const gchar *requestId = g_variant_get_string(requestIdVar, nullptr);
    if (requestId) {
        recognitionResult->requestId = requestId;
    }
    g_variant_unref(requestIdVar);
    dissectRecognitionResultCommon(&iter, recognitionResult);
}

void CoreVisionServiceProxy::handleRecognitionResultCallback(CoreVisionServiceProxy *proxy, GVariant *result) {
    if (proxy == nullptr) {
        if (result) {
            g_variant_unref(result);
        }
        return;
    }

    if (proxy->recognitionCallback_ != nullptr) {
        TextRecognitionResult textRecognitionResult = {};
        if (g_variant_is_of_type(result, G_VARIANT_TYPE("(ssa(sa(ii)d)is)"))) {
            dissectRecognitionResultWithRequestId(result, &textRecognitionResult);
        } else {
            dissectRecognitionResult(result, &textRecognitionResult);
        }
        g_variant_unref(result);
        proxy->recognitionCallback_(&textRecognitionResult, proxy->recognitionUserData_);
        return;
    }

    LOGE("Recognizing callback is not set!");
    g_variant_unref(result);
}

CoreVisionServer &CoreVisionServer::getInstance() {
    static CoreVisionServer instance;
    return instance;
}

CoreVisionServer::~CoreVisionServer() { destroy(); }

void CoreVisionServer::destroy() const {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr) {
        g_object_unref(connection_);
    }
}

void CoreVisionServer::init() { init(serverUnixPath); }

void CoreVisionServer::init(const std::string &unixPath) {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr && !g_dbus_connection_is_closed(connection_)) {
        return;
    }
    GError *error = nullptr;
    connection_ = g_dbus_connection_new_for_address_sync(
        unixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, nullptr, &error);
    if (connection_ == nullptr) {
        g_printerr("Error connecting to D-Bus address %s: %s\n", unixPath.c_str(), error->message);
        g_error_free(error);
    }
}

void CoreVisionServiceProxy::setRecognitionResultCallback(TextRecognitionResultCallback callback, void *userData) {
    recognitionCallback_ = callback;
    recognitionUserData_ = userData;
}

void CoreVisionServiceProxy::setReconnectedCallback(ReconnectedCallback callback) {
    reconnectedCallback_ = std::move(callback);
}

void CoreVisionServiceProxy::setSubjectSegmentationResultCallback(SubjectSegmentationResultCallback callback,
                                                                  void *userData) {
    subjectSegmentationCallback_ = callback;
    subjectSegmentationUserData_ = userData;
}

CoreVisionServiceProxy::~CoreVisionServiceProxy() {
    internalLoopRequested_.store(false, std::memory_order_release);
    stopInternalEventLoop();
    if (proxy_) {
        g_object_unref(proxy_);
    }

    recognitionCallback_ = nullptr;
    reconnectedCallback_ = nullptr;
    subjectSegmentationCallback_ = nullptr;

    {
        std::lock_guard<std::mutex> lock(encodeCallbackMutex_);
        while (!encodeCallbackQueue_.empty()) {
            auto &info = encodeCallbackQueue_.front();
            if (info.callback) {
                info.callback(AI_COMMON_TASK_CANCELLED, info.userData);
            }
            encodeCallbackQueue_.pop();
        }
    }
}

bool CoreVisionServiceProxy::connectToServer() {
    CoreVisionServer::getInstance().init();
    if (!CoreVisionServer::getInstance().available()) {
        g_printerr(
            "Error creating core vision server proxy: Server proxy "
            "connection is unavailable.\n");
        return false;
    }

    if (proxy_ != nullptr) {
        g_object_unref(proxy_);
        proxy_ = nullptr;
    }

    GError *error = nullptr;
    auto *connection = CoreVisionServer::getInstance().getConnection();
    proxy_ = ai_runtime_core_vision_service_proxy_new_sync(connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath,
                                                           nullptr, &error);
    if (proxy_ == nullptr) {
        LOGE("Error creating core vision service proxy: %s\n", error->message);
        g_error_free(error);
        return false;
    }

    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(proxy_), 5 * 60 * 1000);
    refreshRuntimeProtocolVersion();
    return true;
}

int CoreVisionServiceProxy::initEngine(const char *config, int *sessionId) {
    int errorCode = AISDK_NO_ERROR;
    GError *error = nullptr;
    bool success =
        ai_runtime_core_vision_service_call_init_sync(proxy_, config, sessionId, &errorCode, nullptr, &error);
    if (not success) {
        if (error) {
            LOGE("Error calling init: {}:{}", error->code, error->message);
            g_error_free(error);
            return AISDK_RUNTIME_ERROR;
        }
    }

    return errorCode;
}

void CoreVisionServiceProxy::initSubjectSegmentationEngine(const SubjectSegmentationInitCallback &callback,
                                                           void *userData) {
    if (proxy_ == nullptr) {
        LOGE("Proxy not exist!");
        return;
    }
    if (!ensureProtocolSupportedForApi("init_subject_segmentation", nullptr)) {
        if (callback) {
            int errcode = AI_COMMON_RUNTIME_OUTDATED;
            callback(-1, errcode, userData);
        }
        return;
    }
    subjectSegmentationInitCallback_ = callback;
    subjectSegmentationInitUserData_ = userData;
    ai_runtime_core_vision_service_call_init_subject_segmentation(proxy_, nullptr, onSubjectSegmentationInitCallback,
                                                                  this);
}

void CoreVisionServiceProxy::destroyTextRecognition(int sessionId) {
    ai_runtime_core_vision_service_call_destroy_sync(proxy_, sessionId, nullptr, nullptr);
}

void CoreVisionServiceProxy::destroySubjectSegmentation(int sessionId) {
    if (proxy_ == nullptr) {
        LOGE("Proxy not exist!");
        return;
    }
    if (!ensureProtocolSupportedForApi("destroy_subject_segmentation", nullptr)) {
        return;
    }
    ai_runtime_core_vision_service_call_destroy_subject_segmentation_sync(proxy_, sessionId, nullptr, nullptr);
}

void CoreVisionServiceProxy::recognizeTextFromImageDataAsync(int sessionId, const char *data,
                                                             unsigned int imageDataLength) {
    if (proxy_ == nullptr) {
        LOGE("Proxy not exist!");
        return;
    }
    std::string imageData(data, imageDataLength);
    std::string base64ImageData = util::base64Encode(imageData);
    ai_runtime_core_vision_service_call_recognize_text_from_image_data(proxy_, sessionId, base64ImageData.c_str(),
                                                                       base64ImageData.size(), nullptr,
                                                                       onRecognitionResultFromImageDataCallback, this);
}

void CoreVisionServiceProxy::recognizeTextFromImageDataAsync(int sessionId, const char *requestId, const char *data,
                                                             unsigned int imageDataLength) {
    if (proxy_ == nullptr) {
        LOGE("Proxy not exist!");
        return;
    }
    if (!ensureProtocolSupportedForApi("recognize_text_from_image_data_with_request_id", nullptr)) {
        if (recognitionCallback_) {
            TextRecognitionResult result{
                .errorCode = AI_COMMON_RUNTIME_OUTDATED,
                .errorMessage = "Error calling recognize_text_from_image_data_with_request_id: runtime outdated"};
            if (requestId) {
                result.requestId = requestId;
            }
            recognitionCallback_(&result, recognitionUserData_);
        }
        return;
    }
    std::string imageData(data, imageDataLength);
    std::string base64ImageData = util::base64Encode(imageData);
    ai_runtime_core_vision_service_call_recognize_text_from_image_data_with_request_id(
        proxy_, sessionId, requestId, base64ImageData.c_str(), base64ImageData.size(), nullptr,
        onRecognitionResultFromImageDataWithRequestIdCallback, this);
}

void CoreVisionServiceProxy::recognizeTextFromImageFileAsync(int sessionId, const char *imageFile) {
    if (proxy_ == nullptr) {
        LOGE("Proxy not exist!");
        return;
    }
    ai_runtime_core_vision_service_call_recognize_text_from_image_file(proxy_, sessionId, imageFile, nullptr,
                                                                       onRecognitionResultFromImageFileCallback, this);
}

void CoreVisionServiceProxy::recognizeTextFromImageFileAsync(int sessionId, const char *requestId,
                                                             const char *imageFile) {
    if (proxy_ == nullptr) {
        LOGE("Proxy not exist!");
        return;
    }
    if (!ensureProtocolSupportedForApi("recognize_text_from_image_file_with_request_id", nullptr)) {
        if (recognitionCallback_) {
            TextRecognitionResult result{
                .errorCode = AI_COMMON_RUNTIME_OUTDATED,
                .errorMessage = "Error calling recognize_text_from_image_file_with_request_id: runtime outdated"};
            if (requestId) {
                result.requestId = requestId;
            }
            recognitionCallback_(&result, recognitionUserData_);
        }
        return;
    }
    ai_runtime_core_vision_service_call_recognize_text_from_image_file_with_request_id(
        proxy_, sessionId, requestId, imageFile, nullptr, onRecognitionResultFromImageFileWithRequestIdCallback, this);
}

void CoreVisionServiceProxy::subjectSegmentationFromImageFileAsync(int sessionId, const char *requestId,
                                                                   const char *imageFile, const char *config) {
    if (proxy_ == nullptr) {
        LOGE("Proxy not exist!");
        return;
    }
    if (!ensureProtocolSupportedForApi("subject_segmentation_from_image_file_async", nullptr)) {
        if (subjectSegmentationCallback_) {
            SubjectSegmentationResult result{
                .errorCode = AI_COMMON_RUNTIME_OUTDATED,
                .errorMessage = "Error calling subject_segmentation_from_image_file: runtime outdated"};
            if (requestId) {
                result.requestId = requestId;
            }
            subjectSegmentationCallback_(&result, subjectSegmentationUserData_);
        }
        return;
    }
    if (isSessionIdInvalid(sessionId)) {
        if (subjectSegmentationCallback_) {
            SubjectSegmentationResult result{.errorCode = AI_COMMON_INVALID_SESSION,
                                             .errorMessage = "Invalid session id."};
            if (requestId) {
                result.requestId = requestId;
            }
            subjectSegmentationCallback_(&result, subjectSegmentationUserData_);
        }
        return;
    }
    if ((imageFile == nullptr || !util::fileExists(imageFile)) && subjectSegmentationCallback_) {
        SubjectSegmentationResult result{.errorCode = AI_COMMON_FILE_NOT_FOUND,
                                         .errorMessage = "The image file is empty or does not exist."};
        if (requestId) {
            result.requestId = requestId;
        }
        subjectSegmentationCallback_(&result, subjectSegmentationUserData_);
        LOGE("Image file does not exist: {}", imageFile);
        return;
    }
    if (!util::hasReadPermission(imageFile) && subjectSegmentationCallback_) {
        SubjectSegmentationResult result{.errorCode = AI_COMMON_PERMISSION_DENIED,
                                         .errorMessage = "Image file permission denied."};

        if (requestId) {
            result.requestId = requestId;
        }
        subjectSegmentationCallback_(&result, subjectSegmentationUserData_);
        LOGE("The image file does not have read permission.");
        return;
    }

    ai_runtime_core_vision_service_call_subject_segmentation_from_image_file_async(
        proxy_, sessionId, requestId, imageFile, config, nullptr, onSubjectSegmentationResultFromFileCallback, this);
}

void CoreVisionServiceProxy::subjectSegmentationFromImageDataAsync(int sessionId, const char *requestId,
                                                                   const char *data, unsigned int imageDataLength,
                                                                   const char *config) {
    if (proxy_ == nullptr) {
        LOGE("Proxy not exist!");
        return;
    }
    if (!ensureProtocolSupportedForApi("subject_segmentation_from_image_data_async", nullptr)) {
        if (subjectSegmentationCallback_) {
            SubjectSegmentationResult result{
                .errorCode = AI_COMMON_RUNTIME_OUTDATED,
                .errorMessage = "Error calling subject_segmentation_from_image_data: runtime outdated"};
            if (requestId) {
                result.requestId = requestId;
            }
            subjectSegmentationCallback_(&result, subjectSegmentationUserData_);
        }
        return;
    }
    if (isSessionIdInvalid(sessionId)) {
        if (subjectSegmentationCallback_) {
            SubjectSegmentationResult result{.errorCode = AI_COMMON_INVALID_SESSION,
                                             .errorMessage = "Invalid session id."};
            if (requestId) {
                result.requestId = requestId;
            }
            subjectSegmentationCallback_(&result, subjectSegmentationUserData_);
        }
        return;
    }
    if (data == nullptr || imageDataLength == 0) {
        SubjectSegmentationResult result{.errorCode = AI_COMMON_INVALID_PARAMETER,
                                         .errorMessage = "Image data is empty."};
        if (requestId) {
            result.requestId = requestId;
        }
        if (subjectSegmentationCallback_) {
            subjectSegmentationCallback_(&result, subjectSegmentationUserData_);
        }
        return;
    }
    std::string imageData(data, imageDataLength);
    std::string base64ImageData = util::base64Encode(imageData);
    ai_runtime_core_vision_service_call_subject_segmentation_from_image_data_async(
        proxy_, sessionId, requestId, base64ImageData.c_str(), base64ImageData.size(), config, nullptr,
        onSubjectSegmentationResultFromDataCallback, this);
}

void CoreVisionServiceProxy::enableInternalEventLoop(bool enable) {
    if (enable) {
        bool expected = false;
        if (!internalLoopRequested_.compare_exchange_strong(expected, true, std::memory_order_acq_rel)) {
            return;
        }
        startInternalEventLoop();
        if (eventLoopContext_ == nullptr) {
            internalLoopRequested_.store(false, std::memory_order_release);
        }
    } else {
        bool expected = true;
        if (!internalLoopRequested_.compare_exchange_strong(expected, false, std::memory_order_acq_rel)) {
            return;
        }
        stopInternalEventLoop();
    }
}

void CoreVisionServiceProxy::startInternalEventLoop() {
    if (eventLoopThread_.joinable()) {
        return;
    }

    eventLoopContext_ = g_main_context_default();
    if (eventLoopContext_ == nullptr) {
        return;
    }

    g_main_context_ref(eventLoopContext_);
    try {
        eventLoopThread_ = std::thread([this]() {
            while (internalLoopRequested_.load(std::memory_order_acquire)) {
                g_main_context_iteration(eventLoopContext_, TRUE);
            }
        });
    } catch (const std::system_error &error) {
        LOGE("Failed to start internal event loop thread: {}", error.what());
        g_main_context_unref(eventLoopContext_);
        eventLoopContext_ = nullptr;
        internalLoopRequested_.store(false, std::memory_order_release);
    }
}

void CoreVisionServiceProxy::stopInternalEventLoop() {
    if (eventLoopContext_ != nullptr) {
        g_main_context_wakeup(eventLoopContext_);
    }

    if (eventLoopThread_.joinable()) {
        eventLoopThread_.join();
    }

    if (eventLoopContext_ != nullptr) {
        g_main_context_unref(eventLoopContext_);
        eventLoopContext_ = nullptr;
    }
}

void CoreVisionServiceProxy::onRecognitionResultFromImageFileCallback(GObject *sourceObject, GAsyncResult *res,
                                                                      gpointer userData) {
    GVariant *result = nullptr;
    GError *error = nullptr;
    auto *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    bool ret = ai_runtime_core_vision_service_call_recognize_text_from_image_file_finish(
        (AiRuntimeCoreVisionService *)sourceObject, &result, res, &error);
    if (!ret) {
        if (error) {
            LOGE("Error calling recognizeTextFromImageFile: {}:{}", error->code, error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
        if (result) {
            g_variant_unref(result);
        }
        return;
    }

    CoreVisionServiceProxy::handleRecognitionResultCallback(proxy, result);
}

void CoreVisionServiceProxy::onRecognitionResultFromImageFileWithRequestIdCallback(GObject *sourceObject,
                                                                                   GAsyncResult *res,
                                                                                   gpointer userData) {
    GVariant *result = nullptr;
    GError *error = nullptr;
    auto *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    bool ret = ai_runtime_core_vision_service_call_recognize_text_from_image_file_with_request_id_finish(
        (AiRuntimeCoreVisionService *)sourceObject, &result, res, &error);
    if (!ret) {
        if (error) {
            LOGE("Error calling recognizeTextFromImageFileWithRequestId: {}:{}", error->code, error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
        if (result) {
            g_variant_unref(result);
        }
        return;
    }

    CoreVisionServiceProxy::handleRecognitionResultCallback(proxy, result);
}

void CoreVisionServiceProxy::onRecognitionResultFromImageDataCallback(GObject *sourceObject, GAsyncResult *res,
                                                                      gpointer userData) {
    GVariant *result = nullptr;
    GError *error = nullptr;
    auto *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    bool ret = ai_runtime_core_vision_service_call_recognize_text_from_image_data_finish(
        (AiRuntimeCoreVisionService *)sourceObject, &result, res, &error);
    if (!ret) {
        if (error) {
            LOGE("Error calling recognizeTextFromImageData: {}:{}", error->code, error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
        if (result) {
            g_variant_unref(result);
        }
        return;
    }

    CoreVisionServiceProxy::handleRecognitionResultCallback(proxy, result);
}

void CoreVisionServiceProxy::onRecognitionResultFromImageDataWithRequestIdCallback(GObject *sourceObject,
                                                                                   GAsyncResult *res,
                                                                                   gpointer userData) {
    GVariant *result = nullptr;
    GError *error = nullptr;
    auto *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    bool ret = ai_runtime_core_vision_service_call_recognize_text_from_image_data_with_request_id_finish(
        (AiRuntimeCoreVisionService *)sourceObject, &result, res, &error);
    if (!ret) {
        if (error) {
            LOGE("Error calling recognizeTextFromImageDataWithRequestId: {}:{}", error->code, error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
        if (result) {
            g_variant_unref(result);
        }
        return;
    }

    CoreVisionServiceProxy::handleRecognitionResultCallback(proxy, result);
}

void CoreVisionServiceProxy::onSubjectSegmentationResultFromFileCallback(GObject *sourceObject, GAsyncResult *res,
                                                                         gpointer userData) {
    GVariant *result = nullptr;
    GError *error = nullptr;
    auto *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    bool ret = ai_runtime_core_vision_service_call_subject_segmentation_from_image_file_async_finish(
        (AiRuntimeCoreVisionService *)sourceObject, &result, res, &error);
    if (!ret) {
        if (error) {
            LOGE("Error calling subjectSegmentationFromImageFile: {}:{}", error->code, error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
        if (result) {
            g_variant_unref(result);
        }
        return;
    }

    if (proxy->subjectSegmentationCallback_ != nullptr) {
        SubjectSegmentationResult subjectSegmentationResult = {};
        dissectSubjectSegmentationResult(result, &subjectSegmentationResult);
        g_variant_unref(result);
        proxy->subjectSegmentationCallback_(&subjectSegmentationResult, proxy->subjectSegmentationUserData_);
    } else {
        LOGE("SubjectSegmentation callback is not set!");
    }
}

void CoreVisionServiceProxy::onSubjectSegmentationResultFromDataCallback(GObject *sourceObject, GAsyncResult *res,
                                                                         gpointer userData) {
    GVariant *result = nullptr;
    GError *error = nullptr;
    auto *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    bool ret = ai_runtime_core_vision_service_call_subject_segmentation_from_image_data_async_finish(
        (AiRuntimeCoreVisionService *)sourceObject, &result, res, &error);
    if (!ret) {
        if (error) {
            LOGE("Error calling subjectSegmentationFromImageData: {}:{}", error->code, error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
        if (result) {
            g_variant_unref(result);
        }
        return;
    }

    if (proxy->subjectSegmentationCallback_ != nullptr) {
        SubjectSegmentationResult subjectSegmentationResult = {};
        dissectSubjectSegmentationResult(result, &subjectSegmentationResult);
        g_variant_unref(result);
        proxy->subjectSegmentationCallback_(&subjectSegmentationResult, proxy->subjectSegmentationUserData_);
    } else {
        LOGE("SubjectSegmentation callback is not set!");
    }
}

void CoreVisionServiceProxy::onSubjectSegmentationInitCallback(GObject *sourceObject, GAsyncResult *res,
                                                               gpointer userData) {
    gint sessionId;
    gint errorCode;
    GError *error = nullptr;
    auto *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    bool ret = ai_runtime_core_vision_service_call_init_subject_segmentation_finish(
        (AiRuntimeCoreVisionService *)sourceObject, &sessionId, &errorCode, res, &error);
    if (!ret) {
        LOGE("Error calling initSubjectSegmentation: {}:{}", error->code, error->message);
        sessionId = -1;
        errorCode = AISDK_RUNTIME_ERROR;
    }

    if (proxy->subjectSegmentationInitCallback_) {
        proxy->subjectSegmentationInitCallback_(sessionId, errorCode, proxy->subjectSegmentationInitUserData_);
    }
}

void CoreVisionServiceProxy::handleErrorOccurred(int errorCode) {
    LOGE("Server disconnected.");
    switch (errorCode) {
        case G_IO_ERROR_CLOSED: {
            handleServerClosed();
            break;
        }
        case G_IO_ERROR_TIMED_OUT: {
            handleServerTimeout();
            break;
        }
    }
}

void CoreVisionServiceProxy::handleServerClosed() {
    LOGE("Server closed unexpectedly.");
    if (recognitionCallback_) {
        TextRecognitionResult result{.errorCode = AISDK_RUNTIME_ERROR, .errorMessage = "Server closed unexpectedly."};
        recognitionCallback_(&result, recognitionUserData_);
    }

    if (reconnectServer()) {
        if (reconnectedCallback_) {
            LOGE("Reconnected to server And call back.\n");
            reconnectedCallback_();
        }
    }

    reconnectAttempts_ = 0;
}

void CoreVisionServiceProxy::handleServerTimeout() {
    if (recognitionCallback_) {
        TextRecognitionResult result{.errorCode = AISDK_SERVICE_TIMEOUT, .errorMessage = "Server timeout."};
        recognitionCallback_(&result, recognitionUserData_);
    }
}

bool CoreVisionServiceProxy::reconnectServer() {
    if (reconnectAttempts_ >= maxReconnectAttempts) {
        LOGE("Max reconnect attempts reached");
        return false;
    }

    reconnectAttempts_++;
    LOGD("Reconnect attempt {}.", reconnectAttempts_);

    if (!connectToServer()) {
        LOGE("Failed to reconnect to server");
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return reconnectServer();
    }

    LOGI("Successfully reconnected to server.");

    return true;
}

void CoreVisionServiceProxy::onSegmentationInitCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData) {
    CoreVisionServiceProxy *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    gint sessionId{};
    gint errorCode{};
    GError *error{nullptr};
    bool success = ai_runtime_core_vision_service_call_init_segmentation_finish(
        (AiRuntimeCoreVisionService *)sourceObject, &sessionId, &errorCode, res, &error);
    if (!success) {
        LOGE("Error calling initSegmentation:", error->code, ":", error->message);
        sessionId = -1;
        errorCode = AI_COMMON_RUNTIME_ERROR;
    }
    if (sessionId != -1) {
        proxy->connectSegmentationResultSignal(sessionId);
    }
    std::lock_guard locker(proxy->segmentationInitMutex_);
    if (proxy->segmentationInitCallback_) {
        proxy->segmentationInitCallback_(sessionId, errorCode, proxy->segmentationInitUserData_);
    }
}

void CoreVisionServiceProxy::initSegmentationEngineAsync(const SegmentationInitCallback &callback, void *userData) {
    if (proxy_ == nullptr) {
        LOGE("Proxy is nullptr!");
        return;
    }
    if (!ensureProtocolSupportedForApi("init_segmentation", nullptr)) {
        if (callback) {
            callback(-1, AI_COMMON_RUNTIME_OUTDATED, userData);
        }
        return;
    }
    {
        std::lock_guard locker(segmentationInitMutex_);
        segmentationInitCallback_ = callback;
        segmentationInitUserData_ = userData;
    }
    ai_runtime_core_vision_service_call_init_segmentation(proxy_, nullptr, onSegmentationInitCallback, this);
}

void CoreVisionServiceProxy::destroySegmentation(int sessionId) {
    if (proxy_ == nullptr) {
        LOGE("Proxy is nullptr!");
        return;
    }
    {
        std::lock_guard locker(encodeCallbackMutex_);
        while (!encodeCallbackQueue_.empty()) {
            SegmentationEncodeCallbackInfo &info = encodeCallbackQueue_.front();
            if (info.callback) {
                info.callback(AI_COMMON_TASK_CANCELLED, info.userData);
            }
            encodeCallbackQueue_.pop();
        }
    }
    if (isSessionIdInvalid(sessionId)) {
        // 未初始化场景
        std::lock_guard locker(segmentationInitMutex_);
        if (segmentationInitCallback_) {
            segmentationInitCallback_(-1, AI_COMMON_TASK_CANCELLED, segmentationInitUserData_);
        }
    } else {
        std::lock_guard locker(segmentationResultMutex_);
        if (segmentationResultCallback_) {
            SegmentationResult segmentationResult{};
            segmentationResult.errorCode = AI_COMMON_TASK_CANCELLED;
            segmentationResultCallback_(&segmentationResult, segmentationResultUserData_);
        }
    }
    {
        std::lock_guard locker(segmentationInitMutex_);
        segmentationInitCallback_ = nullptr;
        segmentationInitUserData_ = nullptr;
    }
    {
        std::lock_guard locker(segmentationResultMutex_);
        segmentationResultCallback_ = nullptr;
        segmentationResultUserData_ = nullptr;
    }
    ai_runtime_core_vision_service_call_destroy_segmentation_sync(proxy_, sessionId, nullptr, nullptr);
}

void CoreVisionServiceProxy::setSegmentationResultCallback(const SegmentationResultCallback &callback, void *userData) {
    std::lock_guard locker(segmentationResultMutex_);
    segmentationResultCallback_ = callback;
    segmentationResultUserData_ = userData;
}

void CoreVisionServiceProxy::onSegmentationEncodeImageFileFinished(GObject *sourceObject, GAsyncResult *res,
                                                                   gpointer userData) {
    CoreVisionServiceProxy *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    GError *error{nullptr};
    gint errorCode{};
    ai_runtime_core_vision_service_call_encode_segmentation_from_image_file_finish(
        (AiRuntimeCoreVisionService *)sourceObject, &errorCode, res, &error);

    SegmentationEncodeCallbackInfo callbackInfo;
    {
        std::lock_guard<std::mutex> lock(proxy->encodeCallbackMutex_);
        if (proxy->encodeCallbackQueue_.empty()) {
            return;
        }
        callbackInfo = proxy->encodeCallbackQueue_.front();
        proxy->encodeCallbackQueue_.pop();
    }
    if (callbackInfo.callback) {
        callbackInfo.callback(errorCode, callbackInfo.userData);
    }
}

void CoreVisionServiceProxy::encodeImageFileAsync(int sessionId, const std::string &imageFile,
                                                  const SegmentationEncodeCallback &callback, void *userData) {
    if (isSessionIdInvalid(sessionId)) {
        LOGE("Invalid session id.");
        if (callback) {
            callback(AI_COMMON_INVALID_SESSION, userData);
        }
        return;
    }

    if (!ensureProtocolSupportedForApi("encode_segmentation_from_image_file", nullptr)) {
        if (callback) {
            callback(AI_COMMON_RUNTIME_OUTDATED, userData);
        }
        return;
    }
    {
        std::lock_guard<std::mutex> lock(encodeCallbackMutex_);
        encodeCallbackQueue_.push({callback, userData});
    }
    ai_runtime_core_vision_service_call_encode_segmentation_from_image_file(
        proxy_, sessionId, imageFile.c_str(), nullptr, onSegmentationEncodeImageFileFinished, this);
}

void CoreVisionServiceProxy::encodeImageDataAsync(int sessionId, const std::vector<uint8_t> &imageData,
                                                  const SegmentationEncodeCallback &callback, void *userData) {
    if (isSessionIdInvalid(sessionId)) {
        LOGE("Invalid session id.");
        if (callback) {
            callback(AI_COMMON_INVALID_SESSION, userData);
        }
        return;
    }
    if (!ensureProtocolSupportedForApi("encode_segmentation_from_base64_image_data", nullptr)) {
        if (callback) {
            callback(AI_COMMON_RUNTIME_OUTDATED, userData);
        }
        return;
    }
    {
        std::lock_guard<std::mutex> lock(encodeCallbackMutex_);
        encodeCallbackQueue_.push({callback, userData});
    }
    std::string base64ImageData = util::base64Encode(imageData);
    ai_runtime_core_vision_service_call_encode_segmentation_from_base64_image_data(
        proxy_, sessionId, base64ImageData.c_str(), nullptr, onSegmentationEncodeImageFileFinished, this);
}

int CoreVisionServiceProxy::clearCache(int sessionId) {
    if (isSessionIdInvalid(sessionId)) {
        LOGE("Invalid session id.");
        return AI_COMMON_INVALID_SESSION;
    }
    int errorCode{AI_COMMON_SUCCESS};
    if (!ensureProtocolSupportedForApi("clear_segmentation_cache", nullptr)) {
        errorCode = AI_COMMON_RUNTIME_OUTDATED;
        return errorCode;
    }
    ai_runtime_core_vision_service_call_clear_segmentation_cache_sync(proxy_, sessionId, &errorCode, nullptr, nullptr);
    return errorCode;
}

void CoreVisionServiceProxy::runAsync(int sessionId_, const std::vector<PixelPoint> &points, uint32_t minArea) {
    if (isSessionIdInvalid(sessionId_)) {
        LOGE("Invalid session id.");
        std::lock_guard locker(segmentationResultMutex_);
        if (segmentationResultCallback_) {
            SegmentationResult result{};
            result.errorCode = AI_COMMON_INVALID_SESSION;
            result.errorMessage = "Invalid session id.";
            segmentationResultCallback_(&result, segmentationResultUserData_);
        }
        return;
    }
    if (!ensureProtocolSupportedForApi("run_segmentation", nullptr)) {
        std::lock_guard locker(segmentationResultMutex_);
        if (segmentationResultCallback_) {
            SegmentationResult result{};
            result.errorCode = AI_COMMON_RUNTIME_OUTDATED;
            result.errorMessage = "Outdated protocol.";
            segmentationResultCallback_(&result, segmentationResultUserData_);
        }
        return;
    }
    if (points.empty()) {
        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE("a(uu)"));
        GVariant *pointsVariant = g_variant_builder_end(&builder);
        ai_runtime_core_vision_service_call_run_segmentation(proxy_, sessionId_, pointsVariant, minArea, nullptr,
                                                             nullptr, nullptr);
    } else {
        GVariantBuilder builder;
        g_variant_builder_init(&builder, G_VARIANT_TYPE_ARRAY);
        for (const PixelPoint &point : points) {
            if (point.x < 0 || point.y < 0) {
                std::lock_guard locker(segmentationResultMutex_);
                if (segmentationResultCallback_) {
                    SegmentationResult result{};
                    result.errorCode = AI_COMMON_IMAGE_COORD_OUT_OF_BOUNDS;
                    result.errorMessage = "Invalid point.";
                    segmentationResultCallback_(&result, segmentationResultUserData_);
                    return;
                }
            }
            g_variant_builder_add(&builder, "(uu)", point.x, point.y);
        }
        GVariant *pointsVariant = g_variant_builder_end(&builder);
        ai_runtime_core_vision_service_call_run_segmentation(proxy_, sessionId_, pointsVariant, minArea, nullptr,
                                                             nullptr, nullptr);
    }
}

void CoreVisionServiceProxy::onSegmentationResultCallback(GDBusConnection *connection, const gchar *senderName,
                                                          const gchar *objectPath, const gchar *interfaceName,
                                                          const gchar *signalName, GVariant *parameters,
                                                          gpointer userData) {
    CoreVisionServiceProxy *proxy = static_cast<CoreVisionServiceProxy *>(userData);
    std::lock_guard locker(proxy->segmentationResultMutex_);
    if (proxy->segmentationResultCallback_ != nullptr) {
        SegmentationResult result = dissectSegmentationResult(parameters);
        proxy->segmentationResultCallback_(&result, proxy->segmentationResultUserData_);
    } else {
        LOGE("Segmentation result callback is not set!");
    }
}

void CoreVisionServiceProxy::connectSegmentationResultSignal(int sessionId) {
    GDBusConnection *connection = CoreVisionServer::getInstance().getConnection();
    std::string interfaceName = interface + std::to_string(sessionId);
    g_dbus_connection_signal_subscribe(connection, nullptr, interfaceName.c_str(), segmentationResultSignal, objectPath,
                                       nullptr, G_DBUS_SIGNAL_FLAGS_NONE, onSegmentationResultCallback, this, nullptr);
}

void CoreVisionServiceProxy::refreshRuntimeProtocolVersion() {
    if (proxy_ == nullptr) {
        return;
    }

    const gchar *versionRaw = ai_runtime_core_vision_service_get_runtime_version(proxy_);
    const ProtocolVersion parsedVersion = parseProtocolVersion(versionRaw);
    if (parsedVersion.major == 0 && parsedVersion.minor == 0 && parsedVersion.patch == 0) {
        const ProtocolVersion fallbackVersion{1, 0, 0};
        LOGW("Runtime protocol version is invalid:", versionRaw ? versionRaw : "null", ", fallback to ",
             versionToString(fallbackVersion));
        setRuntimeProtocolVersion(fallbackVersion);
        return;
    }

    setRuntimeProtocolVersion(parsedVersion);
    if (isSdkAheadOfRuntime()) {
        LOGW("SDK protocol", versionToString(getSdkProtocolVersion()), "is newer than ", versionToString(parsedVersion),
             "some APIs may be disabled based on requirements.");
    } else {
        LOGD("Runtime protocol", versionToString(parsedVersion), "is compatible with SDK",
             versionToString(getSdkProtocolVersion()));
    }
}

bool CoreVisionServiceProxy::isSessionIdInvalid(const int sessionId) { return sessionId <= 0; }

bool CoreVisionServiceProxy::ensureProtocolSupportedForApi(const char *apiName, int *errorCode) {
    if (ensureApiSupported(apiName, errorCode)) {
        return true;
    }

    const ProtocolRequirement *requirement = findApiRequirement(apiName);
    ProtocolRequirement defaultRequirement{ProtocolVersion{1, 0, 0}, kProtocolVersionMax};
    const ProtocolRequirement &activeRequirement = requirement ? *requirement : defaultRequirement;

    if (apiName) {
        LOGW("Protocol mismatch for API:", apiName, "runtime:", versionToString(getRuntimeProtocolVersion()),
             "sdk:", versionToString(getSdkProtocolVersion()), "min:", versionToString(activeRequirement.minVersion),
             "max:", versionToString(activeRequirement.maxVersion));
    }

    return false;
}

}  // namespace vision
}  // namespace core
}  // namespace kyai

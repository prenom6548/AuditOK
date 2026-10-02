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

#include "genaivisionserviceproxy.h"

#include <gio/gio.h>

#include <thread>

#include "_imageconfig.h"
#include "_imageresult.h"
#include "logger.h"

namespace kylin {
namespace genai {
namespace vision {
const std::string serverUnixPath =
    "unix:path=/tmp/.kylin-ai-runtime-unix/" + std::to_string(getuid()) + "/genai-vision.sock";
const char* objectPath = "/com/kylin/AiRuntime/GenAiVision";
const char* interface = "com.kylin.AiRuntime.GenAiVision";
const char* visionResultSignal = "GenAiVisionResult";
const int resultCallbackTimeoutMs = 3 * 60 * 1000;
std::vector<uint8_t> base64Decode(const std::string& input) {
    gsize decodedLength{};
    guchar* decodedData = g_base64_decode(input.c_str(), &decodedLength);
    std::vector<uint8_t> result(decodedData, decodedData + decodedLength);
    g_free(decodedData);
    return result;
}

bool runtimeSupportsRequestId() {
    ProtocolVersion version = getRuntimeProtocolVersion();
    return version.major > 1 || (version.major == 1 && version.minor >= 3);
}

_VisionImageResult dissectImageResult(GVariant* parameters) {
    GVariantIter iter;
    g_variant_iter_init(&iter, parameters);

    GVariant* imageData = g_variant_iter_next_value(&iter);
    GVariant* format = g_variant_iter_next_value(&iter);
    GVariant* height = g_variant_iter_next_value(&iter);
    GVariant* width = g_variant_iter_next_value(&iter);
    GVariant* total = g_variant_iter_next_value(&iter);
    GVariant* index = g_variant_iter_next_value(&iter);
    GVariant* errorCode = g_variant_iter_next_value(&iter);
    GVariant* errorMsg = g_variant_iter_next_value(&iter);
    GVariant* requestId = (runtimeSupportsRequestId()) ? g_variant_iter_next_value(&iter) : nullptr;

    if (!imageData || !format || !height || !width || !total || !index || !errorCode || !errorMsg) {
        g_variant_unref(imageData);
        g_variant_unref(format);
        g_variant_unref(height);
        g_variant_unref(width);
        g_variant_unref(total);
        g_variant_unref(index);
        g_variant_unref(errorCode);
        g_variant_unref(errorMsg);
        LOGE("Generate image result error: image data is nullptr!");
        return _VisionImageResult{};
    }

    char* resultImageBase64Data = (char*)g_variant_get_data(imageData);
    int* resultFormat = (int*)g_variant_get_data(format);
    int* resultHeight = (int*)g_variant_get_data(height);
    int* resultWidth = (int*)g_variant_get_data(width);
    int* resultTotal = (int*)g_variant_get_data(total);
    int* resultIndex = (int*)g_variant_get_data(index);
    int* resultErrorCode = (int*)g_variant_get_data(errorCode);
    char* resultErrorMsg = (char*)g_variant_get_data(errorMsg);
    char* resultRequestId = nullptr;
    if (requestId) {
        resultRequestId = (char*)g_variant_get_string(requestId, nullptr);
    }

    if (!resultImageBase64Data || !resultFormat || !resultHeight || !resultWidth || !resultTotal || !resultIndex ||
        !resultErrorCode || !resultErrorMsg) {
        LOGE("Generate image result error: image data is nullptr!");
        g_variant_unref(imageData);
        g_variant_unref(format);
        g_variant_unref(height);
        g_variant_unref(width);
        g_variant_unref(total);
        g_variant_unref(index);
        g_variant_unref(errorCode);
        g_variant_unref(errorMsg);
        return _VisionImageResult{};
    }
    LOGD("prompt image result:");
    LOGD("format", *resultFormat);
    LOGD("width", *resultWidth);
    LOGD("height", *resultHeight);
    LOGD("total", *resultTotal);
    LOGD("index", *resultIndex);
    LOGD("error code", *resultErrorCode);
    LOGD("error message", resultErrorMsg);
    LOGD("rquest id:", resultRequestId);

    std::vector<uint8_t> theImageData = base64Decode(resultImageBase64Data);

    _VisionImageResult visionImageResult{
        ImageFormat(*resultFormat), *resultWidth,  *resultHeight, *resultTotal, *resultIndex, theImageData,
        *resultErrorCode,           resultErrorMsg};
    if (resultRequestId) {
        visionImageResult.requestId = resultRequestId;
    }
    g_variant_unref(imageData);
    g_variant_unref(format);
    g_variant_unref(height);
    g_variant_unref(width);
    g_variant_unref(total);
    g_variant_unref(index);
    g_variant_unref(errorCode);
    g_variant_unref(errorMsg);
    if (requestId) {
        g_variant_unref(requestId);
    }
    return visionImageResult;
}

GenAiVisionServer& GenAiVisionServer::getInstance() {
    static GenAiVisionServer instance{};
    return instance;
}

GenAiVisionServer::~GenAiVisionServer() { destroy(); }

void GenAiVisionServer::init(const std::string& unixPath) {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr && !g_dbus_connection_is_closed(connection_)) {
        return;
    }
    GError* error = nullptr;
    connection_ = g_dbus_connection_new_for_address_sync(
        unixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, /* GDBusAuthObserver */
        nullptr,                                                                  /* GCancellable */
        &error);
    if (connection_ == nullptr) {
        g_printerr("Error connecting to D-Bus address %s: %s\n", unixPath.c_str(), error->message);
        g_error_free(error);

        return;
    }
}

void GenAiVisionServer::init() { init(serverUnixPath); }

void GenAiVisionServer::destroy() {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr) {
        g_object_unref(connection_);
    }
}

GenAiVisionServiceProxy::~GenAiVisionServiceProxy() {
    delete[] stylesArray_;
    stylesArray_ = nullptr;

    delete[] sizesArray_;
    sizesArray_ = nullptr;
    GenAiVisionServiceProxy::enableInternalEventLoop(false);
}

bool GenAiVisionServiceProxy::connectToServer() {
    GenAiVisionServer::getInstance().init();
    if (!GenAiVisionServer::getInstance().available()) {
        g_printerr(
            "Error creating core speech server proxy: Server proxy "
            "connection is unavailable.\n");
        return false;
    }

    if (proxy_) {
        g_object_unref(proxy_);
        proxy_ = nullptr;
    }

    GError* error{nullptr};
    auto* connection = GenAiVisionServer::getInstance().getConnection();
    proxy_ = ai_runtime_gen_ai_vision_service_proxy_new_sync(connection, G_DBUS_PROXY_FLAGS_NONE, nullptr, objectPath,
                                                             nullptr, &error);
    if (proxy_ == nullptr) {
        if (error) {
            g_printerr("Error creating text processor proxy %s: %s\n", objectPath, error->message);
            g_error_free(error);
        }

        return false;
    }

    g_dbus_proxy_set_default_timeout(G_DBUS_PROXY(proxy_), resultCallbackTimeoutMs);

    refreshRuntimeProtocolVersion();
    return true;
}

bool GenAiVisionServiceProxy::reconnectServer() {
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

void GenAiVisionServiceProxy::setImageResultCallback(VisionImageResultCallback callback, void* userData) {
    visionImageResultCallback_ = callback;
    visionImageResultUserData_ = userData;
}

void GenAiVisionServiceProxy::setReconnectedCallback(const ReconnectedCallback& callback) {
    reconnectedCallback_ = std::move(callback);
}

int GenAiVisionServiceProxy::initEngine(int* sessionId) {
    int errorCode = AISDK_NO_ERROR;
    GError* error{nullptr};
    if (!proxy_) {
        LOGE("Failed to init engine: AiRuntimeGenAiVisionService is null!");
        return -1;
    }

    bool success = ai_runtime_gen_ai_vision_service_call_init_sync(proxy_, sessionId, &errorCode, nullptr, nullptr);

    if (!success) {
        if (error) {
            LOGE("Error calling init: {}:{}", error->code, error->message);
            g_error_free(error);
            return AISDK_RUNTIME_ERROR;
        }
    }
    if (*sessionId != -1) {
        connectImageResultSignals(*sessionId);
    }

    return errorCode;
}

void GenAiVisionServiceProxy::setImageConfig(int sessionId, const std::string& config) {
    if (!proxy_) {
        LOGE("Failed to set image config: AiRuntimeGenAiVisionService is null!");
        return;
    }

    bool ret = ai_runtime_gen_ai_vision_service_call_set_image_config_sync(proxy_, sessionId, config.c_str(), nullptr,
                                                                           nullptr);
    if (!ret) {
        LOGE("GenAiVisionServiceProxy: error to set image config");
    }
}

const char** GenAiVisionServiceProxy::getSupportedImageStyle(int sessionId, int* number) {
    if (!number) {
        LOGE("Failed to get supported image style: number is null!");
        return {};
    }

    gchar* imageStyle = nullptr;
    GError* error = nullptr;
    if (!proxy_) {
        LOGE(
            "Failed to get supported image style: AiRuntimeGenAiVisionService "
            "is null!");
        return {};
    }

    bool ret = ai_runtime_gen_ai_vision_service_call_get_supported_image_style_sync(proxy_, sessionId, &imageStyle,
                                                                                    nullptr, &error);

    if (!imageStyle) {
        LOGE("Failed to get supported image size: imageStyle is null!");
        return {};
    }

    std::vector<std::string> styleVector = getImageStyleArryFromJsonString(imageStyle);
    *number = styleVector.size();

    if (stylesArray_) {
        delete[] stylesArray_;
        stylesArray_ = nullptr;
    }
    stylesArray_ = new const char*[*number];
    for (int i = 0; i < *number; ++i) {
        stylesArray_[i] = styleVector[i].c_str();
    }

    return stylesArray_;
}

const ImageSize* GenAiVisionServiceProxy::getSupportedImageSize(int sessionId, int* number) {
    if (!number) {
        LOGE("Failed to get supported image size: number is null!");
        return {};
    }
    gchar* imageSize = nullptr;
    GError* error = nullptr;

    if (!proxy_) {
        LOGE(
            "Failed to get supported image size: AiRuntimeGenAiVisionService "
            "is null!");
        return {};
    }

    bool ret = ai_runtime_gen_ai_vision_service_call_get_supported_image_size_sync(proxy_, sessionId, &imageSize,
                                                                                   nullptr, &error);

    if (!imageSize) {
        LOGE("Failed to get supported image size: imageSize is null!");
        return {};
    }
    std::vector<ImageSize> sizeVector = getImageSizeArryFromJsonString(imageSize);
    *number = sizeVector.size();

    if (sizesArray_) {
        delete[] sizesArray_;
        sizesArray_ = nullptr;
    }
    sizesArray_ = new ImageSize[*number];
    for (int i = 0; i < *number; ++i) {
        sizesArray_[i].width = sizeVector[i].width;
        sizesArray_[i].height = sizeVector[i].height;
    }

    return sizesArray_;
}

const int GenAiVisionServiceProxy::getSupportedImageNumber(int sessionId) {
    gint number;
    GError* error = nullptr;

    if (!proxy_) {
        LOGE(
            "Failed to get supported image number: AiRuntimeGenAiVisionService "
            "is null!");
        return 0;
    }
    bool ret = ai_runtime_gen_ai_vision_service_call_get_supported_image_number_sync(proxy_, sessionId, &number,
                                                                                     nullptr, &error);
    if (!ret && error) {
        LOGE("Failed to get the number of supported images:", error->message);
        return 0;
    }

    return number;
}

void GenAiVisionServiceProxy::generateImageAsync(int sessionId, const std::string& prompt, const std::string& request) {
    if (!proxy_) {
        LOGE("Failed to generate image: AiRuntimeGenAiVisionService is null!");
        VisionImageResult errorResult{ImageFormat::IMAGE_FORMAT_UNKNOWN,         0, 0, 0, 0, {},
                                      AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR, ""};
        if (visionImageResultCallback_) {
            visionImageResultCallback_(&errorResult, visionImageResultUserData_);
        }

        return;
    }
    isStopped_ = false;
    reconnectAttempts_ = 0;
    currentSessionId_ = sessionId;
    currentRequestId_.clear();
    if (ensureProtocolSupportedForApi("generate_image_with_request_id", nullptr)) {
        ai_runtime_gen_ai_vision_service_call_generate_image_with_request_id(
            proxy_, sessionId, "", request.c_str(), nullptr, onImageGenerationWithRequestIdFinishedCallback, this);
    } else {
        ai_runtime_gen_ai_vision_service_call_generate_image(proxy_, sessionId, prompt.c_str(), nullptr,
                                                             onImageGenerationFinishedCallback, this);
    }
}

int GenAiVisionServiceProxy::generateImageAsyncWithRequestId(int sessionId, const std::string& requestString,
                                                             const std::string& requestId) {
    if (!ensureProtocolSupportedForApi("generate_image_with_request_id", nullptr)) {
        return AI_COMMON_RUNTIME_OUTDATED;
    }

    currentRequestId_ = requestId;
    if (!proxy_) {
        LOGE("Failed to generate image: AiRuntimeGenAiVisionService is null!");
        _VisionImageResult errorResult{ImageFormat::IMAGE_FORMAT_UNKNOWN,         0, 0, 0, 0, {},
                                       AiSdkCommonErrorCode::AISDK_RUNTIME_ERROR, ""};
        errorResult.requestId = currentRequestId_;
        if (visionImageResultCallback_) {
            visionImageResultCallback_(&errorResult, visionImageResultUserData_);
        }

        return AISDK_RUNTIME_ERROR;
    }

    isStopped_ = false;
    reconnectAttempts_ = 0;
    currentSessionId_ = sessionId;
    ai_runtime_gen_ai_vision_service_call_generate_image_with_request_id(
        proxy_, sessionId, requestId.c_str(), requestString.c_str(), nullptr,
        onImageGenerationWithRequestIdFinishedCallback, this);
    return AI_COMMON_SUCCESS;
}

void GenAiVisionServiceProxy::refreshRuntimeProtocolVersion() {
    if (proxy_ == nullptr) {
        return;
    }

    const gchar* versionRaw = ai_runtime_gen_ai_vision_service_get_runtime_version(proxy_);
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

bool GenAiVisionServiceProxy::ensureProtocolSupportedForApi(const char* apiName, int* errorCode) {
    if (ensureApiSupported(apiName, errorCode)) {
        return true;
    }

    const ProtocolRequirement* requirement = findApiRequirement(apiName);
    ProtocolRequirement defaultRequirement{ProtocolVersion{1, 0, 0}, kProtocolVersionMax};
    const ProtocolRequirement& activeRequirement = requirement ? *requirement : defaultRequirement;

    if (apiName) {
        LOGW("Protocol mismatch for API:", apiName, "runtime:", versionToString(getRuntimeProtocolVersion()),
             "sdk:", versionToString(getSdkProtocolVersion()), "min:", versionToString(activeRequirement.minVersion),
             "max:", versionToString(activeRequirement.maxVersion));
    }

    return false;
}

void GenAiVisionServiceProxy::startInternalEventLoop() {
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
    } catch (const std::system_error& error) {
        LOGE("Failed to start internal event loop thread: {}", error.what());
        g_main_context_unref(eventLoopContext_);
        eventLoopContext_ = nullptr;
        internalLoopRequested_.store(false, std::memory_order_release);
    }
}

void GenAiVisionServiceProxy::stopInternalEventLoop() {
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

void GenAiVisionServiceProxy::enableInternalEventLoop(bool enable) {
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

void GenAiVisionServiceProxy::connectImageResultSignals(int sessionId) {
    auto* connection = GenAiVisionServer::getInstance().getConnection();
    std::string interfaceName = interface + std::to_string(sessionId);
    g_dbus_connection_signal_subscribe(connection, nullptr, interfaceName.c_str(), visionResultSignal, objectPath,
                                       nullptr, G_DBUS_SIGNAL_FLAGS_NONE, onImageResult, this, nullptr);
}

void GenAiVisionServiceProxy::onImageResult(GDBusConnection* connection, const gchar* senderName,
                                            const gchar* objectPath, const gchar* interfaceName,
                                            const gchar* signalName, GVariant* parameters, gpointer userData) {
    auto* proxy = static_cast<GenAiVisionServiceProxy*>(userData);
    if (proxy->isStopped_) {
        return;
    }

    if (!proxy->visionImageResultCallback_) {
        LOGE("GenAiVisionServiceProxy: vision image callback is not set!");
        return;
    }

    auto result = dissectImageResult(parameters);
    if (result.requestId.empty()) {
        result.requestId = proxy->currentRequestId_;
    }
    proxy->visionImageResultCallback_(&result, proxy->visionImageResultUserData_);
}

void GenAiVisionServiceProxy::onImageGenerationFinishedCallback(GObject* sourceObject, GAsyncResult* res,
                                                                gpointer userData) {
    GError* error{nullptr};
    auto* proxy = static_cast<GenAiVisionServiceProxy*>(userData);
    bool result = ai_runtime_gen_ai_vision_service_call_generate_image_finish(
        (AiRuntimeGenAiVisionService*)sourceObject, res, &error);
    if (!result) {
        if (error == nullptr) {
            g_printerr("Error calling image generation: Unknown error\n");
        } else {
            g_printerr("Error calling image generation: %s\n", error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
    }
}

void GenAiVisionServiceProxy::onImageGenerationWithRequestIdFinishedCallback(GObject* sourceObject, GAsyncResult* res,
                                                                             gpointer userData) {
    GError* error{nullptr};
    auto* proxy = static_cast<GenAiVisionServiceProxy*>(userData);
    bool result = ai_runtime_gen_ai_vision_service_call_generate_image_with_request_id_finish(
        (AiRuntimeGenAiVisionService*)sourceObject, res, &error);
    if (!result) {
        if (error == nullptr) {
            g_printerr("Error calling image generation: Unknown error\n");
        } else {
            g_printerr("Error calling image generation: %s\n", error->message);
            proxy->handleErrorOccurred(error->code);
            g_error_free(error);
        }
    }
}

void GenAiVisionServiceProxy::stopImageGeneration() {
    GError* error{nullptr};
    if (proxy_) {
        ai_runtime_gen_ai_vision_service_call_stop_image_generation(proxy_, currentSessionId_, nullptr, nullptr,
                                                                    &error);
        if (error) {
            LOGE("Error stopping image generation:", error->message);
            g_error_free(error);
        }
    } else {
        LOGE(
            "Failed to stop image generation: AiRuntimeGenAiVisionService is "
            "null!");
    }

    isStopped_ = true;
}

int GenAiVisionServiceProxy::stopGeneration(int sessionId, const char* requestId) {
    if (!proxy_) {
        LOGE("Failed to stop generation: AiRuntimeGenAiVisionService is null!");
        return AISDK_RUNTIME_ERROR;
    }

    GError* error = nullptr;

    // 如果 requestId 为空，停止所有请求
    if (requestId == nullptr || requestId[0] == '\0') {
        ai_runtime_gen_ai_vision_service_call_stop_image_generation_sync(proxy_, sessionId, nullptr, &error);
    } else {
        // 如果 requestId 非空，停止指定请求
        ai_runtime_gen_ai_vision_service_call_stop_image_generation_with_request_id_sync(proxy_, sessionId, requestId,
                                                                                         nullptr, &error);
    }

    if (error) {
        LOGE("Error stopping image generation:", error->message);
        g_error_free(error);
        return AISDK_RUNTIME_ERROR;
    }

    return AISDK_NO_ERROR;
}

void GenAiVisionServiceProxy::handleErrorOccurred(int errorCode) {
    LOGE("Error occurred with code: {}", errorCode);
    switch (errorCode) {
        case G_IO_ERROR_CLOSED:
            handleServerClosed();
            break;
        case G_IO_ERROR_TIMED_OUT:
            handleResultTimeout();
            break;
        default:
            handleGeneralError();
            break;
    }
}

void GenAiVisionServiceProxy::handleServerClosed() {
    LOGE("Server closed unexpectedly.");
    if (visionImageResultCallback_) {
        _VisionImageResult result{.errorCode = AISDK_RUNTIME_ERROR, .errorMessage = "Server closed unexpectedly."};
        result.requestId = currentRequestId_;
        visionImageResultCallback_(&result, visionImageResultUserData_);
    }

    if (reconnectServer()) {
        if (reconnectedCallback_) {
            reconnectedCallback_();
        }
    }
}

void GenAiVisionServiceProxy::handleResultTimeout() {
    if (visionImageResultCallback_) {
        _VisionImageResult result{.errorCode = AISDK_MODEL_RUN_TIME_OUT, .errorMessage = "Run model time out."};
        result.requestId = currentRequestId_;
        visionImageResultCallback_(&result, visionImageResultUserData_);
    }

    stopImageGeneration();
}

void GenAiVisionServiceProxy::handleGeneralError() {
    if (visionImageResultCallback_) {
        _VisionImageResult result{.errorCode = AISDK_RUNTIME_ERROR, .errorMessage = "General error occurred."};
        result.requestId = currentRequestId_;
        visionImageResultCallback_(&result, visionImageResultUserData_);
    }
}

bool GenAiVisionServiceProxy::parseJson(const std::string& params, Json::Value& root) const {
    Json::Reader reader;
    bool parsingSuccessful = reader.parse(params, root);
    if (!parsingSuccessful) {
        LOGE("Failed to parse JSON:", reader.getFormattedErrorMessages());
        return false;
    }
    return true;
}

std::vector<std::string> GenAiVisionServiceProxy::getImageStyleArryFromJsonString(const std::string& params) {
    Json::Value root;
    if (!parseJson(params, root)) {
        return {};
    }

    if (!root.isArray()) {
        LOGE("Input is not a JSON array");
        return {};
    }

    std::vector<std::string> result;
    for (const auto& value : root) {
        result.push_back(value.asString());
    }

    return result;
}

std::vector<ImageSize> GenAiVisionServiceProxy::getImageSizeArryFromJsonString(const std::string& params) const {
    Json::Value root;
    if (!parseJson(params, root)) {
        return {};
    }

    if (!root.isArray()) {
        LOGE("Input is not a JSON array");
        return {};
    }

    std::vector<ImageSize> result;
    for (const auto& value : root) {
        std::string ratio = value.asString();
        size_t pos = ratio.find('x');
        if (pos == std::string::npos) {
            LOGE("Invalid size format:", ratio);
            continue;
        }

        int width = std::stoi(ratio.substr(0, pos));
        int height = std::stoi(ratio.substr(pos + 1));
        result.push_back(ImageSize{width, height});
    }

    return result;
}

}  // namespace vision
}  // namespace genai
}  // namespace kylin

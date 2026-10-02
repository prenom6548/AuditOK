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

#ifndef _CORE_VISION_SERVICE_PROXY_H_
#define _CORE_VISION_SERVICE_PROXY_H_

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "_textrecognitionconfig.h"
#include "corevisionserviceglue.h"
#include "error.h"
#include "protocolversion.h"
#include "segmentation.h"
#include "subjectsegmentation.h"
#include "subjectsegmentationresult.h"
#include "textrecognition.h"

namespace kyai {
namespace core {
namespace vision {
using ReconnectedCallback = std::function<void()>;

class CoreVisionServer {
public:
    static CoreVisionServer &getInstance();

    ~CoreVisionServer();

    void init();
    bool available() const { return connection_ != nullptr; }

    GDBusConnection *getConnection() const { return connection_; }

private:
    CoreVisionServer() = default;
    void init(const std::string &unixPath);
    void destroy() const;

private:
    GDBusConnection *connection_{nullptr};
    mutable std::mutex mutex_;
};

struct SegmentationEncodeCallbackInfo {
    SegmentationEncodeCallback callback;
    void *userData;
};
class CoreVisionServiceProxy {
    using SubjectSegmentationInitCallback = std::function<void(int sessionId, int errorCode, void *userData)>;
    using SegmentationInitCallback = std::function<void(int sessionId, int errorCode, void *userData)>;

public:
    ~CoreVisionServiceProxy();

    bool connectToServer();
    bool reconnectServer();

    void setRecognitionResultCallback(TextRecognitionResultCallback callback, void *userData);
    void setReconnectedCallback(ReconnectedCallback callback);
    void setSubjectSegmentationResultCallback(SubjectSegmentationResultCallback callback, void *userData);

    int initEngine(const char *config, int *sessionId);
    void initSubjectSegmentationEngine(const SubjectSegmentationInitCallback &callback, void *userData);

    void destroyTextRecognition(int sessionId);
    void destroySubjectSegmentation(int sessionId);

    void recognizeTextFromImageFileAsync(int sessionId, const char *imageFile);
    void recognizeTextFromImageFileAsync(int sessionId, const char *requestId, const char *imageFile);

    void recognizeTextFromImageDataAsync(int sessionId, const char *data, unsigned int imageDataLength);
    void recognizeTextFromImageDataAsync(int sessionId, const char *requestId, const char *data,
                                         unsigned int imageDataLength);
    void subjectSegmentationFromImageFileAsync(int sessionId, const char *requestId, const char *imageFile,
                                               const char *config);

    void subjectSegmentationFromImageDataAsync(int sessionId, const char *requestId, const char *data,
                                               unsigned int imageDataLength, const char *config);

    void enableInternalEventLoop(bool enable);

    // 开始：通用分割
    void initSegmentationEngineAsync(const SegmentationInitCallback &callback, void *userData);
    void destroySegmentation(int sessionId);
    void setSegmentationResultCallback(const SegmentationResultCallback &callback, void *userData);
    void encodeImageFileAsync(int sessionId, const std::string &imageFile, const SegmentationEncodeCallback &callback,
                              void *userData);
    void encodeImageDataAsync(int sessionId, const std::vector<uint8_t> &imageData,
                              const SegmentationEncodeCallback &callback, void *userData);
    int clearCache(int sessionId);
    void runAsync(int sessionId_, const std::vector<PixelPoint> &points, uint32_t minArea);
    static void onSegmentationResultCallback(GDBusConnection *connection, const gchar *senderName,
                                             const gchar *objectPath, const gchar *interfaceName,
                                             const gchar *signalName, GVariant *parameters, gpointer userData);
    void connectSegmentationResultSignal(int sessionId);
    // 结束：通用分割

private:
    void startInternalEventLoop();
    void stopInternalEventLoop();

    static void onRecognitionResultFromImageFileCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void onRecognitionResultFromImageFileWithRequestIdCallback(GObject *sourceObject, GAsyncResult *res,
                                                                      gpointer userData);
    static void onRecognitionResultFromImageDataCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void onRecognitionResultFromImageDataWithRequestIdCallback(GObject *sourceObject, GAsyncResult *res,
                                                                      gpointer userData);
    static void handleRecognitionResultCallback(CoreVisionServiceProxy *proxy, GVariant *result);
    static void onSubjectSegmentationResultFromFileCallback(GObject *sourceObject, GAsyncResult *res,
                                                            gpointer userData);
    static void onSubjectSegmentationResultFromDataCallback(GObject *sourceObject, GAsyncResult *res,
                                                            gpointer userData);
    static void onSubjectSegmentationInitCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void onSegmentationInitCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void onSegmentationEncodeImageFileFinished(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    void handleErrorOccurred(int errorCode);
    void handleServerClosed();
    void handleServerTimeout();
    void refreshRuntimeProtocolVersion();
    bool ensureProtocolSupportedForApi(const char *apiName, int *errorCode = nullptr);
    bool isSessionIdInvalid(int sessionId);

private:
    TextRecognitionResultCallback recognitionCallback_{nullptr};
    ReconnectedCallback reconnectedCallback_{nullptr};
    void *recognitionUserData_{nullptr};
    SubjectSegmentationResultCallback subjectSegmentationCallback_{nullptr};
    void *subjectSegmentationUserData_{nullptr};
    SubjectSegmentationInitCallback subjectSegmentationInitCallback_{nullptr};
    void *subjectSegmentationInitUserData_{nullptr};

    // 通用分割
    std::mutex segmentationInitMutex_;
    std::mutex segmentationResultMutex_;
    SegmentationInitCallback segmentationInitCallback_{nullptr};
    SegmentationResultCallback segmentationResultCallback_{nullptr};
    void *segmentationInitUserData_{nullptr};
    void *segmentationResultUserData_{nullptr};

    AiRuntimeCoreVisionService *proxy_{nullptr};

    int reconnectAttempts_{0};
    static constexpr int maxReconnectAttempts{10};
    std::atomic<bool> internalLoopRequested_{false};
    std::thread eventLoopThread_;
    GMainContext *eventLoopContext_{nullptr};

    std::queue<SegmentationEncodeCallbackInfo> encodeCallbackQueue_;
    std::mutex encodeCallbackMutex_;
};
}  // namespace vision
}  // namespace core
}  // namespace kyai
#endif

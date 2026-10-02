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

#ifndef _GENAI_VISION_SERVICE_PROXY_H_
#define _GENAI_VISION_SERVICE_PROXY_H_

#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "error.h"
#include "genaivisionserviceglue.h"
#include "imageresult.h"
#include "protocolversion.h"

namespace kylin {
namespace genai {
namespace vision {

using ReconnectedCallback = std::function<void()>;
class IGenAiVisionServiceProxy {
public:
    virtual ~IGenAiVisionServiceProxy() = default;
    virtual bool connectToServer() = 0;
    virtual bool reconnectServer() = 0;

    virtual void setImageResultCallback(VisionImageResultCallback callback, void *userData) = 0;
    virtual void setReconnectedCallback(const ReconnectedCallback &callback) = 0;

    virtual int initEngine(int *sessionId) = 0;
    virtual void setImageConfig(int sessionId, const std::string &config) = 0;

    virtual const char **getSupportedImageStyle(int sessionId, int *number) = 0;
    virtual const ImageSize *getSupportedImageSize(int sessionId, int *number) = 0;
    virtual const int getSupportedImageNumber(int sessionId) = 0;

    virtual void generateImageAsync(int sessionId, const std::string &prompt, const std::string &request) = 0;
    virtual int generateImageAsyncWithRequestId(int sessionId, const std::string &requestString,
                                                const std::string &requestId) = 0;

    virtual int stopGeneration(int sessionId, const char *requestId) = 0;

    virtual void enableInternalEventLoop(bool enable) = 0;
};

class GenAiVisionServer {
public:
    static GenAiVisionServer &getInstance();

    ~GenAiVisionServer();

    void init();
    bool available() const { return connection_ != nullptr; }

    GDBusConnection *getConnection() const { return connection_; }

private:
    void init(const std::string &unixPath);
    void destroy();

private:
    GDBusConnection *connection_{nullptr};
    mutable std::mutex mutex_;
};

class GenAiVisionServiceProxy : public IGenAiVisionServiceProxy {
public:
    ~GenAiVisionServiceProxy() override;
    bool connectToServer() override;
    bool reconnectServer() override;

    void setImageResultCallback(VisionImageResultCallback callback, void *userData) override;
    void setReconnectedCallback(const ReconnectedCallback &callback) override;

    int initEngine(int *sessionId) override;
    void setImageConfig(int sessionId, const std::string &config) override;

    const char **getSupportedImageStyle(int sessionId, int *number) override;
    const ImageSize *getSupportedImageSize(int sessionId, int *number) override;
    const int getSupportedImageNumber(int sessionId) override;

    void generateImageAsync(int sessionId, const std::string &prompt, const std::string &request);

    int generateImageAsyncWithRequestId(int sessionId, const std::string &requestString,
                                        const std::string &requestId) override;

    int stopGeneration(int sessionId, const char *requestId) override;

    void enableInternalEventLoop(bool enable) override;

private:
    void startInternalEventLoop();

    void stopInternalEventLoop();

    void connectImageResultSignals(int sessionId);
    void refreshRuntimeProtocolVersion();
    bool ensureProtocolSupportedForApi(const char *apiName, int *errorCode = nullptr);

    static void onImageResult(GDBusConnection *connection, const gchar *senderName, const gchar *objectPath,
                              const gchar *interfaceName, const gchar *signalName, GVariant *parameters,
                              gpointer userData);

    static void onImageGenerationFinishedCallback(GObject *sourceObject, GAsyncResult *res, gpointer userData);
    static void onImageGenerationWithRequestIdFinishedCallback(GObject *sourceObject, GAsyncResult *res,
                                                               gpointer userData);
    void stopImageGeneration();
    void handleErrorOccurred(int errorCode);
    void handleServerClosed();
    void handleResultTimeout();
    void handleGeneralError();

    bool parseJson(const std::string &params, Json::Value &root) const;
    std::vector<std::string> getImageStyleArryFromJsonString(const std::string &params);
    std::vector<ImageSize> getImageSizeArryFromJsonString(const std::string &params) const;

private:
    VisionImageResultCallback visionImageResultCallback_{nullptr};
    ReconnectedCallback reconnectedCallback_{nullptr};
    void *visionImageResultUserData_{nullptr};

    AiRuntimeGenAiVisionService *proxy_{nullptr};
    GenAiVisionServer *genaiVisionServer_{nullptr};

    const char **stylesArray_{nullptr};
    ImageSize *sizesArray_{nullptr};

    int currentSessionId_{0};
    std::string currentRequestId_{};
    bool isStopped_{false};

    guint timeoutId_{0};
    int reconnectAttempts_{0};
    static constexpr int maxReconnectAttempts = 10;

    std::atomic<bool> internalLoopRequested_{false};
    GMainContext *eventLoopContext_{nullptr};
    std::thread eventLoopThread_;
};

}  // namespace vision
}  // namespace genai
}  // namespace kylin
#endif

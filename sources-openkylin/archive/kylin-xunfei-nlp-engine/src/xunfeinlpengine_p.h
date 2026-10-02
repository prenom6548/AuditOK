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

#ifndef XUNFEINLPENGINE_P_H
#define XUNFEINLPENGINE_P_H

#include <atomic>
#include <mutex>
#include <curl/curl.h>
#include <jsoncpp/json/json.h>
#include <kylin-ai/ai-engine/aiengineerror.h>
#include <kylin-ai/ai-engine/textgenerationengine.h>

class XunfeiNlpEnginePrivate {
public:
    XunfeiNlpEnginePrivate();
    ~XunfeiNlpEnginePrivate();
    std::string engineName() const;
    bool isCloud() const { return true; }
    bool isBuiltInEngine() const { return false; }
    bool isCustomModelEngine() const { return false; }

    void setConfig(const std::string &config);
    bool setCurrentModel(const std::string &model);
    std::string currentModel() const { return currentModel_; }
    std::string currentModelName() const { return currentModelName_; }
    std::string modelInfo() const;

    void setTextGenerationResultCallback(const ai_engine::gen_ai::nlp::TextGenerationResultCallback &callback);

    ai_engine::result::AiEngineBaseResult generateContent(const std::string &input);
    ai_engine::result::AiEngineBaseResult chat(const std::string &message);
    void stopGenerateContent();
    void stopChat();

    ai_engine::result::AiEngineBaseResult init();
    ai_engine::result::AiEngineBaseResult destroy();

private:
    void threadSafeCallChatResultCallback(const ai_engine::gen_ai::nlp::TextGenerationResult &result) const;

    void handleReceiveData();
    std::string receiveChatData(int *retCode) const;
    bool isNetworkError(int retCode) const;
    bool isChatDataReady(int retCode) const;
    bool needReReceiveData(int retCode) const;

    void handleNetworkError(int retCode);
    void handleServerError(int errorCode, const std::string &errorMessage);
    void handleChatData(const std::string &chatData, bool *isFinished);

    void decreaseChatCounter();
    void increaseChatCounter();
    bool isChatRunning() const;

    CURLcode testConnectToServer();
    void addContext(const std::string &role, const std::string &content);
    ai_engine::result::AiEngineBaseResult buildChatMessage(const std::string &message);

private:
    std::string appId_;
    std::string apiKey_;
    std::string secretKey_;
    std::string currentModel_{"generalv3.5"};
    std::string currentModelName_{"xunfei-generalv3.5"};
    std::string currentDisplayName_{"讯飞-Spark Max"};
    std::string hostUrl_{"wss://spark-api.xf-yun.com/v3.5/chat"};
    int maxTokens_;

    mutable std::mutex mutex_;
    ai_engine::gen_ai::nlp::TextGenerationResultCallback textGenerationResultCallback_{nullptr};

    Json::Value context_;
    void clearContext();
    void removeLastContext();
    std::vector<Json::Value> streamChatData_;

    bool connectWebsocket(const std::string &hostUrl);
    void closeWebsocket();

    CURL *currentCurl_{nullptr};
    ai_engine::EngineError currentError_;

    bool initResources();
    void releaseResources();

    bool doSendData(const char *data, int dataLength, int type);
    int getMaxTokens(const std::string &model);

    bool waitForStopped();
    std::atomic<bool> isStopped_{false};
    std::atomic<int> chatCounter_{0};

    std::atomic_bool stream_{true};
    std::string receivedContentTemp_;

    Json::Value searchInfo_;
};

#endif // XUNFEINLPENGINE_P_H

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

#ifndef SERVICES_CORETEXTEMBEDDINGSERVICE_H
#define SERVICES_CORETEXTEMBEDDINGSERVICE_H

#include <gio/gio.h>

#include <memory>
#include <set>

#include "coretextembeddingserviceglue.h"
#include "engineselector/engineselector.h"
#include "textembedding.h"

class CoreTextEmbeddingService : public std::enable_shared_from_this<CoreTextEmbeddingService> {
public:
    CoreTextEmbeddingService(GDBusConnection& connection, EngineSelector& engineSelector);
    ~CoreTextEmbeddingService();

    void stop();

private:
    void exportSkeleton();
    void unexportSkeleton();
    int generateRandomSessionId();
    std::string getProtocolVersion();
    void setRuntimeProtocolVersion(const std::string& version);
    std::shared_ptr<CoreTextEmbeddingService> getSharedObject() { return shared_from_this(); }
    bool parseTextEmbeddingInput(const std::string& jsonInput, std::string& text, int& sessionId) const;
    void releaseRandomSessionId(int sessionId);
    bool isTextEmbeddingSessionIdValid(int sessionId);
    bool getDefaultModel(const std::string& modelInfo, std::string& modelName);

    const std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> selectTextEmbeddingEngine(
        std::string engineName) const;

private:
    static bool onHandleInitEngineV2(AiRuntimeCoreTextEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                     gpointer userData);

    static bool onHandleInitEngine(AiRuntimeCoreTextEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                   const gchar* inputJson, gpointer userData);

    static bool onHandleInitModel(AiRuntimeCoreTextEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                  gint sessionId, const gchar* model, gpointer userData);

    static bool onHandleEmbeddingText(AiRuntimeCoreTextEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                      const gchar* inputJson, gpointer userData);

    static bool onHandleGetModelList(AiRuntimeCoreTextEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                     gint sessionId, gpointer userData);

    static bool onHandleGetModelInfo(AiRuntimeCoreTextEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                     gint sessionId, gpointer userData);

private:
    bool isExported_ = false;
    AiRuntimeCoreTextEmbeddingService* delegate_ = nullptr;
    GDBusConnection& connection_;
    EngineSelector& engineSelector_;
    std::set<int> sessionIdSet_;
    std::map<int, std::shared_ptr<TextEmbedding>> textEmbeddings_;
    std::vector<gulong> handlerIds_;
};

#endif

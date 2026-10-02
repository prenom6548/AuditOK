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

#ifndef SERVICES_COREIMAGEEMBEDDINGSERVICE_H
#define SERVICES_COREIMAGEEMBEDDINGSERVICE_H

#include <gio/gio.h>

#include <memory>
#include <set>

#include "coreimageembeddingserviceglue.h"
#include "engineselector/engineselector.h"
#include "imageembedding.h"

class CoreImageEmbeddingService : public std::enable_shared_from_this<CoreImageEmbeddingService> {
public:
    CoreImageEmbeddingService(GDBusConnection& connection, EngineSelector& engineSelector);

    ~CoreImageEmbeddingService();

    void stop();

private:
    void exportSkeleton();
    void unexportSkeleton();
    int generateRandomSessionId();
    std::string getProtocolVersion();
    void setRuntimeProtocolVersion(const std::string& version);
    bool parseEmbeddingInput(const std::string& json, const std::string& dataKey, std::string& dataValue,
                             int& sessionId) const;
    std::shared_ptr<CoreImageEmbeddingService> getSharedObject() { return shared_from_this(); }
    void releaseRandomSessionId(int sessionId);
    bool isImageEmbeddingSessionIdValid(int sessionId);
    bool getDefaultModel(const std::string& modelInfo, std::string& modelName);

    const std::unique_ptr<ai_engine::core_ai::embedding::AbstractEmbeddingEngine> selectImageEmbeddingEngine(
        std::string engineName) const;

private:
    static bool onHandleInitEngineV2(AiRuntimeCoreImageEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                     gpointer userData);

    static bool onHandleInitEngine(AiRuntimeCoreImageEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                   const gchar* inputJson, gpointer userData);

    static bool onHandleInitModel(AiRuntimeCoreImageEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                  gint sessionId, const gchar* model, gpointer userData);

    static bool onHandleEmbeddingText(AiRuntimeCoreImageEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                      const gchar* inputJson, gpointer userData);

    static bool onHandleEmbeddingImageFile(AiRuntimeCoreImageEmbeddingService* delegate,
                                           GDBusMethodInvocation* invocation, const gchar* inputJson,
                                           gpointer userData);

    static bool onHandleEmbeddingBase64Image(AiRuntimeCoreImageEmbeddingService* delegate,
                                             GDBusMethodInvocation* invocation, const gchar* inputJson,
                                             gpointer userData);

    static bool onHandleGetModelList(AiRuntimeCoreImageEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                     gint sessionId, gpointer userData);

    static bool onHandleGetModelInfo(AiRuntimeCoreImageEmbeddingService* delegate, GDBusMethodInvocation* invocation,
                                     gint sessionId, gpointer userData);

private:
    bool isExported_ = false;
    AiRuntimeCoreImageEmbeddingService* delegate_ = nullptr;
    GDBusConnection& connection_;
    EngineSelector& engineSelector_;
    std::set<int> sessionIdSet_;
    std::map<int, std::shared_ptr<ImageEmbedding>> imageEmbeddings_;
    std::vector<gulong> handlerIds_;
};

#endif

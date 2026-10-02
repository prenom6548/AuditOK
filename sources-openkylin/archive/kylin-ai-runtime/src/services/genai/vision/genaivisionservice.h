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

#ifndef SERVICES_GENAIVISIONSERVICE_H
#define SERVICES_GENAIVISIONSERVICE_H

#include <gio/gio.h>
#include <kylin-ai/aiengine.h>

#include <map>
#include <memory>
#include <set>
#include <string>

#include "engineselector/engineselector.h"
#include "genaivisionserviceglue.h"
#include "imagegenerator.h"

class GenAiVisionService : public std::enable_shared_from_this<GenAiVisionService> {
public:
    GenAiVisionService(GDBusConnection& connection, EngineSelector& engineSelector);

    ~GenAiVisionService();

    void stop();

private:
    void exportSkeleton();
    void connectSignal();
    std::string getProtocolVersion();
    void setRuntimeProtocolVersion(const std::string& version);
    void unexportSkeleton();
    int generateRandomSessionId();
    bool isSessionIdValid(int sessionId);

    GVariant* buildGVariantFromImageResult(const genai::ImageGenerationResult& result);
    void emitImageResultSignal(const std::string& interfaceName, const genai::ImageGenerationResult& result);
    static bool onHandleInit(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                             gpointer userData);

    static bool onHandleGenerateImage(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                                      gint sessionId, const gchar* prompt, gpointer userData);

    static bool onHandleGenerateImageWithRequestId(AiRuntimeGenAiVisionService* service,
                                                   GDBusMethodInvocation* invocation, gint sessionId,
                                                   const gchar* requestId, const gchar* config, gpointer userData);
    static bool onHandleStopImageGeneration(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                                            gint sessionId, gpointer userData);
    static bool onHandleStopImageGenerationWithRequestId(AiRuntimeGenAiVisionService* service,
                                                         GDBusMethodInvocation* invocation, gint sessionId,
                                                         const gchar* requestId, gpointer userData);
    static bool onHandleGetSupportedImageNumber(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                                                gint sessionId, gpointer userData);

    static bool onHandleGetSupportedImageSize(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                                              gint sessionId, gpointer userData);

    static bool onHandleGetSupportedImageStyle(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                                               gint sessionId, gpointer userData);

    static bool onHandleSetImageConfig(AiRuntimeGenAiVisionService* service, GDBusMethodInvocation* invocation,
                                       gint sessionId, const gchar* imageConfig, gpointer userData);

private:
    std::shared_ptr<GenAiVisionService> getSharedObject() { return shared_from_this(); }

private:
    bool isExported_ = false;
    AiRuntimeGenAiVisionService* delegate_ = nullptr;

    GDBusConnection& connection_;

    EngineSelector& engineSelector_;

    std::shared_ptr<ai_engine::gen_ai::vision::AbstractImageGenerationEngine> visionEngine_{nullptr};

    std::set<int> sessionIdSet_;
    std::map<int, std::shared_ptr<ImageGenerator>> imageGenerator_;

    std::map<int, std::atomic_bool> modelChangedMap_{};
    std::vector<gulong> handlerIds_;
};

#endif

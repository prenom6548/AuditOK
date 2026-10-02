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

#ifndef SERVICES_COREVISIONSERVICE_H
#define SERVICES_COREVISIONSERVICE_H

#include <gio/gio.h>
#include <kylin-ai/aiengine.h>

#include <map>
#include <memory>
#include <set>
#include <string>

#include "corevisionserviceglue.h"
#include "engine/aienginepluginmanager.h"
#include "engineselector/engineselector.h"
#include "segmentationsession.h"
#include "subjectsegmentation.h"
#include "textrecognition.h"

class CoreVisionService : public std::enable_shared_from_this<CoreVisionService> {
public:
    CoreVisionService(GDBusConnection& connection, EngineSelector& engineSelector);
    ~CoreVisionService();

    void stop();

private:
    void exportSkeleton();
    void connectSignal();
    void unexportSkeleton();
    std::string getProtocolVersion();
    void setRuntimeProtocolVersion(const std::string& version);
    int generateRandomSessionId();
    void releaseRandomSessionId(int sessionId);
    bool isTextRecognitionSessionIdValid(int sessionId);
    bool isSubjectSegmentationSessionIdValid(int sessionId);
    bool isSegmentationSessionIdValid(int sessionId);
    DeployType parseDeployType(const std::string& typeStr);
    int parseModeConfig(const std::string& jsonString);

    const std::unique_ptr<ai_engine::core_ai::vision::AbstractTextRecognitionEngine> selectTextRecognitionEngine();

    static void completeRecognizeTextFromImageFile(const TextRecognitionResult& result,
                                                   AiRuntimeCoreVisionService* service,
                                                   GDBusMethodInvocation* invocation);

    static void completeRecognizeTextFromImageFileWithRequestId(const TextRecognitionResult& result,
                                                                AiRuntimeCoreVisionService* service,
                                                                GDBusMethodInvocation* invocation);
    static void completeRecognizeTextFromImageData(const TextRecognitionResult& result,
                                                   AiRuntimeCoreVisionService* service,
                                                   GDBusMethodInvocation* invocation);
    static void completeRecognizeTextFromImageDataWithRequestId(const TextRecognitionResult& result,
                                                                AiRuntimeCoreVisionService* service,
                                                                GDBusMethodInvocation* invocation);

    static void completeSubjectSegmentationFromImageFile(const SubjectSegmentationResult& result,
                                                         AiRuntimeCoreVisionService* service,
                                                         GDBusMethodInvocation* invocation);

    static void completeSubjectSegmentationFromImageData(const SubjectSegmentationResult& result,
                                                         AiRuntimeCoreVisionService* service,
                                                         GDBusMethodInvocation* invocation);

    static bool onHandleInit(AiRuntimeCoreVisionService* service, GDBusMethodInvocation* invocation,
                             const gchar* modelconfig, gpointer userData);

    static bool onHandleDestroyTextRecognition(AiRuntimeCoreVisionService* service, GDBusMethodInvocation* invocation,
                                               gint sessionId, gpointer userData);

    static bool onHandleTextRecognitionFromImageFile(AiRuntimeCoreVisionService* service,
                                                     GDBusMethodInvocation* invocation, int sessionId,
                                                     const gchar* imageFilePath, gpointer userData);
    static bool onHandleTextRecognitionFromImageFileWithRequestId(AiRuntimeCoreVisionService* service,
                                                                  GDBusMethodInvocation* invocation, int sessionId,
                                                                  const gchar* requestId, const gchar* imageFilePath,
                                                                  gpointer userData);

    static bool onHandleTextRecognitionFromImageData(AiRuntimeCoreVisionService* service,
                                                     GDBusMethodInvocation* invocation, int sessionId,
                                                     const gchar* imageData, unsigned int imageDataLength,
                                                     gpointer userData);
    static bool onHandleTextRecognitionFromImageDataWithRequestId(AiRuntimeCoreVisionService* service,
                                                                  GDBusMethodInvocation* invocation, int sessionId,
                                                                  const gchar* requestId, const gchar* imageData,
                                                                  unsigned int imageDataLength, gpointer userData);

    static bool onHandleInitSubjectSegmentation(AiRuntimeCoreVisionService* service, GDBusMethodInvocation* invocation,
                                                gpointer userData);

    static bool onHandleSubjectSegmentationFromImageFileAsync(AiRuntimeCoreVisionService* service,
                                                              GDBusMethodInvocation* invocation, int sessionId,
                                                              const gchar* requestId, const gchar* imageFilePath,
                                                              const gchar* config, gpointer userData);

    static bool onHandleSubjectSegmentationFromImageDataAsync(AiRuntimeCoreVisionService* service,
                                                              GDBusMethodInvocation* invocation, int sessionId,
                                                              const gchar* requestId, const gchar* imageData,
                                                              unsigned int imageDataLength, const gchar* config,
                                                              gpointer userData);

    static bool onHandleDestroySubjectSegmentation(AiRuntimeCoreVisionService* service,
                                                   GDBusMethodInvocation* invocation, gint sessionId,
                                                   gpointer userData);

    static bool onHandleInitSegmentation(AiRuntimeCoreVisionService* service, GDBusMethodInvocation* invocation,
                                         gpointer userData);

    static bool onHandleDestroySegmentation(AiRuntimeCoreVisionService* service, GDBusMethodInvocation* invocation,
                                            gint sessionId, gpointer userData);

    static bool onHandleEncodeSegmentationFromImageFile(AiRuntimeCoreVisionService* service,
                                                        GDBusMethodInvocation* invocation, gint sessionId,
                                                        const gchar* imageFilePath, gpointer userData);

    static bool onHandleEncodeSegmentationFromImageData(AiRuntimeCoreVisionService* service,
                                                        GDBusMethodInvocation* invocation, gint sessionId,
                                                        const gchar* base64ImageData, gpointer userData);

    static bool onHandleClearSegmentationCache(AiRuntimeCoreVisionService* service, GDBusMethodInvocation* invocation,
                                               gint sessionId, gpointer userData);

    static bool onHandleRunSegmentation(AiRuntimeCoreVisionService* service, GDBusMethodInvocation* invocation,
                                        gint sessionId, GVariant* pointsVariant, guint32 minArea, gpointer userData);

private:
    std::shared_ptr<CoreVisionService> getSharedObject() { return shared_from_this(); }

    void sendSegmentationResult(int sessionId, const SegmentationSession::SegmentationResult& result);

private:
    bool isExported_ = false;
    AiRuntimeCoreVisionService* delegate_ = nullptr;

    GDBusConnection& connection_;

    EngineSelector engineSelector_;

    std::set<int> sessionIdSet_;
    std::map<int, std::shared_ptr<TextRecognition>> textRecognitions_;
    std::map<int, std::shared_ptr<SubjectSegmentation>> subjectSegmentations_;
    std::map<int, std::shared_ptr<SegmentationSession>> segmentationSessions_;
    std::vector<gulong> handlerIds_;
};

#endif
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

#ifndef SERVICES_IMAGEEMBEDDINGPROCESSORPROXY_H
#define SERVICES_IMAGEEMBEDDINGPROCESSORPROXY_H

#include <gio/gio.h>

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "_modelinfo.h"
#include "coreimageembeddingserviceglue.h"
#include "embeddingcommon.h"
#include "imageembeddingserver.h"

namespace kyai::coreai::embedding {
class ImageEmbeddingProcessorProxy
    : public std::enable_shared_from_this<ImageEmbeddingProcessorProxy> {
public:
    using ImageEmbeddingResultCallback =
        std::function<void(EmbeddingResult *result, void *userData)>;

    struct EmbeddingRequestContext {
        ImageEmbeddingResultCallback callback;
        void *userData;
        std::weak_ptr<ImageEmbeddingProcessorProxy> proxy;
    };

public:
    ~ImageEmbeddingProcessorProxy() = default;

    int connect();

    int initEngine();

    int initModel(const std::string &model);

    EmbeddingResult *embeddingText(const std::string &text);

    EmbeddingResult *embeddingImage(const std::string &filePath);

    EmbeddingResult *embeddingBase64Image(const std::string &fileData);

    void embeddingTextAsync(const std::string &text,
                            ImageEmbeddingResultCallback callback,
                            void *userData);

    void embeddingImageAsync(const std::string &filePath,
                             ImageEmbeddingResultCallback callback,
                             void *userData);

    void embeddingBase64ImageAsync(const std::string &fileData,
                                   ImageEmbeddingResultCallback callback,
                                   void *userData);

    _EmbeddingModelList *getModelList(int *errorCode);

    std::string getModelInfo();

private:
    int init_v1(int errorCode);

    int init_v2(int errorCode);

    bool parseModelList(const std::string &modelList,
                        _EmbeddingModelList &list);

    // 新 SDK + 新 runtime 场景下，如果用户未显式
    // initModel，则按规则选择默认模型并加载
    int ensureDefaultModelLoadedIfNeeded();

    int getDefaultModel(std::string &model);

    static EmbeddingResult parseResult(const std::string &jsonResult);

    static void embeddingTextAsyncCallback(GObject *object, GAsyncResult *res,
                                           gpointer data);
    static void embeddingImageAsyncCallback(GObject *object, GAsyncResult *res,
                                            gpointer data);
    static void embeddingBase64ImageAsyncCallback(GObject *object,
                                                  GAsyncResult *res,
                                                  gpointer data);

    bool ensureProtocolSupportedForApi(const char *apiName,
                                       int *errorCode = nullptr);
    int reconnectAndRetry();
    void handleConnectionError(GError *gError);
    std::string handleErrorOccurred(GError *gError);

private:
    AiRuntimeCoreImageEmbeddingService *delegate_ = nullptr;
    ImageEmbeddingServer *server_ = nullptr;
    int sessionId_ = -1;

    bool init_ = false;
    std::string model_ = "";
    std::string runtimeVersion_ = "1.0.0";
    _EmbeddingModelList modelList_;
};
}  // namespace kyai::coreai::embedding
#endif
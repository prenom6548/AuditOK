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

#ifndef IMAGEEMBEDDINGSESSION_H
#define IMAGEEMBEDDINGSESSION_H

#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "_modelinfo.h"
#include "embedding.h"
#include "embeddingcommon.h"
#include "imageembeddingprocessorproxy.h"

class _ImageEmbeddingSession {
public:
    _ImageEmbeddingSession();
    ~_ImageEmbeddingSession();

    int connect();

    int init();

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

    void enableInternalEventLoop(bool enable);

private:
    void startEventLoop();
    void stopEventLoop();

private:
    std::shared_ptr<kyai::coreai::embedding::ImageEmbeddingProcessorProxy>
        imageEmbeddingProcessorProxy_;
    GMainLoop *loop_ = nullptr;
    std::thread thread_;
    std::mutex mutex_;
};

#endif

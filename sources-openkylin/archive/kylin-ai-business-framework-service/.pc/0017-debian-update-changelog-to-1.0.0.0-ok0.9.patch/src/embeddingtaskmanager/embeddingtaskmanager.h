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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef EMBEDDINGTASKMANAGER_H
#define EMBEDDINGTASKMANAGER_H

#include "embeddingtaskmanager/embeddingtask.h"
#include "embeddingtaskmanager/baseembeddingservice.h"

#include <memory>

class EmbeddingTaskManager
{
public:
    static EmbeddingTaskManager* getInstance() {
        static EmbeddingTaskManager manager;
        return &manager;
    }

    ~EmbeddingTaskManager();
    void addImageSideTask(const EmbeddingTask& task);
    void addTextSideTask(const EmbeddingTask& task);

    std::vector<float> getImageSideResult(TaskID id);
    std::vector<float> getTextSideResult(TaskID id);

private:
    EmbeddingTaskManager();

private:
    std::unique_ptr<BaseEmbeddingService> textEmbeddingService_;
    std::unique_ptr<BaseEmbeddingService> imageEmbeddingService_;
};

#endif // EMBEDDINGTASKMANAGER_H

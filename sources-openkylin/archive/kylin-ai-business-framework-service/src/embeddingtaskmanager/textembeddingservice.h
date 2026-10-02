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

#ifndef TEXTEMBEDDINGSERVICE_H
#define TEXTEMBEDDINGSERVICE_H

#include "embeddingtaskmanager/baseembeddingservice.h"
#include "utils/textembedder.h"

#include <memory>

class TextEmbeddingService : public BaseEmbeddingService
{
public:
    FeatureStatus checkServiceFeatureStatus() override;

protected:
    void initResourceInThread() override;
    std::vector<float> doEmbeddingInThread(const EmbeddingTask& task) override;

private:
    std::unique_ptr<TextEmbedder> embedder_;
};

#endif // TEXTEMBEDDINGSERVICE_H

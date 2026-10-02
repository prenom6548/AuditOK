/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
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

#ifndef IMAGEEMBEDDINGPROXY_H
#define IMAGEEMBEDDINGPROXY_H

#include <string>
#include <vector>

typedef struct _ImageEmbeddingSession ImageEmbeddingSession;

namespace scene {

class ImageEmbeddingProxy {
public:
    ImageEmbeddingProxy() = default;
    ~ImageEmbeddingProxy();
    bool init();

    std::vector<float> embeddingText(const std::string& text);

    std::vector<float> embeddingImage(const std::string& imageBase64);
    std::vector<float> embeddingImageFile(const std::string& filepath);
    std::string modelInfo();

private:
    void destory();

private:
    ImageEmbeddingSession* session_;
};
}  // namespace scene

#endif  // IMAGEEMBEDDINGPROXY_H

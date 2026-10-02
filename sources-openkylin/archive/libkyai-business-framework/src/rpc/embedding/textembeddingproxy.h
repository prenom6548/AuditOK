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

#ifndef TEXTEMBEDDINGPROXY_H
#define TEXTEMBEDDINGPROXY_H

#include <string>
#include <vector>

typedef struct _TextEmbeddingSession TextEmbeddingSession;

namespace scene {
class TextEmbeddingProxy {
public:
    TextEmbeddingProxy() = default;
    ~TextEmbeddingProxy();
    bool init();

    std::vector<float> embeddingText(const std::string& text);
    std::string modelInfo();

private:
    void destory();

private:
    TextEmbeddingSession* session_ = nullptr;
};
}  // namespace scene

#endif  // TEXTEMBEDDINGPROXY_H

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

#ifndef GENAITEXTENGINE_H
#define GENAITEXTENGINE_H

#include <functional>
#include <string>

#include "aiengine.h"
#include "aiengineerror.h"
#include "aiengineresult.h"

namespace ai_engine {
namespace gen_ai {
namespace nlp {

struct TextGenerationResult : result::AiEngineModelResult {
    std::string text;
};

using TextGenerationResultCallback =
    std::function<void(const TextGenerationResult &result)>;

class AbstractTextGenerationEngine : public ai_engine::AbstractAiEngine {
public:
    virtual ~AbstractTextGenerationEngine() {}

    virtual void setTextGenerationResultCallback(
        TextGenerationResultCallback callback) = 0;

    // 初始化对话模块
    virtual result::AiEngineBaseResult init() = 0;

    // 销毁对话模块
    virtual result::AiEngineBaseResult destroy() = 0;

    // 生成内容
    virtual result::AiEngineBaseResult generateContent(
        const std::string &input) = 0;

    // 对话
    // message 为json格式的字符串
    virtual result::AiEngineBaseResult chat(const std::string &message) = 0;

    // 停止生成
    virtual void stopGenerateContent() = 0;

    // 停止对话
    virtual void stopChat() = 0;
};

}  // namespace nlp
}  // namespace gen_ai
}  // namespace ai_engine

#endif  // GENAITEXTENGINE_H

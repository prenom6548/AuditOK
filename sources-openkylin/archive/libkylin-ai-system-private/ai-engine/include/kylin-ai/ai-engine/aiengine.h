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

#ifndef AIENGINE_LM_H
#define AIENGINE_LM_H

#include <string>
namespace ai_engine {

class AbstractAiEngine {
public:
    virtual ~AbstractAiEngine() {}

    // 引擎的名称
    virtual std::string engineName() const = 0;

    // 是否是云端引擎
    virtual bool isCloud() const = 0;

    // 是否是内置引擎
    // AI子系统提供内置的模型，调用者不可以指定和切换模型
    // 包括向量化模型、OCR等
    virtual bool isBuiltInEngine() const = 0;

    // 是否是自定义模型引擎
    // 自定义模型引擎需要满足openai chat api接口格式
    virtual bool isCustomModelEngine() const { return false; }

    // 引擎使用的模型信息
    // json格式的字符串，包括模型的名称，厂商，需要的配置信息等
    // 向量化
    virtual std::string modelInfo() const = 0;

    // 设置当前引擎使用的模型
    // 针对使用一个模型实现所有功能的引擎，如果是内置引擎，则该接口无效
    virtual bool setCurrentModel(const std::string &model) {
        return false;
    }

    // 当前引擎使用的模型
    // 针对使用一个模型实现所有功能的引擎，返回的是modelVersion
    virtual std::string currentModel() const { return {}; }

    // 当前引擎使用的模型名称
    virtual std::string currentModelName() const { return {}; }

    // 设置模型的配置信息
    // 1. json格式的字符串，部分字段需要从modelInfo中获取
    // 2. 云端模型可能需要配置账号信息
    // 3. 自定义模型需要配置模型的请求地址、apiKey和模型模型等
    virtual void setConfig(const std::string &config) {};
};

}  // namespace ai_engine

#endif  // AIENGINE_H

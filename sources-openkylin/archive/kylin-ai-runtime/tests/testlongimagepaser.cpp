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

#include <kylin-ai/aiengine.h>

#include <iostream>
#include <sstream>
#include <string>

#include "../src/engine/aienginepluginmanager.h"
#include "../src/engineselector/engineselector.h"
#include "../src/longchatimageparser/processer/longimageparser.h"
#include "../src/longchatimageparser/utils/jsonexporter.h"
#include "../src/server/servermanager.h"
#include "../src/utils/logger.h"
#include "engine/aienginepluginmanager.h"
#include "threadpool/async.h"

/**
 * @brief 长图OCR功能整体测试
 *
 * 测试目标：
 * 1. 验证 LongImageParser 可以正常创建
 * 2. 调用整体接口处理测试图片
 * 3. 检查返回的结果是否有效
 */

int main() {
    try {
        // 创建 LongImageParser 实例
        LOGI("[1] 创建 LongImageParser 实例...");
        // auto TextRecgEngine = ai_engine::AiEnginePluginManager::getInstance().createTextRecognitionEngine();

        // logger::init();

        ai_engine::AiEnginePluginManager &aiEnginePluginManager = ai_engine::AiEnginePluginManager::getInstance();
        ModelConfigManager &modelConfigManager = ModelConfigManager::getInstance();
        EngineSelector engineSelector(aiEnginePluginManager, modelConfigManager);

        auto TextRecgEngine = engineSelector.selectTextRecognitionEngine();

        if (TextRecgEngine == nullptr) {
            LOGE("TextRecgEngine == nullptr)");
            return -1;
        }

        parser::LongImageParser parser(std::move(TextRecgEngine));
        LOGI("✓ 实例创建成功");
        std::filesystem::path dir = std::filesystem::path(__FILE__).parent_path();
        std::filesystem::path path = dir.parent_path() / "test" / "images" / "image copy 8.png";
        std::string test_image_path = path.string();
        utils::ProcessResult result = parser.extractImageContents(test_image_path);

        // 检查处理结果
        if (result.isSuccess()) {
            LOGI("✓ 处理成功");
            if (result.hasResult()) {
                std::string result_str = result.getResultString();
                // 解析JSON并打印完整的chat_messages
                try {
                    Json::Value json_result;
                    Json::CharReaderBuilder builder;
                    std::string errors;
                    std::istringstream iss(result_str);
                    Json::parseFromStream(builder, iss, &json_result, &errors);
                    if (json_result.isMember("chat_messages")) {
                        LOGI("\n处理结果（JSON格式）:");
                        LOGI(export_ns::json_dump(json_result["chat_messages"], 2));
                    } else {
                        LOGI("\n完整处理结果:");
                        LOGI(result_str);
                    }
                } catch (...) {
                    // JSON解析失败，输出原始字符串
                    LOGI("\n完整处理结果:");
                    // std::cout << result_str << std::endl;
                    LOGI(result_str);
                }
            }

            LOGI("处理完成！");

            LOGI((std::string("处理耗时: ") + std::to_string(result.getProcessingTimeMs()) + " 毫秒").c_str());
        } else {
            LOGI("✗ 处理失败 (预期结果)");

            LOGI((std::string("    错误码: ") + std::to_string(result.getErrorCodeInt())).c_str());

            LOGI((std::string("    错误描述: ") + result.getFullErrorDescription()).c_str());
            if (result.isCritical()) {
                LOGI("    这是一个严重错误");
            }
        }
        return 0;

    } catch (const std::exception &e) {
        std::cerr << "\n✗ 测试过程中发生异常: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\n✗ 测试过程中发生未知异常" << std::endl;
        return 2;
    }
}

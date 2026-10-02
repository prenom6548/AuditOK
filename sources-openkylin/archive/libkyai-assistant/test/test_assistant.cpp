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


#include <gio/gio.h>
#include <gio/giotypes.h>

#include <iostream>
#include <thread>

#include "aiconfig/modelconfig.h"
#include "kylin-ai/private/osassistant/error.h"
#include "kylin-ai/private/osassistant/osassistant.h"

void setXunfeiAuthens() {
    kyai::config::model::ModelConfig mc;
    std::vector<kyai::config::model::MultiAuthentication> modelsAuthens;
    modelsAuthens.push_back(
        {"", {{"", "appId", ""}, {"", "apiKey", ""}, {"", "secretKey", ""}}});

    mc.setModelAuthentications("讯飞-Spark Max", modelsAuthens);
}
void setBaiduAuthens() {
    kyai::config::model::ModelConfig mc;
    std::vector<kyai::config::model::MultiAuthentication> modelsAuthens;
    modelsAuthens.push_back({"", {{"", "apiKey", ""}, {"", "secretKey", ""}}});

    mc.setModelAuthentications("百度-ERNIE-Bot-4", modelsAuthens);
}

void setXunfeiAuthens2() {
    kyai::config::model::ModelConfig mc;
    std::vector<kyai::config::model::MultiAuthentication> modelsAuthens;
    modelsAuthens.push_back(
        {"", {{"", "appId", ""}, {"", "apiKey", ""}, {"", "secretKey", ""}}});

    mc.setModelAuthentications("讯飞-图片生成", modelsAuthens);
}

void setBaiduAuthens2() {
    kyai::config::model::ModelConfig mc;
    std::vector<kyai::config::model::MultiAuthentication> modelsAuthens;
    modelsAuthens.push_back(
        {"", {{"", "appId", ""}, {"", "apiKey", ""}, {"", "secretKey", ""}}});
    mc.setModelAuthentications("百度-文心AI作画高级版", modelsAuthens);
}

void setModelConfig(const std::string &modelName) {
    setBaiduAuthens();
    setXunfeiAuthens();
    kyai::config::model::ModelConfig mc;
    mc.setCurrentModelName(kyai::config::model::AiCapability::Nlp,
                           kyai::config::model::DeployType::PublicCloud,
                           modelName);
}

void setModelConfig2(const std::string &modelName) {
    setBaiduAuthens2();
    setXunfeiAuthens2();

    kyai::config::model::ModelConfig mc;
    mc.setCurrentModelName(kyai::config::model::AiCapability::Vision,
                           kyai::config::model::DeployType::PublicCloud,
                           modelName);
}

void test01() {
    using namespace kyai::assistant;

    ChatResultCallback callback = [](const std::string &message) {
        std::cout << message << '\n';
    };

    OsAssistant assistant;
    Error error = assistant.init();
    if (error) {
        std::cout << "初始化错误" << '\n';
    }

    assistant.setChatAsyncCallback(callback);

    assistant.chatAsync(R"(
                {
                    "message_type": "chat",
                    "content": [
                        {"type": "text", "text": "你好"}
                    ]
                }
            )");

    auto functions = assistant.supportedFeatures();
    if (functions.containsFeature(Features::DocumentQA)) {
        std::cout << "支持文档问答" << std::endl;
    } else {
        std::cout << "不支持文档问答" << std::endl;
    }

    GMainLoop *loop = g_main_loop_new(nullptr, false);
    std::thread ctrlThread([loop, &assistant] {
        while (std::getchar() != '\n') {
        }
        std::cout << "生成一张图片" << '\n';
        assistant.chatAsync(R"(
                    {
                        "message_type": "chat",
                        "content": [
                            {"type": "text", "text": "生成一张图片"}
                        ]
                    }
                )");

        while (std::getchar() != '\n') {
        }
        std::cout << "退出" << '\n';
        g_main_loop_quit(loop);
    });
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
}

void test02() {
    setModelConfig("百度-ERNIE-Bot-4");
    setModelConfig2("百度-文心AI作画高级版");

    kyai::assistant::OsAssistant assistant;
    assistant.setChatAsyncCallback([](const std::string &result) {
        std::fprintf(stdout, "Os Assistant Result: %s\n", result.c_str());
    });

    std::vector<kyai::assistant::Prompt> allPrompts1 = assistant.prompts();
    for (auto item : allPrompts1) {
        fprintf(stderr, "get all prompts: %i %s %s %i\n", item.id,
                item.name.c_str(), item.content.c_str(), item.type);
    }

    auto *loop_ = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop_);
}

void testImagHandler() {
    kyai::assistant::OsAssistant assistant;

    assistant.setChatAsyncCallback([](const std::string &result) {
        std::fprintf(stdout, "Os Assistant Result: %s\n", result.c_str());
    });
    assistant.init();

    assistant.chatAsync(R"(
                    {
                        "message_type": "chat",
                        "content": [
                            {"type": "image_url", "image_url": "dddd"}
                        ]
                    }
                )");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    assistant.chatAsync(R"(
                    {
                        "message_type": "tool_call",
                            "file_type": "image",
                            "content": [
                                {"type": "tool_id", "tool_id": 1},
                                {"type": "image_url", "image_url": "/tmp/1.png"}
                            ]
                    }
                )");

    auto *loop = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop);
}

void testText2Image() {
    kyai::assistant::OsAssistant assistant;

    assistant.setChatAsyncCallback([](const std::string &result) {
        std::fprintf(stdout, "Os Assistant Result: %s\n", result.c_str());
    });
    assistant.init();

    for (int i = 0; i < 10; i++) {
        assistant.chatAsync(R"(
                    {
                        "message_type": "chat",
                        "content": [
                            {"type": "text", "text": "生成一张小狗的图片"}
                        ]
                    }
                )");
    }

    auto *loop = g_main_loop_new(nullptr, false);
    g_main_loop_run(loop);
}

int main() {
    test01();
    testImagHandler();
    testText2Image();
    return 0;
}
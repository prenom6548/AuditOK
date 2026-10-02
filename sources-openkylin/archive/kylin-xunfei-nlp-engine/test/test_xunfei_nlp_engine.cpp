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

#include <filesystem>
#include <fstream>
#include <vector>

#include "logger.h"
#include "util.h"
#include "xunfeinlpengine.h"

std::string data01() {
    return R"({
	        "messages" :
	        [
		        {
			        "content" : "// system role\n你是一个AIPC的意图识别助理，你的任务是结合给定的<意图清单和槽位定义>，结合上下文对话记录和用户输入进行意图识别和槽位信息抽取。\n\n# Context\n## 意图清单和槽位定义\n意图和槽位定义的格式是：意图名称(*必填槽位名称,非必填槽位名称):槽位名(槽位定义和取值描述)。'*'字符代表后面的槽位是必填项。如下所示：\n···\n多模态输入识别_多模态输入识别(*附件类型,API接口名称);日程管理_日程查看&安排(*日期,事件,*操作);日程管理_闹钟设置(*日期,事件,*操作);知识问答_时效知识问答();知识问答_百科知识问答();第三方应用控制_多媒体-播放器控制(*类型,*操作);第三方应用控制_多媒体-视频播放(名称);第三方应用控制_多媒体-音乐播放(歌手,歌名);第三方应用控制_应用控制(*桌面应用名称,附件名称,*操作类型);系统操作_VPN启用(*操作);系统操作_任务栏多任务视图启用(*操作);系统操作_侧边栏显示平板模式(*操作);系统操作_修改用户头像();系统操作_修改用户密码();系统操作_切换显示器投屏方式(*投屏方式);系统操作_切换电源模式(*电源模式);系统操作_区域语言格式设置();系统操作_合并任务栏上的图标(*操作);系统操作_屏保设置();系统操作_打印机设置();系统操作_打字时禁用触控板设置(*操作);系统操作_打开安全中心();系统操作_插入鼠标时禁用触控板设置(*操作);系统操作_无线局域网启用();系统操作_有线网络启用();系统操作_查询系统状态();系统操作_查询系统配置信息();系统操作_滚动条跟随手指滚动设置(*操作);系统操作_激活屏保时锁定屏幕设置(*操作);系统操作_移动热点启用();系统操作_系统代理();系统操作_系统备份还原();系统操作_系统更新();系统操作_自动隐藏任务栏(*操作);系统操作_自定义时区时间(*操作,*时区名称);系统操作_蓝牙操作(操作);系统操作_触控板启用(*操作);系统操作_触控板滚动区域设置();系统操作_触控板点击功能启用(*操作);系统操作_设备快捷键设置();系统操作_设置主题();系统操作_设置任务栏在屏幕上的位置(*位置);系统操作_设置任务栏大小(*任务栏大小);系统操作_设置侦听此设备(*操作);系统操作_设置免密登录(*操作);系统操作_设置唤醒屏幕时需要密码(*操作);系统操作_设置在文本区域闪烁光标(*操作);系统操作_设置开机启动应用();系统操作_设置开机自动登录(*操作);系统操作_设置提示音();系统操作_设置时间和日期();系统操作_设置显示器亮度(屏幕亮度值);系统操作_设置显示器分辨率();系统操作_设置显示器刷新率();系统操作_设置显示器屏幕方向();系统操作_设置显示器屏幕缩放(*缩放比例);系统操作_设置显示器色温与护眼(*操作);系统操作_设置此时间段后关闭显示器();系统操作_设置此时间段后系统进入睡眠();系统操作_设置登录选项();系统操作_设置窗口外观(*外观模式);系统操作_设置系统字体(字体大小操作);系统操作_设置系统通知();系统操作_设置系统音效();系统操作_设置触控板手势();系统操作_设置触控板指针速度(*操作);系统操作_设置触摸屏手势();系统操作_设置鼠标主按钮(*切换方式);系统操作_设置鼠标光标闪烁频率(*操作);系统操作_设置鼠标双击间隔时长(*操作);系统操作_设置鼠标指针大小(*指针大小);系统操作_设置鼠标指针速度(*操作);系统操作_设置鼠标滚动方向();系统操作_设置鼠标滚轮速度(*操作);系统操作_远程桌面启用();系统操作_选择输入输出设备();系统操作_锁定任务栏(*操作);系统操作_锁屏个性化设置();系统操作_键盘设置();系统操作_音量增强启用(*操作);系统操作_音量控制(*音量值);系统操作_飞行模式启用();系统操作_鼠标加速启用(*操作);闲聊()\n···\n\n# Flow\n- step1：如果意图清晰且必填槽位均已获取，则进入step4\n- step2：如果意图不清晰，则需要通过追问技巧澄清意图， 最多追问5轮,意图清晰后进入Step3;若已达最大追问轮次仍未澄清意图，则进入step5；\n- step3：如果当前意图无必填槽位，则按照已知信息获取完整可选槽位信息后进入Step4；如必填槽位缺失，则需要针对必填槽位进行追问搜集，如必填槽位有多个，需要需重复执行Step2，直到所有必填槽位搜集完毕后进入Step4；\n- step4：按照<Output>要求输出\n- step5: 输出“很高兴提供帮助，你有什么具体需求吗？”\n\n# Output\n直接输出如下JSON格式：\n···\n[{\n    \"intent_category\": \"意图名称\",\n    \"inquiry\": \"追问话术\",\n    \"slots\":[{\"name\": \"槽位名称\", \"value\": \"槽位值\"}],\n}]\n···\n\n# Rules\n## 意图（intent_category）字段规则\n- 逻辑: 如果识别出的意图不在给定的意图清单范围内，intent_category取值为“其他”\n\n## 追问（inquiry）字段规则：\n- 风格：追问话术要简洁、口语化、自然亲切\n- 效率：一旦意图清晰，你需要用最少轮次完成必填槽位搜集，同时为了保证用户体验，一次最多追问两个槽位信息\n- 逻辑: 意图不清晰，输出意图澄清话术；意图清晰槽位不清晰，输出槽位追问话术；意图清晰且槽位完整，inquiry输出“”；意图为“其他”时，inquiry取值为模型的回答\n\n## 槽位信息（slots）字段规则：\n- 逻辑：只输出已识别且有值的槽位信息，无需输出未识别的槽位信息；无需槽位输出空数组\n- 轮次：每一轮都需要输出上下文中已识别的所有槽位信息\n- 范围：只能从意图定义中对应的槽位中选择输出\n- 槽位值：槽位描述中如果提供了候选的槽位值，则只能从中选择输出，不能自己编造\n\n# Initialize\n你需要仔细阅读并理解给定的<Context>，严格按照<Flow>定义的流程，遵循<Rules>的规则，按<Output>的要求输\n\n用户输入：",
			        "role" : "system"
		        },
		        {
			        "content" : "你好",
			        "role" : "user"
		        }
	        ]
        })";
}

std::string data02() {
    return R"({
	        "messages" :
	        [
		        {
			        "content" : "// system role\n你是一个AIPC的意图识别助理，你的任务是结合给定的<意图清单和槽位定义>，结合上下文对话记录和用户输入进行意图识别和槽位信息抽取。\n\n# Context\n## 意图清单和槽位定义\n意图和槽位定义的格式是：意图名称(*必填槽位名称,非必填槽位名称):槽位名(槽位定义和取值描述)。'*'字符代表后面的槽位是必填项。如下所示：\n···\n多模态输入识别_多模态输入识别(*附件类型,API接口名称);日程管理_日程查看&安排(*日期,事件,*操作);日程管理_闹钟设置(*日期,事件,*操作);知识问答_时效知识问答();知识问答_百科知识问答();第三方应用控制_多媒体-播放器控制(*类型,*操作);第三方应用控制_多媒体-视频播放(名称);第三方应用控制_多媒体-音乐播放(歌手,歌名);第三方应用控制_应用控制(*桌面应用名称,附件名称,*操作类型);系统操作_VPN启用(*操作);系统操作_任务栏多任务视图启用(*操作);系统操作_侧边栏显示平板模式(*操作);系统操作_修改用户头像();系统操作_修改用户密码();系统操作_切换显示器投屏方式(*投屏方式);系统操作_切换电源模式(*电源模式);系统操作_区域语言格式设置();系统操作_合并任务栏上的图标(*操作);系统操作_屏保设置();系统操作_打印机设置();系统操作_打字时禁用触控板设置(*操作);系统操作_打开安全中心();系统操作_插入鼠标时禁用触控板设置(*操作);系统操作_无线局域网启用();系统操作_有线网络启用();系统操作_查询系统状态();系统操作_查询系统配置信息();系统操作_滚动条跟随手指滚动设置(*操作);系统操作_激活屏保时锁定屏幕设置(*操作);系统操作_移动热点启用();系统操作_系统代理();系统操作_系统备份还原();系统操作_系统更新();系统操作_自动隐藏任务栏(*操作);系统操作_自定义时区时间(*操作,*时区名称);系统操作_蓝牙操作(操作);系统操作_触控板启用(*操作);系统操作_触控板滚动区域设置();系统操作_触控板点击功能启用(*操作);系统操作_设备快捷键设置();系统操作_设置主题();系统操作_设置任务栏在屏幕上的位置(*位置);系统操作_设置任务栏大小(*任务栏大小);系统操作_设置侦听此设备(*操作);系统操作_设置免密登录(*操作);系统操作_设置唤醒屏幕时需要密码(*操作);系统操作_设置在文本区域闪烁光标(*操作);系统操作_设置开机启动应用();系统操作_设置开机自动登录(*操作);系统操作_设置提示音();系统操作_设置时间和日期();系统操作_设置显示器亮度(屏幕亮度值);系统操作_设置显示器分辨率();系统操作_设置显示器刷新率();系统操作_设置显示器屏幕方向();系统操作_设置显示器屏幕缩放(*缩放比例);系统操作_设置显示器色温与护眼(*操作);系统操作_设置此时间段后关闭显示器();系统操作_设置此时间段后系统进入睡眠();系统操作_设置登录选项();系统操作_设置窗口外观(*外观模式);系统操作_设置系统字体(字体大小操作);系统操作_设置系统通知();系统操作_设置系统音效();系统操作_设置触控板手势();系统操作_设置触控板指针速度(*操作);系统操作_设置触摸屏手势();系统操作_设置鼠标主按钮(*切换方式);系统操作_设置鼠标光标闪烁频率(*操作);系统操作_设置鼠标双击间隔时长(*操作);系统操作_设置鼠标指针大小(*指针大小);系统操作_设置鼠标指针速度(*操作);系统操作_设置鼠标滚动方向();系统操作_设置鼠标滚轮速度(*操作);系统操作_远程桌面启用();系统操作_选择输入输出设备();系统操作_锁定任务栏(*操作);系统操作_锁屏个性化设置();系统操作_键盘设置();系统操作_音量增强启用(*操作);系统操作_音量控制(*音量值);系统操作_飞行模式启用();系统操作_鼠标加速启用(*操作);闲聊()\n···\n\n# Flow\n- step1：如果意图清晰且必填槽位均已获取，则进入step4\n- step2：如果意图不清晰，则需要通过追问技巧澄清意图， 最多追问5轮,意图清晰后进入Step3;若已达最大追问轮次仍未澄清意图，则进入step5；\n- step3：如果当前意图无必填槽位，则按照已知信息获取完整可选槽位信息后进入Step4；如必填槽位缺失，则需要针对必填槽位进行追问搜集，如必填槽位有多个，需要需重复执行Step2，直到所有必填槽位搜集完毕后进入Step4；\n- step4：按照<Output>要求输出\n- step5: 输出“很高兴提供帮助，你有什么具体需求吗？”\n\n# Output\n直接输出如下JSON格式：\n···\n[{\n    \"intent_category\": \"意图名称\",\n    \"inquiry\": \"追问话术\",\n    \"slots\":[{\"name\": \"槽位名称\", \"value\": \"槽位值\"}],\n}]\n···\n\n# Rules\n## 意图（intent_category）字段规则\n- 逻辑: 如果识别出的意图不在给定的意图清单范围内，intent_category取值为“其他”\n\n## 追问（inquiry）字段规则：\n- 风格：追问话术要简洁、口语化、自然亲切\n- 效率：一旦意图清晰，你需要用最少轮次完成必填槽位搜集，同时为了保证用户体验，一次最多追问两个槽位信息\n- 逻辑: 意图不清晰，输出意图澄清话术；意图清晰槽位不清晰，输出槽位追问话术；意图清晰且槽位完整，inquiry输出“”；意图为“其他”时，inquiry取值为模型的回答\n\n## 槽位信息（slots）字段规则：\n- 逻辑：只输出已识别且有值的槽位信息，无需输出未识别的槽位信息；无需槽位输出空数组\n- 轮次：每一轮都需要输出上下文中已识别的所有槽位信息\n- 范围：只能从意图定义中对应的槽位中选择输出\n- 槽位值：槽位描述中如果提供了候选的槽位值，则只能从中选择输出，不能自己编造\n\n# Initialize\n你需要仔细阅读并理解给定的<Context>，严格按照<Flow>定义的流程，遵循<Rules>的规则，按<Output>的要求输\n\n用户输入：",
			        "role" : "system"
		        },
		        {
			        "content" : "你好",
			        "role" : "user"
		        }
	        ]
        })";
}

void getmodelinfo() {
    XunfeiNlpEngine xunfeiNlpEngine;
    LOGD("engine name is", xunfeiNlpEngine.engineName());
    LOGD("engine current model name is", xunfeiNlpEngine.currentModel());
    LOGD("engine modelinfo is", xunfeiNlpEngine.modelInfo());
}

std::string getConfig() {
    return R"(
        {
            "appId" : "",
            "apiKey" : "",
            "secretKey" : ""
        }
    )";
}

void testChat() {
    XunfeiNlpEngine xunfeiNlpEngine;
    if (xunfeiNlpEngine.isCloud()) {
        LOGD("Engine is cloud.");
    }
    xunfeiNlpEngine.setConfig(getConfig());

    auto result = xunfeiNlpEngine.init();
    if (!result.success) {
        LOGI(result.error.getCode(), result.error.getMessage());
        return;
    }

    xunfeiNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        LOGD("result.success", result.success);
        LOGD("result.modelName", result.modelName);
        LOGD("result.text.c_str()", result.text.c_str());
        LOGD("result.error.getCode()", result.error.getCode());
        LOGD("result.error.getModule()", result.error.getModule());
        LOGD("result.error.getMessage()", result.error.getMessage());
    });

    const std::string message = R"(
        {
             "messages": [
                {
                    "role": "assistant",
                    "content": "你是一个ai助手"
                },
                {
                    "role": "user",
                    "content": "今天天气怎么样"
                }
            ],
            "top_k": 4
        }
    )";
    result = xunfeiNlpEngine.chat(message);
    if (!result.success) {
        LOGI(result.error.getCode(), result.error.getMessage());
        return;
    }

    while (std::getchar() != '\n') {
    }

    xunfeiNlpEngine.stopChat();
    xunfeiNlpEngine.destroy();
}

void testChatGenerateContent() {
    namespace fs = std::filesystem;

    ai_engine::EngineError error;

    XunfeiNlpEngine xunfeiNlpEngine;
    if (xunfeiNlpEngine.isCloud()) {
        LOGDF("有网络");
    }
    xunfeiNlpEngine.setConfig(getConfig());
    auto result = xunfeiNlpEngine.init();
    if (result.success != true) {
        LOGE("xunfeiNlpEngine init error", result.error.getCode(), result.error.getMessage());
    };

    xunfeiNlpEngine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        LOGD("result.success", result.success);
        LOGD("result.modelName", result.modelName);
        LOGD("result.text.c_str()", result.text.c_str());
        LOGD("result.error.getCode()", result.error.getCode());
        LOGD("result.error.getModule()", result.error.getModule());
        LOGD("result.error.getMessage()", result.error.getMessage());
    });
    const std::string prompt = R"(
        {
             "prompt1": "你是谁",
             "prompt": "你是谁"
        }
    )";
    xunfeiNlpEngine.generateContent(prompt);

    while (std::getchar() != '\n') {
        xunfeiNlpEngine.stopGenerateContent();
        xunfeiNlpEngine.destroy();
    }
}

void testChatWithWebSearch() {
    XunfeiNlpEngine engine;
    if (engine.isCloud()) {
        printf("NLP engine is cloud.\n");
    }
    engine.setConfig(getConfig());
    bool setResult = engine.setCurrentModel("generalv3.5");

    if (setResult != true) {
        printf("Set current error.\n");
    }
    auto result = engine.init();
    if (!result.success) {
        printf("NLP engine init error: %d, %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }
    engine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = xunfei_nlp_util::stringToJson(result.text);
        printf("%s\n", json["reasoning_content"].asString().c_str());
        printf("%s\n", json["result"].asString().c_str());
        if (json.isMember("search_info") && json["search_info"].isArray()) {
            for (const auto &item : json["search_info"]) {
                printf("%s\n", xunfei_nlp_util::jsonToString(item).c_str());
            }
        }
        if (json["is_end"].asBool()) {
            printf("\n");
        }
    });
    const std::string message = R"(
        {
             "messages":
             [
                {
                    "role": "system",
                    "content": "你是一个助手"
                },
                {
                    "role": "user",
                    "content": "巴勒斯坦现状如何？"
                }
            ],
            "top_k": 4,
            "stream": false,
            "web_search": true
        }
    )";
    result = engine.chat(message);
    if (result.success != true) {
        printf("NLP engine chat error: %d, %s\n", result.error.getCode(), result.error.getMessage().c_str());
    }
    while (std::getchar() != '\n') {
    }
    engine.stopChat();
    engine.destroy();
}

void testGenerateContentWithWebSearch() {
    XunfeiNlpEngine engine;
    if (engine.isCloud()) {
        printf("NLP engine is cloud.");
    }
    engine.setConfig(getConfig());
    bool setResult = engine.setCurrentModel("generalv3.5");
    if (setResult != true) {
        printf("Set current error.");
    }
    auto result = engine.init();
    if (!result.success) {
        printf("NLP engine init error: %d, %s\n", result.error.getCode(), result.error.getMessage().c_str());
        return;
    }

    engine.setTextGenerationResultCallback([](const ai_engine::gen_ai::nlp::TextGenerationResult &result) {
        if (!result.success) {
            fprintf(stderr, "Error info: %d %s\n", result.error.getCode(), result.error.getMessage().c_str());
            return;
        }
        Json::Value json = xunfei_nlp_util::stringToJson(result.text);
        printf("%s\n", json["result"].asString().c_str());
        if (json.isMember("search_info") && json["search_info"].isArray()) {
            for (const auto &item : json["search_info"]) {
                printf("%s\n", xunfei_nlp_util::jsonToString(item).c_str());
            }
        }
        if (json["is_end"].asBool()) {
            printf("\n");
        }
    });
    const std::string message = R"(
        {
            "prompt": "巴勒斯坦现状如何？",
            "stream": false,
            "web_search": true
        }
    )";
    result = engine.generateContent(message);
    if (result.success != true) {
        printf("NLP engine chat error: %d, %s\n", result.error.getCode(), result.error.getMessage().c_str());
    }

    while (std::getchar() != '\n') {
    }

    engine.stopGenerateContent();
    engine.destroy();
}

int main() {
    getmodelinfo();
    testChat();
    testChatGenerateContent();
    testChatWithWebSearch();
    testGenerateContentWithWebSearch();
    return 0;
}
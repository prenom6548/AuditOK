/*
 * Copyright 2025 KylinSoft Co., Ltd.
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

#ifndef DIALOGUE_MESSAGE_H
#define DIALOGUE_MESSAGE_H

#include <glib.h>
#include <jsoncpp/json/json.h>

#include <filesystem>
#include <functional>
#include <list>
#include <set>
#include <string>
#include <vector>

#include "utils/logger.h"

inline std::filesystem::path intentsPath() { return std::filesystem::path(CONFIGS_PATH) / "intents"; }

// 定义命名空间来组织相关结构体
namespace DialogueData {
// 对话状态中的槽信息
struct Slot {
    std::string name;
    bool isRequired{false};
    std::list<std::string> values;

    std::string toString() const {
        return fmt::format("name: {} isRequired: {} values: {}", name, isRequired, fmt::join(values, " "));
    }
};

// 意图识别模型的输出结果
struct Intent {
    std::string name;
    std::string userInput;
    std::string inquiry;
    std::string abnormalSlotScript;
    std::vector<Slot> slots;
};

// 多意图的情况
struct curTurnIntent {
    std::vector<Intent> curIntent;
};

// 当前轮次对话状态
struct DialogueState {
    std::vector<Slot> slots;
    std::list<std::string> system_acts;
    std::list<std::string> user_acts;
    std::string toString() const {
        std::string temp;
        temp += "slots: ";
        for (auto &slot : slots) {
            temp += slot.toString();
        }
        temp += " ";
        temp += fmt::format("system_acts: {}", fmt::join(system_acts, " "));
        temp += " ";
        temp += fmt::format("user_acts: {}", fmt::join(user_acts, " "));
        return temp;
    }
};

struct TokenCount {
    int totalCharacters;
    int englishTokens;
    int chineseTokens;
    int totalTokens;
};

// 对话中的每一轮要存储的信息
struct DialogueTurn {
    int turn_num;
    bool first_turn_flag;
    std::string role;
    std::string user_input;
    std::string intent_category;
    std::string user_file_path;   // 文档路径
    std::string image_file_path;  // 图片路径
    std::string system_response;
    std::string thinking_response;
    int query_token_number;
    int response_token_number;
    std::string intent_model_result;
    std::string knowledge_base_name;
    std::list<std::string> turn_keywords;
    DialogueState dialogue_state;

    std::string toString() const {
        std::string ret;
        ret += fmt::format("turn_num              = {}\n", turn_num);
        ret += fmt::format("first_turn_flag       = {}\n", first_turn_flag);
        ret += fmt::format("role                  = {}\n", role);
        ret += fmt::format("user_input            = {}\n", user_input);
        ret += fmt::format("intent_category       = {}\n", intent_category);
        ret += fmt::format("user_file_path        = {}\n", user_file_path);
        ret += fmt::format("image_file_path       = {}\n", image_file_path);
        ret += fmt::format("query_token_number    = {}\n", query_token_number);
        ret += fmt::format("system_response       = {}\n", system_response);
        ret += fmt::format("thinking_response       = {}\n", thinking_response);
        ret += fmt::format("response_token_number = {}\n", response_token_number);
        ret += fmt::format("intent_model_result   = {}\n", intent_model_result);
        ret += fmt::format("knowledge_base_name   = {}\n", knowledge_base_name);
        ret += fmt::format("turn_keywords         = {}\n", fmt::join(turn_keywords, " "));
        ret += fmt::format("dialogue_state        = {}\n", dialogue_state.toString());
        return ret;
    }
};

// 按照dialogue_id存储整个对话数据
struct Dialogue {
    std::string dialogue_id;
    std::vector<DialogueTurn> dialogue;
};

struct APINameSearchResult {
    bool found;
    std::string category;
};

struct CallSuccessIdentifier {
    bool successRes{false};
    std::string responseContent;
    Json::Value additionalContent;
    bool slotMissing{false};
    bool isMultiModalDoc{false};
    int errorCode{0};
    std::string errorMessage;
    bool intentSupported{false};
    std::string modelName;
    std::string scheduleDate;
};

struct InformationSource {
    std::string name;
    std::string url;
    int id{-1};

    bool operator<(const InformationSource &other) const { return id < other.id; }
};

struct FileSearchIdentifier {
    int errorCode{0};
    std::string errorMessage;
    std::string responseContent;
    bool successRes{false};
    std::set<InformationSource> sources;
};

struct KnowledgeBaseResult {
    int errorCode{0};
    std::string errorMessage;
    std::string knowledgeBaseName;
};

struct IntentResult {
    bool intentSupported{false};
    int errorCode{0};
    std::string errorMessage;
};

}  // namespace DialogueData

// 配置信息
namespace DialogueConfig {
// 设置最大筛选的轮数
const int maxFilteredRounds = 3;
const int maxRetrieveRounds = 5;  // 固定，别调整

// 中文字符与一个token对应关系
const int zhBytes = 3;               // 中文字符占用的字节
const float zhCorrespondence = 1.5;  // 1.5个汉字
// 英文字符与token对应关系
const int enBytes = 1;           // 英文字符占用的字节
const int enCorrespondence = 3;  // 3个字母

// 引导轮次的功能项数
const int numberForGuidedFunction = 3;

// 需要添加上下文的指令场景
const std::vector<int> nameForNeedContent = {0, 1, 2, 3};

// 意图识别模型prompt文件路径--端模型
const std::filesystem::path intentPromptFile = intentsPath() / "data/intent_prompt.txt";
const std::string intentPromptFileStr = intentPromptFile.string();

// 意图识别模型prompt文件路径--云模型
const std::filesystem::path intentPromptCloudFile = intentsPath() / "data/intent_prompt_cloud.txt";
const std::string intentPromptFileCloudStr = intentPromptCloudFile.string();

// 意图识别模型路径
const std::filesystem::path intentModelPath = intentsPath() / "model/qwen-1.5-1.8B";
const std::string intentModelPathStr = intentModelPath.string();

// 历史会话存储路径
const std::filesystem::path dialogueHistoryFile = g_get_home_dir();

// 用户输入附件时的提示文本
const std::vector<std::string> rubricText = {"用户输入了一个文档", "用户输入了一张图片"};

// 文档/图片类接口名称
const std::filesystem::path interfaceNameFile = intentsPath() / "data/functional_interface_name.json";
const std::string interfaceNameFileStr = interfaceNameFile.string();

// 意图、槽位json文件
const std::string intentSlotFile = intentsPath() / "data/intent_slots.json";
const std::string intentSlotDataFile = intentsPath() / "data/intent_slot_data.json";

// 意图规则匹配权重参数
const double low_intent_threshold = 0.7;
const double mid_intent_threshold = 0.94;
const double high_intent_threshold = 0.97;
const double query_weight = 0.8;
const double entity_total_weight = 0.2;

// 槽位正则匹配的python脚本
const std::string slotRegexPythonScript = intentsPath() / "scripts";

// 已支持的系统接口
const std::string supportedSystemInterfacesFile = intentsPath() / "data/supported_system_interfaces.txt";

// 单独处理话术的意图场景
const std::vector<std::string> intentIndividualScene = {
    "系统操作_蓝牙操作",          "第三方应用控制_多媒体-音乐播放", "第三方应用控制_多媒体-视频播放",
    "系统操作_自定义时区时间",    "系统操作_设置免密登录",          "系统操作_设置开机自动登录",
    "系统操作_切换显示器投屏方式"};  // , "第三方应用控制_应用控制"

const std::vector<std::string> addedIntentScenarios = {
    "系统操作_电源控制", "系统操作_录屏", "第三方应用控制_查询天气"};

// 闲聊场景
const std::vector<std::string> idleChatName = {"其他_闲聊()", "知识问答_时效知识问答()", "知识问答_百科知识问答()"};

// 指令类场景
const std::list<std::string> scenarioInstructionName = {
    "知识问答_百科知识问答",     "知识问答_时效知识问答", "其他_闲聊",
    "分支场景切换_文本扩写",     "分支场景切换_文本润色", "分支场景切换_翻译-中译英",
    "分支场景切换_翻译-英译中",  "分支场景切换_内容校对", "分支场景切换_总结概括",
    "分支场景切换_新建日程",     "分支场景切换_生成周报", "分支场景切换_生成待办",
    "分支场景切换_文档标签提取", "分支场景切换_文档摘要"};

const std::filesystem::path ragPromptFile = intentsPath() / "scene_prompt/rag_document_qa_prompt.txt";
const std::string ragPromptFileStr = ragPromptFile.string();

const std::filesystem::path missingSlotScriptFile = intentsPath() / "data/missing_slot_script.json";
const std::string missingSlotScriptFileStr = missingSlotScriptFile.string();

// 应用类意图场景
const std::list<std::string> applicationCategoriesIntent = {"系统操作", "第三方应用控制", "日程管理"};

const std::filesystem::path slotsValueFile = intentsPath() / "data/intent_slots_value.json";
const std::string slotsValueFileStr = slotsValueFile.string();

const std::vector<std::string> slotNormalizationFilterList = {"多模态输入识别_多模态输入识别(*附件类型,API接口名称)",
                                                              "日程管理_闹钟设置(*日期,事件,*操作)",
                                                              "日程管理_日程查看&安排(*日期,事件,*操作)"};

// 支持整数的意图场景
const std::map<std::string, std::pair<int, int>> integerScenarios = {
    {"系统操作_设置显示器亮度(*屏幕亮度值)", {0, 100}}, {"系统操作_音量控制(*音量值)", {0, 100}},
    {"系统操作_设置鼠标指针速度(*操作)", {1, 10}},      {"系统操作_设置鼠标滚轮速度(*操作)", {1, 10}},
    {"系统操作_设置触控板指针速度(*操作)", {1, 8}},     {"系统操作_设置鼠标双击间隔时长(*操作)", {170, 1000}}};

// 支持小数数值的意图场景
const std::map<std::string, std::pair<int, int>> decimalScenarios = {{"系统操作_设置鼠标指针速度(*操作)", {1, 10}}};

// 有槽位但仅打开页面的意图场景
const std::vector<std::string> slotsAvailableOnlyPageOpeningScene = {};

// 文生图关键词
const std::vector<std::string> wordJudgment{"绘制",   "创造", "生成", "打造", "制作", "创作", "显现", "呈现",
                                            "产生",   "输出", "创建", "绘制", "生成", "设计", "描绘", "展示",
                                            "制作",   "需要", "想要", "画",   "构思", "表现", "呈现", "开发",
                                            "可视化", "模拟", "想看", "形成", "渲染", "合成", "图示", "绘画"};

const std::vector<std::string> mainFont{"图片", "图像", "插图", "照片", "画作", "海报", "草图"};

}  // namespace DialogueConfig

namespace DialogueCallback {

// 对话过程中回调函数定义
enum Type : int {
    ChatNormal,
    ChatErrorOccurred,
    ImageGenerating,
    ImageGenerated,
    ImageErrorOccurred,
    ChatGuidedNormal,
    SystemOperationNormal,
    SystemOperationErrorOccurred,
    IntentNormal,
    IntentErrorOccurred,
    EmbeddingNormal,
    EmbeddingErrorOccurred,
    RagInterfaceNormal,
    RagInterfaceErrorOccurred
};

enum class DialogueControlErrorCode {
    NoError = 0,
    InvalidInput,
    SystemOperationFailed,
    IntentionRecognitionFailed,

};

// IntentionRecognition向外输出的函数
using OutputCallback = std::function<void(Type type, const std::string &)>;

struct ChatData {
    Type type_{ChatErrorOccurred};
    std::string text_;
    int errorCode_{0};
    std::string errorMessage_;
    std::string modelName_;
    std::string feedbackUrl_;
};
// 与ChatModel接口交互的同步回调函数
using ChatModelCallback = std::function<void(const ChatData &)>;

// 开始：文生图接口回调函数定义
struct TextToImageResult {
    bool success{};
    std::string imageFormat;
    int height_{};
    int width_{};
    int total_{};  // 总共生成的图片数量
    int index_{};  // 当前数据对应的图片的索引
    std::string imageBase64Data_;
    int errorCode_{};
    std::string errorMessage_;
    Type type_{ImageErrorOccurred};
    std::string modelName_;
};
using TextToImageCallback = std::function<void(const TextToImageResult &)>;
// 结束：文生图接口回调函数定义

// 开始：抠图接口回调函数定义
struct ImageMattingResult {
    bool success{};
    std::string imageFormat;
    std::string imageBase64Data;
    int errorCode{};
    std::string errorMessage;
    Type type{ImageErrorOccurred};
    std::string modelName;
};
using ImageMattingCallback = std::function<void(const ImageMattingResult &result)>;
// 结束：抠图接口回调函数定义

// 开始：图像二值化接口回调函数定义
struct ImageBinarizationResult {
    bool success{};
    std::string imageFormat;
    std::string imageBase64Data;
    int errorCode{};
    std::string errorMessage;
    Type type{ImageErrorOccurred};
    std::string modelName;
};
using ImageBinarizationCallback = std::function<void(const ImageBinarizationResult &result)>;
// 结束：图像二值化接口回调函数定义

// 文本向量化
struct TextEmbeddingResult {
    std::vector<float> embedding;
    Type type{EmbeddingErrorOccurred};
    int errorCode{};
    std::string errorMessage;
};
using TextEmbeddingCallback = std::function<void(const TextEmbeddingResult &result)>;

// 知识库
struct KBChatInfo {
    std::string content;
    std::string guideFilePath;
    int guideToolId{-1};
    std::string toolCmd;
    int toolCmdId{-1};

    bool operator<(const KBChatInfo &other) const { return guideToolId < other.guideToolId; }
};

struct KBChatResult {
    bool isFinished;
    std::string model;
    std::string reasoningContent;
    std::string response;
    int errorCode;
    std::set<KBChatInfo> sources;
};
using KBChatStreamCallback = std::function<void(const KBChatResult &response)>;
}  // namespace DialogueCallback

#endif  // DIALOGUE_MESSAGE_H

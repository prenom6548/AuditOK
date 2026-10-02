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

#include "intentionrecognition.h"

#include <jsoncpp/json/json.h>

#include <algorithm>
#include <cctype>
#include <ctime>
#include <future>
#include <list>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "assistant/error.h"
#include "assistant/message.h"
#include "assistant/session.h"
#include "config/prompttemplate.h"
#include "dialoguemessage.h"
#ifdef SUPPORT_VECTOR_DB
#include "fileinference/fileinference.h"
#include "slotsnormalization.h"
#endif
#include "imagegentrigger.h"
#include "imagehandler/imagehandler.h"
#include "modelinference/modelinference.h"
#include "systemoperation/startuptools.h"
#include "systemscenarioclass.h"
#include "threadpool/async.h"
#include "utils/logger.h"
#include "utils/util.h"
#include "variabletypeconversion.h"

namespace {
// 系统助手/文档问答
constexpr int systemAssistantPromptId = int(config::PromptTemplate::PromptId::FileQuestionAnswering);
constexpr int meetingInformationExtractionPromptId =
    int(config::PromptTemplate::PromptId::MeetingInformationExtraction);
constexpr int todoListGenerationPromptId = int(config::PromptTemplate::PromptId::TodoListGeneration);
constexpr int weeklyReportGenerationPromptId = int(config::PromptTemplate::PromptId::WeeklyReportGeneration);
constexpr int defaultPromptId = int(config::PromptTemplate::PromptId::Default);
const std::string userNamePrefix = "user_dialogue_id_";  // 用户名

// TODO 后续要移动到配置文件
std::string getLongChatImageParserPrompt() {
    return R"(
你是一个「微信聊天内容解读助手」。

你的任务是：**仅基于用户提供的聊天记录（result_to_llms）进行整理与分析**，并默认输出固定的四个部分：总结、主题、内容分析、建议。

==================
【核心约束规则】
==================

1. 你【只能】使用聊天记录中明确出现的信息作答
   - 不允许推测、补充背景、常识推断
   - 不允许使用“可能、应该、看起来像”等主观判断

2. 若出现指代不清（如“这个”“他”“那样”“上面那个”）：
   - 必须明确标注：**“指代不明确”**
   - 严禁自行推断指代对象

3. 若聊天记录信息不足以支撑某个结论或建议：
   - 必须明确写：**“未在聊天记录中找到相关信息”** 或 **“聊天记录中未体现”**
   - 但仍然必须输出完整四部分结构

4. 关于无效或非聊天内容的处理规则（必须遵守!!!）：
   - 聊天记录中可能包含由截图 UI、状态栏、按钮、图标或 OCR 误识别产生的文本
   - 对于明显不构成自然语言、连续语义或正常聊天表达的内容：
     - 不允许评价其“混乱”“错误”“OCR 失败”等
     - 不允许分析其成因
     - 不允许将其作为聊天内容的一部分进行解读
   - #只能采用以下中性方式处理：
     -“该部分未包含可识别的有效聊天信息”
     - 或在内容分析中直接跳过，不做描述

==================
【强制输出结构（必须完整输出）】
==================

无论任何情况下，你的回答【必须且只能】包含以下四个部分，顺序固定，不得缺失：

#### 一、总结
- 用 1～3 句话概括这段聊天的核心结论、主要进展或关键信息
- 内容必须能够在聊天记录中直接找到依据

#### 二、主题
- 概括这段聊天正在讨论的主要话题（1～3 条即可）
- 仅描述“在聊什么”，不评价、不延伸、不升华

#### 三、内容分析
- 对聊天内容进行**整体性、条理化的说明和分析**
- 可按时间顺序、发言角色或话题演进进行描述
- 只复述和整理聊天中已经出现的信息
- 如涉及指代不清的内容，必须明确说明“指代不明确”
- 不使用列表式结构化引用，不逐条罗列消息

#### 四、建议
- 仅基于聊天记录中已经出现的问题、分工、诉求、风险或待办事项
- 不得引入聊天中未出现的新行动或方案
- 若聊天中未体现可执行建议，必须写：
  **“聊天记录中未体现明确的后续建议”**

==================
【重要说明】
==================

- 聊天记录来源于图片 OCR 结果（result_to_llms）
- 聊天记录按真实顺序排列，可能包含：
  - type: group_name / time / chat
- time 类型为重要上下文时间线索

只输出最终结果，不要解释规则或分析过程。
)";
}

}  // namespace

IntentionRecognition::IntentionRecognition() {
    LOGI("Load intention recognition model success: {}", DialogueConfig::intentModelPathStr);

    loadIntentAndSlots();  // 加载意图和槽位
    loadJsonContent();     // 加载文档/图片类功能接口名称

#ifdef SUPPORT_VECTOR_DB
    slotsNormalization_ = std::make_shared<SlotsNormalization>();
#endif

    // 加载可支持的系统接口名称
    supportedSystemInterfaces_ = TypeTrans::readTxtToList(DialogueConfig::supportedSystemInterfacesFile);
    // 获取无槽位标签
    getIntentionScenariosWithoutSlots();

    // 加载RAG的prompt
    LOGI("Import RAG system prompt: {}", DialogueConfig::ragPromptFileStr);

    ragPrompt_ = getPrompt(DialogueConfig::ragPromptFileStr);
    LOGI("RAG prompt: {}", ragPrompt_);

    missingSlotScript_ = loadJsonFile(DialogueConfig::missingSlotScriptFileStr);

#ifdef SUPPORT_VECTOR_DB
    slotsValueNormalJson_ = loadJsonFile(DialogueConfig::slotsValueFileStr);
#endif

    // 生成会话id
    dialogueId_ = userNamePrefix + TypeTrans::getTimeStr();
    // 此次会话的id
    LOGI("Current dialogue ID: {}", dialogueId_);

    bool success = longChatImageParser_.init();
    if (!success) {
        LOGE("Failed to initialize long chat image parser.");
    }
}

IntentionRecognition::~IntentionRecognition() {
    if (imageHandlerInterface_) {
        imageHandlerInterface_->setTextToImageCallback(nullptr);
        imageHandlerInterface_->setImageMattingCallback(nullptr);
        imageHandlerInterface_->setImageBinarizationCallback(nullptr);
    }
    if (modelInferenceInterface_) {
        modelInferenceInterface_->setChatModelCallback(nullptr);
    }
    outputCallback_ = nullptr;
}

void IntentionRecognition::getIntentionScenariosWithoutSlots() {
    // 获取无槽位意图标签
    for (const auto &intentLabel : intentSlotJsonData_.getMemberNames()) {
        if (intentSlotJsonData_.isObject() && intentSlotJsonData_[intentLabel].isObject() &&
            intentLabel == "noSlotsLabel") {
            for (const auto &key : intentSlotJsonData_[intentLabel].getMemberNames()) {
                size_t splitIndex = key.find('(');
                std::string noSlotName = key.substr(0, splitIndex);
                intentionScenariosWithoutSlots_.push_back(noSlotName);
            }
        }
    }
}

void IntentionRecognition::setSystemOperationInterface(SystemOperationInterface *interface) {
    if (systemOperationInterface_ == nullptr) {
        systemOperationInterface_ = interface;
    }
}

#ifdef SUPPORT_VECTOR_DB
void IntentionRecognition::deleteCustomizeKnowledgeBase(const std::string &knowledgeBaseName) const {
    if (fileInferenceInterface_ != nullptr) {
        return fileInferenceInterface_->deleteFileKnowledgeBase(knowledgeBaseName);
    }
}

void IntentionRecognition::setFileInferenceInterface(FileInferenceInterface *interface) {
    fileInferenceInterface_ = interface;
}

void IntentionRecognition::setTextEmbeddingInterface(TextEmbeddingInterface *interface) {
    stringEmbeddingInterface_ = interface;
    if (stringEmbeddingInterface_ == nullptr) {
        return;
    }
    intentSlotRecognizer_.setTextEmbeddingInterface(interface);
}

void IntentionRecognition::setKnowledgeBaseInterface(KnowledgeBaseInterface *interface) {
    knowledgeBaseInterface_ = interface;
    if (knowledgeBaseInterface_ == nullptr) {
        return;
    }
    int errorCode = int(assistant::Error::getErrorCodeFromAdvancedKnowledgeBase(knowledgeBaseInterface_->init()));
    if (errorCode) {
        LOGE("KnowledgeBaseInferenceInterface init failed");
        callOutputCallback(DialogueCallback::Type::ChatErrorOccurred,
                           assistant::message::createErrorMessageResult(errorCode, ""));
        return;
    }

    knowledgeBaseInterface_->setKBChatStreamCallback([this](const DialogueCallback::KBChatResult &result) {
        LOGD("[KnowledgeBase] chatStream callback");
        if (!result.errorCode) {
            Json::Value resultText;
            std::set<assistant::session::InformationSource> sources;
            for (const auto &source : result.sources) {
                sources.insert({source.content, source.guideFilePath, source.guideToolId});
            }
            Json::Value informationSourcesJson = assistant::message::createJsonInformationSources(sources);
            Json::Value toolSourcesJson = assistant::message::createToolInformationSources(result.sources);
            if (!dialogueStop_) {
                resultText["information_sources"] = informationSourcesJson;
            }
            resultText["tool_sources"] = toolSourcesJson;
            if (!result.reasoningContent.empty()) {
                resultText["reasoning_content"] = result.reasoningContent;
            }
            resultText["result"] = result.response;
            resultText["is_end"] = result.isFinished;
            std::string message =
                assistant::message::createTextMessageFromStreamChatResult(resultText, result.model, "");
            callOutputCallback(DialogueCallback::Type::ChatNormal, message);
            return;
        }
        if (result.errorCode == advanced::knowledgebase::KBErrorCode::RETRIEVAL_NO_RESULTS) {
            Json::Value resultText;
            resultText["result"] = "当前未检索到相关参考资料。建议您补充更多细节信息，或上传相关文档后重试。";
            resultText["is_end"] = true;
            std::string message =
                assistant::message::createTextMessageFromStreamChatResult(resultText, result.model, "");
            callOutputCallback(DialogueCallback::Type::ChatNormal, message);
            return;
        }
        int errorCode = int(assistant::Error::getErrorCodeFromAdvancedKnowledgeBase(result.errorCode));
        callOutputCallback(DialogueCallback::Type::ChatErrorOccurred,
                           assistant::message::createErrorMessageResult(errorCode, ""));
    });
}
#endif

void IntentionRecognition::dealChatModelOutput(const DialogueCallback::ChatData &result,
                                               const std::set<DialogueData::InformationSource> &ragSources) {
    Json::Value resultText = util::stringToJson(result.text_);

    if (resultText.isMember("result")) {
        chatReplyCache_ += resultText["result"].asString();
        thinkingChatReplyCache_ += resultText["reasoning_content"].asString();
    }

    if (!result.modelName_.empty()) {
        currentModelNameInUse_ = result.modelName_;
    }

    if (!result.feedbackUrl_.empty()) {
        feedbackUrlForCloudModel_ = result.feedbackUrl_;
    }

    // 处理is_end
    if (resultText.isMember("is_end")) {
        if (resultText["is_end"].asBool()) {
            chatGenCompleteText_ = chatReplyCache_;
            chatReplyCache_.clear();
            thinkingChatGenCompleteText_ = thinkingChatReplyCache_;
            thinkingChatReplyCache_.clear();
        }
    }

    std::string message =
        assistant::message::createTextMessageFromStreamChatResult(resultText, result.modelName_, result.feedbackUrl_);

    if (promptId_ != meetingInformationExtractionPromptId && promptId_ != todoListGenerationPromptId) {
        callOutputCallback(result.type_, message);
    } else {
        // 新建日程、生成待办场景，需要输出思考过程
        if (resultText.isMember("reasoning_content") && !resultText["reasoning_content"].empty()) {
            callOutputCallback(result.type_, message);
        }
    }
}

void IntentionRecognition::setModelInferenceInterface(ModelInferenceInterface *interface) {
    modelInferenceInterface_ = interface;
    modelInferenceInterface_->setChatModelCallback([this](const DialogueCallback::ChatData &chatData) {
        if (chatData.type_ == DialogueCallback::ChatNormal) {
            // 获取生成的文本内容
            dealChatModelOutput(chatData, ragFileSources_);
        } else if (chatData.type_ == DialogueCallback::IntentNormal) {
            if (dialogueStop_) {
                textCallback("", intentModelDisplayName(), false, 0, "", DialogueCallback::IntentNormal, "");
            }
        } else if (chatData.type_ == DialogueCallback::ChatErrorOccurred) {
            LOGE("Error info: {} {}.", chatData.errorCode_, chatData.errorMessage_);
            callOutputCallback(chatData.type_, assistant::message::createErrorMessageResult(chatData.errorCode_,
                                                                                            chatData.errorMessage_));
            chatGenCompleteText_.clear();
            chatReplyCache_.clear();
            thinkingChatGenCompleteText_.clear();
            thinkingChatReplyCache_.clear();
        }
    });
}

void IntentionRecognition::setImageHandlerInterface(ImageHandlerInterface *interface) {
    imageHandlerInterface_ = interface;
    if (imageHandlerInterface_ == nullptr) {
        return;
    }
    imageHandlerInterface_->setTextToImageCallback([this](const DialogueCallback::TextToImageResult &result) {
        std::string message;
        if (result.type_ == DialogueCallback::ImageErrorOccurred) {
            message = assistant::message::createImageDataMessageWithErrorCode(result.modelName_, "image_generation",
                                                                              result.errorCode_, result.errorMessage_);
            callOutputCallback(DialogueCallback::ImageErrorOccurred, message);
            return;
        }
        assistant::message::ImageData imageData{};
        imageData.success = result.success;
        imageData.modelName = result.modelName_;
        imageData.errorCode = result.errorCode_;
        imageData.errorMessage = result.errorMessage_;
        if (result.type_ == DialogueCallback::ImageGenerated) {
            imageData.imageFormat = result.imageFormat;
            imageData.imageBase64Data = result.imageBase64Data_;
            imageData_ = message;
        }

        // 获取feedbackUrl
        std::string feedbackStr = imageHandlerInterface_->getFeedbackUrl(result.modelName_);
        message = assistant::message::createImageDataMessage(
            imageData, result.type_ == DialogueCallback::ImageGenerating, feedbackStr);
        callOutputCallback(result.type_, message);
    });
    imageHandlerInterface_->setImageMattingCallback([this](const DialogueCallback::ImageMattingResult &result) {
        if (result.type == DialogueCallback::ImageErrorOccurred) {
            callOutputCallback(DialogueCallback::ImageErrorOccurred,
                               assistant::message::createImageDataMessageWithErrorCode(
                                   result.modelName, "image_generation", result.errorCode, result.errorMessage));
            return;
        }

        assistant::message::ImageData imageData{};
        imageData.success = result.success;
        imageData.modelName = result.modelName;
        imageData.errorCode = result.errorCode;
        imageData.errorMessage = result.errorMessage;
        if (result.type == DialogueCallback::ImageGenerated) {
            imageData.imageFormat = result.imageFormat;
            imageData.imageBase64Data = result.imageBase64Data;
        }
        callOutputCallback(result.type, assistant::message::createImageDataMessage(
                                            imageData, result.type == DialogueCallback::ImageGenerating));
    });

    imageHandlerInterface_->setImageBinarizationCallback(
        [this](const DialogueCallback::ImageBinarizationResult &result) {
            if (result.type == DialogueCallback::ImageErrorOccurred) {
                callOutputCallback(DialogueCallback::ImageErrorOccurred,
                                   assistant::message::createImageDataMessageWithErrorCode(
                                       result.modelName, "image_generation", result.errorCode, result.errorMessage));
                return;
            }

            assistant::message::ImageData imageData{};
            imageData.success = result.success;
            imageData.modelName = result.modelName;
            imageData.errorCode = result.errorCode;
            imageData.errorMessage = result.errorMessage;
            if (result.type == DialogueCallback::ImageGenerated) {
                imageData.imageFormat = result.imageFormat;
                imageData.imageBase64Data = result.imageBase64Data;
            }
            callOutputCallback(result.type, assistant::message::createImageDataMessage(
                                                imageData, result.type == DialogueCallback::ImageGenerating));
        });
}

void IntentionRecognition::setOutputCallback(const DialogueCallback::OutputCallback &callback) {
    outputCallback_ = callback;
}

void IntentionRecognition::loadJsonContent() {
    functionInterfaceName_ = loadJsonFile(DialogueConfig::interfaceNameFileStr);
}

Json::Value IntentionRecognition::getfunctionInterfaceName() { return functionInterfaceName_; }

void IntentionRecognition::stopDialogue() {
    dialogueStop_ = true;
    updateKbFileStop_ = true;
    if (modelInferenceInterface_ != nullptr) {
        modelInferenceInterface_->stopChat();
    }
    if (imageHandlerInterface_) {
        imageHandlerInterface_->stopImageBinarization();
        imageHandlerInterface_->stopImageMatting();
    }
#ifdef SUPPORT_VECTOR_DB
    if (knowledgeBaseInterface_) {
        knowledgeBaseInterface_->stopChat();
    }
#endif
}

DialogueData::CallSuccessIdentifier IntentionRecognition::callSystemOperationInterface(
    const DialogueData::Intent &scenarioIntent) const {
    if (systemOperationInterface_ != nullptr) {
        return systemOperationInterface_->callSystemControlInterface(scenarioIntent);
    }
    return {};
}

// 新建日程
DialogueData::CallSuccessIdentifier IntentionRecognition::callSystemScheduleManagement(
    const std::string &scheduleText) const {
    if (systemOperationInterface_ != nullptr) {
        return systemOperationInterface_->callSystemScheduleManagementInterface(scheduleText);
    }
    return {};
}

// 生成待办
DialogueData::CallSuccessIdentifier IntentionRecognition::callSystemToDoList(const std::string &todoListText) const {
    if (systemOperationInterface_ != nullptr) {
        return systemOperationInterface_->callSystemToDoListInterface(todoListText);
    }
    return {};
}

void IntentionRecognition::callSourceUrlTool(int toolId) const {
    if (systemOperationInterface_ == nullptr) {
        return;
    }
    systemOperationInterface_->callSourceUrlTool(toolId);
}
#ifdef SUPPORT_VECTOR_DB
std::future<DialogueData::FileSearchIdentifier> IntentionRecognition::callFileInferenceInterfaceSearchText(
    const std::string &knowledgeBaseName, const std::string &text, bool useDefault) const {
    if (fileInferenceInterface_ != nullptr) {
        return fileInferenceInterface_->ragSearchText(knowledgeBaseName, text, useDefault);
    }
    return {};
}

DialogueData::CallSuccessIdentifier IntentionRecognition::callFileInferenceInterfaceFileContent(
    const std::string &knowledgeBaseName, const std::string &input) const {
    if (fileInferenceInterface_ != nullptr) {
        return fileInferenceInterface_->fileContent(knowledgeBaseName, input);
    }
    return {};
}

DialogueData::CallSuccessIdentifier IntentionRecognition::callKnowledgeBaseInterface(const std::string &modelInput,
                                                                                     const std::string &model) const {
    DialogueData::CallSuccessIdentifier ret{};
    if (knowledgeBaseInterface_ != nullptr) {
        knowledgeBaseInterface_->chat(modelInput, model);
        ret.successRes = true;
    }
    return ret;
}

DialogueCallback::TextEmbeddingResult IntentionRecognition::callTextEmbeddingInterface(
    const std::string &queryInput) const {
    if (stringEmbeddingInterface_ != nullptr) {
        return stringEmbeddingInterface_->textVectorization(queryInput);
    }
    return {};
}
#endif

std::future<bool> IntentionRecognition::callChatModelInferenceInterface(const std::string &modelInput) const {
    if (modelInferenceInterface_ != nullptr) {
        return modelInferenceInterface_->callChatModelInference(modelInput);
    }
    return {};
}

bool IntentionRecognition::tryRuleBasedIntentRecognition(const Json::Value &userInputJson,
                                                         IntentRecognitionResult &ruleBasedResult,
                                                         DialogueData::CallSuccessIdentifier &result) {
#ifdef SUPPORT_VECTOR_DB
    // 正在进行意图识别的追问时，不使用基于规则的意图识别。
    // 因为基于规则的意图识别可能针对用户的回文匹配到错误的意图。
    if (isIntentInquiring_) {
        return false;
    }

    const Json::Value &messages = userInputJson.get("messages", Json::Value(Json::arrayValue));
    if (!messages.isArray() || messages.empty()) {
        return false;
    }

    const Json::Value &lastMessage = messages[messages.size() - 1];
    const Json::Value &content = lastMessage.get("content", Json::Value());
    if (!content.isString()) {
        return false;
    }

    std::string userInput = content.asString();
    LOGI("userInput = {}", userInput);

    auto buildResponse = [&result](const std::string &category, const std::string &inquiry, const Json::Value &slots) {
        result.successRes = true;
        result.intentSupported = true;
        Json::Value intentJson;
        intentJson["intent_category"] = category;
        intentJson["inquiry"] = inquiry;
        intentJson["slots"] = slots;
        Json::Value intents(Json::arrayValue);
        intents.append(intentJson);
        result.responseContent = util::jsonToString(intents);
    };

    if (userInput.length() > 200) {
        // 输入过长时不进行意图识别，直接返回“闲聊”
        // 基于向量检索的意图识别会调用向量化模型（最大输入 token=8192），过长可能导致向量化失败
        buildResponse("闲聊", "", Json::nullValue);
        return true;
    }

    // 执行基于规则的意图识别
    ruleBasedResult = intentSlotRecognizer_.recognizeIntentAndSlots(userInput);
    LOGI("基于规则意图识别结果: intent_category = {}, success = {}, slots = {}, candidate_intents = {}",
         ruleBasedResult.intent_category, ruleBasedResult.success, util::jsonToString(ruleBasedResult.slots),
         util::jsonToString(ruleBasedResult.candidate_intents));

    // 如果识别成功（闲聊或成功识别意图），构建并返回响应
    if (ruleBasedResult.intent_category == "闲聊" || ruleBasedResult.success) {
        buildResponse(ruleBasedResult.intent_category, ruleBasedResult.inquiry, ruleBasedResult.slots);
        return true;
    }
#endif
    return false;
}

DialogueData::CallSuccessIdentifier IntentionRecognition::performModelBasedIntentRecognition(
    Json::Value &userInputJson, const IntentRecognitionResult &ruleBasedResult) {
    DialogueData::CallSuccessIdentifier ret;

    if (modelInferenceInterface_ == nullptr) {
        return ret;
    }

    loadIntentPromptText();
    if (isBaseModelForIntention_) {
        // 如果是基础模型，构建并更新系统提示
        Json::Value candidateIntents = ruleBasedResult.candidate_intents;
        if (isIntentInquiring_) {
            candidateIntents = candidateIntents_;
            candidateIntents_.clear();
        }
        intentPrompt_ = buildIntentSystemPrompt(candidateIntents);
    }

    Json::Value &messages = userInputJson["messages"];
    if (messages.isArray() && !messages.empty()) {
        Json::Value &systemPromptMessage = messages[0];
        systemPromptMessage["content"] = intentPrompt_;
    }

    // 执行模型推理
    isIntentInquiring_ = false;
    ret = modelInferenceInterface_->callIntentionModelInference(util::jsonToString(userInputJson));

    // 检查模型输出中是否包含追问信息
    std::string modelOutput = ret.responseContent;
    Json::Value modelOutputJson = util::stringToJson(modelOutput);
    // 兼容模型输出为单对象（无最外层数组）的情况：{...} -> [{...}]
    if (modelOutputJson.isObject() && !modelOutputJson.empty()) {
        Json::Value wrapped(Json::arrayValue);
        wrapped.append(modelOutputJson);
        modelOutputJson = wrapped;
        ret.responseContent = util::jsonToString(modelOutputJson);
    }
    if (modelOutputJson.isArray() && !modelOutputJson.empty() && modelOutputJson[0].isMember("inquiry")) {
        std::string inquiry = modelOutputJson[0]["inquiry"].asString();
        if (!inquiry.empty()) {
            isIntentInquiring_ = true;
            candidateIntents_ = ruleBasedResult.candidate_intents;
        }
    }

    LOGI("意图识别模型输出: {}", modelOutput);
    return ret;
}

DialogueData::CallSuccessIdentifier IntentionRecognition::performIntentRecognize(const std::string &modelInput) {
    Json::Value userInputJson = util::stringToJson(modelInput);
    IntentRecognitionResult ruleBasedResult;
    DialogueData::CallSuccessIdentifier ruleBasedRet;

    if (tryRuleBasedIntentRecognition(userInputJson, ruleBasedResult, ruleBasedRet)) {
        return ruleBasedRet;
    }

    return performModelBasedIntentRecognition(userInputJson, ruleBasedResult);
}

std::string IntentionRecognition::intentModelName() const {
    if (modelInferenceInterface_ == nullptr) {
        return "";
    }
    return modelInferenceInterface_->intentModelName();
}

std::string IntentionRecognition::intentModelDisplayName() const {
    if (modelInferenceInterface_ == nullptr) {
        return "";
    }
    return modelInferenceInterface_->intentModelDisplayName();
}

std::string IntentionRecognition::chatModelName() const {
    if (modelInferenceInterface_ == nullptr) {
        return "";
    }
    return modelInferenceInterface_->chatModelName();
}

bool IntentionRecognition::isChatModelConfigured() const { return modelInferenceInterface_->isChatModelConfigured(); }
#ifdef SUPPORT_VECTOR_DB
std::future<DialogueData::KnowledgeBaseResult> IntentionRecognition::createFileKnowledgeBase() const {
    // 创建向量数据库
    if (fileInferenceInterface_ != nullptr) {
        return fileInferenceInterface_->createFileKnowledgeBase();
    }
    return {};
}

DialogueData::FileSearchIdentifier IntentionRecognition::updateFileKnowledgeBase(const std::string &knowledgeBaseName,
                                                                                 const std::string &docFilePath) const {
    // 更新已有向量数据库
    if (fileInferenceInterface_ == nullptr) {
        LOGE("File inference interface is empty!");
        return {-1, "File inference interface is empty!"};
    }
    return fileInferenceInterface_->updateFileKnowledgeBase(knowledgeBaseName, docFilePath);
}
#endif
DialogueData::CallSuccessIdentifier IntentionRecognition::callImageHandlerInterface(const std::string &prompt) const {
    if (imageHandlerInterface_ != nullptr) {
        return imageHandlerInterface_->textToImage(prompt);
    }
    return {};
}

DialogueData::CallSuccessIdentifier IntentionRecognition::callImageMattingInterface(
    const std::string &imagePath) const {
    if (imageHandlerInterface_ != nullptr) {
        return imageHandlerInterface_->imageMatting(imagePath);
    }
    return {};
}

DialogueData::CallSuccessIdentifier IntentionRecognition::callImageBinarizationInterface(
    const std::string &imagePath) const {
    if (imageHandlerInterface_ != nullptr) {
        return imageHandlerInterface_->imageBinarization(imagePath);
    }
    return {};
}

void IntentionRecognition::callOutputCallback(DialogueCallback::Type type, const std::string &message) const {
    if (outputCallback_ != nullptr) {
        outputCallback_(type, message);
    }
}

void IntentionRecognition::loadIntentAndSlots() {
    intentSlotJsonData_ = loadJsonFile(DialogueConfig::intentSlotFile);

    for (const auto &intentLabel : intentSlotJsonData_.getMemberNames()) {
        if (intentSlotJsonData_.isObject() && intentSlotJsonData_[intentLabel].isObject()) {
            for (const auto &key : intentSlotJsonData_[intentLabel].getMemberNames()) {
                systemScenarioIntentionsTotal_.push_back(key);
            }
        }
    }
}

void IntentionRecognition::loadIntentPromptText() {
    // TODO: Qwen3-8b模型的意图lora模型还没上线，暂时无法通过是否为端侧模型判断
    // isBaseModelForIntention_ = modelInferenceInterface_->isCloudModel();
    std::string modelName = intentModelName();
    isBaseModelForIntention_ = modelName.find("llm_Qwen-2.5-3b") == std::string::npos &&
                               modelName.find("llm_Qwen-3-8b-0") == std::string::npos &&
                               modelName.find("llm_Qwen-3-8b-1") == std::string::npos &&
                               modelName.find("Qwen-2.5-3b_1.0") == std::string::npos;
    if (isBaseModelForIntention_) {
        intentPrompt_ = getPrompt(DialogueConfig::intentPromptFileCloudStr);
    } else {
        intentPrompt_ = getPrompt(DialogueConfig::intentPromptFileStr);
    }
}

void IntentionRecognition::setScenarioInstructionPrompt(int promptId, const std::string &promptText,
                                                        const std::string &instructionName) {
    instructionPrompt_ = promptText;
    promptId_ = promptId;
    instructionName_ = instructionName;
    promptPassed_ = true;  // 传入成功标识

    if (promptId == 0) {
        defaultPromptContent_ = promptText;
    }
}

void IntentionRecognition::setRagFilePath(const std::string &userFilePath) {
    documentFilePath_ = userFilePath;
    filePassed_ = true;  // 传入成功标识
}

void IntentionRecognition::clearDialogueData() {
    currentDialogue_.dialogue.clear();
    documentFilePath_.clear();
    imageFilePath_.clear();
    resetForPromptId_ = 0;
}

void IntentionRecognition::loadDialogueData(const std::string &userInput) {
    Json::Value jsonUserInput = util::stringToJson(userInput);
    if (!jsonUserInput.isArray()) {
        return;
    }
    DialogueData::DialogueTurn turn;
    for (const auto &item : jsonUserInput) {
        turn.intent_category = "其他_闲聊";
        turn.turn_keywords = {"其他_闲聊"};  // 当前轮次关键词
        if (!item.isMember("assistant") || !item.isMember("user")) {
            continue;
        }
        if (!item["user"].asString().empty()) {
            turn.user_input = item["user"].asString();
        }
        if (!item["assistant"].asString().empty()) {
            turn.system_response = item["assistant"].asString();
        }

        if (!turn.user_input.empty() && !turn.system_response.empty()) {
            DialogueData::TokenCount responseTokenNumber = calculateTokenNumber(turn.system_response);
            turn.response_token_number = responseTokenNumber.totalTokens;
            currentDialogue_.dialogue.push_back(turn);
            printHistoryDialogue();
            turn = DialogueData::DialogueTurn();
        }
    }
}

std::string IntentionRecognition::buildIntentSystemPrompt(const Json::Value &intentSlotDefinitions) const {
    std::string intentSystemPrompt = intentPrompt_;

    std::string placeHolder = "__CANDIDATE_INTENTS__";
    size_t pos = intentSystemPrompt.find(placeHolder);
    if (pos != std::string::npos) {
        // intentSlotDefinitions 不为空时，添加闲聊、百科问答等意图
        Json::Value supplementedCandidateIntent = intentSlotDefinitions;

        if (intentSlotDefinitions.isArray() && !intentSlotDefinitions.empty()) {
#ifdef SUPPORT_VECTOR_DB
            LOGI("检测到候选意图不为空 (size:{}), 开始追加补充意图...", intentSlotDefinitions.size());
            supplementedCandidateIntent = intentSlotRecognizer_.getSupplementedCandidateIntents(intentSlotDefinitions);
#endif
        }
        std::string jsonStr = util::jsonToString(supplementedCandidateIntent);
        LOGI("supplementedCandidateIntent (size:{}): {}", supplementedCandidateIntent.size(), jsonStr);

        intentSystemPrompt.replace(pos, placeHolder.length(), jsonStr);
    }
    return intentSystemPrompt;
}

void IntentionRecognition::clearHistoryDialogue() {
    clearDialogueData();
#ifdef SUPPORT_VECTOR_DB
    // 删除指定的向量数据库
    deleteCustomizeKnowledgeBase(customizeKBName_);

    // 清除数据库名称
    customizeKBName_.clear();
#endif
}

void IntentionRecognition::loadHistoryDialogue(const std::string &history) {
    clearDialogueData();
#ifdef SUPPORT_VECTOR_DB
    // 删除指定的向量数据库
    deleteCustomizeKnowledgeBase(customizeKBName_);

    // 清除数据库名称
    customizeKBName_.clear();
#endif
    // 加载至历史会话
    loadDialogueData(history);
}

void IntentionRecognition::roundOneMultiModalInputRecognition(const std::string &userInput) {
    // TODO 长度检查
    std::string fileTypeValue = userInput.substr(userInput.length() - 6, -1);

    // 填充槽位
    std::vector<DialogueData::Slot> currentSlot;
    currentSlot.push_back(DialogueData::Slot{"附件类型", true, {fileTypeValue}});
    currentSlot.push_back(DialogueData::Slot{"API接口名称", false, {}});

    // 添加其他字段的值
    currentDialogInfo_.intent_category = "多模态输入识别";
    currentDialogInfo_.turn_keywords = {fileTypeValue};

    DialogueData::DialogueState currentState;
    currentState.slots = {currentSlot};
    currentState.system_acts = {"指令场景回复"};  // 引导
    currentState.user_acts = {"请求"};

    // 输出相关API接口及引导话术
    std::string resScript = "收到您的图片啦~需要我对这张图片做什么呢？";
    // 提供给用户n类API接口及引导话术
    std::list<std::string> listResult = TypeTrans::jsonArrayToStdList(functionInterfaceName_[fileTypeValue]);
    std::string concatValue = "\n";
    for (const auto &element : listResult) {
        concatValue += element;
        concatValue += "\t\n";
    }

    currentDialogInfo_.system_response = resScript + concatValue;
    currentDialogInfo_.dialogue_state = currentState;
    currentDialogue_.dialogue.push_back(currentDialogInfo_);
    printHistoryDialogue();

    // 引导类callback
    DialogueCallback::Type dataType = DialogueCallback::ChatGuidedNormal;
    guideCallback(fileTypeValue, resScript, listResult, "");
}

void IntentionRecognition::roundTwoMultiModalInputRecognition(const std::string &userInput,
                                                              const std::string &beforeTurnImage,
                                                              const DialogueData::APINameSearchResult &searchResult) {
    // 用户输入的是功能指令名称
    std::string fileType = searchResult.category;
    // 填充槽位
    std::vector<DialogueData::Slot> currentSlot;
    currentSlot.push_back(DialogueData::Slot{"附件类型", true, {fileType}});
    currentSlot.push_back(DialogueData::Slot{"API接口名称", false, {userInput}});

    // 添加其他字段的值
    currentDialogInfo_.intent_category = "多模态输入识别";
    currentDialogInfo_.turn_keywords = {userInput};

    DialogueData::DialogueState currentState;
    currentState.slots = {currentSlot};
    currentState.system_acts = {"对话生成"};
    currentState.user_acts = {"请求"};
    if (fileType == "文档") {
        // 调用指令类接口：allChatModelInference()

        // 读取指令场景的prompt
        bool success = callChatModelInferenceInterface(instructModelInput_).get();

        // 追加callback中生成的对话内容
        if (success) {
            currentDialogInfo_.system_response = chatGenCompleteText_;
            currentDialogInfo_.thinking_response = thinkingChatGenCompleteText_;
        }
    } else {
        // 图像接口
        // - 调用图像处理的接口, 目前仅支持智能抠图、黑白处理（二值化）
        // - userInput为指令名称，beforeTurnImage为图片路径，此处需要替换为图像处理的接口，该接口根据用户输入的指令，选
        //   择智能抠图，或者其他图像类指令接口
        if (userInput == "抠图") {
            callImageMattingInterface(beforeTurnImage);
        } else if (userInput == "扫描") {
            callImageBinarizationInterface(beforeTurnImage);
        } else if (userInput == "分析") {
            auto result = longChatImageParser_.extractImageContents(beforeTurnImage);
            if (result.errorCode == AI_COMMON_SUCCESS) {
                std::string userMessage = buildLongChatParserChatMessage(result.result);
                bool success = callChatModelInferenceInterface(userMessage).get();
                if (success) {
                    DialogueData::DialogueTurn currentTurn;
                    currentTurn.turn_num = currentDialogue_.dialogue.size();
                    currentTurn.intent_category = "其他_闲聊";
                    currentTurn.turn_keywords = {"其他_闲聊"};  // 当前轮次关键词
                    DialogueData::DialogueState state;
                    state.slots = {};                  // 更新对话状态
                    state.system_acts = {"对话生成"};  // 系统动作
                    state.user_acts = {"请求"};        // 用户动作
                    currentTurn.user_input = result.result;
                    currentTurn.system_response = chatGenCompleteText_;  // 系统响应
                    currentTurn.thinking_response = thinkingChatGenCompleteText_;
                    DialogueData::TokenCount responseTokenNumber = calculateTokenNumber(chatGenCompleteText_);
                    currentTurn.response_token_number = responseTokenNumber.totalTokens;
                    currentTurn.dialogue_state = state;
                    currentDialogue_.dialogue.push_back(currentTurn);
                    printHistoryDialogue();
                    return;
                }
            } else {
                LOGE("Error info: {} {}", result.errorCode, result.errorMessage);
                callOutputCallback(DialogueCallback::Type::ChatErrorOccurred,
                                   assistant::message::createErrorMessageResult(result.errorCode, result.errorMessage));
            }
        }
    }
    currentDialogInfo_.dialogue_state = currentState;
    currentDialogue_.dialogue.push_back(currentDialogInfo_);
    printHistoryDialogue();
}

#ifdef SUPPORT_VECTOR_DB

Json::Value IntentionRecognition::ragRetrieveGeneration(const std::string &responseContent,
                                                        const std::string &userInput, bool isFirstRound) {
    Json::Value dialogueStatus;
    DialogueData::CallSuccessIdentifier callResult;
    DialogueData::DialogueState currentState;
    std::vector<DialogueData::Slot> currentSlot;

    // 生成模型输入
    bool isRagInstruct = true;
    std::string instructName;
    if (isUseCustomizeKB_) {
        instructName = "文档问答";
    } else {
        instructName = "系统指南";
        // "rag检索生成";
    }

    std::string ragUserInput = responseContent + "\nquestion: " + userInput + "\n";

    ragPromptCopy_ = ragPrompt_;

    buildChatModelInput(isFirstRound, ragUserInput, instructName, isRagInstruct, true);

    LOGI("RAG's modelInput = {}", instructModelInput_);

    bool success = callChatModelInferenceInterface(instructModelInput_).get();

    // 追加callback中生成的对话内容
    if (success) {
        callResult.responseContent = chatGenCompleteText_;
    }

    if (!callResult.responseContent.empty() && success) {
        // rag检索生成成功
        LOGI("~~Rag retrieval successful~~ {}", callResult.responseContent);

        // 更新会话状态，填充槽位
        currentSlot.emplace_back();
        currentSlot.back().name = "附件类型";
        currentSlot.back().isRequired = true;
        currentSlot.back().values = {"文档"};

        currentSlot.emplace_back();
        currentSlot.back().name = "API接口名称";
        currentSlot.back().isRequired = false;
        currentSlot.back().values = {"RAG文档问答"};

        // 添加其他字段的值
        currentDialogInfo_.intent_category = instructName;  // 多模态输入识别
        currentDialogInfo_.turn_keywords = {instructName};

        currentState.slots = {currentSlot};
        currentState.system_acts = {"rag检索生成"};
        currentState.user_acts = {"请求"};

        currentDialogInfo_.system_response = callResult.responseContent;
        currentDialogInfo_.thinking_response = thinkingChatGenCompleteText_;
        DialogueData::TokenCount responseTokenNumber = calculateTokenNumber(callResult.responseContent);
        currentDialogInfo_.response_token_number = responseTokenNumber.totalTokens;
        currentDialogInfo_.dialogue_state = currentState;
        currentDialogue_.dialogue.push_back(currentDialogInfo_);
        printHistoryDialogue();

        dialogueStatus = TypeTrans::callSuccessIdentifierToJson(callResult);

    } else {
        // chatModel接口调用出错
        std::string responseStr = "哎呀，系统【RAG检索生成】接口好像出了点小状况哦~ ";
        callResult.responseContent = responseStr;
        dialogueStatus = TypeTrans::callSuccessIdentifierToJson(callResult);

        // 纯文本类callback
        std::string errorMessage = "RAG retrieval generation interface call failed.";
    }
    return dialogueStatus;
}

Json::Value IntentionRecognition::ragProcessingScenario(const std::string &userInput, const std::string &inputFilePath,
                                                        bool isFirstRound, bool updateSuccessTag) {
    Json::Value dialogueStatus;

    LOGI("customizeKBName = {}", customizeKBName_);
    if (customizeKBName_.empty() && documentFilePath_.empty()) {
        currentDialogInfo_.knowledge_base_name = defaultKBName_;
    }

    // 系统助手
    DialogueData::FileSearchIdentifier ragSearchRes;
    std::future<DialogueData::FileSearchIdentifier> result;

    if ((updateSuccessTag || !customizeKBName_.empty()) && isUseCustomizeKB_) {
        // 调用RAG-检索接口 -- 传入文档
        if (updateKbFileStop_) {
            LOGI("Stop to call RAG scenario");
            updateKbFileStop_ = false;
            callOutputCallback(DialogueCallback::ChatNormal, assistant::message::createChatEndMessageResult());
            return dialogueStatus;
        }
        result = callFileInferenceInterfaceSearchText(customizeKBName_, userInput, false);
    } else {
        // 调用RAG服务接口
        DialogueData::CallSuccessIdentifier callResult = callKnowledgeBaseInterface(userInput, chatModelName());
        if (!callResult.successRes) {
            int errorCode = int(assistant::Error::getErrorCodeFromAdvancedKnowledgeBase(callResult.errorCode));
            std::string responseStr;
            DialogueCallback::Type dataType = DialogueCallback::RagInterfaceErrorOccurred;
            textCallback(responseStr, chatModelName(), true, errorCode, callResult.errorMessage, dataType, "");
        }
        return dialogueStatus;
    }

    if (result.wait_for(std::chrono::seconds(300)) == std::future_status::ready) {
        try {
            ragSearchRes = result.get();
            ragFileSources_ = ragSearchRes.sources;
        } catch (const std::exception &e) {
            LOGE("Vectorization retrieval failed: {}", e.what());
        }
    } else {
        // 超时处理
        LOGI("Vectorization retrieval timeout, unable to obtain results.");
    }

    if (ragSearchRes.errorCode == 0 && !ragSearchRes.responseContent.empty()) {
        // 进入rag检索生成场景
        LOGI("~~~~~~~~~~~~~~~~~~~~Enter the RAG search to generate scenarios.~~~~~~~~~~~~~~~~~~~~");
        dialogueStatus = ragRetrieveGeneration(ragSearchRes.responseContent, userInput, isFirstRound);
        return dialogueStatus;
    } else {
        // 检索结果为空，回复话术
        // 纯文本类callback
        std::string responseStr = "当前未检索到相关参考资料。建议您补充更多细节信息，或上传相关文档后重试。";
        bool isError = ragSearchRes.errorCode != 0;
        std::string errorMessage = ragSearchRes.errorMessage;
        DialogueCallback::Type dataType = DialogueCallback::IntentNormal;
        textCallback(responseStr, "", isError, ragSearchRes.errorCode, errorMessage, dataType, "");
        // 重置feedback
        feedbackUrlForCloudModel_ = "";
        return dialogueStatus;
    }
}
#endif
std::string IntentionRecognition::getCompleteSessionFromMemory(bool isFirstRound, const std::string &userInput,
                                                               bool isEndModel) {
    Json::Value historyToUse;
    // 创建"messages"数组
    Json::Value messagesArray(Json::arrayValue);

    // 添加提示词
    Json::Value systemMessage;
    systemMessage["content"] = intentPrompt_;
    systemMessage["role"] = "system";
    messagesArray.append(systemMessage);

    // 直接截取最近N轮的对话内容，并只针对有槽位类的意图，进行历史会话追加
    if (!isFirstRound) {
        int startIndex =
            std::max(0, static_cast<int>(currentDialogue_.dialogue.size()) - DialogueConfig::maxFilteredRounds);

        // 截取第[startIndex]轮以后的历史对话

        for (size_t i = startIndex; i < currentDialogue_.dialogue.size(); ++i) {
            std::list<std::string> tempList = currentDialogue_.dialogue[i].dialogue_state.system_acts;
            std::string intentName_ = currentDialogue_.dialogue[i].intent_category;

            std::string systemResponseStr = currentDialogue_.dialogue[i].system_response;
            std::string intentResultStr = currentDialogue_.dialogue[i].intent_model_result;

            auto it = std::find(tempList.begin(), tempList.end(), "进一步询问");
            auto it_ = std::find(tempList.begin(), tempList.end(), "对话生成");
            auto ragFind_ = std::find(tempList.begin(), tempList.end(), "rag检索生成");
            auto instructFind_ = std::find(tempList.begin(), tempList.end(), "指令场景回复");

            if (it == tempList.end() && it_ == tempList.end() && ragFind_ == tempList.end() &&
                instructFind_ == tempList.end()) {
                // 不属于无槽位、仅打开页面、意图不明确、对话生成、指令场景、rag检索生成等情况
                if (!isEndModel && !systemResponseStr.empty()) {
                    Json::Value userQuery;
                    userQuery["content"] = currentDialogue_.dialogue[i].user_input;
                    userQuery["role"] = "user";
                    messagesArray.append(userQuery);

                    Json::Value sysResponse;
                    sysResponse["content"] = systemResponseStr;
                    sysResponse["role"] = "assistant";

                    messagesArray.append(sysResponse);
                } else if (isEndModel && !intentResultStr.empty()) {
                    Json::Value userQuery;
                    userQuery["content"] = currentDialogue_.dialogue[i].user_input;
                    userQuery["role"] = "user";
                    messagesArray.append(userQuery);

                    Json::Value sysResponse;
                    sysResponse["content"] = intentResultStr;
                    sysResponse["role"] = "assistant";
                    messagesArray.append(sysResponse);
                }
            }
        }
    }

    // 当前轮次的用户输入
    Json::Value userMessage;
    userMessage["content"] = userInput;
    userMessage["role"] = "user";

    messagesArray.append(userMessage);

    // 将"messages"数组添加到根对象
    historyToUse["messages"] = messagesArray;

    std::string modelInput = util::jsonToString(historyToUse);
    LOGI("Intention Model's modelInput = {}", modelInput);
    return modelInput;
}

void IntentionRecognition::buildChatModelInput(bool isFirstRound, const std::string &userInput,
                                               const std::string &instructName, bool isRagInstruct,
                                               bool hasCustomizeKBName) {
    // 获取指令类的上下文
    // 从currentDialogue中获取

    Json::Value historyToUse;
    // 创建"messages"数组
    Json::Value messagesArray(Json::arrayValue);
    Json::Value systemMessage;

    // 添加提示词
    if (isRagInstruct) {  // 使用rag prompt
        LOGI("ragPromptCopy_ = {}", ragPromptCopy_);
        systemMessage["content"] = ragPromptCopy_;
    } else {
        if (!instructionPrompt_.empty()) {
            systemMessage["content"] = instructionPrompt_;
        } else {
            systemMessage["content"] = defaultPromptContent_;
        }
    }

    systemMessage["role"] = "system";
    messagesArray.append(systemMessage);

    // 获取当前模型最大token数
    if (modelInferenceInterface_ != nullptr) {
        int tempMaxTokens = modelInferenceInterface_->getCurrentModelMaxTokens();
        if (tempMaxTokens > 0) {
            maxContextTokens_ = tempMaxTokens;
        }
    }

    if (!isFirstRound) {
        int startIndex = 0;
        bool isMultiRound{false};
        // 计算userInput的总token数
        DialogueData::TokenCount responseTokenNumber = calculateTokenNumber(userInput);
        int tempMaxContextTokens = maxContextTokens_ - responseTokenNumber.totalTokens;

        auto it =
            std::find(DialogueConfig::nameForNeedContent.begin(), DialogueConfig::nameForNeedContent.end(), promptId_);
        int ragTokenTotalNum = 0;
        for (int i = currentDialogue_.dialogue.size() - 1; i >= 0; --i) {
            DialogueData::DialogueTurn perTurnDialogue = currentDialogue_.dialogue[i];
            if (((!isRagInstruct && it != DialogueConfig::nameForNeedContent.end()) ||
                 (isRagInstruct && hasCustomizeKBName)) &&
                perTurnDialogue.intent_category == instructName &&
                (startIndex < DialogueConfig::maxFilteredRounds || modelInferenceInterface_->isCloudModel()) &&
                ragTokenTotalNum < tempMaxContextTokens && !perTurnDialogue.system_response.empty()) {
                // 指令场景仅扩写、润色、翻译三个场景需要上下文
                // 闲聊也需要上下文
                // rag场景需要上下文,不包含检索失败的上下文
                ragTokenTotalNum += perTurnDialogue.response_token_number;
                ++startIndex;
            }

            if (startIndex == DialogueConfig::maxFilteredRounds && !modelInferenceInterface_->isCloudModel() &&
                ragTokenTotalNum < tempMaxContextTokens) {
                startIndex = i;
                isMultiRound = true;
                break;
            } else if (ragTokenTotalNum >= tempMaxContextTokens) {
                // 超出最大上下文限制
                LOGI("Exceeding the maximum context limit, ragTokenTotalNum = {}", ragTokenTotalNum);
                startIndex = i + 1;
                isMultiRound = true;
                break;
            }
        }

        if (!isMultiRound) {
            // 小于等于1轮
            startIndex = 0;
        }

        for (size_t i = startIndex; i < currentDialogue_.dialogue.size(); ++i) {
            DialogueData::DialogueTurn perTurnDialogue = currentDialogue_.dialogue[i];
            if (((isRagInstruct && hasCustomizeKBName) ||
                 (!isRagInstruct && it != DialogueConfig::nameForNeedContent.end())) &&
                perTurnDialogue.intent_category == instructName && !perTurnDialogue.system_response.empty()) {
                Json::Value userQuery;
                userQuery["content"] = perTurnDialogue.user_input;
                userQuery["role"] = "user";
                messagesArray.append(userQuery);

                Json::Value sysResponse;
                sysResponse["content"] = perTurnDialogue.system_response;
                sysResponse["role"] = "assistant";

                messagesArray.append(sysResponse);
            }
        }
    }

    // 当前轮次的用户输入
    Json::Value userMessage;
    userMessage["content"] = userInput;
    userMessage["role"] = "user";

    messagesArray.append(userMessage);

    // 将"messages"数组添加到根对象
    historyToUse["messages"] = messagesArray;

    historyToUse["web_search"] = webSearch_.load();
    historyToUse["enable_thinking"] = enableThinking_.load();

    instructModelInput_ = util::jsonToString(historyToUse);
    LOGI("Instruction Model's modelInput = {}", instructModelInput_);
}

std::string IntentionRecognition::buildLongChatParserChatMessage(const std::string &userInput) const {
    // 携带历史对话信息
    std::string userPrompt = R"(
请基于以下微信聊天记录（OCR结果），默认输出四个部分：
1. 总结
2. 主题
3. 内容分析
4. 建议

【聊天记录（按顺序）】
{result_to_llms}
)";
    userPrompt = util::stringReplace(userPrompt, "{result_to_llms}", userInput);
    Json::Value messagesArray(Json::arrayValue);
    Json::Value systemMessage;
    systemMessage["content"] = getLongChatImageParserPrompt();
    systemMessage["role"] = "system";
    messagesArray.append(systemMessage);
    for (const DialogueData::DialogueTurn &dialogue : currentDialogue_.dialogue) {
        if (dialogue.user_input.empty() || dialogue.system_response.empty()) {
            continue;
        }
        Json::Value userMessage;
        userMessage["content"] = dialogue.user_input;
        userMessage["role"] = "user";
        messagesArray.append(userMessage);
        Json::Value assistantMessage;
        assistantMessage["content"] = dialogue.system_response;
        assistantMessage["role"] = "assistant";
        messagesArray.append(assistantMessage);
    }
    Json::Value newUserMessage;
    newUserMessage["content"] = userPrompt;
    newUserMessage["role"] = "user";
    messagesArray.append(newUserMessage);
    Json::Value chatMessage;
    chatMessage["messages"] = messagesArray;
    chatMessage["web_search"] = webSearch_.load();
    chatMessage["enable_thinking"] = enableThinking_.load();
    return util::jsonToString(chatMessage);
}

void IntentionRecognition::processAmbiguousIntent(const std::string &intentResultStr) {
    std::string responseStr;
    // 检查意图结果字符串中是否包含任何关键词
    bool includeIntentKeyWords = std::any_of(
        intentKeyWords_.cbegin(), intentKeyWords_.cend(),
        [&intentResultStr](const std::string &keyword) { return intentResultStr.find(keyword) != std::string::npos; });

    if (includeIntentKeyWords) {
        // 解析失败且包含意图关键词，说明格式有问题
        responseStr =
            "非常抱歉，当前意图识别结果的格式出现了问题，导致无法顺利解析。还请您再尝试一次，给您造成困扰，深表歉意";
    } else {
        // 解析失败但模型返回了内容，直接返回模型输出
        responseStr = intentResultStr;
    }
    LOGI("response = {}", responseStr);

    // 根据解析是否成功设置系统动作
    DialogueData::DialogueState currentState;
    currentState.system_acts = std::list<std::string>{"对话生成"};
    currentState.user_acts = {"请求"};
    currentState.slots = {};

    // 更新对话信息
    currentDialogInfo_.intent_category = "不明确意图";
    currentDialogInfo_.turn_keywords = {};
    currentDialogInfo_.system_response = responseStr;
    currentDialogInfo_.dialogue_state = currentState;
    currentDialogue_.dialogue.push_back(currentDialogInfo_);

    // 执行回调并重置反馈URL
    textCallback(responseStr, intentModelDisplayName(), false, 0, "", DialogueCallback::IntentNormal, "");
    feedbackUrlForCloudModel_ = "";
}

void IntentionRecognition::guideCallback(const std::string &fileTypeValue, const std::string &resScript,
                                         const std::list<std::string> &listResult, const std::string &modelName) const {
    // 引导类callback格式

    // 结果转换成助手的输出格式
    Json::Value assistantText;

    assistantText["message_type"] = "tool_call";

    if (fileTypeValue == "文档") {
        assistantText["file_type"] = "document";
    } else {
        assistantText["file_type"] = "image";
    }

    // 创建 "content" 数组节点，添加元素
    Json::Value contentArray;

    Json::Value contentElement;
    contentElement["type"] = "text";
    contentElement["text"] = resScript;
    contentArray.append(contentElement);

    for (uint32_t i = 0; i < listResult.size(); ++i) {
        auto it = std::next(listResult.begin(), i);

        Json::Value contentElement_;
        contentElement_["tool_id"] = uint32_t(i + 1);  // guide_func_id
        contentElement_["tool_name"] = *it;
        if (*it == "扫描") {
            contentElement_["tool_description"] = "进行扫描黑白处理";
        } else if (*it == "抠图") {
            contentElement_["tool_description"] = "实现智能识别，一键抠图";
        } else if (*it == "分析") {
            contentElement_["tool_description"] = "微信/蓝信聊天分析";
        }
        contentArray.append(contentElement_);
    }

    assistantText["content"] = contentArray;

    assistantText["error_code"] = 0;
    assistantText["error_message"] = "";
    assistantText["model"] = modelName;

    std::string assistantTextStr = util::jsonToString(assistantText);

    // 调用助手的回调函数
    DialogueCallback::ChatData tempDialogueData;
    tempDialogueData.type_ = DialogueCallback::ChatGuidedNormal;
    tempDialogueData.text_ = assistantTextStr;
    callOutputCallback(tempDialogueData.type_, tempDialogueData.text_);
}

void IntentionRecognition::textCallback(const std::string &responseStr, const std::string &modelName, bool isError,
                                        int errorCode, const std::string &errorMessage, DialogueCallback::Type dataType,
                                        const std::string &feedbackUrl) const {  // 纯文本类的callback格式

    // 结果转换成助手的输出格式
    Json::Value assistantText;

    assistantText["message_type"] = "chat";

    // 创建 "content" 数组节点，添加元素
    Json::Value contentArray;

    Json::Value contentElement;
    contentElement["type"] = "text";
    contentElement["text"]["sentence_id"] = 1;
    contentElement["text"]["is_end"] = true;
    contentElement["text"]["result"] = responseStr;
    contentArray.append(contentElement);

    assistantText["content"] = contentArray;

    if (!isError) {
        assistantText["error_code"] = 0;
        assistantText["error_message"] = "";

        if (modelInferenceInterface_ != nullptr) {
            if (modelInferenceInterface_->isCloudModel() && !feedbackUrl.empty()) {
                assistantText["feedback_url"] = feedbackUrl;
            }
        }
    } else {
        // TODO
        assistantText["error_code"] = 1;
        assistantText["error_message"] = errorMessage;
    }

    assistantText["model"] = modelName;

    Json::StreamWriterBuilder jswBuilder;
    jswBuilder["emitUTF8"] = true;
    std::unique_ptr<Json::StreamWriter> jsWriter(jswBuilder.newStreamWriter());

    std::ostringstream os;
    jsWriter->write(assistantText, &os);
    std::string assistantTextStr = os.str();

    // 调用助手的回调函数
    DialogueCallback::ChatData tempDialogueData;
    tempDialogueData.text_ = assistantTextStr;

    if (!isError) {
        tempDialogueData.type_ = dataType;
        callOutputCallback(tempDialogueData.type_, tempDialogueData.text_);

    } else {
        tempDialogueData.type_ = dataType;
        tempDialogueData.errorCode_ = errorCode;
        tempDialogueData.errorMessage_ = errorMessage;

        callOutputCallback(tempDialogueData.type_, assistant::message::createErrorMessageResult(
                                                       tempDialogueData.errorCode_, tempDialogueData.errorMessage_));
    }
}

void IntentionRecognition::intentRecognitionModelProcessingScenarios(const std::string &userInput, bool isFirstRound) {
    // 进入意图识别场景
    LOGI("~~~~~~~~~~~~~~~~~~~~Enter the intent recognition scenario.~~~~~~~~~~~~~~~~~~");
    // 使用意图识别模型推理的场景
    DialogueData::Intent curIntent;  // 意图
    DialogueData::DialogueState currentState;

    // 获取当前轮次完整会话
    modelInput_ = getCompleteSessionFromMemory(isFirstRound, userInput, !isBaseModelForIntention_);

    // 意图识别模型进行推理
    DialogueData::CallSuccessIdentifier intentRecoginzeResult = performIntentRecognize(modelInput_);

    // 异常码判断
    LOGI("intentRecoginzeResult.errorCode = {}", intentRecoginzeResult.errorCode);
    if (!intentRecoginzeResult.successRes) {
        callOutputCallback(DialogueCallback::IntentErrorOccurred,
                           assistant::message::createErrorMessageResult(intentRecoginzeResult.errorCode,
                                                                        intentRecoginzeResult.errorMessage));
        // 重置feedback
        feedbackUrlForCloudModel_ = "";
        return;
    }

    if (dialogueStop_) {
        // 这里不能把dialogueStop_设置为false。
        // 背景：弱网环境下
        // 原因：在使用大模型在进行意图识别时，chat接口是一个阻塞接口，必须模型生成结束后chat函数才结束。而且
        // 当前终止为了能立即响应，实际上是“假”终止，收到请求后调用回调立即反馈给用户，实际上还在进行生成或
        // 在进行终止。如果这里设为false，在连续两次输入意图识别指令，然后终止时，第一次会走到这里会把该变量设
        // 为false，第二次执行到这里时就会继续执行后面的槽位提取等流程了。
        // 传回助手一个结束标识符
        textCallback("", intentModelDisplayName(), false, 0, "", DialogueCallback::SystemOperationNormal, "");
        return;
    }

    // 解析意图结果
    std::string intentResultStr = intentRecoginzeResult.responseContent;
    LOGI("intentResultStr = {}", intentResultStr);
    intentResultStr = TypeTrans::replaceChinesePunctuation(intentResultStr);

    Json::Value intentResultJson = util::stringToJson(intentResultStr);
    if (intentResultJson.empty()) {
        // 解析错误
        LOGI("intentResultStr parse error.");
    } else {
        // 解析意图识别结果
        LOGI("Analyze intent recognition results: {}", intentResultStr);
        LOGI("Intent recognition output of json Data. size() = {}", intentResultJson.size());
        if (intentResultJson.isArray() && intentResultJson.size() == 1) {  // 单意图
            // 获取数组的第一个元素
            const Json::Value &firstIntent = intentResultJson[0];
            if (!firstIntent["intent_category"].empty()) {
                curIntent.name = firstIntent["intent_category"].asString();
                LOGI("Intent Name: {}", curIntent.name);
            }
            if (!firstIntent["inquiry"].empty()) {
                curIntent.inquiry = firstIntent["inquiry"].asString();
                LOGI("inquiry: {}", curIntent.inquiry);
            }
            const Json::Value &slotsJson = firstIntent["slots"];
            LOGI("Slots:");
            if (!firstIntent.isNull() && slotsJson.isArray()) {
                for (const auto &slotJson : slotsJson) {
                    DialogueData::Slot slot;
                    slot.name = slotJson["name"].asString();
                    slot.values.push_back(slotJson["value"].asString());
                    LOGI("  Slot Name: {}, Slot Value: {}", slot.name, slotJson["value"].asString());
                    curIntent.slots.push_back(slot);
                }
            }
        }
    }

    currentDialogInfo_.intent_model_result = intentResultStr;

    // 根据意图、或者解析结果parseSuccessFlag进行判断
    if (curIntent.name.empty() || intentResultStr.empty()) {
        curIntent.name = "闲聊";
        curIntent.inquiry = "";
        curIntent.slots = {};
    } else if (intentResultJson.empty()) {
        LOGI("由于意图不够清晰明确，抑或模型未能生成指定格式的输出，系统自动进行了对话回复的生成。");
        processAmbiguousIntent(intentResultStr);
        return;
    }

    // 意图明确
    auto mapFindIt = intentMap_.find(curIntent.name);
    std::string curIntentName = (mapFindIt != intentMap_.end()) ? mapFindIt->second : curIntent.name;

    std::string srcIntentName = curIntent.name;
    curIntent.name = curIntentName;

    std::list<std::string> eleSearchRes = TypeTrans::findElementContainingString(
        systemScenarioIntentionsTotal_,  // 系统操作、多模态、应用控制等系统控制类场景
        curIntent.name);                 // 判断是否为当前体系中的意图

    // 修改成规范的意图名称
    if (!eleSearchRes.empty()) {
        LOGI("eleSearchRes = {}", fmt::join(eleSearchRes, "\t"));
        std::string firstElement = eleSearchRes.front();
        curIntent.name = firstElement;
        LOGI("After conversion, curIntent.name = {}", curIntent.name);
    }

    auto findIt = std::find(DialogueConfig::idleChatName.begin(), DialogueConfig::idleChatName.end(), curIntent.name);

    if (eleSearchRes.empty() || (findIt != DialogueConfig::idleChatName.end())) {
        // 其他/闲聊、知识问答等场景
        // 生成模型输入
        bool isRagInstruct = false;
        std::string instructName = "其他_闲聊";
        buildChatModelInput(isFirstRound, userInput, instructName, isRagInstruct, false);

        LOGI("ModelInput for chatting and qa = {}", instructModelInput_);

        // 调用端模型（默认模型）进行处理并返回结果,
        // 使用默认提示词，不用传入
        bool success = callChatModelInferenceInterface(instructModelInput_).get();

        // 追加callback中生成的对话内容
        if (success) {
            intentRecoginzeResult.responseContent = chatGenCompleteText_;
        }

        // 更新对话状态
        size_t splitIndex = curIntent.name.find('(');
        currentDialogInfo_.intent_category = curIntent.name.substr(0, splitIndex);
        currentDialogInfo_.turn_keywords = {curIntent.name};  // 当前轮次关键词
        currentState.slots = {};                              // 更新对话状态

        currentState.system_acts = {"对话生成"};                                     // 系统动作
        currentState.user_acts = {"请求"};                                           // 用户动作
        currentDialogInfo_.system_response = intentRecoginzeResult.responseContent;  // 系统响应
        currentDialogInfo_.thinking_response = thinkingChatGenCompleteText_;

        currentDialogInfo_.dialogue_state = currentState;
        currentDialogue_.dialogue.push_back(currentDialogInfo_);
        printHistoryDialogue();

        TypeTrans::callSuccessIdentifierToJson(intentRecoginzeResult);
        return;
    } else if (!eleSearchRes.empty() && eleSearchRes.size() == 1 && findIt == DialogueConfig::idleChatName.end()) {
        // 用户意图在当前体系中存在
        auto supportedFindIt =
            std::find(supportedSystemInterfaces_.begin(), supportedSystemInterfaces_.end(), curIntent.name);

        if (supportedFindIt != supportedSystemInterfaces_.end()) {
            LOGI("可支持的系统控制场景");

            DialogueData::Intent curIntentCopy = curIntent;

            // 单意图
            // 使用front函数获取第一个元素
            std::string firstElement = eleSearchRes.front();

            LOGI("firstElement = {}", firstElement);

            bool slotIdentifierRes{false};
            auto openPageFindIt = std::find(DialogueConfig::slotsAvailableOnlyPageOpeningScene.begin(),
                                            DialogueConfig::slotsAvailableOnlyPageOpeningScene.end(), curIntent.name);

            if (firstElement.rfind("()") == firstElement.size() - 2 ||
                openPageFindIt != DialogueConfig::slotsAvailableOnlyPageOpeningScene.end()) {
                slotIdentifierRes = true;  // 无槽位判断
            }

            // 拆分意图和槽位
            size_t splitIndex = firstElement.find('(');
            curIntent.name = firstElement.substr(0, splitIndex);  // 意图

            Json::Value slotList;  // 槽位列表

            currentDialogInfo_.intent_category = curIntent.name;  // 当前轮次的意图
            currentDialogInfo_.turn_keywords = {curIntent.name};  // 当前轮次关键词

            if (!slotIdentifierRes) {
                // 有槽位的意图场景
                LOGI("~~~~有槽位的意图场景，规范化槽位信息~~~~");
                std::vector<DialogueData::Slot> currentSlotCopy;

                slotList = intentSlotJsonData_["hasSlotsLabel"][firstElement];
                std::string slotName_;

                // 需要遍历所有的槽位
                for (const auto &slotName : slotList) {
                    slotName_ = slotName.asString();
                    LOGI("slotName = {}", slotName_);

                    currentSlotCopy.emplace_back();

                    size_t foundPos = slotName_.find('*');
                    if (foundPos != std::string::npos) {
                        // 存在，必填槽
                        LOGI("\tRequired slot");
                        std::regex pattern("\\*");
                        slotName_ = std::regex_replace(slotName_, pattern, "");

                        currentSlotCopy.back().name = slotName_;
                        currentSlotCopy.back().isRequired = true;
                    } else {
                        currentSlotCopy.back().name = slotName_;
                        currentSlotCopy.back().isRequired = false;
                    }

                    currentSlotCopy.back().values = {};
                    LOGI("isRequired = {}", currentSlotCopy.back().isRequired);
                }

                // 填充当前轮次意图的所有槽位
                for (auto &eleSlot : currentSlotCopy) {
                    LOGI("eleSlot.name = {}", eleSlot.name);

                    for (auto &curSlot : curIntent.slots) {
                        LOGI("\tcurSlot.name = {}", curSlot.name);

                        if ((eleSlot.name == curSlot.name ||
                             (curSlot.name == "类型" && curIntent.name == "多模态输入识别_多模态输入识别" &&
                              eleSlot.name == "API接口名称")) &&
                            !curSlot.values.empty()) {
                            for (auto &valueEle : curSlot.values) {
                                LOGI("\t\tvalueEle = {}", valueEle);
                                if (valueEle == "大" && curIntent.name == "系统操作_音量控制") {
                                    valueEle = "增大";
                                } else if (valueEle == "小" && curIntent.name == "系统操作_音量控制") {
                                    valueEle = "减小";
                                } else if (valueEle == "高" && curIntent.name == "系统操作_音量控制") {
                                    valueEle = "调高";
                                } else if (valueEle == "低" && curIntent.name == "系统操作_音量控制") {
                                    valueEle = "调低";
                                } else if (valueEle == "快进" && curIntent.name ==
                                                                     "第三方应用控制_多媒体-"
                                                                     "播放器控制") {
                                    valueEle = "下一首";
                                } else if (valueEle == "快退" && curIntent.name ==
                                                                     "第三方应用控制_多媒体-"
                                                                     "播放器控制") {
                                    valueEle = "上一首";
                                } else if (valueEle == "重复播放" &&
                                           curIntent.name == "第三方应用控制_多媒体-播放器控制") {
                                    valueEle = "单曲循环";
                                }
                                eleSlot.values.push_back(valueEle);
                            }
                        }
                    }
                }

                currentState.slots = currentSlotCopy;  // 更新对话状态

                curIntent.slots = currentSlotCopy;  // 重新更新curIntent
                curIntentCopy.slots = currentSlotCopy;

#ifdef SUPPORT_VECTOR_DB
                // 槽位取值规范化处理
                curIntentCopy = slotNormalizationProcess(curIntentCopy, slotsValueNormalJson_);
                curIntent.slots = curIntentCopy.slots;  // 重新更新curIntent
#endif
            }
            // 打印当前轮次意图槽位信息
            LOGI("~~~~~~~~~~~~Print current round intent slot information.~~~~~~~~~~~~~~");
            LOGI("curIntent.name = {}", curIntent.name);

            for (const auto &ele : curIntent.slots) {
                LOGI("\tcurSlot.name = {}", ele.name);
                LOGI("\tcurSlot.isRequired = {}", ele.isRequired);
                for (const auto &slotValue : ele.values) {
                    LOGI("\t\tValue = {}", slotValue);
                }
            }

            // 进入系统控制流程
            DialogueData::CallSuccessIdentifier systemControlResult =
                systemApplicationControlMain(curIntent, userInput, slotIdentifierRes);

            // 更新对话状态
            if (systemControlResult.slotMissing) {
                currentState.system_acts = {"询问"};  // 系统动作

            } else if (systemControlResult.isMultiModalDoc || slotIdentifierRes) {
                currentState.system_acts = {"对话生成"};  // 系统动作
            } else {
                currentState.system_acts = {"回复"};  // 系统动作
            }

            // 更新对话状态
            currentState.slots = curIntent.slots;
            currentState.user_acts = {"请求"};                                         // 用户动作
            currentDialogInfo_.system_response = systemControlResult.responseContent;  // 系统响应
            currentDialogInfo_.dialogue_state = currentState;
            currentDialogue_.dialogue.push_back(currentDialogInfo_);

            TypeTrans::callSuccessIdentifierToJson(systemControlResult);
            return;
        } else {
            // 目前尚未开放此系统控制场景
            size_t splitIndex = curIntent.name.find('(');
            std::string secondLabelName = curIntent.name.substr(0, splitIndex);

            size_t splitIdx = secondLabelName.find('_');
            std::string interfaceName = secondLabelName.substr(splitIdx + 1);
            LOGI("interfaceName = {}", interfaceName);
            std::string verbalTricksStr =
                "系统控制场景【" + interfaceName + "】现阶段尚未开放，敬请期待后续更新与启用。";

            LOGI("verbalTricksStr = {}", verbalTricksStr);
            intentRecoginzeResult.responseContent = verbalTricksStr;
            TypeTrans::callSuccessIdentifierToJson(intentRecoginzeResult);

            // 文本类callback
            std::string modelName = intentModelDisplayName();
            bool isError = false;
            std::string errorMessage;
            DialogueCallback::Type dataType = DialogueCallback::IntentNormal;
            textCallback(intentRecoginzeResult.responseContent, modelName, isError, 0, errorMessage, dataType, "");
            // 重置feedback
            feedbackUrlForCloudModel_ = "";
            return;
        }
    } else {
        // 多意图的处理
        std::string verbalTricksStr =
            "AI助手在多意图处理与识别场景方面的功能尚未开启，暂无法为您提供相应服务。我们正不懈努力，致力于在后续的升级"
            "中完善此功能，以期给您带来更为卓越、智能的使用体验";

        LOGD("{}", verbalTricksStr);

        intentRecoginzeResult.responseContent = verbalTricksStr;
        TypeTrans::callSuccessIdentifierToJson(intentRecoginzeResult);

        // 文本类callback
        std::string modelName = intentModelDisplayName();
        bool isError = false;
        std::string errorMessage;
        DialogueCallback::Type dataType = DialogueCallback::IntentNormal;
        textCallback(intentRecoginzeResult.responseContent, modelName, isError, 0, errorMessage, dataType, "");
        // 重置feedback
        feedbackUrlForCloudModel_ = "";
        return;
    }
}

void IntentionRecognition::imageInstructionCallback(const std::string &responseStr, const std::string &modelName,
                                                    bool isError, int errorCode, const std::string &errorMessage,
                                                    DialogueCallback::Type dataType, const std::string &fileType,
                                                    int toolId, const std::string &toolDescription,
                                                    const std::string &toolData,
                                                    const std::string &feedbackUrl) const {  // 图像指令类的callback格式

    // 结果转换成助手的输出格式
    Json::Value assistantText;

    assistantText["message_type"] = "tool_call";
    if (fileType == "文档") {
        assistantText["file_type"] = "document";
    } else if (fileType == "图片") {
        assistantText["file_type"] = "image";
    } else {
        assistantText["file_type"] = "schedule";  // 日历
    }

    // 创建 "content" 数组节点，添加元素
    Json::Value contentArray;

    Json::Value contentElement;
    contentElement["type"] = "text";
    contentElement["text"] = responseStr;
    contentArray.append(contentElement);

    Json::Value contentElementLink;
    contentElementLink["tool_id"] = toolId;
    contentElementLink["tool_name"] = fileType;
    contentElementLink["tool_description"]["description"] = toolDescription;
    contentElementLink["tool_description"]["parameter"] = toolData;
    contentArray.append(contentElementLink);

    assistantText["content"] = contentArray;

    if (!isError) {
        assistantText["error_code"] = 0;
        assistantText["error_message"] = "";
        if (modelInferenceInterface_ != nullptr) {
            if (modelInferenceInterface_->isCloudModel() && !feedbackUrl.empty()) {
                assistantText["feedback_url"] = feedbackUrl;
            }
        }
    } else {
        assistantText["error_code"] = 1;
        assistantText["error_message"] = errorMessage;
    }

    assistantText["model"] = modelName;

    Json::StreamWriterBuilder jswBuilder;
    jswBuilder["emitUTF8"] = true;
    std::unique_ptr<Json::StreamWriter> jsWriter(jswBuilder.newStreamWriter());

    std::ostringstream os;
    jsWriter->write(assistantText, &os);
    std::string assistantTextStr = os.str();
    LOGI("assistantText = {}", assistantTextStr);

    // 调用助手的回调函数
    DialogueCallback::ChatData tempDialogueData;
    tempDialogueData.text_ = assistantTextStr;

    if (!isError) {
        tempDialogueData.type_ = dataType;
        callOutputCallback(tempDialogueData.type_, tempDialogueData.text_);

    } else {
        tempDialogueData.type_ = dataType;
        tempDialogueData.errorCode_ = errorCode;
        tempDialogueData.errorMessage_ = errorMessage;

        callOutputCallback(tempDialogueData.type_, assistant::message::createErrorMessageResult(
                                                       tempDialogueData.errorCode_, tempDialogueData.errorMessage_));
    }
}

bool IntentionRecognition::isTextToImageCommand(const std::string &prompt) const {
    if (promptId_ != int(config::PromptTemplate::PromptId::Default)) {
        // 有指令的情况下不走文生图
        return false;
    }

    ImageGenTrigger trigger;
    return trigger.shouldTriggerImageGeneration(prompt);
}

DialogueData::CallSuccessIdentifier IntentionRecognition::multiModalInputRecognition(
    const DialogueData::Intent &curTurnIntent, const std::string &userInput) {
    LOGI("Processing the scene -- {}", curTurnIntent.name);

    std::map<std::string, std::list<std::string>> tempSlot;
    DialogueData::CallSuccessIdentifier callResult;

    for (const auto &slot : curTurnIntent.slots) {
        tempSlot[slot.name] = slot.values;
    }

    auto it = tempSlot.find("附件类型");
    if (it != tempSlot.end() && !tempSlot["附件类型"].empty()) {
        auto it_ = tempSlot.find("API接口名称");

        std::string multiCheckRes = TypeTrans::checkList(tempSlot["附件类型"]);
        LOGI("Attachment type check result: {}", multiCheckRes);

        if (multiCheckRes == "图片") {
            // 图像场景是有槽位的
            LOGI("Entering Image Processing Scene.");

            std::string currentApiName = "";
            bool hasValidApiName = false;

            if (it_ != tempSlot.end() && !it_->second.empty()) {
                currentApiName = it_->second.front();
                hasValidApiName = true;
                LOGI("Found API Name in slots: {}", currentApiName);
            }

            if (!hasValidApiName) {
                LOGI("Missing API Name slot. Prompting user for image theme/style.");
                callResult.responseContent = "您想生成一张什么主题风格的图片呢？赶快告诉我吧.";
                callResult.successRes = true;
                callResult.slotMissing = true;

                // 纯文本类callback
                textCallback(callResult.responseContent, intentModelDisplayName(), false, 0, "",
                             DialogueCallback::SystemOperationNormal, "");
            } else if (currentApiName != "文生图") {
                bool isSupportedImageApi = false;

                // 检查是否是图片处理类场景
                if (TypeTrans::findAPIName(currentApiName, functionInterfaceName_, "图片").found ||
                    currentApiName == "图片处理") {
                    isSupportedImageApi = true;
                    LOGI("API '{}' is supported for image processing.", currentApiName);
                }

                if (isSupportedImageApi) {
                    callResult.responseContent = "请先输入一张图片，之后按照助手给出的引导信息输入就行啦~";
                } else {
                    // 不支持的图像类接口或接口名称缺失
                    if (currentApiName.empty()) {
                        // 场景：接口名称为空，说明意图识别不完整，需要引导用户明确
                        LOGW("Image API name is empty. Prompting user to specify the operation.");
                        callResult.responseContent =
                            "哎呀，我好像没弄清楚您具体想对图片做什么操作。麻烦您再具体告诉我一下，我马上帮您处理！";
                    } else {
                        // 场景：有接口名称，但该名称不在支持的图片处理列表中
                        LOGI("Image API '{}' is not supported in the current list.", currentApiName);
                        callResult.responseContent =
                            "很抱歉，您选择的这个图片处理功能暂时还没上线呢，敬请期待后续更新与启用吧~";
                    }
                }

                callResult.successRes = true;

                // 纯文本类callback
                textCallback(callResult.responseContent, intentModelDisplayName(), false, 0, "",
                             DialogueCallback::SystemOperationNormal, "");
            } else {
                // 进入该逻辑的，只能是文生图
                LOGI("Calling Text-to-Image handler for API: {}", currentApiName);

                callResult = callImageHandlerInterface(userInput);
                if (callResult.errorCode != 0 && callResult.errorCode != -1) {
                    // 接口出错
                    callResult.responseContent = "哎呀，不好意思，多模态输入识别场景文生图出现了小状况~";
                } else {
                    // 调用文生图callback
                    // 追加callback中生成的图片内容
                    callResult.responseContent = "以下是精心为您创作生成的图片，敬请查收与欣赏。 ";
                    callResult.additionalContent = imageData_;
                    callResult.successRes = true;
                }
            }
        } else {
            // 非图片类型或类型为空
            LOGI("Entering Multimodal Document Processing Scene. Type: {}", multiCheckRes);

            callResult.isMultiModalDoc = true;
            // 生成模型输入
            bool isRagInstruct = false;
            std::string instructName = "其他_闲聊";
            buildChatModelInput(false, userInput, instructName, isRagInstruct, false);

            LOGI("ModelInput for Multimodal document class = {}", instructModelInput_);

            // 调用端模型（默认模型）进行处理并返回结果,
            // 使用默认提示词，不用传入
            bool success = callChatModelInferenceInterface(instructModelInput_).get();

            // 追加callback中生成的对话内容
            if (success) {
                callResult.responseContent = chatGenCompleteText_;
            }
        }
    } else {
        // 附件类型为空
        LOGI("Slot '附件类型' is missing or empty. Triggering clarification response.");

        callResult.responseContent =
            "实在不好意思，我好像没完全理解您的需求。您能否麻烦再详细说说您想处理的文件类型，以及打算对文件执行怎样的操"
            "作呢？这样我就能更准确地回应您，为您提供更贴心的服务啦。 ";
        callResult.successRes = true;
        callResult.slotMissing = true;

        // 纯文本类callback
        textCallback(callResult.responseContent, intentModelDisplayName(), false, 0, "",
                     DialogueCallback::SystemOperationNormal, "");
    }
    // 重置feedback
    feedbackUrlForCloudModel_ = "";
    return callResult;
}

DialogueData::CallSuccessIdentifier IntentionRecognition::systemApplicationControlMain(
    DialogueData::Intent &curTurnIntent, const std::string &userInput, bool slotIdentifier) {
    LOGI("userInput = {}", userInput);

    // 正在处理的场景
    LOGI("Scenario being processed = systemApplicationControlMain -- {}", curTurnIntent.name);

    size_t splitIndex = curTurnIntent.name.find('_');
    std::string interfaceName = curTurnIntent.name.substr(splitIndex + 1);
    LOGI("interfaceName = {}", interfaceName);

    DialogueData::CallSuccessIdentifier callIdentify;

    // 多模态输入识别模块--用户输入除指令类外的文本,经过意图识别模型推理的处理逻辑
    if (curTurnIntent.name == "多模态输入识别_多模态输入识别") {
        callIdentify = multiModalInputRecognition(curTurnIntent, userInput);
        return callIdentify;
    }

    if (TypeTrans::isStringInList(DialogueConfig::applicationCategoriesIntent,
                                  curTurnIntent.name.substr(0, curTurnIntent.name.find('_')))) {
        if (slotIdentifier) {
            // 无槽位场景
            isIntentInquiring_ = false;
            callIdentify = callSystemOperationInterface(curTurnIntent);  // 调用系统命令接口

            // 异常码判断
            if (!callIdentify.successRes) {
                callIdentify.responseContent =
                    "很抱歉这次未能成功为您弹出【 " + interfaceName + " 】页面，或许您可以尝试换一种与我对话的方式";

            } else {
                callIdentify.responseContent = "您请求的【 " + interfaceName + " 】页面已成功弹出";
            }

        } else {
            // 有槽位场景，命令控制类,直接判断必填槽是否缺失即可
            // 判断必填槽是否存在，且不为空

            std::list<std::string> emptySlotName;
            std::list<std::string> noEleList = {""};
            for (auto &slot : curTurnIntent.slots) {
                LOGI("hasSlot -- slot.isRequired = {}", slot.isRequired);
                if (slot.isRequired == true && (slot.values.empty() || slot.values == noEleList)) {
                    // 槽位为空
                    LOGI("hasSlot -- slot.name = {}. The slot is empty.", slot.name);

                    emptySlotName.push_back(slot.name);  // 必填槽为空的槽位名称
                } else if (slot.isRequired == false && (slot.values.empty() || slot.values == noEleList) &&
                           curTurnIntent.name == "系统操作_蓝牙操作") {
                    slot.values = {"打开"};
                }
            }

            LOGI("emptySlotName.ele = ", fmt::join(emptySlotName, "\t"));

            if (emptySlotName.empty()) {
                // 无必填槽或者必填槽不为空,
                isIntentInquiring_ = false;
                callIdentify = callSystemOperationInterface(curTurnIntent);  // 调用系统命令接口

                // 异常码判断
                if (!callIdentify.successRes ||
                    (callIdentify.errorCode != 0 && callIdentify.errorCode != -1)) {  // 接口出错
                    if (curTurnIntent.name == "系统操作_设置显示器屏幕缩放") {
                        callIdentify.responseContent =
                            "很遗憾，这次【 " + interfaceName +
                            " 】调整不太顺利，可能是缩放比例有误，请您输入100%-150%之间的数值";
                    } else if (std::find(DialogueConfig::intentIndividualScene.begin(),
                                         DialogueConfig::intentIndividualScene.end(),
                                         curTurnIntent.name) != DialogueConfig::intentIndividualScene.end()) {
                        callIdentify.responseContent = "很抱歉这次未能成功为您弹出【 " + interfaceName +
                                                       " 】页面，或许您可以尝试换一种与我对话的方式";
                    } else {
                        callIdentify.responseContent =
                            "很遗憾，这次【 " + interfaceName +
                            " 】调整未能成功，可能是有些细节信息有所遗漏或不太符合要求，烦请您再描述得详细些";
                    }
                } else {
                    bool displayScaling;
                    if (curTurnIntent.name == "系统操作_设置显示器屏幕缩放") {
                        displayScaling = TypeTrans::checkAndSetScaling(curTurnIntent.slots);
                        LOGI("displayScaling = {}", (displayScaling ? "true" : "false"));
                    }

                    if (!displayScaling && curTurnIntent.name == "系统操作_设置显示器屏幕缩放") {
                        callIdentify.responseContent =
                            "很遗憾，这次【 " + interfaceName +
                            " 】调整不太顺利, 可能是缩放比例有误，请您输入100%-150%之间的数值";

                    } else {
                        if (std::find(DialogueConfig::intentIndividualScene.begin(),
                                      DialogueConfig::intentIndividualScene.end(),
                                      curTurnIntent.name) != DialogueConfig::intentIndividualScene.end()) {
                            callIdentify.responseContent = "您请求的【 " + interfaceName + "】页面已成功弹出";
                        } else if (std::find(DialogueConfig::addedIntentScenarios.begin(),
                                             DialogueConfig::addedIntentScenarios.end(),
                                             curTurnIntent.name) != DialogueConfig::addedIntentScenarios.end()) {
                            callIdentify.responseContent = "恭喜您！【 " + interfaceName + " 】已调整成功。";
                        } else if (curTurnIntent.name == "第三方应用控制_应用控制") {
                            callIdentify.responseContent = "恭喜您！应用页面已打开";
                        } else if (curTurnIntent.name == "第三方应用控制_多媒体-播放器控制") {
                            callIdentify.responseContent = "您请求的【 " + interfaceName + " 】调整成功";
                        } else {
                            callIdentify.responseContent =
                                "恭喜您！【 " + interfaceName + " 】调整成功，烦请您移步至设置中查看";
                        }
                    }
                }
            } else {
                // 必填槽为空
                // 若必填槽为空，则询问，进行补槽

                if (!curTurnIntent.inquiry.empty() && curTurnIntent.name != "系统操作_设置系统字体") {
                    callIdentify.responseContent = curTurnIntent.inquiry;
                } else {
                    std::string firstMissingSlot = emptySlotName.front();

                    std::string missingScript;
                    bool scriptFound = false;

                    const std::string &intentNameStr = curTurnIntent.name;
                    auto itIntent =
                        missingSlotScript_.find(intentNameStr.c_str(), intentNameStr.c_str() + intentNameStr.size());

                    if (itIntent != nullptr) {
                        // 意图存在，再找槽位
                        const Json::Value &slotConfig = *itIntent;

                        if (slotConfig.isObject()) {
                            if (slotConfig.isMember(firstMissingSlot)) {
                                missingScript = slotConfig[firstMissingSlot].asString();
                                scriptFound = true;
                            }
                        }
                    }

                    if (!scriptFound && missingScript.empty()) {
                        missingScript =
                            "关于" + interfaceName + "，我需要您提供【" + firstMissingSlot + "】的信息，请告诉我。";
                    }
                    LOGI("~~~~~EmptySlot = {}\tScript = {}", firstMissingSlot, missingScript);

                    callIdentify.responseContent = missingScript;
                }

                callIdentify.successRes = true;
                callIdentify.slotMissing = true;
            }
        }

        // 纯文本类callback
        std::string modelName = intentModelDisplayName();
        if (!callIdentify.successRes || (callIdentify.errorCode != 0 && callIdentify.errorCode != -1)) {
            bool isError = true;
            std::string errorMessage = "System operation interface call failed.";
            DialogueCallback::Type dataType = DialogueCallback::SystemOperationErrorOccurred;
            int errorCode = int(assistant::ErrorCode::SystemOperationFailed);
            textCallback(callIdentify.responseContent, modelName, isError, errorCode, errorMessage, dataType, "");
        } else {
            bool isError = false;
            std::string errorMessage;
            DialogueCallback::Type dataType = DialogueCallback::SystemOperationNormal;
            textCallback(callIdentify.responseContent, modelName, isError, 0, errorMessage, dataType, "");
        }
        // 重置feedback
        feedbackUrlForCloudModel_ = "";
        return callIdentify;
    }

    return callIdentify;
}

Json::Value IntentionRecognition::textInstructionScenario(const std::string &userInput, bool isFirstRound,
                                                          const std::string &userFile) {
    // 文本类指令场景
    DialogueData::CallSuccessIdentifier callResult;
    DialogueData::DialogueState currentState;
    std::vector<DialogueData::Slot> currentSlot;

    currentDialogInfo_.intent_category = instructionName_;
    currentDialogInfo_.turn_keywords = {instructionName_};

    currentState.slots = {currentSlot};
    currentState.system_acts = {"指令场景回复"};
    currentState.user_acts = {"请求"};

    // 调用对应的功能接口，进行处理，调用指令类接口
    LOGI("Call instruction class interface：callChatModelInference()");

    // 生成模型输入
    bool isRagInstruct = false;
    std::string documentContent;
    std::string instructUserInput;

    if (!userFile.empty() && (promptId_ == meetingInformationExtractionPromptId ||
                              promptId_ == todoListGenerationPromptId || promptId_ == weeklyReportGenerationPromptId)) {
#ifdef SUPPORT_VECTOR_DB
        // 指令场景（除系统助手）不需要知识库
        documentContent =
            callFileInferenceInterfaceFileContent(defaultKBName_, userFile).responseContent;  // 获取文件内容
#endif
        instructUserInput = documentContent + "\nquestion: " + userInput + "\n";

    } else {
        instructUserInput = userInput;
    }

    buildChatModelInput(isFirstRound, instructUserInput, instructionName_, isRagInstruct, false);
    LOGI("Instruction Model's modelInput = {}", instructModelInput_);

    bool success = callChatModelInferenceInterface(instructModelInput_).get();

    // 追加callback中生成的对话内容
    if (success) {
        callResult.responseContent = chatGenCompleteText_;
        if (!dialogueStop_) {
            // 处理日程和待办场景
            if (promptId_ == meetingInformationExtractionPromptId) {
                LOGI("chatGenCompleteText_ = {}", chatGenCompleteText_);
                DialogueData::CallSuccessIdentifier callSuccess = callSystemScheduleManagement(chatGenCompleteText_);

                if (callSuccess.successRes == true) {
                    // 使用图像指令类callback
                    std::string responseStr = "您的日程已新建成功！您可以在 日历 插件中查看、修改或删除此日程。";
                    std::string modelName = currentModelNameInUse_;
                    std::string errorMessage;
                    std::string descriptionStr = "在日历插件中查看、修改或删除此日程";
                    std::string scheduleDate = callSuccess.scheduleDate;
                    DialogueCallback::Type dataType = DialogueCallback::SystemOperationNormal;
                    imageInstructionCallback("", modelName, false, 0, errorMessage, dataType, "日程", 21,
                                             descriptionStr, scheduleDate, feedbackUrlForCloudModel_);
                    // 重置feedback
                    feedbackUrlForCloudModel_ = "";
                } else {
                    // 纯文本类callback
                    std::string responseStr =
                        "非常抱歉，我无法为您新建日程，因为您没有提供开始时间等信息，或者您提供的信息我无法识别。为了成"
                        "功新建日程，请确保您提供了清晰、准确的日程信息。";
                    std::string modelName = currentModelNameInUse_;
                    int errorCode = int(assistant::ErrorCode::CreateScheduleFailed);
                    std::string errorMessage = responseStr;
                    DialogueCallback::Type dataType = DialogueCallback::SystemOperationErrorOccurred;
                    textCallback(responseStr, modelName, true, errorCode, errorMessage, dataType, "");
                }
            } else if (promptId_ == todoListGenerationPromptId) {
                DialogueData::CallSuccessIdentifier callSuccess = callSystemToDoList(chatGenCompleteText_);

                if (callSuccess.successRes == true) {
                    // 使用图像指令类callback
                    std::string todoListHeadStr = "以下是为您生成的待办:\n";
                    std::string responseStr = todoListHeadStr + chatGenCompleteText_;
                    std::string modelName = currentModelNameInUse_;
                    std::string errorMessage;
                    std::string descriptionStr = "在日历插件中查看、修改或删除此日程";
                    std::string scheduleDate = callSuccess.scheduleDate;
                    DialogueCallback::Type dataType = DialogueCallback::SystemOperationNormal;
                    imageInstructionCallback(responseStr, modelName, false, 0, errorMessage, dataType, "待办", 22,
                                             descriptionStr, scheduleDate, feedbackUrlForCloudModel_);
                    // 重置feedback
                    feedbackUrlForCloudModel_ = "";
                } else {
                    // 纯文本类callback
                    std::string responseStr =
                        "非常抱歉，我无法为您生成待办，因为您没有提供开始时间等信息，或者您提供的信息我无法识别。为了成"
                        "功生成待办，请确保您提供了清晰、准确的日程信息。";
                    std::string modelName = currentModelNameInUse_;
                    int errorCode = int(assistant::ErrorCode::CreateToDoListFailed);
                    std::string errorMessage = responseStr;
                    DialogueCallback::Type dataType = DialogueCallback::SystemOperationErrorOccurred;
                    textCallback(responseStr, modelName, true, errorCode, errorMessage, dataType, "");
                }
            }
        } else {
            if (promptId_ == todoListGenerationPromptId || promptId_ == meetingInformationExtractionPromptId) {
                dialogueStop_ = false;
                // 传回助手一个结束标识符
                std::string responseStr;
                std::string errorMessage;
                std::string modelName = currentModelNameInUse_;
                DialogueCallback::Type dataType = DialogueCallback::SystemOperationNormal;
                textCallback(responseStr, modelName, false, 0, errorMessage, dataType, feedbackUrlForCloudModel_);
            }
        }
    } else {
        callResult.responseContent = "哎呀，系统【对话】接口好像出了点小状况哦~ ";
    }

    currentDialogInfo_.dialogue_state = currentState;
    currentDialogInfo_.system_response = callResult.responseContent;
    DialogueData::TokenCount responseTokenNumber = calculateTokenNumber(callResult.responseContent);
    currentDialogInfo_.response_token_number = responseTokenNumber.totalTokens;
    currentDialogue_.dialogue.push_back(currentDialogInfo_);
    printHistoryDialogue();

    Json::Value dialogueStatus = TypeTrans::callSuccessIdentifierToJson(callResult);

    // 重置，目前设定的选一次指令，默认一直为该指令场景,以下三种场景是单次的
    if (promptId_ == meetingInformationExtractionPromptId || promptId_ == todoListGenerationPromptId ||
        promptId_ == weeklyReportGenerationPromptId) {
        promptPassed_ = false;
        promptId_ = defaultPromptId;
        instructionName_ = "其他_闲聊";
        instructionPrompt_ = defaultPromptContent_;
    }

    // 重置feedback
    feedbackUrlForCloudModel_ = "";
    return dialogueStatus;
}
#ifdef SUPPORT_VECTOR_DB
std::pair<std::string, std::string> IntentionRecognition::processSlotValue(const Json::Value &slotsList,
                                                                           const std::string &slotStr,
                                                                           const std::string &intentName,
                                                                           const std::string &slotName) {
    if (!TypeTrans::isStringInJsonArray(slotsList, slotStr)) {
        LOGI("TypeTrans::isPureUTF8ChineseText(slotStr) = {}, slotStr = {}", TypeTrans::isPureUTF8ChineseText(slotStr),
             slotStr);
        if (TypeTrans::isPureUTF8ChineseText(slotStr)) {
            // 文本类槽位，且不在槽位取值范围内，进行规划范处理
            // 计算相似度，选取top1作为规范化结果
            // 处理文本型槽位
            LOGI("Processing text-based slots.");
            Json::Value vectorSearchResult;
            std::pair<std::string, std::string> replacedSlot = {"", "未对槽位进行规范化转换"};

            // 调用文本向量化接口
            DialogueCallback::TextEmbeddingResult embeddingResult = callTextEmbeddingInterface(slotStr);
            LOGI("embeddingResult.embedding.size = {}", embeddingResult.embedding.size());

            std::vector<float> textVectorResult = embeddingResult.embedding;
            if (!textVectorResult.empty() && textVectorResult.size() == modelDimension_) {
                // 调用向量数据库检索接口
                slotsNormalization_ = std::make_shared<SlotsNormalization>();
                vectorSearchResult = slotsNormalization_->search(textVectorResult, slotStr, slotName, intentName);
                // 打印向量化检索结果
                LOGI("vectorSearchResult : {}", util::jsonToString(vectorSearchResult));

                if (!vectorSearchResult["Top_K_Label"].empty() && vectorSearchResult["Top_K_Label"].isArray()) {
                    replacedSlot = {vectorSearchResult["Top_K_Label"][0].asString(), "已成功规范化槽位"};
                    return replacedSlot;
                }
            }
            return {slotStr, "向量化服务异常，未做规范化转换"};
        } else if (TypeTrans::isInteger(slotStr) || TypeTrans::isInteger(TypeTrans::removeSuffix(slotStr))) {
            // 如果是整数，判断是否在取值范围内，不超出则不用进一步处理；对于超出取值范围的进行就近处理
            // 处理数值型槽位 -- 整数
            LOGI("Processing numerical slots - integers.");

            auto result = TypeTrans::isSlotInRange(slotStr, intentName, DialogueConfig::integerScenarios);
            if (!result.first.empty()) {
                // 最终处理后的值为
                LOGI("\tThe final processed value is: {}", result.first);
            }
            if (!result.second.empty()) {
                // 提示信息
                LOGI("\tReminder information: {}", result.second);
            }

            return result;
        } else if (TypeTrans::isDecimal(slotStr) || TypeTrans::isDecimal(TypeTrans::removeSuffix(slotStr))) {
            // 如果是小数，判断是否在取值范围内，不超出则不用进一步处理；对于超出取值范围的进行就近处理
            // 处理数值型槽位 -- 小数
            LOGI("Processing numerical slots - decimals.");

            auto decimalResult = TypeTrans::isSlotInDecimalRange(slotStr, intentName, DialogueConfig::decimalScenarios,
                                                                 DialogueConfig::integerScenarios);
            if (!decimalResult.first.empty()) {
                // 最终处理后的值为
                LOGI("\tThe final processed value is: {}", decimalResult.first);
            }
            if (!decimalResult.second.empty()) {
                // 提示信息
                LOGI("\tReminder information: {}", decimalResult.second);
            }

            return decimalResult;
        } else {
            // 槽位类型不支持的场景，将槽位置为空, 并返回引导的话术
            return {"", "不合理的槽位类型，请按提示重新输入"};
        }
    } else {
        // 本身是规范化槽位，不用进一步处理
        return {slotStr, "规范化槽位"};
    }
}

DialogueData::Intent IntentionRecognition::slotNormalizationProcess(DialogueData::Intent &curIntentSlots,
                                                                    const Json::Value &slotsValueJson) {
    // 文本向量化进行槽位规范化处理
    LOGI("~~~~Text vectorization for slot normalization processing.~~~~");
    auto findSlot = std::find(DialogueConfig::slotNormalizationFilterList.begin(),
                              DialogueConfig::slotNormalizationFilterList.end(), curIntentSlots.name);

    if (findSlot == DialogueConfig::slotNormalizationFilterList.end()) {
        LOGI("curIntentSlots.name = {}", curIntentSlots.name);

        for (auto &ele : curIntentSlots.slots) {
            if (slotsValueJson.isMember(curIntentSlots.name) && !slotsValueJson.isNull() && slotsValueJson.isObject()) {
                const Json::Value &slotsValue = slotsValueJson[curIntentSlots.name];
                if (!slotsValue.isNull() && slotsValue.isMember(ele.name) && slotsValue.isObject()) {
                    // 查找 "屏幕亮度值" 节点
                    const Json::Value &slotsValueList = slotsValue[ele.name];

                    // 打印
                    LOGI("\tslotsValue = {}", util::jsonToString(slotsValueList));

                    if (slotsValueList.isArray() && !slotsValueList.empty()) {
                        for (auto &slot_ : ele.values) {
                            LOGI("\t\tValue = {}", slot_);
                            // 字符串slot_是否在slotsValueList中
                            LOGI("\t\tString \"{}\", is it in slotsValueList: {}", slot_,
                                 (TypeTrans::isStringInJsonArray(slotsValueList, slot_) ? "yes" : "no"));

                            std::pair<std::string, std::string> replaceSlotResult =
                                processSlotValue(slotsValueList, slot_, curIntentSlots.name, ele.name);

                            // 替换槽位值
                            if (!replaceSlotResult.first.empty()) {
                                // 槽位规范化处理后的值为
                                LOGI("The value after slot normalization processing is: {}\t{}",
                                     replaceSlotResult.first, replaceSlotResult.second);
                                slot_ = replaceSlotResult.first;
                            } else if (!replaceSlotResult.second.empty()) {
                                // 存在无法识别或不满足条件的槽位
                                LOGI("There are slots that cannot be identified or do not meet the conditions: {}",
                                     replaceSlotResult.second);
                                if (ele.name != "桌面应用名称") {
                                    slot_ = replaceSlotResult.first;
                                }
                                curIntentSlots.abnormalSlotScript = replaceSlotResult.second;
                            }
                        }
                    } else {
                        // slotsValueList 不是数组或者为空，无法进行规范化处理
                        LOGI("slotsValueList is not an array or empty, unable to perform normalization processing.");
                    }
                } else {
                    // "槽位 -- ele.name 不在规范化的槽位列表中，不需要做规范化处理
                    LOGI(
                        "Slot - {} is not in the standardized slot list and does not require normalization processing.",
                        ele.name);
                }
            } else {
                // 意图 -- curIntentSlots.name 不在槽位规范化的意图列表中，不需要做规范化处理
                LOGI(
                    "Intent - {} is not in the intent list for slot normalization and does not require normalization "
                    "processing.",
                    curIntentSlots.name);
            }
        }
    } else {
        // 意图 -- curIntentSlots.name 在过滤列表中，不需要做规范化处理
        LOGI("Intention - {} does not require normalization in the filtering list.", curIntentSlots.name);
    }

    return curIntentSlots;
}

void IntentionRecognition::updateFileToKb(bool &updateSuccess) {
    updateKbFileStop_ = false;

    std::future<DialogueData::FileSearchIdentifier> createKbResultFuture = cpr::async([this] {
        std::lock_guard<std::mutex> lock(updateKbFileMutex_);
        // 加锁，防止重复调用
        return updateFileKnowledgeBase(customizeKBName_, documentFilePath_);
    });
    // 超时时间控制在300秒，500毫秒做一次轮询
    for (int i = 0; i < 300 * 2; ++i) {
        std::future_status status = createKbResultFuture.wait_for(std::chrono::milliseconds(500));
        if (status == std::future_status::ready) {
            DialogueData::FileSearchIdentifier createKBResult = createKbResultFuture.get();
            if (createKBResult.errorCode) {
                std::string responseMessage = "向量数据库创建失败，请检查向量数据库服务是否正常。";
                DialogueCallback::Type dataType = DialogueCallback::IntentErrorOccurred;
                textCallback(responseMessage, "", true, createKBResult.errorCode, createKBResult.errorMessage, dataType,
                             "");
                return;
            }
            // 创建知识库成功
            LOGI("Successfully created knowledge base, {}", customizeKBName_);
            updateSuccess = true;
            break;
        }
        if (updateKbFileStop_) {
            // 用户主动中断的场景
            LOGE("Stopping create knowledge base.");
            // 文本类callback
            callOutputCallback(DialogueCallback::ChatNormal, assistant::message::createChatEndMessageResult());
            return;
        }
    }
}

void IntentionRecognition::createCustomizeDataBase(const std::string &userFile, bool &updateSuccess) {
    std::future<DialogueData::KnowledgeBaseResult> createResult;

    // 创建或新增向量数据库
    if (isFirstAfferentFile_) {
        DialogueData::CallSuccessIdentifier processResult;
        // 初始会话或者清除上下文恢复到初始状态
        createResult = createFileKnowledgeBase();
        if (createResult.wait_for(std::chrono::seconds(300)) == std::future_status::ready) {
            DialogueData::KnowledgeBaseResult createKBResult = createResult.get();
            customizeKBName_ = createKBResult.knowledgeBaseName;
            if (!customizeKBName_.empty()) {
                updateFileToKb(updateSuccess);
            } else {
                // 创建知识库失败，自定义用户数据库名称为空
                LOGE("Failed to create knowledge base, custom user database name is empty.");
                // 文本类callback
                std::string responseStr =
                    R"(创建知识库失败,自定义用户数据库名称为空。请检查向量数据库接口服务是否正常。)";
                bool isError = true;
                std::string errorMessage = createKBResult.errorMessage;
                int errorCode = createKBResult.errorCode;
                DialogueCallback::Type dataType = DialogueCallback::RagInterfaceErrorOccurred;
                textCallback(responseStr, "", isError, errorCode, errorMessage, dataType, "");
                processResult.responseContent = responseStr;
                return;
            }

        } else {
            // 超时处理
            // 文本类callback
            std::string responseStr = R"(创建知识库失败,接口服务请求超时，请检查向量数据库接口服务是否正常。)";
            LOGI("responseStr: {}", responseStr);

            bool isError = true;
            std::string errorMessage = "Create vector database timeout, unable to obtain results.";
            int errorCode = int(assistant::ErrorCode::UnknownError);
            DialogueCallback::Type dataType = DialogueCallback::RagInterfaceErrorOccurred;
            textCallback(responseStr, "", isError, errorCode, errorMessage, dataType, "");
            processResult.responseContent = responseStr;
            return;
        }
        currentDialogInfo_.knowledge_base_name = customizeKBName_;

        // 重置
        isFirstAfferentFile_ = false;

    } else if (!isFirstAfferentFile_ && !userFile.empty() && !customizeKBName_.empty()) {
        // 更新已有的向量数据库
        updateFileToKb(updateSuccess);

        currentDialogInfo_.knowledge_base_name = customizeKBName_;
    }
}

#endif

DialogueData::TokenCount IntentionRecognition::calculateTokenNumber(const std::string &userInput) const {
    // 计算总字符数
    int userInputLength = static_cast<int>(userInput.size());
    // 计算英文字符数
    int englishCharactersNumber = TypeTrans::countEnglishChars(userInput);
    int enTokens = static_cast<int>(std::ceil(static_cast<float>(englishCharactersNumber) / DialogueConfig::enBytes /
                                              DialogueConfig::enCorrespondence));
    // 中文字符数转token
    int chineseCharactersNumber = userInputLength - englishCharactersNumber;
    int zhTokens = static_cast<int>(std::ceil(static_cast<float>(chineseCharactersNumber) / DialogueConfig::zhBytes /
                                              DialogueConfig::zhCorrespondence));
    int totalTokenNum = enTokens + zhTokens;

    return {userInputLength, enTokens, zhTokens, totalTokenNum};
}

bool IntentionRecognition::isScheduleToolCall(const Json::Value &jsonMessage) const {
    return jsonMessage.isMember("file_type") && jsonMessage["file_type"].asString() == "schedule";
}

bool IntentionRecognition::isIntentRecognitionCall(const Json::Value &jsonUserInput) const {
    bool isIntentRecognition = true;
    if (jsonUserInput.isMember("intent_recognition") && jsonUserInput["intent_recognition"].isBool()) {
        isIntentRecognition = jsonUserInput["intent_recognition"].asBool();
    }
    return isIntentRecognition;
}

void IntentionRecognition::dialogueControlMain(bool isFirstRound, const std::string &userInput) {
    LOGI("isFirstRound: {}, userInput: {}", isFirstRound, userInput);

    std::string userText;
    std::string userFilePath;   // 文档路径
    std::string imageFilePath;  // 图片路径
    dialogueStop_ = false;
    updateKbFileStop_ = false;

    Json::Value jsonUserInput = util::stringToJson(userInput);
    if (not jsonUserInput.isMember("content")) {
        callOutputCallback(
            DialogueCallback::ChatErrorOccurred,
            assistant::message::createErrorMessageResult(int(assistant::ErrorCode::InputTextInvalid), "输入参数异常"));
        return;
    }

    int toolId{-1};
    for (const auto &item : jsonUserInput["content"]) {
        if (item.isMember("text")) {
            userText = item["text"].asString();
        }
        if (item.isMember("document_url")) {
            userFilePath = item["document_url"].asString();
        }
        if (item.isMember("image_url")) {
            imageFilePath = item["image_url"].asString();
        }
        if (item.isMember("tool_id")) {
            toolId = item["tool_id"].asInt();
        }
    }

    if (jsonUserInput.isMember("web_search") && jsonUserInput["web_search"].isBool()) {
        webSearch_ = jsonUserInput["web_search"].asBool();
    }
    if (jsonUserInput.isMember("enable_thinking") && jsonUserInput["enable_thinking"].isBool()) {
        enableThinking_ = jsonUserInput["enable_thinking"].asBool();
    }

    // 会话id
    currentDialogue_.dialogue_id = dialogueId_;
    DialogueData::CallSuccessIdentifier processResult;
    LOGI("currentDialogue.dialogue_id = {}", currentDialogue_.dialogue_id);
    if (isFirstRound) {
        // 初始化当前轮次对话变量
        currentDialogInfo_.turn_num = currentDialogue_.dialogue.size();  // 当前会话轮数，和索引保持一致
        currentDialogInfo_.first_turn_flag = isFirstRound;
        currentDialogInfo_.role = "user/sys";
        currentDialogInfo_.user_input = userText;
        // 第一轮
        LOGI("First round: \n{}", currentDialogInfo_.toString());
    } else {
        // 加载 currentDialogue.dialogue_id的历史会话数据
        // 当前会话轮数，和索引保持一致
        currentDialogInfo_.turn_num = currentDialogue_.dialogue.size();
        currentDialogInfo_.first_turn_flag = isFirstRound;
        currentDialogInfo_.role = "user";
        currentDialogInfo_.user_input = userText;
        // 非第一轮
        LOGI("Not the first round: \n{}", currentDialogInfo_.toString());
    }

    if (!isIntentRecognitionCall(jsonUserInput)) {
        LOGD("{}", "进入闲聊/知识问答场景.");
        // 生成模型输入
        buildChatModelInput(isFirstRound, userText, "其他_闲聊", false, false);
        // 调用端模型（默认模型）进行处理并返回结果，使用默认提示词，不用传入
        bool success = callChatModelInferenceInterface(instructModelInput_).get();
        // 追加callback中生成的对话内容
        if (success) {
            processResult.responseContent = chatGenCompleteText_;
        }

        // 更新对话状态
        currentDialogInfo_.intent_category = "其他_闲聊";
        currentDialogInfo_.turn_keywords = {"其他_闲聊"};  // 当前轮次关键词
        DialogueData::DialogueState currentState;
        currentState.slots = {};                                             // 更新对话状态
        currentState.system_acts = {"对话生成"};                             // 系统动作
        currentState.user_acts = {"请求"};                                   // 用户动作
        currentDialogInfo_.system_response = processResult.responseContent;  // 系统响应
        currentDialogInfo_.thinking_response = thinkingChatGenCompleteText_;
        DialogueData::TokenCount responseTokenNumber = calculateTokenNumber(processResult.responseContent);
        currentDialogInfo_.response_token_number = responseTokenNumber.totalTokens;
        currentDialogInfo_.dialogue_state = currentState;
        currentDialogue_.dialogue.push_back(currentDialogInfo_);
        printHistoryDialogue();
        return;
    }

    if (promptId_ == int(config::PromptTemplate::PromptId::IntentionRecognition)) {
        if (modelInferenceInterface_ == nullptr) {
            assistant::ErrorCode errorCode{assistant::ErrorCode::IntentNlpModelNotFound};
            std::string message = assistant::message::createErrorMessageResult(int(errorCode), std::string{});
            callOutputCallback(DialogueCallback::ChatErrorOccurred, message);
            return;
        }
        if (modelInferenceInterface_->isIntentModelNotLoaded()) {
            assistant::ErrorCode errorCode{assistant::ErrorCode::IntentNlpModelNotFound};
            std::string message = assistant::message::createErrorMessageResult(int(errorCode), std::string{});
            callOutputCallback(DialogueCallback::ChatErrorOccurred, message);
            return;
        }
        if (modelInferenceInterface_->intentModelName().empty()) {
            assistant::ErrorCode errorCode{assistant::ErrorCode::IntentNlpModelNotFound};
            std::string message = assistant::message::createErrorMessageResult(int(errorCode), std::string{});
            callOutputCallback(DialogueCallback::ChatErrorOccurred, message);
            return;
        }
        std::string ret = StartUpTools::getInstance().systemControl(userText);
        if (!ret.empty()) {
            callOutputCallback(DialogueCallback::Type::ChatNormal, ret);
            return;
        }
    }

    // 系统助手工具调用
    if (assistant::message::isSourceUrlToolCall(jsonUserInput)) {
        int theToolId = assistant::message::toolCallIdFromMessage(jsonUserInput);
        callSourceUrlTool(theToolId);
        return;
    } else if (isScheduleToolCall(jsonUserInput)) {
        // 解析日期信息
        int toolId_{0};
        std::string scheduleDate;
        if (jsonUserInput.isMember("content") && jsonUserInput["content"].isArray() &&
            jsonUserInput["content"].size() > 0) {
            for (unsigned int i = 0; i < jsonUserInput["content"].size(); i++) {
                if (jsonUserInput["content"][i].isMember("type") &&
                    jsonUserInput["content"][i]["type"].asString() == "tool_id" &&
                    jsonUserInput["content"][i].isMember("tool_id") && jsonUserInput["content"][i]["tool_id"].isInt()) {
                    toolId_ = jsonUserInput["content"][i]["tool_id"].asInt();
                } else if (jsonUserInput["content"][i].isMember("type") &&
                           jsonUserInput["content"][i]["type"].asString() == "schedule_date" &&
                           jsonUserInput["content"][i].isMember("schedule_date") &&
                           jsonUserInput["content"][i]["schedule_date"].isString()) {
                    scheduleDate = jsonUserInput["content"][i]["schedule_date"].asString();
                }
            }
        }

        if (toolId_ == 21 || toolId_ == 22) {
            if (!scheduleDate.empty()) {
                // 调用系统打开日历超链接的接口
                systemOperationInterface_->callSystemOpenScheduleInterface(scheduleDate);
                return;
            }
            return;
        }
    }

    // 多模态第一轮处理逻辑，拖入图片文件
    if (!imageFilePath.empty() && toolId == -1) {
        // 传入图片
        LOGD("roundOneMultiModalInputRecognition");
        userText = "用户输入了一张图片";
        currentDialogInfo_.image_file_path = imageFilePath;
        imageFilePath_ = imageFilePath;
        roundOneMultiModalInputRecognition(userText);
        return;
    }

    // 用户输入为文本或者图像指令名称的场景--多模态输入识别的第二轮逻辑
    // TODO toolMap应该从配置文件读取
    std::map<int, std::string> toolMap{{1, "扫描"}, {2, "抠图"}, {3, "分析"}};
    if (toolMap.find(toolId) != toolMap.end()) {
        userText = toolMap[toolId];
    }
    DialogueData::APINameSearchResult imageSearchResult =
        TypeTrans::findAPIName(userText, functionInterfaceName_, "图片");

    if (imageSearchResult.found && !imageFilePath.empty()) {
        LOGI("Second round of multimodal input recognition.");
        roundTwoMultiModalInputRecognition(userText, imageFilePath, imageSearchResult);
        return;
    }

    if (isTextToImageCommand(userText)) {
        // 包含文生图关键词，进入文生图
        LOGI("Text to image command.");
        processResult = callImageHandlerInterface(userText);
        if (!processResult.successRes) {
            // 接口出错
            processResult.responseContent = "哎呀，不好意思，多模态输入识别场景文生图出现了小状况~";
        } else {
            // 调用文生图callback
            // 追加callback中生成的图片内容
            processResult.responseContent = "以下是精心为您创作生成的图片，敬请查收与欣赏。 ";
            processResult.additionalContent = imageData_;
        }

        TypeTrans::callSuccessIdentifierToJson(processResult);

        return;
    }

    currentDialogInfo_.user_file_path = userFilePath;

    // 计算token数量
    DialogueData::TokenCount userTextLength = calculateTokenNumber(userText);
    currentDialogInfo_.query_token_number = userTextLength.totalTokens;

#ifdef SUPPORT_VECTOR_DB
    if (!userFilePath.empty() && promptId_ != meetingInformationExtractionPromptId &&
        promptId_ != todoListGenerationPromptId && promptId_ != weeklyReportGenerationPromptId) {  // 文档问答

        if (documentFilePath_.empty()) {
            // 初次传入文件时，置为true
            isFirstAfferentFile_ = true;
        }

        documentFilePath_ = userFilePath;

        isUseCustomizeKB_ = true;
        // 置promptId_为系统指南
        resetForPromptId_ = systemAssistantPromptId;
        promptId_ = defaultPromptId;
        instructionName_ = "系统指南";
        resetFlagForPromptId_ = true;
    }

    bool updateSuccess{false};
    if (userText.empty() && userFilePath.empty()) {
        // 用户输入为空
        // 纯文本类callback
        bool isError = true;
        std::string errorMessage = "user input can not be empty.";
        std::string responseMessage = "用户输入不能为空哟~ ";
        DialogueCallback::Type dataType = DialogueCallback::ImageErrorOccurred;

        int errorCode = int(assistant::ErrorCode::InputTextInvalid);

        textCallback(responseMessage, "", isError, errorCode, errorMessage, dataType, "");
        processResult.responseContent = responseMessage;
        TypeTrans::callSuccessIdentifierToJson(processResult);
        return;
    } else if (userText.empty() && !userFilePath.empty() &&
               (promptId_ == systemAssistantPromptId || resetForPromptId_ == systemAssistantPromptId) &&
               promptId_ != meetingInformationExtractionPromptId && promptId_ != todoListGenerationPromptId &&
               promptId_ != weeklyReportGenerationPromptId) {
        std::string modelName = intentModelDisplayName();
        bool isError = false;
        int errorCode = 0;
        std::string errorMessage;
        std::string responseMessage = "收到您的文档啦~需要我对这份文档做什么呢?";
        DialogueCallback::Type dataType = DialogueCallback::IntentNormal;

        // 更新对话状态
        currentDialogInfo_.intent_category = modelName;
        currentDialogInfo_.turn_keywords = {modelName};  // 当前轮次关键词
        DialogueData::DialogueState currentState;
        currentState.slots = {};                               // 更新对话状态
        currentState.system_acts = {"对话生成"};               // 系统动作
        currentState.user_acts = {"请求"};                     // 用户动作
        currentDialogInfo_.system_response = responseMessage;  // 系统响应

        currentDialogInfo_.dialogue_state = currentState;
        currentDialogue_.dialogue.push_back(currentDialogInfo_);
        printHistoryDialogue();
        if (!isChatModelConfigured()) {
            // callback
            errorMessage = "模型加载失败";
            dataType = DialogueCallback::ChatErrorOccurred;
            responseMessage = "";
            textCallback(responseMessage, "", true, int(assistant::ErrorCode::NlpModelNotFound), errorMessage, dataType,
                         "");
            processResult.responseContent = responseMessage;
            TypeTrans::callSuccessIdentifierToJson(processResult);
            return;
        }
        // 创建自定义数据库
        createCustomizeDataBase(userFilePath, updateSuccess);
        if (updateSuccess) {
            // callback
            textCallback(responseMessage, "", isError, errorCode, errorMessage, dataType, "");
        }
        processResult.responseContent = responseMessage;
        TypeTrans::callSuccessIdentifierToJson(processResult);
        return;
    } else if (!userText.empty() && !documentFilePath_.empty() &&
               (promptId_ == systemAssistantPromptId || resetForPromptId_ == systemAssistantPromptId) &&
               promptId_ != meetingInformationExtractionPromptId && promptId_ != todoListGenerationPromptId &&
               promptId_ != weeklyReportGenerationPromptId) {
        if (!isChatModelConfigured()) {
            // callback
            std::string errorMessage = "模型加载失败";
            DialogueCallback::Type dataType = DialogueCallback::ChatErrorOccurred;
            std::string responseMessage;
            textCallback(responseMessage, "", true, int(assistant::ErrorCode::NlpModelNotFound), errorMessage, dataType,
                         "");
            processResult.responseContent = responseMessage;
            TypeTrans::callSuccessIdentifierToJson(processResult);
            return;
        }
        // 创建自定义数据库
        createCustomizeDataBase(userFilePath, updateSuccess);
    }
#endif
    if (promptPassed_ and promptId_ != systemAssistantPromptId and promptId_ != defaultPromptId and
        promptId_ != int(config::PromptTemplate::PromptId::IntentionRecognition)) {
        // 文本类指令场景
        LOGI("Text command scenario.");
        textInstructionScenario(userText, isFirstRound, userFilePath);
        return;
    }

    // 系统控制场景（意图识别）
    if (promptId_ == int(config::PromptTemplate::PromptId::IntentionRecognition)) {
        intentRecognitionModelProcessingScenarios(userText, isFirstRound);
        return;
    }
#ifdef SUPPORT_VECTOR_DB
    // RAG场景停止会话
    if (updateKbFileStop_) {
        LOGI("Stop to call RAG scenario");
        updateKbFileStop_ = false;
        callOutputCallback(DialogueCallback::ChatNormal, assistant::message::createChatEndMessageResult());
        return;
    }

    // RAG场景（系统助手、文档问答）
    if ((promptId_ == systemAssistantPromptId || resetForPromptId_ == systemAssistantPromptId) &&
        promptId_ != meetingInformationExtractionPromptId && promptId_ != todoListGenerationPromptId &&
        promptId_ != weeklyReportGenerationPromptId) {
        if (!isChatModelConfigured()) {
            // callback
            std::string errorMessage = "模型加载失败";
            DialogueCallback::Type dataType = DialogueCallback::ChatErrorOccurred;
            std::string responseMessage;
            textCallback(responseMessage, "", true, int(assistant::ErrorCode::NlpModelNotFound), errorMessage, dataType,
                         "");
            processResult.responseContent = responseMessage;
            TypeTrans::callSuccessIdentifierToJson(processResult);
            return;
        }
        // RAG场景（系统助手、文档问答）
        LOGI("RAG scenario (system assistant, document Q&A).");
        ragProcessingScenario(userText, userFilePath, isFirstRound, updateSuccess);
        return;
    }
#endif
    if (modelInferenceInterface_ == nullptr) {
        assistant::ErrorCode errorCode{assistant::ErrorCode::IntentNlpModelNotFound};
        std::string message = assistant::message::createErrorMessageResult(int(errorCode), std::string{});
        callOutputCallback(DialogueCallback::ChatErrorOccurred, message);
        return;
    }
    if (modelInferenceInterface_->isIntentModelNotLoaded()) {
        assistant::ErrorCode errorCode{assistant::ErrorCode::IntentNlpModelNotFound};
        std::string message = assistant::message::createErrorMessageResult(int(errorCode), std::string{});
        callOutputCallback(DialogueCallback::ChatErrorOccurred, message);
        return;
    }
    if (modelInferenceInterface_->intentModelName().empty()) {
        assistant::ErrorCode errorCode{assistant::ErrorCode::IntentNlpModelNotFound};
        std::string message = assistant::message::createErrorMessageResult(int(errorCode), std::string{});
        callOutputCallback(DialogueCallback::ChatErrorOccurred, message);
        return;
    }
    std::string ret = StartUpTools::getInstance().systemControl(userText);
    if (!ret.empty()) {
        callOutputCallback(DialogueCallback::Type::ChatNormal, ret);
        return;
    }
    intentRecognitionModelProcessingScenarios(userText, isFirstRound);
}

void IntentionRecognition::printHistoryDialogue() const {
    std::string log;
    for (const DialogueData::DialogueTurn &dialogueTurn : currentDialogue_.dialogue) {
        log += fmt::format("================================================================================\n");
        log += fmt::format("{}", dialogueTurn.toString());
        log += fmt::format("================================================================================\n");
    }
    LOGI("{}", log);
}

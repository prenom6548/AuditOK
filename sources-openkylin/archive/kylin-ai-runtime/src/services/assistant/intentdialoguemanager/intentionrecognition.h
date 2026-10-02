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

#ifndef SERVICES_ASSISTANT_INTENTION_RECOGNITION_H
#define SERVICES_ASSISTANT_INTENTION_RECOGNITION_H

#include <jsoncpp/json/json.h>

#include <functional>
#include <future>
#include <string>

#include "dialoguemessage.h"
#include "imagehandler/imagehandlerinterface.h"
#include "intentrecognizer/intenttypes.h"
#include "modelinference/modelinferenceinterface.h"
#ifdef SUPPORT_VECTOR_DB
#include "fileinference/fileinferenceinterface.h"
#include "intentrecognizer/intentslotrecognizer.h"
#include "slotsnormalization.h"
#include "textembedding/textembeddinginterface.h"
#endif
#include "longchatimageparser.h"
#include "systemoperation/systemoperationinterface.h"
#include "systemscenarioclass.h"

// 意图识别
class IntentionRecognition : public BaseFunction {
public:
    // 构造函数
    IntentionRecognition();  // isFirstRound
    ~IntentionRecognition();

    void setSystemOperationInterface(SystemOperationInterface *interface);
#ifdef SUPPORT_VECTOR_DB
    void setFileInferenceInterface(FileInferenceInterface *interface);
    void setTextEmbeddingInterface(TextEmbeddingInterface *interface);
    void setKnowledgeBaseInterface(KnowledgeBaseInterface *interface);
#endif
    void setModelInferenceInterface(ModelInferenceInterface *interface);
    void setImageHandlerInterface(ImageHandlerInterface *interface);

    void setOutputCallback(const DialogueCallback::OutputCallback &callback);

    // 加载意图识别模型的 prompt 文件
    void loadIntentPromptText();

    // 加载文档/图片类功能接口名称
    void loadJsonContent();

    // 加载意图和槽位,筛选出包含槽位的意图场景
    void loadIntentAndSlots();

    // 助手调用传指令的prompt
    void setScenarioInstructionPrompt(int promptId, const std::string &promptText, const std::string &instructionName);

    // RAG用户输入的文件路径
    void setRagFilePath(const std::string &userFilePath);

    // 清楚历史会话内容
    void clearHistoryDialogue();
    void loadHistoryDialogue(const std::string &history);

    void dialogueControlMain(bool isFirstRound, const std::string &userInput);

    Json::Value getfunctionInterfaceName();

    void stopDialogue();

private:
    DialogueData::CallSuccessIdentifier callSystemOperationInterface(const DialogueData::Intent &scenarioIntent) const;

    DialogueData::CallSuccessIdentifier callSystemScheduleManagement(const std::string &scheduleText) const;

    DialogueData::CallSuccessIdentifier callSystemToDoList(const std::string &todoListText) const;

    void callSourceUrlTool(int toolId) const;

    std::future<DialogueData::FileSearchIdentifier> callFileInferenceInterfaceSearchText(
        const std::string &knowledgeBaseName, const std::string &text, bool useDefault) const;

    DialogueData::CallSuccessIdentifier callFileInferenceInterfaceFileContent(const std::string &knowledgeBaseName,
                                                                              const std::string &input) const;

    std::future<bool> callChatModelInferenceInterface(const std::string &modelInput) const;

    DialogueData::CallSuccessIdentifier performIntentRecognize(const std::string &modelInput);

    bool tryRuleBasedIntentRecognition(const Json::Value &userInputJson, IntentRecognitionResult &ruleBasedResult,
                                       DialogueData::CallSuccessIdentifier &result);

    DialogueData::CallSuccessIdentifier performModelBasedIntentRecognition(
        Json::Value &userInputJson, const IntentRecognitionResult &ruleBasedResult);

    DialogueCallback::TextEmbeddingResult callTextEmbeddingInterface(const std::string &queryInput) const;

    std::string intentModelName() const;

    std::string intentModelDisplayName() const;

    std::string chatModelName() const;

    bool isChatModelConfigured() const;

    DialogueData::CallSuccessIdentifier callImageHandlerInterface(const std::string &prompt) const;

    DialogueData::CallSuccessIdentifier callImageMattingInterface(const std::string &imagePath) const;

    DialogueData::CallSuccessIdentifier callImageBinarizationInterface(const std::string &imagePath) const;

    DialogueData::CallSuccessIdentifier callKnowledgeBaseInterface(const std::string &modelInput,
                                                                   const std::string &model) const;

    void getIntentionScenariosWithoutSlots();

    void callOutputCallback(DialogueCallback::Type type, const std::string &message) const;
    bool isScheduleToolCall(const Json::Value &jsonMessage) const;

    bool isIntentRecognitionCall(const Json::Value &jsonUserInput) const;

    // 系统应用控制
    DialogueData::CallSuccessIdentifier systemApplicationControlMain(DialogueData::Intent &curTurnIntent,
                                                                     const std::string &userInput, bool slotIdentifier);

    // 多模态输入识别第一轮处理逻辑
    void roundOneMultiModalInputRecognition(const std::string &userInput);

    // 多模态输入识别，用户输入指令类文本的处理逻辑
    void roundTwoMultiModalInputRecognition(const std::string &userInput, const std::string &beforeTurnImage,
                                            const DialogueData::APINameSearchResult &searchResult);

    bool isTextToImageCommand(const std::string &prompt) const;

    Json::Value textInstructionScenario(const std::string &userInput, bool isFirstRound, const std::string &userFile);

    DialogueData::CallSuccessIdentifier multiModalInputRecognition(const DialogueData::Intent &curTurnIntent,
                                                                   const std::string &userInput);

    Json::Value ragRetrieveGeneration(const std::string &responseContent, const std::string &userInput,
                                      bool isFirstRound);

    Json::Value ragProcessingScenario(const std::string &userInput, const std::string &inputFilePath, bool isFirstRound,
                                      bool updateSuccessTag);

    std::string getCompleteSessionFromMemory(bool isFirstRound, const std::string &userInput, bool isEndModel);

    // 意图不明确的情况
    void processAmbiguousIntent(const std::string &curModelIntent);

    void intentRecognitionModelProcessingScenarios(const std::string &userInput, bool isFirstRound);

    void dealChatModelOutput(const DialogueCallback::ChatData &result,
                             const std::set<DialogueData::InformationSource> &ragSources);

    void buildChatModelInput(bool isFirstRound, const std::string &userInput, const std::string &instructName,
                             bool isRagInstruct, bool hasCustomizeKBName);

    std::string buildLongChatParserChatMessage(const std::string &userInput) const;

    DialogueData::TokenCount calculateTokenNumber(const std::string &userInput) const;

    std::pair<std::string, std::string> processSlotValue(const Json::Value &slotsList, const std::string &slotStr,
                                                         const std::string &intentName, const std::string &slotName);

    DialogueData::Intent slotNormalizationProcess(DialogueData::Intent &curIntentSlots,
                                                  const Json::Value &slotsValueJson);

    // 引导类
    void guideCallback(const std::string &fileTypeValue, const std::string &resScript,
                       const std::list<std::string> &listResult, const std::string &modelName) const;

    // 纯文本类
    void textCallback(const std::string &responseStr, const std::string &modelName, bool isError, int errorCode,
                      const std::string &errorMessage, DialogueCallback::Type dataType,
                      const std::string &feedbackUrl) const;

    // 图像指令类
    void imageInstructionCallback(const std::string &responseStr, const std::string &modelName, bool isError,
                                  int errorCode, const std::string &errorMessage, DialogueCallback::Type dataType,
                                  const std::string &fileType, int toolId, const std::string &toolDescription,
                                  const std::string &toolData, const std::string &feedbackUrl) const;

    // 创建向量数据库
    std::future<DialogueData::KnowledgeBaseResult> createFileKnowledgeBase() const;

    void updateFileToKb(bool &updateSuccess);

    void createCustomizeDataBase(const std::string &userFilePath, bool &updateSuccess);

    // 更新已有向量数据库
    DialogueData::FileSearchIdentifier updateFileKnowledgeBase(const std::string &knowledgeBaseName,
                                                               const std::string &docFilePath) const;

    // 删除指定名称的向量数据库
    void deleteCustomizeKnowledgeBase(const std::string &knowledgeBaseName) const;

    void clearDialogueData();

    void loadDialogueData(const std::string &userInput);

    void printHistoryDialogue() const;

    std::string buildIntentSystemPrompt(const Json::Value &intentSlotDefinitions) const;

private:
    // 初始化
    DialogueMemory dialogueMemory_;  // 会话管理

    // 当前轮次数据
    DialogueData::Dialogue currentDialogue_;
    // 当前轮次要存储的对话信息
    DialogueData::DialogueTurn currentDialogInfo_;
    // 会话id
    std::string dialogueId_;
    // 意图识别模型类型
    bool isBaseModelForIntention_{true};
    // 表示上一轮模型输出中存在非空 inquiry
    bool isIntentInquiring_{false};
    // 用于在追问轮构建 system prompt 时填充 __CANDIDATE_INTENTS__
    Json::Value candidateIntents_;
    // 意图prompt
    std::string intentPrompt_;
    // 文档/图像类的功能接口名称，做引导用
    Json::Value functionInterfaceName_;
    // 系统操作、应用控制、多模态等系统控制类场景的意图；不包括闲聊，知识问答
    std::vector<std::string> systemScenarioIntentionsTotal_;

    // 无槽位的意图场景
    std::vector<std::string> intentionScenariosWithoutSlots_;

    // 存储全部意图和槽位字典
    Json::Value intentSlotJsonData_;

    // 缺失槽位话术
    Json::Value missingSlotScript_;

    // 建立原字符串到目标字符串的映射关系
    std::map<std::string, std::string> intentMap_ = {
        {"其他", "闲聊"},
        {"音乐播放", "多媒体-音乐播放"},
        {"视频播放", "多媒体-视频播放"},
        {"播放器控制", "多媒体-播放器控制"},
        {"多媒体-图片创作", "多模态输入识别"},
        {"图片创作", "多模态输入识别"},
        {"应用控制", "第三方应用控制_应用控制"},
        {"意图为其他定义比较宽泛，严格上来说只要不在给定意图清单范围内都属于"
         "其他，其他场景的话用模型兜底回复，闲聊也属于其他，此外模型闲聊的话"
         "也要考虑到安全合规",
         "闲聊"},
        {"全局搜索", "闲聊"}};

    std::vector<std::string> intentKeyWords_ = {"intent_category", "slots", "inquiry"};

    std::vector<std::string> supportedSystemInterfaces_;

    // 模型的输入
    std::string currentModelNameInUse_;
    std::string feedbackUrlForCloudModel_;
    std::string modelInput_;
    // 指令类场景prompt
    std::string instructionPrompt_;
    int promptId_{};
    bool dialogueStop_{false};
    std::string instructionName_;
    bool promptPassed_{false};
    // 指令类模型的输入
    std::string instructModelInput_;
    std::string defaultPromptContent_;

    // rag模型的输入
    std::string ragModelInput_;
    // rag prompt
    std::string ragPrompt_;
    // 存储替换文档内容后的rag prompt
    std::string ragPromptCopy_;
    // 默认向量数据库名称
    std::string defaultKBName_ = "systemKnowledgebase";
    // 自定义向量数据库名称
    std::string customizeKBName_;
    std::set<DialogueData::InformationSource> ragFileSources_;
    bool isUseCustomizeKB_{false};
    bool resetFlagForPromptId_{false};
    int resetForPromptId_{0};
    int maxContextTokens_{2048};

    // 文档路径
    std::string documentFilePath_;
    bool isFirstAfferentFile_{false};

    // 图片路径
    std::string imageFilePath_;

    bool filePassed_{false};
    std::string documentContent_;
    const std::string replacedStringForRag_{"{scenario-definitions}"};

    SystemOperationInterface *systemOperationInterface_{nullptr};
#ifdef SUPPORT_VECTOR_DB
    IntentSlotRecognizer intentSlotRecognizer_;
    FileInferenceInterface *fileInferenceInterface_{nullptr};
    TextEmbeddingInterface *stringEmbeddingInterface_{nullptr};
    KnowledgeBaseInterface *knowledgeBaseInterface_{nullptr};
#endif
    ModelInferenceInterface *modelInferenceInterface_{nullptr};
    ImageHandlerInterface *imageHandlerInterface_{nullptr};

    // 外部设置进来，用来输出的回调函数
    DialogueCallback::OutputCallback outputCallback_{nullptr};

    // 文生图base64内容
    std::string imageData_;
    // chatModel生成的完整回复
    std::string chatGenCompleteText_;
    std::string thinkingChatGenCompleteText_;
    // 用于追加chatModel生成的流式输出
    std::string chatReplyCache_;
    std::string thinkingChatReplyCache_;

#ifdef SUPPORT_VECTOR_DB
    // 文本向量化结果
    std::shared_ptr<SlotsNormalization> slotsNormalization_;
#endif
    Json::Value slotsValueNormalJson_;
    int modelDimension_ = 768;

    std::mutex updateKbFileMutex_;
    std::atomic_bool updateKbFileStop_{false};
    std::atomic_bool webSearch_{true};
    std::atomic_bool enableThinking_{false};

    LongChatImageParser longChatImageParser_;
};

#endif  // SERVICES_ASSISTANT_INTENTION_RECOGNITION_H

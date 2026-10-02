#pragma once

#include <kylin-ai/genai/text/chat.h>

#include <condition_variable>
#include <queue>
#include <set>

#include "common/common.h"

namespace knowledgebase {
typedef struct _ChatResult ChatResult;

class InferenceEngine {
public:
    InferenceEngine();
    ~InferenceEngine();
    InferenceEngine(const InferenceEngine&) = delete;
    InferenceEngine& operator=(const InferenceEngine&) = delete;

public:
    enum class ParseState {
        NORMAL_TEXT,         // 正常输出文本状态
        POSSIBLE_TAG_START,  // 遇到了<字符，可能是一个标签的开始
        INSIDE_SOURCE_TAG    // 在<Source>标签内部
    };
    struct CallbackContext {
        std::string outputBuffer;
        std::string output;
        std::mutex mtx;
        StreamingChatCallback callback;
        std::vector<KnowledgeItem> contents;
        std::condition_variable cv;
        bool isCompleted = false;
        bool isScoreMatching = true;
        bool isRetrievalResultVisible = true;
        // 用来解析<Source>X</Source>之前的值
        std::string pendingOutput;
        ParseState state = ParseState::NORMAL_TEXT;
        std::vector<std::string> chunkIds;
        std::vector<std::string> referenceFileIds;
        std::vector<int> chunkIndexInContents;
        int chunkNumber = 0;
        KBErrorCode callbackErrCode;
        std::string callbackErrMsg;

        void reset();
    };
    struct StreamingOutputBlock {
        std::string result;
        std::vector<std::pair<int, int>> referenceIndices;
    };
    // 异步方法：流式生成回答
    void generateAnswerStreaming(const std::string& query, const RetrievalResult& retrievalResult,
                                 const std::string& chatHistory, const ChatConfig& config,
                                 StreamingChatCallback& callback);
    static void inferenceCallback(ChatResult* result, void* user_data);
    void stopChat();

private:
    // 处理流式输出内容，检查 "<score> 和 </score> 之间的内容是否为 "可参考"，解析有效的输出消息
    static std::vector<StreamingOutputBlock> parseValidOutputMessage(CallbackContext* ctx);
    // 处理流式输出内容，过滤掉 <Source> 标签及其内容
    static std::vector<StreamingOutputBlock> parseSourceTags(CallbackContext* ctx);
    static void sendResponseMessage(CallbackContext* ctx, StreamingChatResponse& response);
    std::string buildGenerateContext(const std::string& userPrompt, const std::string& query,
                                     CallbackContext& contexts);

private:
    CallbackContext callbackContext_;
    GenAiTextSession* session_ = nullptr;
    ChatModelConfig* chatConfig_ = nullptr;
};

}  // namespace knowledgebase

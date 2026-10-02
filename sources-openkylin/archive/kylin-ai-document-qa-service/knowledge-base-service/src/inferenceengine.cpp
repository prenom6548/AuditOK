#include "inferenceengine.h"

#include <kyai-business-framework/log.h>
#include <kylin-ai/genai/text/error.h>

#include <cmath>
#include <nlohmann/json.hpp>
#include <sstream>

#include "common/embedding.h"
#include "common/utils.h"
#include "configmanager.h"

namespace knowledgebase {
std::vector<InferenceEngine::StreamingOutputBlock> InferenceEngine::parseSourceTags(CallbackContext* ctx) {
    const std::string sourceStart = "<Source>";
    const std::string sourceEnd = "</Source>";
    std::vector<StreamingOutputBlock> outputBlocks;
    std::string result;

    for (char c : ctx->output) {
        switch (ctx->state) {
            case ParseState::NORMAL_TEXT:
                if (c == '<') {
                    ctx->state = ParseState::POSSIBLE_TAG_START;
                    ctx->pendingOutput = "<";
                } else {
                    result += c;
                }
                break;

            case ParseState::POSSIBLE_TAG_START:
                ctx->pendingOutput += c;
                if (ctx->pendingOutput.size() >= sourceStart.size()) {
                    // 检查是否匹配<Source>
                    if (ctx->pendingOutput == sourceStart) {
                        ctx->state = ParseState::INSIDE_SOURCE_TAG;
                    } else {
                        result += ctx->pendingOutput;
                        ctx->pendingOutput.clear();
                        ctx->state = ParseState::NORMAL_TEXT;
                    }
                }
                // 检查是否不可能匹配<Source>
                else if (sourceStart.compare(0, ctx->pendingOutput.size(), ctx->pendingOutput) != 0) {
                    result += ctx->pendingOutput;
                    ctx->pendingOutput.clear();
                    ctx->state = ParseState::NORMAL_TEXT;
                }
                break;

            case ParseState::INSIDE_SOURCE_TAG:
                // 将<Source>前的文本存到输出列表中
                if (!result.empty()) {
                    outputBlocks.emplace_back(StreamingOutputBlock{.result = result});
                    result.clear();
                }
                ctx->pendingOutput += c;

                // 检查是否遇到</Source>
                if (ctx->pendingOutput.size() >= sourceEnd.size()) {
                    size_t endPos = ctx->pendingOutput.find(sourceEnd);
                    if (endPos != std::string::npos) {
                        size_t startPos = sourceStart.size();
                        // 提取并输出标签内容
                        std::string content = ctx->pendingOutput.substr(startPos, endPos - startPos);
                        if (!content.empty() && ctx->isRetrievalResultVisible) {
                            try {
                                std::vector<int> referenceChunkIndexs = parseStringToIntVector(content, ',');
                                if (referenceChunkIndexs.empty()) {
                                    PRINT_ERROR("Invalid reference chunk index: %s\n", content.c_str());
                                    break;
                                }

                                StreamingOutputBlock output;
                                size_t chunkNumber = ctx->chunkNumber;
                                for (size_t i = 0; i < referenceChunkIndexs.size(); i++) {
                                    int refChunkIndex = referenceChunkIndexs[i];
                                    if (refChunkIndex > 0 && refChunkIndex <= static_cast<int>(chunkNumber)) {
                                        // 根据参考来源索引获取文件ID
                                        int chunkIndexInContents = ctx->chunkIndexInContents[refChunkIndex - 1];
                                        KnowledgeItem item = ctx->contents[chunkIndexInContents];
                                        std::string fileId;
                                        std::string chunkIndex;
                                        if (item.type_ == KBType::RAG) {
                                            FileChunk* chunk;
                                            chunk = static_cast<FileChunk*>(item.item_.get());
                                            fileId = chunk->fileId_;
                                            chunkIndex = std::to_string(chunk->chunkIndex_);
                                        } else {
                                            FaqItem* faqItem;
                                            faqItem = static_cast<FaqItem*>(item.item_.get());
                                            fileId = "用户手册->FAQ问答";  // 按照chunk->knowledgeBaseId_;
                                            chunkIndex = faqItem->knowledgeId_;
                                        }
                                        std::string chunkId = fileId + "_" + chunkIndex;
                                        // auto iterator = ctx->chunkIds.find(chunkId);
                                        auto iterator = std::find(ctx->chunkIds.begin(), ctx->chunkIds.end(), chunkId);
                                        int refFileNumber = 0;
                                        if (iterator == ctx->chunkIds.end()) {
                                            ctx->chunkIds.push_back(chunkId);
                                            // 先被溯源到的chunk所属的文件排列到前面
                                            ctx->referenceFileIds.push_back(fileId);
                                            refFileNumber = ctx->chunkIds.size();
                                        } else {
                                            refFileNumber = std::distance(ctx->chunkIds.begin(), iterator) + 1;
                                        }

                                        output.referenceIndices.push_back(
                                            std::make_pair(refFileNumber, chunkIndexInContents));
                                    }
                                }
                                outputBlocks.emplace_back(output);
                            } catch (const std::exception& e) {
                                // 如果转换失败，忽略reference file index
                                PRINT_ERROR("Failed to parse reference chunk index: %s\n", content.c_str());
                            }
                        }
                        // 输出剩余内容
                        std::string remaining = ctx->pendingOutput.substr(endPos + sourceEnd.size());
                        if (!remaining.empty()) {
                            outputBlocks.emplace_back(StreamingOutputBlock{.result = remaining});
                        }
                        ctx->pendingOutput.clear();
                        ctx->state = ParseState::NORMAL_TEXT;
                    }
                }
                break;
        }
    }

    if (!result.empty()) {
        outputBlocks.emplace_back(StreamingOutputBlock{.result = result});
    }
    // 大模型发送结束符时可能没有文本输出，需要添加一个空输出
    if (ctx->output.empty()) {
        outputBlocks.emplace_back(StreamingOutputBlock{});
    }
    return outputBlocks;
}

std::vector<InferenceEngine::StreamingOutputBlock> InferenceEngine::parseValidOutputMessage(CallbackContext* ctx) {
    const std::string startTag = "<score>";
    const std::string endTag = "</score>";
    const std::string markTagUseful = "可参考";
    const size_t maxExpectedLength =
        (startTag + endTag + markTagUseful).size() + 10;  // "<score>可参考</score>" 的长度，加10作为冗余

    if (!ctx->isScoreMatching) {
        // 处理流式输出内容，过滤掉 <Source> 标签及其内容
        return parseSourceTags(ctx);
    }

    std::string allOutput = removeWhitespace(ctx->outputBuffer);
    size_t startIdx = allOutput.find(startTag);
    size_t endIdx = allOutput.find(endTag);

    std::vector<StreamingOutputBlock> outputBlocks;
    // 两个标签都存在，且 <score> 在 </score> 前面，才能提取 score 的值
    if (startIdx == std::string::npos || endIdx == std::string::npos || startIdx >= endIdx) {
        if (allOutput.size() >= maxExpectedLength) {
            // 不符合提取条件，但已经超过了最大检测长度，直接将所有结果输出
            PRINT_DEBUG("--allOutput.size(): %zu\n", allOutput.size());
            ctx->isScoreMatching = false;
            outputBlocks.emplace_back(StreamingOutputBlock{.result = ctx->outputBuffer});
        }
        return outputBlocks;
    }

    // 已满足提取条件，提取两者之间的内容
    std::string scoreValue = allOutput.substr(startIdx + startTag.length(), endIdx - (startIdx + startTag.length()));
    // 提取 </score> 标签之后的部分
    std::string validOutputMessage = allOutput.substr(endIdx + endTag.length());

    ctx->isScoreMatching = false;
    if (scoreValue != markTagUseful) {
        ctx->isRetrievalResultVisible = false;
    } else {
        ctx->outputBuffer.clear();
    }

    outputBlocks.emplace_back(StreamingOutputBlock{.result = validOutputMessage});
    return outputBlocks;
}

// 回调函数
void InferenceEngine::inferenceCallback(ChatResult* result, void* user_data) {
    CallbackContext* ctx = static_cast<CallbackContext*>(user_data);
    if (ctx == nullptr) {
        return;
    }
    StreamingChatResponse response;
    KBErrorCode errCode = KBErrorCode::SUCCESS;
    int genErrcode = chat_result_get_error_code(result);
    if (genErrcode != AISDK_NO_ERROR) {
        PRINT_ERROR("error code is :%d\n", genErrcode);
        ctx->callbackErrCode = genaisdkErrorCodeToKBErrorCode(genErrcode);
        ctx->callbackErrMsg = chat_result_get_error_message(result);
        PRINT_ERROR("error message is :%s\n", ctx->callbackErrMsg.c_str());
        std::lock_guard<std::mutex> lock(ctx->mtx);
        ctx->isCompleted = true;
        ctx->callback(response, ctx->callbackErrCode, ctx->callbackErrMsg);
        ctx->cv.notify_all();
        return;
    }
    response.isFinal = chat_result_get_is_end(result);
    if (response.isFinal) {
        PRINT_DEBUG("finish reason message is:%s\n", chat_result_get_finish_reason_message(result));
    }
    response.modelName = chat_result_get_model_name(result);
    const char* msg = chat_result_get_assistant_message(result);
    const char* reasoningMsg = chat_result_get_reasoning_message(result);
    if (msg != nullptr) {
        ctx->outputBuffer += msg;
        ctx->output = msg;
        PRINT_DEBUG("msg:%s\n", msg);
    }
    if (strlen(reasoningMsg) > 0) {
        response.reasoningMsg = reasoningMsg;
        PRINT_DEBUG("reasoningMsg:%s\n", response.reasoningMsg.c_str());
        response.result = "";
        ctx->callback(response, errCode, ctx->callbackErrMsg);
        return;
    }
    PRINT_DEBUG("outputBuffer: %s\n", ctx->outputBuffer.c_str());
    std::vector<StreamingOutputBlock> outputBlocks = parseValidOutputMessage(ctx);
    PRINT_DEBUG("outputBlocks.size: %d\n", outputBlocks.size());
    for (auto& outputBlock : outputBlocks) {
        if (outputBlock.referenceIndices.size() > 0) {
            std::string referenceIds;
            for (int i = 0; i < outputBlock.referenceIndices.size(); i++) {
                referenceIds += std::to_string(outputBlock.referenceIndices.at(i).first);
                referenceIds += " ";
            }
            PRINT_DEBUG("response.referenceId: %s\n", referenceIds.c_str());
        } else {
            PRINT_DEBUG("response.result: %s\n", outputBlock.result.c_str());
        }
    }

    if (ctx->isScoreMatching) {
        PRINT_DEBUG("isScoreMatching~~~~response.isFinal:%d~~~~\n", response.isFinal);
        // 如果在标签匹配时对话结束，不能直接return，需要向前端应用输出isFinal
        if (response.isFinal) {
            std::lock_guard<std::mutex> lock(ctx->mtx);
            ctx->isCompleted = true;
            response.result = ctx->outputBuffer;
            ctx->callback(response, errCode, ctx->callbackErrMsg);
            ctx->cv.notify_all();
        }
        return;
    }

    bool isFinal = response.isFinal;
    for (size_t i = 0; i < outputBlocks.size(); i++) {
        auto& outputBlock = outputBlocks[i];
        response.isFinal = (i == outputBlocks.size() - 1) ? isFinal : false;

        if (!outputBlock.referenceIndices.empty()) {
            response.referenceIndices = outputBlock.referenceIndices;
            response.result.clear();
        } else {
            response.result = outputBlock.result;
            response.referenceIndices.clear();
        }
        sendResponseMessage(ctx, response);
    }
}

void InferenceEngine::sendResponseMessage(CallbackContext* ctx, StreamingChatResponse& response) {
    KBErrorCode errCode = KBErrorCode::SUCCESS;
    if (response.isFinal) {
        if (ctx->isRetrievalResultVisible) {
            response.retrievalData.items = ctx->contents;
            if (ctx->referenceFileIds.empty() == false) {
                // 参考文件 ID 的顺序和回答过程中索引所属文件的顺序一致
                response.referenceFileIds = ctx->referenceFileIds;
            } else {
                std::set<std::string> fileIdSet;
                for (auto& item : ctx->contents) {
                    if (item.type_ == KBType::RAG) {
                        FileChunk* chunk = static_cast<FileChunk*>(item.item_.get());
                        fileIdSet.emplace(chunk->fileId_);
                    }
                }
                response.referenceFileIds.assign(fileIdSet.begin(), fileIdSet.end());
            }
        }
        {
            // 很奇怪，InferenceEngine对象销毁时调用genai_text_destroy_session函数后还会再收到一个end的回调
            std::lock_guard<std::mutex> lock(ctx->mtx);
            if (ctx->isCompleted == false) {
                ctx->callback(response, errCode, ctx->callbackErrMsg);
            }
            ctx->isCompleted = true;
        }
        ctx->cv.notify_all();
    } else {
        ctx->callback(response, errCode, ctx->callbackErrMsg);
    }
}

InferenceEngine::InferenceEngine() {
    session_ = genai_text_create_session();
    if (session_ == nullptr) {
        PRINT_ERROR("创建session失败\n");
        return;
    }

    chatConfig_ = chat_model_config_create();
    if (chatConfig_ == nullptr) {
        PRINT_ERROR("创建chatConfig失败\n");
        return;
    }
}

InferenceEngine::~InferenceEngine() {
    if (session_ != nullptr) {
        genai_text_destroy_session(&session_);
        session_ = nullptr;
    }

    if (chatConfig_ != nullptr) {
        chat_model_config_destroy(&chatConfig_);
        chatConfig_ = nullptr;
    }
}

void InferenceEngine::generateAnswerStreaming(const std::string& query, const RetrievalResult& retrievalResult,
                                              const std::string& chatHistory, const ChatConfig& config,
                                              StreamingChatCallback& callback) {
    assert(session_ != nullptr);
    assert(chatConfig_ != nullptr);

    nlohmann::json chatHistoryJson;
    if (chatHistory.empty() == false) {
        try {
            chatHistoryJson = nlohmann::json::parse(chatHistory);
            PRINT_DEBUG("generateAnswerStreaming chatHistoryJson: %s\n", chatHistoryJson.dump().c_str());
        } catch (const nlohmann::json::exception& e) {
            PRINT_ERROR("Failed to parse chat history: %s\n", e.what());
        }
    }
    KBErrorCode errCode = KBErrorCode::SUCCESS;
    StreamingChatResponse response;
    if (retrievalResult.items.empty()) {
        KBErrorCode errCode = KBErrorCode::RETRIEVAL_NO_RESULTS;
        callback(response, errCode, callbackContext_.callbackErrMsg);
        PRINT_ERROR("检索结果为空，错误码:%d\n", errCode);
        return;
    }

    // 若模型名为空，则系统将根据“设置->AI 模块管理”中该部署类型当前配置的模型进行初始化
    if (config.modelName.empty() == false) {
        chat_model_config_set_name(chatConfig_, config.modelName.c_str());
    }
    chat_model_config_set_top_k(chatConfig_, config.topK);
    chat_model_config_set_top_p(chatConfig_, config.topP);
    chat_model_config_set_temperature(chatConfig_, config.temperature);
    chat_model_config_set_max_completion_tokens(chatConfig_, config.maxTokens);

    genai_text_set_model_config(session_, chatConfig_);
    int errorCode = genai_text_init_session(session_);
    if (errorCode != AISDK_NO_ERROR) {
        errCode = genaisdkErrorCodeToKBErrorCode(errorCode);
        callback(response, errCode, callbackContext_.callbackErrMsg);
        PRINT_ERROR("初始化错误：错误码:%d\n", errorCode);
        return;
    }

    callbackContext_.reset();
    callbackContext_.callback = callback;
    callbackContext_.contents = retrievalResult.items;
    std::string fieldSystemPrompt = ConfigManager::getInstance().getFieldSystemPrompt(retrievalResult.field);
    std::string fieldUserPrompt = ConfigManager::getInstance().getFieldUserPrompt();

    // 设置回调
    genai_text_result_set_callback(session_, inferenceCallback, &callbackContext_);

    // 发起异步请求
    ChatMessage* chatMessage = chat_message_create();
    chat_message_add_system_message(chatMessage, fieldSystemPrompt.c_str());
    PRINT_DEBUG("系统提示词::%s\n", fieldSystemPrompt.c_str());
    if (chatHistoryJson.empty() == false) {
        for (auto item : chatHistoryJson) {
            if (item["role"] == "system") {
                chat_message_add_system_message(chatMessage, item["content"].get<std::string>().c_str());
                PRINT_DEBUG("sys::%s\n", item["content"].get<std::string>().c_str());
            } else if (item["role"] == "user") {
                chat_message_add_user_message(chatMessage, item["content"].get<std::string>().c_str());
                PRINT_DEBUG("user::%s\n", item["content"].get<std::string>().c_str());
            } else if (item["role"] == "assistant") {
                chat_message_add_assistant_message(chatMessage, item["content"].get<std::string>().c_str());
                PRINT_DEBUG("assistant::%s\n", item["content"].get<std::string>().c_str());
            }
        }
    }

    std::string content = buildGenerateContext(fieldUserPrompt, query, callbackContext_);
    PRINT_DEBUG("用户提问: %s\n", content.c_str());
    chat_message_add_user_message(chatMessage, content.c_str());
    PRINT_DEBUG("user::%s\n", content.c_str());
    genai_text_chat_with_history_messages_async(session_, chatMessage);
    PRINT_INFO("开始生成答案...\n");

    // 等待完成
    std::unique_lock<std::mutex> lock(callbackContext_.mtx);
    callbackContext_.cv.wait(lock, [this] { return callbackContext_.isCompleted; });
    PRINT_INFO("生成完成\n");

    chat_message_destroy(&chatMessage);
}

void InferenceEngine::stopChat() {
    assert(session_ != nullptr);
    genai_text_stop_chat(session_);
}

std::string InferenceEngine::buildGenerateContext(const std::string& userPrompt, const std::string& query,
                                                  CallbackContext& contexts) {
    std::string result = userPrompt;

    std::ostringstream contentStream;

    auto item = contexts.contents.begin();
    size_t chunkNumber = 0;

    // FAQ 的内容不可以显示到信息来源中，所以 referenceIndex 都设为 0
    while (item != contexts.contents.end()) {
        if (item->type_ == KBType::RAG) {
            contexts.chunkIndexInContents.push_back(chunkNumber);
            ++chunkNumber;
            FileChunk* chunk = static_cast<FileChunk*>(item->item_.get());
            contentStream << "- referenceIndex " << chunkNumber << ": " << chunk->content_ << "\n";
        } else if (item->type_ == KBType::FAQ) {
            FaqItem* faqItem = static_cast<FaqItem*>(item->item_.get());
            contentStream << "- referenceIndex " << 0 << ": " << faqItem->content_ << "\n";
        }
        ++item;
    }
    contexts.chunkNumber = chunkNumber;

    std::string queryPlaceHolder = "{question}";
    size_t queryPos = userPrompt.find(queryPlaceHolder);
    if (queryPos != std::string::npos) {
        result.replace(queryPos, queryPlaceHolder.length(), query);
    }

    std::string contextPlaceHolder = "{context}";
    size_t contextPos = userPrompt.find(contextPlaceHolder);
    if (contextPos != std::string::npos) {
        result.replace(contextPos, contextPlaceHolder.length(), contentStream.str());
    }
    return result;
}

void InferenceEngine::CallbackContext::reset() {
    callback = nullptr;
    isCompleted = false;
    outputBuffer.clear();
    output.clear();
    isScoreMatching = true;
    isRetrievalResultVisible = true;
    pendingOutput.clear();
    state = ParseState::NORMAL_TEXT;
    chunkIds.clear();
    referenceFileIds.clear();
    chunkIndexInContents.clear();
    chunkNumber = 0;
    callbackErrCode = KBErrorCode::SUCCESS;
    callbackErrMsg = "";
}

}  // namespace knowledgebase

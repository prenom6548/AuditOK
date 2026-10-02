#include <kyai-business-framework/scene.h>

#include <algorithm>

#include "common/utils.h"
#include "configmanager.h"
#include "knowledgebaseservice.h"

namespace knowledgebase {

Response handleInitKnowledgeBaseSession(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;

    int sessionId = knowledgeBaseService.initKnowledgeBaseSession(
        request.getString("appId"), request.getString("password"), request.getString("dbFilePath"), errCode);

    return Response(ERROR_CODE(errCode), message(errCode),
                    nlohmann::json{
                        {"sessionId", sessionId},
                    });
}

Response handleDestroyKnowledgeBaseSession(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    knowledgeBaseService.destroyKnowledgeBaseSession(request.getInt("sessionId"), errCode);

    return Response(ERROR_CODE(errCode), message(errCode));
}

Response handleCreateKnowledgeBase(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    std::string kbType = stringToUpper(request.getString("type"));
    KBType type = (kbType == "FAQ") ? KBType::FAQ : KBType::RAG;
    KnowledgeBaseInfo knowledgeBaseInfo =
        knowledgeBaseService.createKnowledgeBase(request.getInt("sessionId"), request.getString("name"),
                                                 request.getString("description"), type, IndexType::FLAT, errCode);

    return Response(ERROR_CODE(errCode), message(errCode),
                    nlohmann::json{
                        {"knowledgeBaseId", knowledgeBaseInfo.knowledgeBaseId_.c_str()},
                    });
}

Response handleDeleteKnowledgeBase(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    knowledgeBaseService.deleteKnowledgeBase(request.getInt("sessionId"), request.getString("knowledgeBaseId"),
                                             errCode);

    return Response(ERROR_CODE(errCode), message(errCode));
}

Response handleGetKnowledgeBase(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    KnowledgeBaseInfo knowledgeBaseInfo = knowledgeBaseService.getKnowledgeBase(
        request.getInt("sessionId"), request.getString("knowledgeBaseId"), errCode);

    return Response(ERROR_CODE(errCode), message(errCode),
                    nlohmann::json{
                        {"name", knowledgeBaseInfo.name_},
                        {"description", knowledgeBaseInfo.description_},
                        {"knowledgeBaseId", knowledgeBaseInfo.knowledgeBaseId_},
                        {"changeTime", knowledgeBaseInfo.changeTime_},
                        {"indexType", "FLAT"},
                        {"type", knowledgeBaseInfo.type_ == KBType::FAQ ? "FAQ" : "RAG"},
                    });
}

Response handleGetAllKnowledgeBase(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    std::vector<KnowledgeBaseInfo> knowledgeBaseInfos =
        knowledgeBaseService.getAllKnowledgeBase(request.getInt("sessionId"), errCode);

    nlohmann::json jsonArray;
    for (auto& knowledgeBaseInfo : knowledgeBaseInfos) {
        jsonArray.push_back(nlohmann::json{
            {"name", knowledgeBaseInfo.name_},
            {"description", knowledgeBaseInfo.description_},
            {"knowledgeBaseId", knowledgeBaseInfo.knowledgeBaseId_},
            {"changeTime", knowledgeBaseInfo.changeTime_},
            {"indexType", "FLAT"},
            {"type", knowledgeBaseInfo.type_ == KBType::FAQ ? "FAQ" : "RAG"},
        });
    }

    return Response(ERROR_CODE(errCode), message(errCode), jsonArray);
}

Response handleSetKnowledgeBaseName(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    knowledgeBaseService.setKnowledgeBaseName(request.getInt("sessionId"), request.getString("knowledgeBaseId"),
                                              request.getString("name"), errCode);

    return Response(ERROR_CODE(errCode), message(errCode));
}

Response handleSetKnowledgeBaseDescription(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    knowledgeBaseService.setKnowledgeBaseDescription(request.getInt("sessionId"), request.getString("knowledgeBaseId"),
                                                     request.getString("description"), errCode);

    return Response(ERROR_CODE(errCode), message(errCode));
}

void uploadDocuments(const std::string& taskId, const std::string& appId, const std::string& password,
                     const int sessionId, const std::string& knowledgeBaseId,
                     std::vector<std::pair<std::string, std::string> > files) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    auto uploadDocumentCallBack = [&](FileUploadResponse& response, KBErrorCode& errCode) {
        FileUploadStatus status = response.status;
        Response output(ERROR_CODE(errCode), message(errCode),
                        nlohmann::json{{"fileId", response.fileId},
                                       {"progress", response.progress},
                                       {"sessionId", sessionId},
                                       {"status", fileUploadStatusToString(status)}});

        if (status == FileUploadStatus::ALLCOMPLETED) {
            ASYNC_FLOW_RESPONSE_END(taskId, output);
        } else if (status == FileUploadStatus::FAILED && response.fileId == "" &&
                   errCode != KBErrorCode::DOC_FILEID_INVALID) {
            ASYNC_FLOW_RESPONSE_END(taskId, output);
        } else {
            ASYNC_FLOW_RESPONSE_CONTINUE(taskId, output);
        }
    };
    knowledgeBaseService.uploadDocuments(sessionId, knowledgeBaseId, files, uploadDocumentCallBack,
                                         FileProcessorConfig(), errCode);
    if (errCode != KBErrorCode::SUCCESS) {
        Response output(ERROR_CODE(errCode), message(errCode),
                        nlohmann::json{{"fileId", ""}, {"progress", 0}, {"status", "FAILED"}});
        ASYNC_FLOW_RESPONSE_END(taskId, output);
    }
}

Response handleUploadDocuments(const std::string& taskId, const Request& request) {
    KBErrorCode errCode = KBErrorCode::SUCCESS;
    PRINT_INFO("handleUploadDocuments %s\n", request.toString().c_str());
    nlohmann::json input = nlohmann::json::parse(request.toString());
    nlohmann::json files = input["files"];
    std::vector<std::pair<std::string, std::string> > filePairs;
    for (const auto& file : files) {
        std::string fileId = file["fileId"];
        std::string filePath = file["filePath"];
        filePairs.emplace_back(filePath, fileId);
    }

    // 异步处理
    std::thread t(uploadDocuments, taskId, request.getString("appId"), request.getString("password"),
                  request.getInt("sessionId"), request.getString("knowledgeBaseId"), filePairs);
    t.detach();

    return Response(ERROR_CODE(errCode), message(errCode));
}

Response handleDeleteFiles(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    knowledgeBaseService.deleteFiles(request.getInt("sessionId"), request.getString("knowledgeBaseId"),
                                     request.getStringArray("fileIds"), errCode);

    nlohmann::json jsonArray;
    for (auto& fileId : request.getStringArray("fileIds")) {
        jsonArray.push_back(nlohmann::json{
            {"fileId", fileId},
            {"errCode", errCode},
            {"errMsg", message(errCode)},
        });
    }

    return Response(ERROR_CODE(errCode), message(errCode), jsonArray);
}

Response handleGetFileListInKnowledgeBase(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    std::vector<std::string> files = knowledgeBaseService.getFileListInKnowledgeBase(
        request.getInt("sessionId"), request.getString("knowledgeBaseId"), errCode);
    nlohmann::json jsonArray(files);

    return Response(ERROR_CODE(errCode), message(errCode),
                    nlohmann::json{
                        {"fileIds", jsonArray},
                    });
}

Response handleGetFileUploadStatus(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    int progress = 0;
    FileUploadStatus status = FileUploadStatus::PENDING;
    knowledgeBaseService.getFileProcessingProgress(request.getInt("sessionId"), request.getString("knowledgeBaseId"),
                                                   request.getString("fileId"), progress, status, errCode);

    return Response(ERROR_CODE(errCode), message(errCode),
                    nlohmann::json{
                        {"progress", progress},
                        {"status", fileUploadStatusToString(status)},
                    });
}

void retrievalKnowledgeBase(const std::string& taskId, const std::string& appId, const std::string& password,
                            const int sessionId, const std::string& knowledgeBaseId, const std::string& requestId,
                            const std::string& query, const RetrievalConfig& retrievalConfig) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode = KBErrorCode::SUCCESS;
    knowledgeBaseService.retrievalKnowledgeBase(
        sessionId, knowledgeBaseId, requestId, query, retrievalConfig,
        [&](RetrievalResult& response, KBErrorCode& errCode, std::string& errMsg) {
            if (errMsg.empty()) {
                errMsg = message(errCode);
            }
            Response output(ERROR_CODE(errCode), errMsg);
            if (response.items.size() > 0) {
                nlohmann::json chunks, faqs;
                for (const auto& knowledgeItem : response.items) {
                    if (knowledgeItem.type_ == KBType::RAG) {
                        FileChunk* fileChunk = static_cast<FileChunk*>(knowledgeItem.item_.get());
                        nlohmann::json chunk;
                        chunk["knowledgeBaseId"] = fileChunk->knowledgeBaseId_;
                        chunk["fileId"] = fileChunk->fileId_;
                        chunk["chunkIndex"] = fileChunk->chunkIndex_;
                        chunk["content"] = fileChunk->content_;
                        chunk["metaData"] = fileChunk->metaData_;
                        chunk["similarity"] = fileChunk->similarity_;
                        chunks.push_back(chunk);
                    } else if (knowledgeItem.type_ == KBType::FAQ) {
                        FaqItem* faqItem = static_cast<FaqItem*>(knowledgeItem.item_.get());
                        nlohmann::json faq;
                        faq["question"] = faqItem->question_;
                        faq["answer"] = faqItem->content_;
                        faq["similarity"] = faqItem->similarity_;
                    }
                }
                if (chunks.size() > 0) {
                    output.add("chunks", chunks);
                }
                if (faqs.size() > 0) {
                    output.add("items", faqs);
                }
            }

            output.add("requestId", requestId);
            output.add("queryText", query);
            ASYNC_RESPONSE(taskId, output);
        },
        errCode);
}
Response handleRetrievalKnowledgeBase(const std::string& taskId, const Request& request) {
    std::string requestJsonString = request.toString();
    nlohmann::json requestJson = nlohmann::json::parse(requestJsonString);

    RetrievalConfig retrievalConfig = ConfigManager::getInstance().getRetrievalConfig();
    if (requestJson.contains("config")) {
        nlohmann::json config = requestJson["config"];

        // RetrievalConfig 相关设置
        if (config.contains("retrievalMethod")) {
            retrievalConfig.retrievalMethod = stringToRetrievalMethod(config["retrievalMethod"].get<std::string>());
        }
        if (config.contains("rerankMethod")) {
            retrievalConfig.rerankMethod = stringToRerankMethod(config["rerankMethod"].get<std::string>());
        }
        if (config.contains("topK")) {
            retrievalConfig.topK = config["topK"].get<int>();
        }
        if (config.contains("vectorScoreThreshold")) {
            retrievalConfig.vectorScoreThreshold = config["vectorScoreThreshold"].get<float>();
        }
        if (config.contains("bm25ScoreThreshold")) {
            retrievalConfig.bm25ScoreThreshold = config["bm25ScoreThreshold"].get<float>();
        }
        if (config.contains("expandContext")) {
            retrievalConfig.expandContext = config["expandContext"].get<bool>();
        }
        if (config.contains("maxTokens")) {
            retrievalConfig.maxTokens = config["maxTokens"].get<uint32_t>();
        }
        if (config.contains("modelName")) {
            retrievalConfig.queryDiversifyConfig.modelName = config["modelName"].get<std::string>();
        }
        if (config.contains("queryDiversifyEnable")) {
            retrievalConfig.queryDiversifyConfig.enable = config["queryDiversifyEnable"].get<bool>();
        }

        // fileIds 处理
        if (config.contains("fileIds")) {
            std::vector<std::string> fileIds;
            for (auto fileId : config["fileIds"]) {
                fileIds.push_back(fileId.get<std::string>());
            }
            retrievalConfig.fileIds = fileIds;
        }
    }
    // 异步处理
    std::thread t(retrievalKnowledgeBase, taskId, request.getString("appId"), request.getString("password"),
                  request.getInt("sessionId"), request.getString("knowledgeBaseId"), request.getString("requestId"),
                  request.getString("query"), retrievalConfig);
    t.detach();

    KBErrorCode errCode = KBErrorCode::SUCCESS;
    return Response(ERROR_CODE(errCode), message(errCode));
}

void chatWithKnowledgeBase(const std::string& taskId, const std::string& appId, const std::string& password,
                           const int sessionId, const std::string& knowledgeBaseId, const std::string& requestId,
                           const std::string& query, const std::string& chatHistory, const ChatConfig& chatConfig) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode = KBErrorCode::SUCCESS;
    knowledgeBaseService.chatWithKnowledgeBase(
        sessionId, knowledgeBaseId, requestId, query, chatHistory, chatConfig,
        [&](StreamingChatResponse& response, KBErrorCode& errCode, std::string& errMsg) {
            if (errMsg.empty()) {
                errMsg = message(errCode);
            }
            Response output(ERROR_CODE(errCode), errMsg);
            if (response.retrievalData.items.size() > 0) {
                nlohmann::json chunks, faqs;
                for (const auto& knowledgeItem : response.retrievalData.items) {
                    if (knowledgeItem.type_ == KBType::RAG) {
                        FileChunk* fileChunk = static_cast<FileChunk*>(knowledgeItem.item_.get());
                        nlohmann::json chunk;
                        chunk["knowledgeBaseId"] = fileChunk->knowledgeBaseId_;
                        chunk["fileId"] = fileChunk->fileId_;
                        chunk["chunkIndex"] = fileChunk->chunkIndex_;
                        chunk["content"] = fileChunk->content_;
                        chunk["metaData"] = fileChunk->metaData_;
                        chunk["similarity"] = fileChunk->similarity_;
                        chunks.push_back(chunk);
                    } else if (knowledgeItem.type_ == KBType::FAQ) {
                        FaqItem* faqItem = static_cast<FaqItem*>(knowledgeItem.item_.get());
                        nlohmann::json faq;
                        faq["question"] = faqItem->question_;
                        faq["answer"] = faqItem->content_;
                        faq["similarity"] = faqItem->similarity_;
                    }
                }
                if (chunks.size() > 0) {
                    output.add("chunks", chunks);
                }
                if (faqs.size() > 0) {
                    output.add("items", faqs);
                }
            }
            output.add("chatId", response.chatId);
            output.add("chatModel", response.modelName);
            output.add("requestId", requestId);
            output.add("queryText", query);
            output.add("reasoningMsg", response.reasoningMsg);
            output.add("result", response.result);
            output.add("isFinal", response.isFinal);
            output.add("sessionId", sessionId);
            if (response.referenceIndices.size() > 0) {
                nlohmann::json referenceArray;
                for (size_t i = 0; i < response.referenceIndices.size(); i++) {
                    nlohmann::json reference;
                    reference["fileNumber"] = response.referenceIndices[i].first;
                    reference["chunkIndex"] = response.referenceIndices[i].second;
                    referenceArray.push_back(reference);
                }
                output.add("reference", referenceArray);
            }
            if (response.referenceFileIds.empty() == false) {
                output.add("referenceFileIds", response.referenceFileIds);
            }
            PRINT_DEBUG("output:%s\n", output.getData().dump(4).c_str());
            if (response.isFinal) {
                ASYNC_FLOW_RESPONSE_END(taskId, output);
            } else {
                ASYNC_FLOW_RESPONSE_CONTINUE(taskId, output);
            }
        },
        errCode);
}

Response handleChatWithKnowledgeBase(const std::string& taskId, const Request& request) {
    std::string requestJsonString = request.toString();
    nlohmann::json requestJson = nlohmann::json::parse(requestJsonString);

    ChatConfig chatConfig = ConfigManager::getInstance().getChatConfig();
    if (requestJson.contains("config")) {
        nlohmann::json config = requestJson["config"];

        // RetrievalConfig 相关设置
        if (config.contains("retrievalMethod")) {
            chatConfig.retrievalConfig.retrievalMethod =
                stringToRetrievalMethod(config["retrievalMethod"].get<std::string>());
        }
        if (config.contains("rerankMethod")) {
            chatConfig.retrievalConfig.rerankMethod = stringToRerankMethod(config["rerankMethod"].get<std::string>());
        }
        if (config.contains("topK")) {
            chatConfig.retrievalConfig.topK = config["topK"].get<int>();
        }
        if (config.contains("vectorScoreThreshold")) {
            chatConfig.retrievalConfig.vectorScoreThreshold = config["vectorScoreThreshold"].get<float>();
        }
        if (config.contains("bm25ScoreThreshold")) {
            chatConfig.retrievalConfig.bm25ScoreThreshold = config["bm25ScoreThreshold"].get<float>();
        }
        if (config.contains("expandContext")) {
            chatConfig.retrievalConfig.expandContext = config["expandContext"].get<bool>();
        }
        if (config.contains("maxTokens")) {
            chatConfig.retrievalConfig.maxTokens = config["maxTokens"].get<uint32_t>();
        }
        if (config.contains("queryDiversifyEnable")) {
            chatConfig.retrievalConfig.queryDiversifyConfig.enable = config["queryDiversifyEnable"].get<bool>();
        }

        // fileIds 处理
        if (config.contains("fileIds")) {
            std::vector<std::string> fileIds;
            for (auto fileId : config["fileIds"]) {
                fileIds.push_back(fileId.get<std::string>());
            }
            chatConfig.retrievalConfig.fileIds = fileIds;
        }
        if (config.contains("modelName")) {
            chatConfig.modelName = config["modelName"].get<std::string>();
            chatConfig.retrievalConfig.queryDiversifyConfig.modelName = chatConfig.modelName;
            chatConfig.retrievalConfig.chunkEvaluateConfig.modelName = chatConfig.modelName;
        }
        chatConfig.creatNewChat = config.value("creatNewChat", false);
    }

    // 异步处理
    std::thread t(chatWithKnowledgeBase, taskId, request.getString("appId"), request.getString("password"),
                  request.getInt("sessionId"), request.getString("knowledgeBaseId"), request.getString("requestId"),
                  request.getString("query"), request.getValues()["message"].dump(), chatConfig);
    t.detach();

    KBErrorCode errCode = KBErrorCode::SUCCESS;
    return Response(ERROR_CODE(errCode), message(errCode));
}

Response handleStopChat(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    knowledgeBaseService.stopChat(request.getInt("sessionId"), request.getString("requestId"), errCode);

    return Response::success();
}

Response handleStopDocumentsUpload(const Request& request) {
    KnowledgeBaseService& knowledgeBaseService = KnowledgeBaseService::getInstance();
    KBErrorCode errCode;
    knowledgeBaseService.stopDocumentsUpload(request.getInt("sessionId"), request.getString("knowledgeBaseId"),
                                             request.getStringArray("fileIds"), errCode);

    return Response::success();
}

REGISTER_SERVICE_HANDLER("initKnowledgeBaseSession", handleInitKnowledgeBaseSession)
REGISTER_SERVICE_HANDLER("destroyKnowledgeBaseSession", handleDestroyKnowledgeBaseSession)
REGISTER_SERVICE_HANDLER("createKnowledgeBase", handleCreateKnowledgeBase)
REGISTER_SERVICE_HANDLER("deleteKnowledgeBase", handleDeleteKnowledgeBase)
REGISTER_SERVICE_HANDLER("getKnowledgeBase", handleGetKnowledgeBase)
REGISTER_SERVICE_HANDLER("getAllKnowledgeBase", handleGetAllKnowledgeBase)
REGISTER_SERVICE_HANDLER("setKnowledgeBaseName", handleSetKnowledgeBaseName)
REGISTER_SERVICE_HANDLER("setKnowledgeBaseDescription", handleSetKnowledgeBaseDescription)
REGISTER_SERVICE_ASYNC_FLOW_HANDLER("uploadDocuments", handleUploadDocuments)
REGISTER_SERVICE_HANDLER("deleteFiles", handleDeleteFiles)
REGISTER_SERVICE_HANDLER("getFileListInKnowledgeBase", handleGetFileListInKnowledgeBase)
REGISTER_SERVICE_HANDLER("getFileProcessingProgress", handleGetFileUploadStatus)
REGISTER_SERVICE_ASYNC_HANDLER("retrievalKnowledgeBase", handleRetrievalKnowledgeBase)
REGISTER_SERVICE_ASYNC_FLOW_HANDLER("chatWithKnowledgeBase", handleChatWithKnowledgeBase)
REGISTER_SERVICE_HANDLER("stopChat", handleStopChat)
REGISTER_SERVICE_HANDLER("stopUpload", handleStopDocumentsUpload)

}  // namespace knowledgebase

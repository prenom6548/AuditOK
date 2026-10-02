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

#include "slotsnormalization.h"

#include <jsoncpp/json/json.h>

#include <fstream>
#include <string>
#include <typeinfo>

#include "utils/logger.h"
#include "utils/util.h"
#include "variabletypeconversion.h"

const std::string systemSlotsDBPath = "/usr/share/kylin-ai-runtime/knowledge-base/vector.db";

SlotsNormalization::SlotsNormalization() {
    client_ = VectorDB::Database::Create();

    dbConnectFuture_ = std::async(std::launch::async, [this]() {
        VectorDB::ConnectParam param("kylin-ai-runtime");
        auto status = client_->Connect(param);
        if (!checkStatus("Failed to connect milvus server:", status)) {
            LOGE("Failed to connect milvus server.");
        } else {
            LOGI("Connect to milvus server success.");
        }
        status = client_->LoadDBFile(systemSlotsDBPath, true, "k6bf1d_349ecd");
        if (!checkStatus("Failed to load slot normalization data base:", status)) {
            LOGE("Failed to load slot normalization data base.");
        } else {
            LOGI("Load slot normalization data base：{}", systemSlotsDBPath);
        }
    });
}

bool SlotsNormalization::checkStatus(const std::string &prefix, const VectorDB::Status &status) {
    if (!status.IsOk()) {
        LOGI("!status.IsOk(): {} {} {} ", prefix, static_cast<int>(status.Code()), status.Message());
        return false;
    }
    return true;
}

Json::Value SlotsNormalization::search(const std::vector<float> &queryVector, const std::string &queryText,
                                       const std::string &slotName, const std::string &intentName) {
    // 根据向量化结果进行检索，语义搜索
    // 设置返回条数
    VectorDB::SearchArguments arguments(collectionName_, filterNumber_);
    // 查询输出哪些列
    arguments.AddOutputField(DEFAULT_ID_FIELD_NAME);
    arguments.AddOutputField(DYNAMIC_FIELD_NAME);
    arguments.AddOutputField(DEFAULT_VECTOR_FIELD_NAME);
    // 设置过滤条件
    std::string expr = "slot_name == '" + slotName + "' and intent_name == '" + intentName + "'";
    LOGI("expr: {}", expr);

    arguments.SetExpression(expr);

    arguments.SetGuaranteeTimestamp(VectorDB::GuaranteeStrongTs());

    arguments.AddTargetVector(DEFAULT_VECTOR_FIELD_NAME, std::move(queryVector));

    waitForDBConnection();
    VectorDB::SearchResults search_results{};
    auto status = client_->Search(arguments, search_results);
    bool requestSuccess = checkStatus("Failed to search:", status);
    LOGI("\tSearch success. result size: {}", search_results.Results().size());

    Json::Value tempVectorSearch;

    if (requestSuccess) {
        tempVectorSearch["ToBeSearchedText"] = queryText;

        std::vector<Json::Value> similarityResult;
        std::vector<std::string> slotValue;

        for (auto &result : search_results.Results()) {
            auto &ids = result.Ids().IntIDArray();
            auto &distances = result.Scores();
            if (ids.size() != distances.size()) {
                LOGI("\tIllegal result!");
                continue;
            }

            if (ids.empty()) {
                LOGI("\tEmpty result!");
                continue;
            }

            auto idField = result.OutputField(DEFAULT_ID_FIELD_NAME);
            std::shared_ptr<VectorDB::Int64FieldData> idFieldPtr =
                std::static_pointer_cast<VectorDB::Int64FieldData>(idField);
            auto &idData = idFieldPtr->Data();

            auto jsonField = result.OutputField(DYNAMIC_FIELD_NAME);
            std::shared_ptr<VectorDB::JsonFieldData> jsonFieldPtr =
                std::static_pointer_cast<VectorDB::JsonFieldData>(jsonField);
            auto &jsonData = jsonFieldPtr->Data();

            auto vectorField = result.OutputField(DEFAULT_VECTOR_FIELD_NAME);
            std::shared_ptr<VectorDB::FloatVecFieldData> vectorFieldPtr =
                std::static_pointer_cast<VectorDB::FloatVecFieldData>(vectorField);
            auto &vectorData = vectorFieldPtr->Data();

            for (size_t i = 0; i < ids.size(); ++i) {
                LOGI("\tID: {} \tSimilarity: {} \t{}: {} \t{}: {}", ids[i], distances[i], idFieldPtr->Name(), idData[i],
                     jsonFieldPtr->Name(), jsonData[i].dump());

                Json::Value tempJsonData = util::stringToJson(jsonData[i].dump());

                if (distances[i] >= similarityThreshold_) {
                    Json::Value distanceRes;
                    distanceRes["slotValue"] = tempJsonData["slot_value"].asString();
                    distanceRes["slotName"] = tempJsonData["slot_name"].asString();
                    distanceRes["intentName"] = tempJsonData["intent_name"].asString();
                    distanceRes["Distance"] = distances[i];

                    similarityResult.push_back(distanceRes);
                    slotValue.push_back(tempJsonData["slot_value"].asString());
                }
            }
        }

        Json::Value similarityArray(Json::arrayValue);
        for (const auto &value : similarityResult) {
            similarityArray.append(value);
        }
        tempVectorSearch["Top_K_Results"] = similarityArray;

        Json::Value slotValueArray(Json::arrayValue);
        for (const auto &value : slotValue) {
            slotValueArray.append(value);
        }
        tempVectorSearch["Top_K_Label"] = slotValueArray;
    }

    return tempVectorSearch;
}

void SlotsNormalization::waitForDBConnection() {
    if (dbConnectFuture_.valid()) {
        dbConnectFuture_.wait();
    }
}

SlotsNormalization::~SlotsNormalization() {
    if (dbConnectFuture_.valid()) {
        LOGI("SlotsNormalization::~SlotsNormalization: waiting for db connection future");
        dbConnectFuture_.wait();
        LOGI("SlotsNormalization::~SlotsNormalization: db connection future completed");
    }
    if (client_) {
        auto status = client_->Disconnect();
        checkStatus("Failed to Disconnect milvus server:", status);
        LOGI("Successfully disconnected from milvus server.");
    }
}

/*
 * Copyright 2026 KylinSoft Co., Ltd.
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

#pragma once

#include <Database.h>

namespace intent {

struct IntentRecord {
    std::string intent_text;
    std::vector<float> intent_vector;
    std::string intent_type;
    std::string intent_name;
    std::string primary_intent_name;
};

struct IntentResult {
    std::string intent_name;
    float intent_score;
    std::string intent_type;
    std::string recall_text;
};

class VectorDatabase {
public:
    VectorDatabase();
    ~VectorDatabase();

    bool hasCollection();
    bool createCollection();
    bool dropCollection();

    bool insert(std::vector<IntentRecord>& intentRecords);
    std::vector<IntentResult> search(const std::vector<float>& queryEmbeddings) const;

private:
    bool connectVectorDatabase();

private:
    bool isInitialized_ = false;
    const std::string appId_ = "kylin-ai-intent-recognition";
    const std::string intentVectorDBPath_ = "/usr/share/kylin-ai-runtime/intent-recognition/intent.db";
    const std::string collectionName_ = "system_control";
    std::shared_ptr<VectorDB::Database> vectorDatabase_ = nullptr;
    VectorDB::ConnectParam connectParam_;
    std::string vectorDBPath_;
};

}  // namespace intent
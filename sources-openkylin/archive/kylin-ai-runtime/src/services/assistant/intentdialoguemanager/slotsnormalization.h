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

#ifndef SLOTS_NORMALIZATION_H
#define SLOTS_NORMALIZATION_H

#include <jsoncpp/json/json.h>

#include <future>
#include <random>

#include "Database.h"

class SlotsNormalization {
public:
    // 里面调用了 connect
    SlotsNormalization();
    ~SlotsNormalization();

    bool checkStatus(const std::string &prefix, const VectorDB::Status &status);

    // 根据向量查询
    Json::Value search(const std::vector<float> &queryVector, const std::string &queryText, const std::string &slotName,
                       const std::string &intentName);

private:
    void waitForDBConnection();

private:
    std::shared_ptr<VectorDB::Database> client_;
    std::string collectionName_ = "SLOTS";
    int dimension_ = 768;
    int filterNumber_ = 1;
    float similarityThreshold_ = 0.7f;
    std::future<void> dbConnectFuture_;
};

#endif  // SLOTS_NORMALIZATION_H
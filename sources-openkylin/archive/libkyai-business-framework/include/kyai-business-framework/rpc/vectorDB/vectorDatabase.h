/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DISABLE_VECTOR_ENGINE

#pragma once

#include <Database.h>
#include <kysdk-vector-engine-client/types/ConnectParam.h>

#include <shared_mutex>

namespace scene {
class VectorDatabase {
public:
    VectorDatabase(VectorDB::ConnectParam param, std::string dbFile, bool encrypt, std::string key);
    ~VectorDatabase();

    // 快速创建 collection 方式，主键名称：id，DataType::INT64，向量列：vector，DataType::FLOAT_VECTOR
    VectorDB::Status createCollection(const std::string& name, int dimension, bool autoId = true,
                                      bool enableDynamicField = true);

    VectorDB::Status createCollection(const VectorDB::CollectionSchema& schema, const VectorDB::IndexDesc& desc);

    VectorDB::Status dropCollection(const std::string& name);

    VectorDB::Status insert(const std::string& name, const std::vector<VectorDB::FieldDataPtr>& fields,
                            VectorDB::DmlResults& results);

    VectorDB::Status search(const VectorDB::SearchArguments& arguments, VectorDB::SearchResults& results,
                            int timeout = 0);

    VectorDB::Status query(const VectorDB::QueryArguments& arguments, VectorDB::QueryResults& results, int timeout = 0);

    VectorDB::Status upsert(const std::string& name, const std::vector<VectorDB::FieldDataPtr>& fields,
                            VectorDB::DmlResults& results);

    VectorDB::Status deleteData(const std::string& name, const std::string& expression, VectorDB::DmlResults& results);

    VectorDB::Status hasCollection(const std::string& name, bool& exist);

private:
    VectorDB::Status connectAndLoadDB();
    VectorDB::Status getConnection();
    void disconnect();

private:
    std::shared_ptr<VectorDB::Database> client;
    VectorDB::ConnectParam param_;
    std::string dbFile_;
    bool encrypt_;
    std::string key_;

    std::shared_mutex mtx;
    bool isConnected;  // 目前只支持一个连接，后续场景需要多个连接再升级连接池
};

#define CHECK_STATUS(status)          \
    do {                              \
        VectorDB::Status _s = status; \
        if (!_s.IsOk()) {             \
            return _s;                \
        }                             \
    } while (0)

}  // namespace scene

#endif
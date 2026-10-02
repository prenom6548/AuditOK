// Copyright (C) 2019-2024 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

#include "storage.h"
#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Transaction.h>
#include <cstdint>
#include <exception>
#include <memory>
#include <utility>
#include <vector>
#include "collection_data.h"
#include "log/Log.h"
#include "pb/schema.pb.h"
#include "string_util.hpp"

namespace milvus::local {

Storage::Storage(std::string db_file, bool encrypt, std::string key)
    : db_file_(std::move(db_file)), encrypt_(encrypt), key_(std::move(key)) {
}

Storage::~Storage() = default;

bool
Storage::QuickDatabaseCheck() {
    if (!db_ptr_) {
        LOG_ERROR("Database check failed, db_ptr_ is nullptr");
        return false;
    }

    try {
        // 执行完整性检查（PRAGMA）
        SQLite::Statement query(*db_ptr_, "PRAGMA quick_check;");

        while (query.executeStep()) {
            std::string result = query.getColumn(0);
            if (result != "ok") {
                LOG_ERROR("Database quick check failed. err: {}", result);
                return false;
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Database quick check failed. err: {}", e.what());
        return false;
    }

    return true;
}

bool
Storage::Open() {
    try {
        db_ptr_ = std::make_unique<SQLite::Database>(
            db_file_,
            SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE |
                SQLite::OPEN_FULLMUTEX);

        if (encrypt_) {
            std::string sql = string_util::SFormat("PRAGMA key='{}'", key_);
            LOG_INFO("Storage use encrypt db, sql: {}", sql);
            db_ptr_->exec(sql);
        }

        SQLite::Transaction transaction(*db_ptr_.get());
        if (!cm_.Init(db_ptr_.get())) {
            return false;
        }
        transaction.commit();

        std::vector<std::string> names;
        cm_.CollectionNames(&names);
        for (const auto& name : names) {
            collections_.emplace(
                name, std::make_unique<CollectionData>(name.c_str()));
        }
        return true;
    } catch (std::exception& e) {
        LOG_ERROR("Open storage failed, err: {}", e.what());
        return false;
    }
}

bool
Storage::CreateCollection(const std::string& collection_name,
                          const std::string& pk_name,
                          const std::string& schema_proto) {
    SQLite::Transaction transaction(*db_ptr_.get());
    if (!cm_.CreateCollection(
            db_ptr_.get(), collection_name, pk_name, schema_proto)) {
        return false;
    }
    auto data_ptr = std::make_unique<CollectionData>(collection_name.c_str());
    if (!data_ptr->CreateCollection(db_ptr_.get())) {
        return false;
    }
    collections_[collection_name] = std::move(data_ptr);
    transaction.commit();
    return true;
}

bool
Storage::DropCollection(const std::string& collection_name) {
    SQLite::Transaction transaction(*db_ptr_.get());
    if (!cm_.DropCollection(db_ptr_.get(), collection_name)) {
        LOG_ERROR("Delete collection: {}'s meta failed", collection_name);
        return false;
    }
    if (!collections_[collection_name]->DropCollection(db_ptr_.get())) {
        LOG_ERROR("Delete collection: {}'s data failed", collection_name);
        return false;
    }
    collections_.erase(collection_name);
    transaction.commit();
    return true;
}

bool
Storage::LoadCollection(const std::string& collection_name,
                        int64_t start,
                        int64_t size,
                        std::vector<std::string>* out_rows) {
    collections_[collection_name]->Load(db_ptr_.get(), start, size, out_rows);
    return static_cast<int64_t>(out_rows->size()) == size;
}

bool
Storage::GetCollectionSchema(const std::string& collection_name,
                             std::string* output_info_str) {
    output_info_str->assign(cm_.GetCollectionSchema(collection_name).c_str());
    return true;
}

bool
Storage::CreateIndex(const std::string& collection_name,
                     const std::string& index_name,
                     const std::string& index_proto) {
    SQLite::Transaction transaction(*db_ptr_.get());
    if (!cm_.CreateIndex(
            db_ptr_.get(), collection_name, index_name, index_proto)) {
        return false;
    }
    transaction.commit();
    return true;
}

bool
Storage::GetIndex(const std::string& collection_name,
                  const std::string& index_name,
                  std::string* output_index_proto) {
    return cm_.GetCollectionIndex(
        collection_name, index_name, output_index_proto);
}

void
Storage::GetAllIndex(const std::string& collection_name,
                     const std::string& exclude,
                     std::vector<std::string>* index_protos) {
    cm_.GetAllIndex(collection_name, exclude, index_protos);
}

bool
Storage::DropIndex(const std::string& collection_name,
                   const std::string& index_name) {
    SQLite::Transaction transaction(*db_ptr_.get());
    if (!cm_.DropIndex(db_ptr_.get(), collection_name, index_name)) {
        return false;
    }
    transaction.commit();
    return true;
}

int
Storage::Insert(const std::string& collection_name,
                const std::vector<Row>& rows) {
    SQLite::Transaction transaction(*db_ptr_.get());
    for (const auto& row : rows) {
        if (collections_[collection_name]->Insert(db_ptr_.get(),
                                                  std::get<0>(row).c_str(),
                                                  std::get<1>(row)) < 0) {
            return -1;
        }
    }
    transaction.commit();
    return rows.size();
}

int
Storage::Delete(const std::string& collection_name,
                const std::vector<std::string>& ids) {
    SQLite::Transaction transaction(*db_ptr_.get());
    int n = collections_[collection_name]->Delete(db_ptr_.get(), ids);
    transaction.commit();
    return n;
}

int64_t
Storage::Count(const std::string& collection_name) {
    SQLite::Transaction transaction(*db_ptr_.get());
    int64_t n = collections_[collection_name]->Count(db_ptr_.get());
    transaction.commit();
    return n;
}

}  // namespace milvus::local

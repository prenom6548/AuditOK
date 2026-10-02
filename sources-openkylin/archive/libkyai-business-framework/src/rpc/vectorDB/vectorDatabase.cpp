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

#include "rpc/vectorDB/vectorDatabase.h"

#include "log.h"

using namespace scene;

VectorDatabase::VectorDatabase(VectorDB::ConnectParam param, std::string dbFile, bool encrypt, std::string key)
    : isConnected(false),
      param_(std::move(param)),
      dbFile_(std::move(dbFile)),
      encrypt_(encrypt),
      key_(std::move(key)),
      client(VectorDB::Database::Create()) {}

VectorDB::Status VectorDatabase::connectAndLoadDB() {
    mtx.lock();
    if (isConnected) {
        mtx.unlock();
        return VectorDB::Status::OK();
    }

    auto status = client->Connect(param_);
    if (!status.IsOk()) {
        mtx.unlock();
        return status;
    }

    status = client->LoadDBFile(dbFile_, encrypt_, key_);
    if (status.IsOk()) {
        isConnected = true;
    }
    mtx.unlock();
    return status;
}

VectorDB::Status VectorDatabase::getConnection() {
    mtx.lock_shared();
    if (isConnected) {
        mtx.unlock_shared();
        return VectorDB::Status::OK();
    }
    mtx.unlock_shared();

    return connectAndLoadDB();
}

VectorDB::Status VectorDatabase::hasCollection(const std::string& name, bool& exist) {
    CHECK_STATUS(getConnection());
    CHECK_STATUS(client->HasCollection(name, exist));
    return VectorDB::Status::OK();
}

VectorDB::Status VectorDatabase::createCollection(const std::string& name, int dimension, bool autoId,
                                                  bool enableDynamicField) {
    CHECK_STATUS(getConnection());

    bool exist = false;
    CHECK_STATUS(hasCollection(name, exist));
    if (exist) {
        return VectorDB::Status::OK();
    }

    CHECK_STATUS(client->CreateCollection(name, dimension, autoId, enableDynamicField));
    return VectorDB::Status::OK();
}

VectorDB::Status VectorDatabase::createCollection(const VectorDB::CollectionSchema& schema,
                                                  const VectorDB::IndexDesc& desc) {
    CHECK_STATUS(getConnection());

    bool exist = false;
    CHECK_STATUS(hasCollection(schema.Name(), exist));
    if (exist) {
        return VectorDB::Status::OK();
    }

    CHECK_STATUS(client->CreateCollection(schema, desc));
    return VectorDB::Status::OK();
}

VectorDB::Status VectorDatabase::dropCollection(const std::string& name) {
    CHECK_STATUS(getConnection());

    bool exist = false;
    CHECK_STATUS(hasCollection(name, exist));
    if (!exist) {
        return VectorDB::Status::OK();
    }

    CHECK_STATUS(client->DropCollection(name));
    return VectorDB::Status::OK();
}

VectorDB::Status VectorDatabase::insert(const std::string& name, const std::vector<VectorDB::FieldDataPtr>& fields,
                                        VectorDB::DmlResults& results) {
    CHECK_STATUS(getConnection());
    CHECK_STATUS(client->Insert(name, fields, results));
    return VectorDB::Status::OK();
}

VectorDB::Status VectorDatabase::search(const VectorDB::SearchArguments& arguments, VectorDB::SearchResults& results,
                                        int timeout) {
    CHECK_STATUS(getConnection());
    CHECK_STATUS(client->Search(arguments, results, timeout));
    return VectorDB::Status::OK();
}

VectorDB::Status VectorDatabase::query(const VectorDB::QueryArguments& arguments, VectorDB::QueryResults& results,
                                       int timeout) {
    CHECK_STATUS(getConnection());
    CHECK_STATUS(client->Query(arguments, results, timeout));
    return VectorDB::Status::OK();
}

VectorDB::Status VectorDatabase::upsert(const std::string& name, const std::vector<VectorDB::FieldDataPtr>& fields,
                                        VectorDB::DmlResults& results) {
    CHECK_STATUS(getConnection());
    CHECK_STATUS(client->Upsert(name, fields, results));
    return VectorDB::Status::OK();
}

VectorDB::Status VectorDatabase::deleteData(const std::string& name, const std::string& expression,
                                            VectorDB::DmlResults& results) {
    CHECK_STATUS(getConnection());
    CHECK_STATUS(client->Delete(name, expression, results));
    return VectorDB::Status::OK();
}

void VectorDatabase::disconnect() {
    mtx.lock();
    if (!isConnected) {
        mtx.unlock();
        return;
    }

    auto status = client->Disconnect();
    if (status.IsOk()) {
        isConnected = false;
        mtx.unlock();
        return;
    }
    mtx.unlock();
}

VectorDatabase::~VectorDatabase() { disconnect(); }

#endif
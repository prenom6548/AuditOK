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

#include "milvus_proxy.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include "common.h"
#include "common/Types.h"
#include "log/Log.h"
#include "create_collection_task.h"
#include "create_index_task.h"
#include "delete_task.h"
#include "insert_task.h"
#include "milvus_id.hpp"
#include "milvus_local.h"
#include "pb/common.pb.h"
#include "pb/milvus.pb.h"
#include "pb/plan.pb.h"
#include "pb/schema.pb.h"
#include "pb/segcore.pb.h"
#include "query_task.h"
#include "retrieve_result.h"
#include "hybrid_search_task.h"
#include "schema_util.h"
#include "search_result.h"
#include "search_task.h"
#include "status.h"
#include "string_util.hpp"
#include "type.h"
#include "upsert_task.h"
#include "SQLiteCpp/Database.h"

namespace milvus::local {
MilvusProxy::MilvusProxy(const std::string& conf_file) {
    SetConfValueFromFile(conf_file);

    SQLite::Database db(GetDbFilePath(),
                        SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
    std::string sql = "PRAGMA key='kylin-ai-vector-engine'";
    db.exec(sql);

    if (!db.tableExists("t_app_db_path")) {
        sql = R"(CREATE TABLE IF NOT EXISTS t_app_db_path (
                    app_id TEXT NOT NULL DEFAULT '',
                    db_file TEXT NOT NULL DEFAULT '',
                    encrypt INTEGER NOT NULL DEFAULT 0,
                    PRIMARY KEY (db_file)))";
        db.exec(sql);
        return;
    }

    // 判断文件是否还存在
    std::vector<std::string> delete_db_file;
    sql = "SELECT db_file FROM t_app_db_path";
    SQLite::Statement query(db, sql);
    while (query.executeStep()) {
        std::string db_file = query.getColumn(0);
        if (!std::filesystem::exists(db_file)) {
            delete_db_file.push_back(db_file);
        }
    }

    if (!delete_db_file.empty()) {
        for (const auto& item : delete_db_file) {
            sql = "DELETE FROM t_app_db_path WHERE db_file = ?";
            SQLite::Statement statement(db, sql);
            statement.bind(1, item);
            statement.exec();
        }
    }
}

void
MilvusProxy::SetConfValueFromFile(const std::string& file_name) {
    conf_value_ = nlohmann::json();
    std::ifstream ifs(file_name);
    if (!ifs.is_open()) {
        LOG_ERROR("SetConfValueFromFile open file {} failed.", file_name);
        return;
    }

    try {
        conf_value_ = nlohmann::json::parse(ifs);
        ifs.close();
        LOG_INFO(
            "{} parse conf value success. {}", file_name, conf_value_.dump());
    } catch (const std::exception& e) {
        ifs.close();
        LOG_ERROR("{} parse conf value failed. error: {}", file_name, e.what());
    }
}

std::string
MilvusProxy::GetDbFilePath() {
    std::string home_path =
        std::getenv("HOME") ? std::string(std::getenv("HOME")) : "";
    std::string db_path = home_path + "/.local/share/kylin-ai-vector-engine/";

    if (!std::filesystem::exists(db_path)) {
        // 创建目录
        bool ret = std::filesystem::create_directories(db_path);
        if (!ret) {
            LOG_ERROR("Create db path failed.");
            return "";
        }
        LOG_INFO("Create db path success.");
    }
    return db_path + "kylin-ai-vector-engine.db";
}

void
MilvusProxy::SaveAppDBRelation(const std::string& app_id,
                               const std::string& db_file,
                               bool encrypt) {
    SQLite::Database db(GetDbFilePath(), SQLite::OPEN_READWRITE);
    std::string sql = "PRAGMA key='kylin-ai-vector-engine'";
    db.exec(sql);

    sql =
        "REPLACE INTO t_app_db_path(app_id, db_file, encrypt) VALUES(?, ?, ?)";
    SQLite::Statement statement(db, sql);
    statement.bind(1, app_id);
    statement.bind(2, db_file);
    statement.bind(3, (encrypt ? 1 : 0));
    statement.exec();
}

Status
MilvusProxy::GetLocal(const std::string& app_id,
                      std::shared_ptr<MilvusLocal>& local) {
    std::shared_lock<std::shared_mutex> lock(rw_mutex_);
    auto it = milvus_locals_.find(app_id);
    if (it == milvus_locals_.end()) {
        return Status::LocalStorageNotFound();
    }
    local = it->second;
    return Status::Ok();
}

Status
MilvusProxy::LoadDBFile(
    const std::string& app_id,
    const ::milvus::proto::milvus::LoadDBFileRequest* request) {
    // 开启加密，秘钥不能为空
    if (request->encrypt() && request->key().empty()) {
        return Status::ParameterInvalid();
    }

    std::unique_lock<std::shared_mutex> lock(rw_mutex_);
    auto it = milvus_locals_.find(app_id);
    if (it != milvus_locals_.end()) {
        LOG_INFO("LoadDBFile erase exist local.");
        milvus_locals_.erase(it);
    }

    int release_time = conf_value_.value("release_time", 300);
    auto local = std::make_shared<MilvusLocal>(
        request->db_file(), request->encrypt(), request->key(), release_time);
    if (!local->Init()) {
        return Status::LocalStorageInitFailed();
    }
    milvus_locals_.insert(std::make_pair(app_id, local));
    LOG_INFO("LoadDBFile insert new local storage.");

    SaveAppDBRelation(app_id, request->db_file(), request->encrypt());
    return Status::Ok();
}

Status
MilvusProxy::LoadCollection(const std::string& app_id,
                            const std::string& collection_name) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    return local->LoadCollection(collection_name);
}

Status
MilvusProxy::GetLoadState(
    const std::string& app_id,
    const std::string& collection_name,
    ::milvus::proto::milvus::GetLoadStateResponse* response) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    auto s = local->GetLoadState(collection_name);
    if (s.Code() == ErrCollectionNotFound) {
        response->set_state(
            ::milvus::proto::common::LoadState::LoadStateNotExist);
        return Status::Ok();
    }

    if (s.Code() == ErrCollectionNotLoaded) {
        response->set_state(
            ::milvus::proto::common::LoadState::LoadStateNotLoad);
        return Status::Ok();
    }

    if (s.Code() == ErrCollectionLoaded) {
        response->set_state(
            ::milvus::proto::common::LoadState::LoadStateLoaded);
        return Status::Ok();
    }
    return s;
}

Status
MilvusProxy::ReleaseCollection(const std::string& app_id,
                               const std::string& collection_name) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));

    // Alignment error code with milvus
    auto s = local->ReleaseCollection(collection_name);
    if (s.Code() == ErrCollectionNotFound) {
        return Status::ParameterInvalid("collection not found[collection={}]",
                                        collection_name);
    }
    return s;
}

Status
MilvusProxy::HasCollection(const std::string& app_id,
                           const std::string& collection_name,
                           ::milvus::proto::milvus::BoolResponse* response) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));

    std::string tmp;
    Status status = local->GetCollection(collection_name, &tmp);
    response->set_value(status.IsOk());
    // collection 不存在的情况，返回值 Status 映射为 ok，通过 response 判断是否存在
    // 其他类型错误不做转换
    if (status.Code() == ErrCode::ErrCollectionNotFound) {
        LOG_INFO(
            "HasCollection change error code from collection-not-found to ok.");
        status = Status::Ok();
    }
    return status;
}

Status
MilvusProxy::CreateCollection(
    const std::string& app_id,
    const ::milvus::proto::milvus::CreateCollectionRequest* r) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));

    ::milvus::proto::schema::CollectionSchema schema;
    CHECK_STATUS(CreateCollectionTask(r).Process(&schema));

    for (const auto& field : schema.fields()) {
        if (field.is_primary_key()) {
            return local->CreateCollection(
                schema.name(), field.name(), schema.SerializeAsString());
        }
    }
    return Status::FieldNotFound("Lost primary key field");
}

Status
MilvusProxy::CreateIndex(const std::string& app_id,
                         const ::milvus::proto::milvus::CreateIndexRequest* r) {
    ::milvus::proto::schema::CollectionSchema schema;
    CHECK_STATUS(GetSchemaInfo(app_id, r->collection_name(), &schema));

    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    CHECK_STATUS(local->LoadCollection(r->collection_name()));

    // get all index
    milvus::proto::segcore::FieldIndexMeta field_meta;
    CHECK_STATUS(CreateIndexTask(r, &schema).Process(&field_meta));
    return local->CreateIndex(r->collection_name(),
                              field_meta.index_name(),
                              field_meta.SerializeAsString());
}

Status
MilvusProxy::LimitTotalDataCount(const std::shared_ptr<MilvusLocal>& local,
                                 int64_t num) {
    std::vector<std::string> collection_names;
    local->GetAllCollections(&collection_names);

    int64_t total_count = num;
    for (const auto& item : collection_names) {
        int64_t count = 0;
        CHECK_STATUS(local->GetNumRowsOfCollection(item, &count));
        total_count += count;
        LOG_INFO("LimitTotalDataCount collection_name: {}, data_count: {}.",
                 item,
                 count);
    }
    int64_t limit_count = conf_value_.value("max_count", 500000);
    LOG_INFO("LimitTotalDataCount total count: {}, limit: {}.",
             total_count,
             limit_count);

    return (total_count < limit_count) ? Status::Ok()
                                       : Status::LimitTotalDataCount();
}

Status
MilvusProxy::Insert(const std::string& app_id,
                    const ::milvus::proto::milvus::InsertRequest* r,
                    ::milvus::proto::schema::IDs* ids) {
    ::milvus::proto::schema::CollectionSchema schema;
    CHECK_STATUS(GetSchemaInfo(app_id, r->collection_name(), &schema));

    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    CHECK_STATUS(local->LoadCollection(r->collection_name()));
    Rows rows;
    auto insert_task = InsertTask(
        const_cast<::milvus::proto::milvus::InsertRequest*>(r), &schema);
    CHECK_STATUS(insert_task.Process(&rows));
    // 校验总数据行数是否超限
    CHECK_STATUS(LimitTotalDataCount(local, rows.size()));
    std::vector<std::string> insert_ids;
    local->Insert(r->collection_name(), rows, &insert_ids);

    if (insert_task.PkType() == ::milvus::proto::schema::DataType::Int64) {
        for (const auto& id : insert_ids) {
            ids->mutable_int_id()->add_data(std::stoll(id));
        }
    } else {
        for (const auto& id : insert_ids) {
            ids->mutable_str_id()->add_data(id);
        }
    }
    return Status::Ok();
}

Status
MilvusProxy::Upsert(const std::string& app_id,
                    const ::milvus::proto::milvus::UpsertRequest* r,
                    ::milvus::proto::schema::IDs* ids) {
    ::milvus::proto::schema::CollectionSchema schema;
    CHECK_STATUS(GetSchemaInfo(app_id, r->collection_name(), &schema));

    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    CHECK_STATUS(local->LoadCollection(r->collection_name()));
    auto task = UpsertTask(r, &schema);
    ::milvus::proto::milvus::InsertRequest insert_q;
    ::milvus::proto::schema::IDs delete_ids;
    CHECK_STATUS(task.Process(&insert_q, &delete_ids));
    std::vector<std::string> storage_ids;
    if (delete_ids.has_int_id()) {
        for (const auto& id : delete_ids.int_id().data()) {
            storage_ids.push_back(std::to_string(id));
        }
    } else {
        for (const auto& id : delete_ids.str_id().data()) {
            storage_ids.push_back(id);
        }
    }
    CHECK_STATUS(local->DeleteByIds(r->collection_name(),
                                    delete_ids.SerializeAsString(),
                                    storage_ids.size(),
                                    storage_ids));
    return Insert(app_id, &insert_q, ids);
}

Status
MilvusProxy::Search(const std::string& app_id,
                    const ::milvus::proto::milvus::SearchRequest* r,
                    ::milvus::proto::milvus::SearchResults* search_result) {
    ::milvus::proto::schema::CollectionSchema schema;
    CHECK_STATUS(GetSchemaInfo(app_id, r->collection_name(), &schema));

    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    CHECK_STATUS(local->LoadCollection(r->collection_name()));

    // get index
    std::vector<std::string> all_index;
    CHECK_STATUS(local->GetAllIndexs(r->collection_name(), &all_index));
    return DoSearch(app_id, r, schema, all_index, search_result);
}

Status
MilvusProxy::DoSearch(const std::string& app_id,
                      const ::milvus::proto::milvus::SearchRequest* r,
                      const ::milvus::proto::schema::CollectionSchema& schema,
                      const std::vector<std::string>& all_index,
                      ::milvus::proto::milvus::SearchResults* search_result) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));

    std::string placeholder_group;
    ::milvus::proto::plan::PlanNode plan;
    std::vector<int64_t> nqs, topks;

    SearchTask task(const_cast<::milvus::proto::milvus::SearchRequest*>(r),
                    &schema,
                    &all_index);
    CHECK_STATUS(task.Process(&plan, &placeholder_group, &nqs, &topks));
    SearchResult result(nqs, topks);
    CHECK_STATUS(local->Search(r->collection_name(),
                               plan.SerializeAsString(),
                               placeholder_group,
                               &result));
    search_result->set_collection_name(r->collection_name());
    task.PostProcess(result, search_result);
    if (search_result->results().has_ids()) {
        return Status::Ok();
    } else {
        return Status::Ok("search result is empty");
    }
}

Status
MilvusProxy::HybridSearch(
    const std::string& app_id,
    const ::milvus::proto::milvus::HybridSearchRequest* r,
    ::milvus::proto::milvus::SearchResults* search_result) {
    ::milvus::proto::schema::CollectionSchema schema;
    CHECK_STATUS(GetSchemaInfo(app_id, r->collection_name(), &schema));

    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    CHECK_STATUS(local->LoadCollection(r->collection_name()));

    // get index
    std::vector<std::string> all_index;
    CHECK_STATUS(local->GetAllIndexs(r->collection_name(), &all_index));
    HyBridSearchTask task(&schema, &all_index);
    std::vector<::milvus::proto::milvus::SearchRequest> search_requests;
    CHECK_STATUS(task.ProcessSearch(r, &search_requests));

    std::vector<::milvus::proto::milvus::SearchResults> search_results;
    for (const auto& req : search_requests) {
        ::milvus::proto::milvus::SearchResults ret;
        CHECK_STATUS(DoSearch(app_id, &req, schema, all_index, &ret));
        search_results.push_back(ret);
    }
    CHECK_STATUS(task.PostProcessSearch(search_results, search_result));
    if (schema_util::IDsSize(search_result->results().ids()) == 0) {
        return Status::Ok();
    }
    // requery, get output field data
    ::milvus::proto::milvus::QueryRequest query_req;
    task.ProcessQuery(r, search_result, &query_req);
    ::milvus::proto::plan::PlanNode plan;
    QueryTask qtask(&query_req, &schema);
    CHECK_STATUS(qtask.Process(&plan));

    RetrieveResult query_result;
    CHECK_STATUS(local->Retrieve(
        r->collection_name(), plan.SerializeAsString(), &query_result));

    return task.PostProcess(query_result, search_result);
}

Status
MilvusProxy::Query(const std::string& app_id,
                   const ::milvus::proto::milvus::QueryRequest* r,
                   ::milvus::proto::milvus::QueryResults* query_result) {
    ::milvus::proto::schema::CollectionSchema schema;
    CHECK_STATUS(GetSchemaInfo(app_id, r->collection_name(), &schema));

    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    CHECK_STATUS(local->LoadCollection(r->collection_name()));
    ::milvus::proto::plan::PlanNode plan;
    QueryTask task(r, &schema);
    CHECK_STATUS(task.Process(&plan));

    RetrieveResult result;
    CHECK_STATUS(local->Retrieve(
        r->collection_name(), plan.SerializeAsString(), &result));

    query_result->set_collection_name(r->collection_name());
    task.PostProcess(result, query_result);
    return Status::Ok();
}

Status
MilvusProxy::Delete(const std::string& app_id,
                    const ::milvus::proto::milvus::DeleteRequest* r,
                    ::milvus::proto::milvus::MutationResult* response) {
    ::milvus::proto::schema::CollectionSchema schema;
    CHECK_STATUS(GetSchemaInfo(app_id, r->collection_name(), &schema));

    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    CHECK_STATUS(local->LoadCollection(r->collection_name()));

    ::milvus::proto::plan::PlanNode plan;
    CHECK_STATUS(DeleteTask(r, &schema).Process(&plan));

    RetrieveResult result;
    CHECK_STATUS(local->Retrieve(
        r->collection_name(), plan.SerializeAsString(), &result));

    ::milvus::proto::segcore::RetrieveResults seg_result;
    seg_result.ParseFromArray(result.retrieve_result_.proto_blob,
                              result.retrieve_result_.proto_size);
    auto ids_str = seg_result.ids().SerializeAsString();
    std::vector<std::string> storage_ids;
    if (seg_result.ids().has_int_id()) {
        for (const auto& id : seg_result.ids().int_id().data()) {
            storage_ids.push_back(std::to_string(id));
            response->mutable_ids()->mutable_int_id()->add_data(id);
        }
    } else {
        for (const auto& id : seg_result.ids().str_id().data()) {
            storage_ids.push_back(id);
            response->mutable_ids()->mutable_str_id()->add_data(id);
        }
    }
    if (storage_ids.size() != 0) {
        CHECK_STATUS(local->DeleteByIds(
            r->collection_name(), ids_str, storage_ids.size(), storage_ids));
    }
    response->set_delete_cnt(storage_ids.size());
    return Status::Ok();
}

Status
MilvusProxy::GetSchemaInfo(const std::string& app_id,
                           const std::string& collection_name,
                           ::milvus::proto::schema::CollectionSchema* schema) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));

    std::string schema_proto;
    CHECK_STATUS(local->GetCollection(collection_name, &schema_proto));
    if (!schema->ParseFromString(schema_proto)) {
        LOG_ERROR("Failed to parse schema info: {}", schema_proto);
        return Status::ServiceInternal();
    }
    return Status::Ok();
}

Status
MilvusProxy::DescribeCollection(
    const std::string& app_id,
    const ::milvus::proto::milvus::DescribeCollectionRequest* request,
    ::milvus::proto::milvus::DescribeCollectionResponse* response) {
    ::milvus::proto::schema::CollectionSchema schema;
    CHECK_STATUS(GetSchemaInfo(app_id, request->collection_name(), &schema));
    auto mutable_schema = response->mutable_schema();
    mutable_schema->set_name(schema.name());
    mutable_schema->set_description(schema.description());
    mutable_schema->set_enable_dynamic_field(schema.enable_dynamic_field());
    mutable_schema->mutable_properties()->CopyFrom(schema.properties());
    for (const auto& field : schema.fields()) {
        if (field.name() == kTimeStampFieldName ||
            field.name() == kRowIdFieldName || field.is_dynamic())
            continue;
        mutable_schema->add_fields()->CopyFrom(field);
    }
    return Status::Ok();
}

Status
MilvusProxy::GetIndex(
    const std::string& app_id,
    const std::string& collection_name,
    const std::string& index_name,
    ::milvus::proto::milvus::DescribeIndexResponse* response) {
    ::milvus::proto::schema::CollectionSchema schema;
    if (!GetSchemaInfo(app_id, collection_name, &schema).IsOk()) {
        auto err =
            string_util::SFormat("Can not find collection {}", collection_name);
        LOG_ERROR(err);
        return Status::CollectionNotFound(err);
    }

    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    if (index_name.empty()) {
        std::vector<std::string> all_index;
        CHECK_STATUS(local->GetAllIndexs(collection_name, &all_index));
        for (const auto& index : all_index) {
            CHECK_STATUS(
                ParseIndex(index, schema, response->add_index_descriptions()));
        }
        return Status::Ok();
    } else {
        std::string index_proto;
        CHECK_STATUS(
            local->GetIndex(collection_name, index_name, &index_proto));
        return ParseIndex(
            index_proto, schema, response->add_index_descriptions());
    }
}

Status
MilvusProxy::DropIndex(const std::string& app_id,
                       const std::string& collection_name,
                       const std::string& index_name) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    CHECK_STATUS(local->LoadCollection(collection_name));
    return local->DropIndex(collection_name, index_name);
}

Status
MilvusProxy::ParseIndex(const std::string& index_proto,
                        const ::milvus::proto::schema::CollectionSchema& schema,
                        ::milvus::proto::milvus::IndexDescription* index) {
    milvus::proto::segcore::FieldIndexMeta field_index;
    if (!field_index.ParseFromString(index_proto)) {
        return Status::ServiceInternal("Error index info in db");
    }

    auto field_id = field_index.fieldid();
    for (const auto& field : schema.fields()) {
        if (field.fieldid() == field_id) {
            index->set_field_name(field.name());
        }
    }
    index->set_index_name(field_index.index_name());
    index->set_indexid(GetIndexId(field_index.index_name()));
    index->set_state(::milvus::proto::common::IndexState::Finished);
    for (const auto& param : field_index.type_params()) {
        auto new_param = index->add_params();
        new_param->set_key(param.key());
        new_param->set_value(param.value());
    }

    for (const auto& param : field_index.index_params()) {
        auto new_param = index->add_params();
        new_param->set_key(param.key());
        new_param->set_value(param.value());
    }

    for (const auto& param : field_index.user_index_params()) {
        auto new_param = index->add_params();
        new_param->set_key(param.key());
        new_param->set_value(param.value());
    }
    return Status::Ok();
}

Status
MilvusProxy::DropCollection(const std::string& app_id,
                            const std::string& collection_name) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    return local->DropCollection(collection_name);
}

Status
MilvusProxy::ListCollection(const std::string& app_id,
                            std::vector<std::string>* collections) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));
    local->GetAllCollections(collections);
    return Status::Ok();
}

Status
MilvusProxy::GetCollectionStatistics(
    const std::string& app_id,
    const std::string& collection_name,
    ::milvus::proto::milvus::GetCollectionStatisticsResponse* r) {
    std::shared_ptr<MilvusLocal> local;
    CHECK_STATUS(GetLocal(app_id, local));

    int64_t num = -1;
    CHECK_STATUS(local->GetNumRowsOfCollection(collection_name, &num));
    auto s = r->add_stats();
    s->set_key("row_count");
    s->set_value(std::to_string(num));
    return Status::Ok();
}

}  // namespace milvus::local

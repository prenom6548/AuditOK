// Copyright (C) 2019-2020 Zilliz. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software distributed under the License
// is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
// or implied. See the License for the specific language governing permissions and limitations under the License

#include "segcore/segment_c.h"

#include <memory>

#include "common/FieldData.h"
#include "common/LoadInfo.h"
#include "common/Types.h"
#include "common/type_c.h"
#include "google/protobuf/text_format.h"
#include "log/Log.h"
#include "mmap/Types.h"
#include "segcore/Collection.h"
#include "segcore/SegmentGrowingImpl.h"
#include "segcore/Utils.h"

//////////////////////////////    common interfaces    //////////////////////////////
CStatus
NewSegment(CCollection collection,
           SegmentType seg_type,
           int64_t segment_id,
           CSegmentInterface* newSegment) {
    try {
        auto col = static_cast<milvus::segcore::Collection*>(collection);

        std::unique_ptr<milvus::segcore::SegmentInterface> segment;
        switch (seg_type) {
            case Growing: {
                segment = milvus::segcore::CreateGrowingSegment(
                    col->get_schema(), col->get_index_meta(), segment_id);
                break;
            }
            default:
                PanicInfo(milvus::UnexpectedError,
                          "invalid segment type: {}",
                          static_cast<int>(seg_type));
        }

        *newSegment = segment.release();
        return milvus::SuccessCStatus();
    } catch (std::exception& e) {
        return milvus::FailureCStatus(&e);
    }
}

void
DeleteSegment(CSegmentInterface c_segment) {
    auto s = static_cast<milvus::segcore::SegmentInterface*>(c_segment);
    delete s;
}

void
DeleteSearchResult(CSearchResult search_result) {
    auto res = static_cast<milvus::SearchResult*>(search_result);
    delete res;
}

CStatus
Search(CSegmentInterface c_segment,
       CSearchPlan c_plan,
       CPlaceholderGroup c_placeholder_group,
       uint64_t timestamp,
       CSearchResult* result) {
    try {
        auto segment = (milvus::segcore::SegmentInterface*)c_segment;
        auto plan = (milvus::query::Plan*)c_plan;
        auto phg_ptr = reinterpret_cast<const milvus::query::PlaceholderGroup*>(
            c_placeholder_group);

        auto search_result = segment->Search(plan, phg_ptr, timestamp);
        if (!milvus::PositivelyRelated(
                plan->plan_node_->search_info_.metric_type_)) {
            for (auto& dis : search_result->distances_) {
                dis *= -1;
            }
        }
        *result = search_result.release();
        return milvus::SuccessCStatus();
    } catch (std::exception& e) {
        return milvus::FailureCStatus(&e);
    }
}

void
DeleteRetrieveResult(CRetrieveResult* retrieve_result) {
    std::free(const_cast<void*>(retrieve_result->proto_blob));
}

CStatus
Retrieve(CSegmentInterface c_segment,
         CRetrievePlan c_plan,
         uint64_t timestamp,
         CRetrieveResult* result,
         int64_t limit_size,
         bool ignore_non_pk) {
    try {
        auto segment =
            static_cast<milvus::segcore::SegmentInterface*>(c_segment);
        auto plan = static_cast<const milvus::query::RetrievePlan*>(c_plan);

        auto retrieve_result =
            segment->Retrieve(plan, timestamp, limit_size, ignore_non_pk);

        auto size = retrieve_result->ByteSizeLong();
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);
        retrieve_result->SerializePartialToArray(buffer.get(), size);

        result->proto_blob = buffer.release();
        result->proto_size = size;

        return milvus::SuccessCStatus();
    } catch (std::exception& e) {
        return milvus::FailureCStatus(&e);
    }
}

CStatus
RetrieveByOffsets(CSegmentInterface c_segment,
                  CRetrievePlan c_plan,
                  CRetrieveResult* result,
                  int64_t* offsets,
                  int64_t len) {
    try {
        auto segment =
            static_cast<milvus::segcore::SegmentInterface*>(c_segment);
        auto plan = static_cast<const milvus::query::RetrievePlan*>(c_plan);

        auto retrieve_result = segment->Retrieve(plan, offsets, len);

        auto size = retrieve_result->ByteSizeLong();
        std::unique_ptr<uint8_t[]> buffer(new uint8_t[size]);
        retrieve_result->SerializePartialToArray(buffer.get(), size);

        result->proto_blob = buffer.release();
        result->proto_size = size;

        return milvus::SuccessCStatus();
    } catch (std::exception& e) {
        return milvus::FailureCStatus(&e);
    }
}

int64_t
GetMemoryUsageInBytes(CSegmentInterface c_segment) {
    auto segment = static_cast<milvus::segcore::SegmentInterface*>(c_segment);
    auto mem_size = segment->GetMemoryUsageInBytes();
    return mem_size;
}

int64_t
GetRowCount(CSegmentInterface c_segment) {
    auto segment = static_cast<milvus::segcore::SegmentInterface*>(c_segment);
    auto row_count = segment->get_row_count();
    return row_count;
}

// TODO: segmentInterface implement get_deleted_count()
int64_t
GetDeletedCount(CSegmentInterface c_segment) {
    auto segment =
        reinterpret_cast<milvus::segcore::SegmentInterface*>(c_segment);
    auto deleted_count = segment->get_deleted_count();
    return deleted_count;
}

int64_t
GetRealCount(CSegmentInterface c_segment) {
    // not accurate, pk may exist in deleted record and not in insert record.
    // return GetRowCount(c_segment) - GetDeletedCount(c_segment);
    auto segment =
        reinterpret_cast<milvus::segcore::SegmentInterface*>(c_segment);
    return segment->get_real_count();
}

bool
HasRawData(CSegmentInterface c_segment, int64_t field_id) {
    auto segment =
        reinterpret_cast<milvus::segcore::SegmentInterface*>(c_segment);
    return segment->HasRawData(field_id);
}

//////////////////////////////    interfaces for growing segment    //////////////////////////////
CStatus
Insert(CSegmentInterface c_segment,
       int64_t reserved_offset,
       int64_t size,
       const int64_t* row_ids,
       const uint64_t* timestamps,
       const uint8_t* data_info,
       const uint64_t data_info_len) {
    try {
        auto segment = static_cast<milvus::segcore::SegmentGrowing*>(c_segment);
        auto insert_record_proto =
            std::make_unique<milvus::InsertRecordProto>();
        auto suc =
            insert_record_proto->ParseFromArray(data_info, data_info_len);
        AssertInfo(suc, "failed to parse insert data from records");

        segment->Insert(reserved_offset,
                        size,
                        row_ids,
                        timestamps,
                        insert_record_proto.get());
        return milvus::SuccessCStatus();
    } catch (std::exception& e) {
        return milvus::FailureCStatus(&e);
    }
}

CStatus
PreInsert(CSegmentInterface c_segment, int64_t size, int64_t* offset) {
    try {
        auto segment = static_cast<milvus::segcore::SegmentGrowing*>(c_segment);
        *offset = segment->PreInsert(size);
        return milvus::SuccessCStatus();
    } catch (std::exception& e) {
        return milvus::FailureCStatus(&e);
    }
}

CStatus
Delete(CSegmentInterface c_segment,
       int64_t reserved_offset,  // deprecated
       int64_t size,
       const uint8_t* ids,
       const uint64_t ids_size,
       const uint64_t* timestamps) {
    auto segment = static_cast<milvus::segcore::SegmentInterface*>(c_segment);
    auto pks = std::make_unique<milvus::proto::schema::IDs>();
    auto suc = pks->ParseFromArray(ids, ids_size);
    AssertInfo(suc, "failed to parse pks from ids");
    try {
        auto res =
            segment->Delete(reserved_offset, size, pks.get(), timestamps);
        return milvus::SuccessCStatus();
    } catch (std::exception& e) {
        return milvus::FailureCStatus(&e);
    }
}

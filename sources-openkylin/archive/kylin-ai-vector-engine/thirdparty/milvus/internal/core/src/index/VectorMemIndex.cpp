// Licensed to the LF AI & Data foundation under one
// or more contributor license agreements. See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership. The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License. You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "index/VectorMemIndex.h"

#include <unistd.h>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "common/Types.h"
#include "common/type_c.h"
#include "fmt/format.h"

#include "index/Index.h"
#include "index/IndexInfo.h"
#include "index/Meta.h"
#include "index/Utils.h"
#include "common/EasyAssert.h"
#include "config/ConfigKnowhere.h"
#include "knowhere/index/index_factory.h"
#include "knowhere/comp/time_recorder.h"
#include "common/BitsetView.h"
#include "common/Consts.h"
#include "common/FieldData.h"
#include "common/File.h"
#include "common/Slice.h"
#include "common/RangeSearchHelper.h"
#include "common/Utils.h"
#include "log/Log.h"
#include "mmap/Types.h"

namespace milvus::index {

template <typename T>
VectorMemIndex<T>::VectorMemIndex(const IndexType& index_type,
                                  const MetricType& metric_type,
                                  const IndexVersion& version)
    : VectorIndex(index_type, metric_type) {
    AssertInfo(!is_unsupported(index_type, metric_type),
               index_type + " doesn't support metric: " + metric_type);

    CheckCompatible(version);
    auto get_index_obj =
        knowhere::IndexFactory::Instance().Create<T>(GetIndexType(), version);
    if (get_index_obj.has_value()) {
        index_ = get_index_obj.value();
    } else {
        auto err = get_index_obj.error();
        if (err == knowhere::Status::invalid_index_error) {
            throw SegcoreError(ErrorCode::Unsupported, get_index_obj.what());
        }
        throw SegcoreError(ErrorCode::KnowhereError, get_index_obj.what());
    }
}

template <typename T>
VectorMemIndex<T>::VectorMemIndex(const CreateIndexInfo& create_index_info)
    : VectorIndex(create_index_info.index_type, create_index_info.metric_type),
      create_index_info_(create_index_info) {
    AssertInfo(!is_unsupported(create_index_info.index_type,
                               create_index_info.metric_type),
               create_index_info.index_type +
                   " doesn't support metric: " + create_index_info.metric_type);
    auto version = create_index_info.index_engine_version;
    CheckCompatible(version);
    auto get_index_obj =
        knowhere::IndexFactory::Instance().Create<T>(GetIndexType(), version);
    if (get_index_obj.has_value()) {
        index_ = get_index_obj.value();
    } else {
        auto err = get_index_obj.error();
        if (err == knowhere::Status::invalid_index_error) {
            throw SegcoreError(ErrorCode::Unsupported, get_index_obj.what());
        }
        throw SegcoreError(ErrorCode::KnowhereError, get_index_obj.what());
    }
}

template <typename T>
knowhere::expected<std::vector<std::shared_ptr<knowhere::IndexNode::iterator>>>
VectorMemIndex<T>::VectorIterators(const milvus::DatasetPtr dataset,
                                   const knowhere::Json& conf,
                                   const milvus::BitsetView& bitset) const {
    return this->index_.AnnIterator(*dataset, conf, bitset);
}

template <typename T>
BinarySet
VectorMemIndex<T>::Serialize(const Config& config) {
    knowhere::BinarySet ret;
    auto stat = index_.Serialize(ret);
    if (stat != knowhere::Status::success)
        PanicInfo(ErrorCode::UnexpectedError,
                  "failed to serialize index: {}",
                  KnowhereStatusString(stat));
    Disassemble(ret);

    return ret;
}

template <typename T>
void
VectorMemIndex<T>::LoadWithoutAssemble(const BinarySet& binary_set,
                                       const Config& config) {
    auto stat = index_.Deserialize(binary_set, config);
    if (stat != knowhere::Status::success)
        PanicInfo(ErrorCode::UnexpectedError,
                  "failed to Deserialize index: {}",
                  KnowhereStatusString(stat));
    SetDim(index_.Dim());
}

template <typename T>
void
VectorMemIndex<T>::Load(const BinarySet& binary_set, const Config& config) {
    milvus::Assemble(const_cast<BinarySet&>(binary_set));
    LoadWithoutAssemble(binary_set, config);
}

template <typename T>
void
VectorMemIndex<T>::BuildWithDataset(const DatasetPtr& dataset,
                                    const Config& config) {
    knowhere::Json index_config;
    index_config.update(config);

    SetDim(dataset->GetDim());

    knowhere::TimeRecorder rc("BuildWithoutIds", 1);
    auto stat = index_.Build(*dataset, index_config);
    if (stat != knowhere::Status::success)
        PanicInfo(ErrorCode::IndexBuildError,
                  "failed to build index, " + KnowhereStatusString(stat));
    rc.ElapseFromBegin("Done");
    SetDim(index_.Dim());
}

template <typename T>
void
VectorMemIndex<T>::AddWithDataset(const DatasetPtr& dataset,
                                  const Config& config) {
    knowhere::Json index_config;
    index_config.update(config);

    knowhere::TimeRecorder rc("AddWithDataset", 1);
    auto stat = index_.Add(*dataset, index_config);
    if (stat != knowhere::Status::success)
        PanicInfo(ErrorCode::IndexBuildError,
                  "failed to append index, " + KnowhereStatusString(stat));
    rc.ElapseFromBegin("Done");
}

template <typename T>
void
VectorMemIndex<T>::Query(const DatasetPtr dataset,
                         const SearchInfo& search_info,
                         const BitsetView& bitset,
                         SearchResult& search_result) const {
    //    AssertInfo(GetMetricType() == search_info.metric_type_,
    //               "Metric type of field index isn't the same with search info");

    auto num_queries = dataset->GetRows();
    knowhere::Json search_conf = PrepareSearchParams(search_info);
    auto topk = search_info.topk_;
    // TODO :: check dim of search data
    auto final = [&] {
        auto index_type = GetIndexType();
        if (CheckKeyInConfig(search_conf, RADIUS)) {
            if (CheckKeyInConfig(search_conf, RANGE_FILTER)) {
                CheckRangeSearchParam(search_conf[RADIUS],
                                      search_conf[RANGE_FILTER],
                                      GetMetricType());
            }
            auto res = index_.RangeSearch(*dataset, search_conf, bitset);
            if (!res.has_value()) {
                PanicInfo(ErrorCode::UnexpectedError,
                          "failed to range search: {}: {}",
                          KnowhereStatusString(res.error()),
                          res.what());
            }
            auto result = ReGenRangeSearchResult(
                res.value(), topk, num_queries, GetMetricType());
            return result;
        } else {
            auto res = index_.Search(*dataset, search_conf, bitset);
            if (!res.has_value()) {
                PanicInfo(ErrorCode::UnexpectedError,
                          "failed to search: {}: {}",
                          KnowhereStatusString(res.error()),
                          res.what());
            }
            return res.value();
        }
    }();

    auto ids = final->GetIds();
    float* distances = const_cast<float*>(final->GetDistance());
    final->SetIsOwner(true);
    auto round_decimal = search_info.round_decimal_;
    auto total_num = num_queries * topk;

    if (round_decimal != -1) {
        const float multiplier = pow(10.0, round_decimal);
        for (int i = 0; i < total_num; i++) {
            distances[i] = std::round(distances[i] * multiplier) / multiplier;
        }
    }
    search_result.seg_offsets_.resize(total_num);
    search_result.distances_.resize(total_num);
    search_result.total_nq_ = num_queries;
    search_result.unity_topK_ = topk;
    std::copy_n(ids, total_num, search_result.seg_offsets_.data());
    std::copy_n(distances, total_num, search_result.distances_.data());
}

template <typename T>
const bool
VectorMemIndex<T>::HasRawData() const {
    return index_.HasRawData(GetMetricType());
}

template <typename T>
std::vector<uint8_t>
VectorMemIndex<T>::GetVector(const DatasetPtr dataset) const {
    auto index_type = GetIndexType();
    if (IndexIsSparse(index_type)) {
        PanicInfo(ErrorCode::UnexpectedError,
                  "failed to get vector, index is sparse");
    }

    auto res = index_.GetVectorByIds(*dataset);
    if (!res.has_value()) {
        PanicInfo(ErrorCode::UnexpectedError,
                  "failed to get vector, " + KnowhereStatusString(res.error()));
    }
    auto tensor = res.value()->GetTensor();
    auto row_num = res.value()->GetRows();
    auto dim = res.value()->GetDim();
    int64_t data_size;
    if (is_in_bin_list(index_type)) {
        data_size = dim / 8 * row_num;
    } else {
        data_size = dim * row_num * sizeof(float);
    }
    std::vector<uint8_t> raw_data;
    raw_data.resize(data_size);
    memcpy(raw_data.data(), tensor, data_size);
    return raw_data;
}

template <typename T>
std::unique_ptr<const knowhere::sparse::SparseRow<float>[]>
VectorMemIndex<T>::GetSparseVector(const DatasetPtr dataset) const {
    auto res = index_.GetVectorByIds(*dataset);
    if (!res.has_value()) {
        PanicInfo(ErrorCode::UnexpectedError,
                  "failed to get vector, " + KnowhereStatusString(res.error()));
    }
    // release and transfer ownership to the result unique ptr.
    res.value()->SetIsOwner(false);
    return std::unique_ptr<const knowhere::sparse::SparseRow<float>[]>(
        static_cast<const knowhere::sparse::SparseRow<float>*>(
            res.value()->GetTensor()));
}

template class VectorMemIndex<float>;
template class VectorMemIndex<uint8_t>;
template class VectorMemIndex<float16>;
template class VectorMemIndex<bfloat16>;

}  // namespace milvus::index

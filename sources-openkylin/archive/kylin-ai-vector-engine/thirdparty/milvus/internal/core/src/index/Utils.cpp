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

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <filesystem>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <functional>
#include <iostream>
#include <unistd.h>
#include <google/protobuf/text_format.h>

#include "common/EasyAssert.h"
#include "common/Exception.h"
#include "common/File.h"
#include "common/FieldData.h"
#include "common/Slice.h"
#include "index/Utils.h"
#include "index/Meta.h"
#include "knowhere/comp/index_param.h"

namespace milvus::index {

size_t
get_file_size(int fd) {
    struct stat s;
    fstat(fd, &s);
    return s.st_size;
}

std::vector<IndexType>
NM_List() {
    static std::vector<IndexType> ret{
        knowhere::IndexEnum::INDEX_FAISS_IVFFLAT,
    };
    return ret;
}

std::vector<IndexType>
BIN_List() {
    static std::vector<IndexType> ret{
        knowhere::IndexEnum::INDEX_FAISS_BIN_IDMAP,
        knowhere::IndexEnum::INDEX_FAISS_BIN_IVFFLAT,
    };
    return ret;
}

std::vector<std::tuple<IndexType, MetricType>>
unsupported_index_combinations() {
    static std::vector<std::tuple<IndexType, MetricType>> ret{
        std::make_tuple(knowhere::IndexEnum::INDEX_FAISS_BIN_IVFFLAT,
                        knowhere::metric::L2),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_INVERTED_INDEX,
                        knowhere::metric::L2),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_INVERTED_INDEX,
                        knowhere::metric::COSINE),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_INVERTED_INDEX,
                        knowhere::metric::HAMMING),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_INVERTED_INDEX,
                        knowhere::metric::JACCARD),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_INVERTED_INDEX,
                        knowhere::metric::SUBSTRUCTURE),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_INVERTED_INDEX,
                        knowhere::metric::SUPERSTRUCTURE),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_WAND,
                        knowhere::metric::L2),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_WAND,
                        knowhere::metric::COSINE),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_WAND,
                        knowhere::metric::HAMMING),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_WAND,
                        knowhere::metric::JACCARD),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_WAND,
                        knowhere::metric::SUBSTRUCTURE),
        std::make_tuple(knowhere::IndexEnum::INDEX_SPARSE_WAND,
                        knowhere::metric::SUPERSTRUCTURE),
    };
    return ret;
}

bool
is_in_bin_list(const IndexType& index_type) {
    return is_in_list<IndexType>(index_type, BIN_List);
}

bool
is_in_nm_list(const IndexType& index_type) {
    return is_in_list<IndexType>(index_type, NM_List);
}

bool
is_unsupported(const IndexType& index_type, const MetricType& metric_type) {
    return is_in_list<std::tuple<IndexType, MetricType>>(
        std::make_tuple(index_type, metric_type),
        unsupported_index_combinations);
}

bool
CheckKeyInConfig(const Config& cfg, const std::string& key) {
    return cfg.contains(key);
}

void
ParseFromString(google::protobuf::Message& params, const std::string& str) {
    auto ok = google::protobuf::TextFormat::ParseFromString(str, &params);
    AssertInfo(ok, "failed to parse params from string");
}

int64_t
GetDimFromConfig(const Config& config) {
    auto dimension = GetValueFromConfig<std::string>(config, "dim");
    AssertInfo(dimension.has_value(), "dimension not exist in config");
    return (std::stoi(dimension.value()));
}

std::string
GetMetricTypeFromConfig(const Config& config) {
    auto metric_type = GetValueFromConfig<std::string>(config, "metric_type");
    AssertInfo(metric_type.has_value(), "metric_type not exist in config");
    return metric_type.value();
}

std::string
GetIndexTypeFromConfig(const Config& config) {
    auto index_type = GetValueFromConfig<std::string>(config, "index_type");
    AssertInfo(index_type.has_value(), "index_type not exist in config");
    return index_type.value();
}

IndexVersion
GetIndexEngineVersionFromConfig(const Config& config) {
    auto index_engine_version =
        GetValueFromConfig<std::string>(config, INDEX_ENGINE_VERSION);
    AssertInfo(index_engine_version.has_value(),
               "index_engine not exist in config");
    return (std::stoi(index_engine_version.value()));
}

Config
ParseConfigFromIndexParams(
    const std::map<std::string, std::string>& index_params) {
    Config config;
    for (auto& p : index_params) {
        config[p.first] = p.second;
    }

    return config;
}

void
ReadDataFromFD(int fd, void* buf, size_t size, size_t chunk_size) {
    lseek(fd, 0, SEEK_SET);
    while (size != 0) {
        const size_t count = (size < chunk_size) ? size : chunk_size;
        const ssize_t size_read = read(fd, buf, count);
        if (size_read != count) {
            throw SegcoreError(
                ErrorCode::UnistdError,
                "read data from fd error, returned read size is " +
                    std::to_string(size_read));
        }

        buf = static_cast<char*>(buf) + size_read;
        size -= static_cast<std::size_t>(size_read);
    }
}

}  // namespace milvus::index

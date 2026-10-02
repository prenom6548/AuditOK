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

#include "common/FieldData.h"

#include "common/Array.h"
#include "common/EasyAssert.h"
#include "common/Exception.h"
#include "common/FieldDataInterface.h"
#include "common/Json.h"
#include "simdjson/padded_string.h"

namespace milvus {

template <typename Type, bool is_type_entire_row>
void
FieldDataImpl<Type, is_type_entire_row>::FillFieldData(const void* source,
                                                       ssize_t element_count) {
    if (element_count == 0) {
        return;
    }

    std::lock_guard lck(tell_mutex_);
    if (length_ + element_count > get_num_rows()) {
        resize_field_data(length_ + element_count);
    }
    std::copy_n(static_cast<const Type*>(source),
                element_count * dim_,
                field_data_.data() + length_ * dim_);
    length_ += element_count;
}

// scalar data
template class FieldDataImpl<bool, true>;
template class FieldDataImpl<unsigned char, false>;
template class FieldDataImpl<int8_t, true>;
template class FieldDataImpl<int16_t, true>;
template class FieldDataImpl<int32_t, true>;
template class FieldDataImpl<int64_t, true>;
template class FieldDataImpl<float, true>;
template class FieldDataImpl<double, true>;
template class FieldDataImpl<std::string, true>;
template class FieldDataImpl<Json, true>;
template class FieldDataImpl<Array, true>;

// vector data
template class FieldDataImpl<int8_t, false>;
template class FieldDataImpl<float, false>;
template class FieldDataImpl<float16, false>;
template class FieldDataImpl<bfloat16, false>;
template class FieldDataImpl<knowhere::sparse::SparseRow<float>, true>;

FieldDataPtr
InitScalarFieldData(const DataType& type, int64_t cap_rows) {
    switch (type) {
        case DataType::BOOL:
            return std::make_shared<FieldData<bool>>(type, cap_rows);
        case DataType::INT8:
            return std::make_shared<FieldData<int8_t>>(type, cap_rows);
        case DataType::INT16:
            return std::make_shared<FieldData<int16_t>>(type, cap_rows);
        case DataType::INT32:
            return std::make_shared<FieldData<int32_t>>(type, cap_rows);
        case DataType::INT64:
            return std::make_shared<FieldData<int64_t>>(type, cap_rows);
        case DataType::FLOAT:
            return std::make_shared<FieldData<float>>(type, cap_rows);
        case DataType::DOUBLE:
            return std::make_shared<FieldData<double>>(type, cap_rows);
        case DataType::STRING:
        case DataType::VARCHAR:
            return std::make_shared<FieldData<std::string>>(type, cap_rows);
        case DataType::JSON:
            return std::make_shared<FieldData<Json>>(type, cap_rows);
        default:
            throw NotSupportedDataTypeException(
                "InitScalarFieldData not support data type " +
                GetDataTypeName(type));
    }
}

}  // namespace milvus

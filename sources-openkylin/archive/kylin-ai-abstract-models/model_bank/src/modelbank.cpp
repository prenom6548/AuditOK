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

#include <filesystem>
#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

#include "triton/backend/backend_common.h"
#include "triton/backend/backend_input_collector.h"
#include "triton/backend/backend_model.h"
#include "triton/backend/backend_model_instance.h"
#include "triton/backend/backend_output_responder.h"
#include "triton/core/tritonbackend.h"
#include "utils.h"

namespace triton {
namespace backend {
namespace modelbank {

// 硬编码模型规则结构
struct HardcodedModelRule {
    std::string repositoryPrefix;  // 仓库目录前缀
    std::string directoryPattern;  // 目录名模式
    std::string triggerCondition;  // 触发条件："missing_yaml" 或 "missing_field"
    std::string fieldName;         // 缺失的字段名（仅当 triggerCondition 为 "missing_field" 时使用）
    nlohmann::json modelData;      // 硬编码的模型数据或字段默认值
};

// 硬编码规则表
static const std::vector<HardcodedModelRule> HARDCODED_RULES = {
    // 规则 1: Qwen-2.5-3b_1.0 在 /usr/share 目录且缺少 yaml 文件时的完整硬编码数据
    {
        "/usr/share/kylin-ai/model-repository",
        "Qwen-2.5-3b_1.0",
        "missing_yaml",
        "",
        {
            {"brand", "qwen"},
            {"capability", "llm"},
            {"debName", "kylin-qwen2.5-3b-gguf-model"},
            {"debVersion", ""},
            {"displayName", "Qwen-2.5-3b_1.0"},
            {"is3rdModel", false},
            {"model", "Qwen-2.5-3b_1.0"},
            {"modelName", "Qwen-2.5-3b_1.0"},
            {"othersInfo", {{"lora", {"GET_LABEL", "INTENT"}}}},
            {"subConfig", nlohmann::json::array()},
            {"supportsThinking", false},
            {"supportsThinkingToggle", false}
        }
    },
    // 规则 2: llm_Qwen-2.5-3b_1.0 在 /opt/appdata 目录且缺少 others_info 字段时的默认值
    {
        "/opt/appdata/kylin-ai/model-repository",
        "llm_Qwen-2.5-3b_1.0",
        "missing_field",
        "others_info",
        {{"lora", {"GET_LABEL", "INTENT"}}}  // othersInfo 字段的默认值
    },
    // 规则 3: 老版llm_Qwen-2.5-3b_1.0 在 /opt/appdata 目录且缺少 brand 字段时的默认值
    {
        "/opt/appdata/kylin-ai/model-repository",
        "llm_Qwen-2.5-3b_1.0",
        "missing_field",
        "brand",
        "qwen"  // 千问固定字段
    }
};

class ModelState : public BackendModel {
public:
    static TRITONSERVER_Error *Create(TRITONBACKEND_Model *triton_model,
                                      ModelState **state);
    virtual ~ModelState() = default;

    std::vector<KymlDevice> devices_;
    std::vector<ModelDeviceBinding> relationships_;

private:
    ModelState(TRITONBACKEND_Model *triton_model)
        : BackendModel(triton_model) {}
};

TRITONSERVER_Error *ModelState::Create(TRITONBACKEND_Model *triton_model,
                                       ModelState **state) {
    try {
        *state = new ModelState(triton_model);
    } catch (const BackendModelException &ex) {
        RETURN_ERROR_IF_TRUE(
            ex.err_ == nullptr, TRITONSERVER_ERROR_INTERNAL,
            std::string("unexpected nullptr in BackendModelException"));
        RETURN_IF_ERROR(ex.err_);
    }

    TRITONBACKEND_ArtifactType artifact_type;
    const char *model_location;
    TRITONSERVER_Error *err = TRITONBACKEND_ModelRepository(
        triton_model, &artifact_type, &model_location);
    if (err != nullptr) {
        std::cerr << "Error getting model repository path: "
                  << TRITONSERVER_ErrorMessage(err) << std::endl;
        return err; // 返回错误
    }

    std::string model_location_str = model_location;
    model_location_str += "/1/";
    if (!std::filesystem::is_regular_file(model_location_str +
                                          "default_model.yaml")) {
        return TRITONSERVER_ErrorNew(
            TRITONSERVER_ERROR_INVALID_ARG,
            ("Invalid model file: " + model_location_str + "default_model.yaml")
                .c_str());
    }

    (*state)->devices_ = getKymlDevicesList();
    (*state)->relationships_ =
        loadModelDeviceBindings(model_location_str + "modeldevicebinding.json");

    return nullptr; // success
}

class ModelInstanceState : public BackendModelInstance {
public:
    static TRITONSERVER_Error *
    Create(ModelState *model_state,
           TRITONBACKEND_ModelInstance *triton_model_instance,
           ModelInstanceState **state);
    virtual ~ModelInstanceState() = default;

    // Get the state of the model that corresponds to this instance.
    ModelState *StateForModel() const { return model_state_; }

private:
    ModelInstanceState(ModelState *model_state,
                       TRITONBACKEND_ModelInstance *triton_model_instance)
        : BackendModelInstance(model_state, triton_model_instance)
        , model_state_(model_state) {}

    ModelState *model_state_;
};

TRITONSERVER_Error *
ModelInstanceState::Create(ModelState *model_state,
                           TRITONBACKEND_ModelInstance *triton_model_instance,
                           ModelInstanceState **state) {
    try {
        *state = new ModelInstanceState(model_state, triton_model_instance);
    } catch (const BackendModelInstanceException &ex) {
        RETURN_ERROR_IF_TRUE(
            ex.err_ == nullptr, TRITONSERVER_ERROR_INTERNAL,
            std::string("unexpected nullptr in BackendModelInstanceException"));
        RETURN_IF_ERROR(ex.err_);
    }

    return nullptr; // success
}

extern "C" {
TRITONSERVER_Error *TRITONBACKEND_ModelInitialize(TRITONBACKEND_Model *model) {
    ModelState *model_state;

    RETURN_IF_ERROR(ModelState::Create(model, &model_state));
    RETURN_IF_ERROR(TRITONBACKEND_ModelSetState(
        model, reinterpret_cast<void *>(model_state)));

    return nullptr; // success
}

TRITONSERVER_Error *TRITONBACKEND_ModelFinalize(TRITONBACKEND_Model *model) {
    void *vstate;
    RETURN_IF_ERROR(TRITONBACKEND_ModelState(model, &vstate));
    ModelState *model_state = reinterpret_cast<ModelState *>(vstate);
    delete model_state;

    return nullptr; // success
}

TRITONSERVER_Error *
TRITONBACKEND_ModelInstanceInitialize(TRITONBACKEND_ModelInstance *instance) {
    // Get the model state associated with this instance's model.
    TRITONBACKEND_Model *model;
    RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceModel(instance, &model));

    void *vmodelstate;
    RETURN_IF_ERROR(TRITONBACKEND_ModelState(model, &vmodelstate));
    ModelState *model_state = reinterpret_cast<ModelState *>(vmodelstate);

    // Create a ModelInstanceState object and associate it with the
    // TRITONBACKEND_ModelInstance.
    ModelInstanceState *instance_state;
    RETURN_IF_ERROR(
        ModelInstanceState::Create(model_state, instance, &instance_state));
    RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceSetState(
        instance, reinterpret_cast<void *>(instance_state)));

    return nullptr; // success
}

TRITONSERVER_Error *
TRITONBACKEND_ModelInstanceFinalize(TRITONBACKEND_ModelInstance *instance) {
    void *vstate;
    RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceState(instance, &vstate));
    ModelInstanceState *instance_state =
        reinterpret_cast<ModelInstanceState *>(vstate);
    delete instance_state;

    return nullptr; // success
}

std::string
GetModelInfoList(const std::vector<std::string> &model_repository_dirs,
                 const std::string &model_capability) {
    // 初始化返回结构
    nlohmann::json vendor_data;
    vendor_data["vendor"] = "";
    vendor_data["models"] = nlohmann::json::array();
    for (const auto &model_repository_dir : model_repository_dirs) {
        printf("model_repository_dir: %s\n", model_repository_dir.c_str());

        std::filesystem::path model_dir_path =
            std::filesystem::path(model_repository_dir);
        if (!std::filesystem::exists(model_dir_path)) {
            continue;
        }

        // 遍历 model_dir_path 下的所有子目录
        for (const auto &entry :
             std::filesystem::directory_iterator(model_dir_path)) {
            if (!entry.is_directory())
                continue;

            std::string model_dir = entry.path().filename();
            std::filesystem::path yaml_path =
                entry.path() / "model_description.yaml";

            // 检查 model_description.yaml 是否存在
            if (!std::filesystem::exists(yaml_path)) {
                // 场景 1：检查是否有硬编码规则匹配（missing_yaml 情况）
                for (const auto &rule : HARDCODED_RULES) {
                    if (rule.repositoryPrefix == model_repository_dir &&
                        rule.directoryPattern == model_dir &&
                        rule.triggerCondition == "missing_yaml") {
                        // 应用硬编码数据
                        vendor_data["models"].push_back(rule.modelData);
                        break;
                    }
                }
                continue;
            }

            try {
                // 读取并解析 YAML
                YAML::Node data = YAML::LoadFile(yaml_path);

                // 检查是否为合法字典且包含必要字段
                if (!data.IsMap())
                    continue;

                std::vector<std::string> required_keys = {
                    "model_name", "model_capability", "model_is3rdModel"};
                bool has_all_keys = true;
                for (const auto &key : required_keys) {
                    if (!data[key]) {
                        has_all_keys = false;
                        break;
                    }
                }

                if (!has_all_keys)
                    continue;

                // 检查 model_capability 是否匹配
                if (!data["model_capability"]) {
                    continue;
                }

                if (!data["model_internal"] ||
                    data["model_internal"].as<bool>()) {
                    continue;
                }

                bool capability_match = false;
                if (data["model_capability"].IsScalar()) {
                    if (data["model_capability"].as<std::string>() ==
                        model_capability) {
                        capability_match = true;
                    }
                } else if (data["model_capability"].IsSequence()) {
                    for (const auto &cap : data["model_capability"]) {
                        if (cap && cap.as<std::string>() == model_capability) {
                            capability_match = true;
                            break;
                        }
                    }
                }

                if (!capability_match)
                    continue;

                // 提取字段
                std::string model_name =
                    data["model_name"] ? data["model_name"].as<std::string>()
                                       : "";
                std::string display_name =
                    data["model_display_name"]
                        ? data["model_display_name"].as<std::string>()
                        : "";
                std::string deb_name =
                    data["model_deb_name"]
                        ? data["model_deb_name"].as<std::string>()
                        : "";
                bool is_3rd_model = data["model_is3rdModel"].as<bool>();
                // 这两个参数表示模型是否支持思考,模型是否开启思考功能
                bool supports_thinking =
                    data["model_supports_thinking"]
                        ? data["model_supports_thinking"].as<bool>()
                        : false;
                bool supports_thinking_toggle =
                    data["model_supports_thinking_toggle"]
                        ? data["model_supports_thinking_toggle"].as<bool>()
                        : false;

                std::string others_info =
                    data["others_info"] ? data["others_info"].as<std::string>()
                                        : "";

                std::string brand =
                    data["brand"] ? data["brand"].as<std::string>() : "";

                // 计算 name_value: 优先 display_name，否则取 model_name
                // 的后半部分
                std::string name_value = display_name;
                if (name_value.empty() && !model_name.empty()) {
                    size_t pos = model_name.find('_');
                    if (pos != std::string::npos) {
                        name_value = model_name.substr(pos + 1);
                    } else {
                        name_value = model_name;
                    }
                }

                // 构造模型信息对象
                nlohmann::json model_entry_json;
                nlohmann::json others_info_json;
                model_entry_json["model"] = model_name;
                model_entry_json["displayName"] = name_value;
                model_entry_json["capability"] = model_capability;
                model_entry_json["subConfig"] =
                    nlohmann::json::array(); // 空数组
                model_entry_json["debName"] = deb_name;
                model_entry_json["debVersion"] = ""; // 留空
                model_entry_json["is3rdModel"] = is_3rd_model;
                model_entry_json["modelName"] = model_name;
                model_entry_json["supportsThinking"] = supports_thinking;
                model_entry_json["supportsThinkingToggle"] =
                    supports_thinking_toggle;
                if (!brand.empty()) {
                    model_entry_json["brand"] = data["brand"].as<std::string>();
                } else {
                    std::cout << "model : " << model_name
                              << " brand is empty" << std::endl;
                    // 场景 2：检查是否有硬编码规则匹配（missing_field 情况）
                    for (const auto &rule : HARDCODED_RULES) {
                        if (rule.repositoryPrefix == model_repository_dir &&
                            rule.directoryPattern == model_dir &&
                            rule.triggerCondition == "missing_field" &&
                            rule.fieldName == "brand") {
                            // 应用硬编码默认值
                            model_entry_json["brand"] = rule.modelData;
                            break;
                        }
                    }
                }
                if (!others_info.empty()) {
                    try {
                        others_info_json = nlohmann::json::parse(others_info);
                        model_entry_json["othersInfo"] = others_info_json;
                    } catch (const std::exception &e) {
                        std::cout << "Can't parsing others_info: " << e.what()
                                  << others_info << std::endl;
                    }
                } else {
                    std::cout << "model : " << model_name
                              << " othersInfo is empty" << std::endl;
                    // 场景 2：检查是否有硬编码规则匹配（missing_field 情况）
                    for (const auto &rule : HARDCODED_RULES) {
                        if (rule.repositoryPrefix == model_repository_dir &&
                            rule.directoryPattern == model_dir &&
                            rule.triggerCondition == "missing_field" &&
                            rule.fieldName == "others_info") {
                            // 应用硬编码默认值
                            model_entry_json["othersInfo"] = rule.modelData;
                            break;
                        }
                    }
                }

                // 按照 displayName 字段进行过滤，字段值相同的跳过 push_back
                bool already_exists = false;
                for (const auto &existing_model : vendor_data["models"]) {
                    if (existing_model.contains("displayName") &&
                        existing_model["displayName"] ==
                            model_entry_json["displayName"]) {
                        already_exists = true;
                        break;
                    }
                }
                printf("model_entry_json: %s\n",
                       model_entry_json.dump().c_str());
                if (!already_exists) {
                    vendor_data["models"].push_back(model_entry_json);
                }

            } catch (const YAML::Exception &e) {
                std::cerr << "YAML parse error in " << yaml_path << ": "
                          << e.what() << std::endl;
                continue;
            } catch (const std::exception &e) {
                std::cerr << "Error processing " << yaml_path << ": "
                          << e.what() << std::endl;
                continue;
            }
        }
    }

    // 转为 JSON 字符串（支持 Unicode）
    return vendor_data.dump(-1, ' ', false,
                            nlohmann::json::error_handler_t::replace);
}

std::vector<char> StringToTritonBytes(const std::string &str,
                                      uint32_t max_length = 100 * 1024 * 1024) {
    uint32_t len = static_cast<uint32_t>(str.size());

    // ✅ 检查长度是否超限
    if (len > max_length) {
        throw std::invalid_argument(
            "String too long: " + std::to_string(len) +
            " bytes (max: " + std::to_string(max_length) + ")");
    }

    // ✅ 构造结果 buffer：4 字节长度头 + 字符串数据
    std::vector<char> buffer;
    buffer.reserve(4 + len); // 预分配空间

    // 写入长度头（小端序，uint32_t）
    buffer.push_back(static_cast<char>(len & 0xFF));
    buffer.push_back(static_cast<char>((len >> 8) & 0xFF));
    buffer.push_back(static_cast<char>((len >> 16) & 0xFF));
    buffer.push_back(static_cast<char>((len >> 24) & 0xFF));

    // 写入字符串数据
    buffer.insert(buffer.end(), str.begin(), str.end());

    return buffer;
}

void ParseLengthPrefixedString(const char *data, std::string &output) {
    // 1. 读取前 4 字节作为长度（小端）
    uint32_t length = *reinterpret_cast<const uint32_t *>(data);

    // 2. 跳过 4 字节，读取字符串内容
    output.assign(data + 4, length);
}

TRITONSERVER_Error *OutputBytes(TRITONBACKEND_Response *response,
                                const std::string &output_name,
                                const std::string &str,
                                const std::vector<int64_t> &shape) {
    TRITONBACKEND_Output *response_output = nullptr;
    // 创建 output，类型是 BYTES
    RETURN_IF_ERROR(TRITONBACKEND_ResponseOutput(
        response, &response_output, output_name.c_str(),
        TRITONSERVER_TYPE_BYTES, shape.data(), shape.size()));

    // 序列化 string → [len][bytes]
    std::vector<char> serialized = StringToTritonBytes(str);

    // 申请 buffer
    void *buffer = nullptr;
    TRITONSERVER_MemoryType actual_memory_type = TRITONSERVER_MEMORY_CPU;
    int64_t actual_memory_type_id = 0;
    RETURN_IF_ERROR(TRITONBACKEND_OutputBuffer(
        response_output, &buffer, serialized.size(), &actual_memory_type,
        &actual_memory_type_id));

    // 拷贝数据到 Triton 分配的 buffer
    memcpy(buffer, serialized.data(), serialized.size());

    return nullptr; // success
}

std::string GetModelInfoFromRepository(ModelState *model_state, std::string key,
                                       std::string engine) {
    std::string results;
    std::string result_json;
    std::vector<std::string> model_repository_dirs;
    model_repository_dirs.push_back("/opt/appdata/kylin-ai/model-repository");
    model_repository_dirs.push_back("/usr/share/kylin-ai/model-repository");
    std::pair<std::string, std::string> cache_key = {key, engine};

    if (key == "model_info_list") {
        result_json = GetModelInfoList(model_repository_dirs, engine);
        printf("result_json-------:%s\n", result_json.c_str());
    } else {
        std::filesystem::path config_file =
            model_state->RepositoryPath() + "/1/default_model.yaml";
        printf("config_file:%s\n", config_file.c_str());
        if (std::filesystem::exists(config_file)) {
            YAML::Node config_data = YAML::LoadFile(config_file);
            if (config_data[key]) {
                result_json = config_data[key].as<std::string>();
            } else {
                result_json = "";
            }
        } else {
            result_json = "";
        }
    }
    if (!result_json.empty()) {
        results = result_json;
    }
    return results;
}

std::string
CheckDeviceSupport(const std::string &json_model_list,
                   const std::vector<KymlDevice> devices_list,
                   const std::vector<ModelDeviceBinding> relationships_list) {
    using json = nlohmann::json;

    // 解析输入 JSON
    auto root = json::parse(json_model_list);

    // 构建 modelName -> 绑定设备列表 的映射，便于快速查找
    std::unordered_map<std::string, std::vector<KymlDevice>> bindingMap;
    for (const auto &binding : relationships_list) {
        bindingMap[binding.modelName] = binding.devices;
    }

    // 遍历 models 数组
    for (auto &modelObj : root["models"]) {
        std::string modelName =
            modelObj["model"]; // 或者用 "modelName"，根据字段一致性

        auto it = bindingMap.find(modelName);
        if (it == bindingMap.end()) {
            // 不在 relationships_list 中 → 默认 available = true
            modelObj["available"] = true;

        } else {
            // 在绑定关系中：检查是否有任一绑定设备存在于 devices_list
            const auto &requiredDevices = it->second;
            bool hasAvailableDevice = false;

            for (const auto &reqDev : requiredDevices) {
                printf("Checking model: %s\n", modelName.c_str());
                printf("  -> Requires device: vendor=%s, chip=%s, type=%d\n",
                       reqDev.vendor.c_str(), reqDev.chipName.c_str(),
                       static_cast<int>(reqDev.type));
                if (std::find(devices_list.begin(), devices_list.end(),
                              reqDev) != devices_list.end()) {
                    hasAvailableDevice = true;
                    break;
                }
            }

            modelObj["available"] = hasAvailableDevice;
        }
    }

    return root.dump(); // 返回修改后的 JSON 字符串
}

TRITONSERVER_Error *
TRITONBACKEND_ModelInstanceExecute(TRITONBACKEND_ModelInstance *instance,
                                   TRITONBACKEND_Request **requests,
                                   const uint32_t request_count) {
    ModelInstanceState *instance_state;
    RETURN_IF_ERROR(TRITONBACKEND_ModelInstanceState(
        instance, reinterpret_cast<void **>(&instance_state)));
    ModelState *model_state = instance_state->StateForModel();

    std::vector<TRITONBACKEND_Response *> responses;
    responses.reserve(request_count);
    for (uint32_t r = 0; r < request_count; ++r) {
        TRITONBACKEND_Request *request = requests[r];
        TRITONBACKEND_Response *response;
        RETURN_IF_ERROR(TRITONBACKEND_ResponseNew(&response, request));
        responses.push_back(response);
    }

    BackendInputCollector collector(
        requests, request_count, &responses, model_state->TritonMemoryManager(),
        false /* pinned_enabled */, nullptr /* stream*/);

    std::vector<std::pair<TRITONSERVER_MemoryType, int64_t>>
        allowed_input_types = {{TRITONSERVER_MEMORY_CPU_PINNED, 0},
                               {TRITONSERVER_MEMORY_CPU, 0}};

    TRITONSERVER_MemoryType input_buffer_memory_type;
    int64_t input_buffer_memory_type_id;

    const bool need_cuda_input_sync = collector.Finalize();
    if (need_cuda_input_sync) {
        LOG_MESSAGE(TRITONSERVER_LOG_ERROR,
                    "'modelbank' backend: unexpected CUDA sync required by "
                    "collector");
    }

    LOG_MESSAGE(TRITONSERVER_LOG_INFO,
                (std::string("model ") + model_state->Name() +
                 ": requests in batch " + std::to_string(request_count))
                    .c_str());

    uint32_t input_count;
    TRITONBACKEND_RequestInputCount(*requests, &input_count);

    TRITONBACKEND_Input *input;
    const char *name;
    TRITONSERVER_DataType datatype;
    const int64_t *shape;
    uint32_t dims_count;
    size_t data_byte_size;
    std::string key;
    std::string engine;
    for (size_t count = 0; count < input_count; count++) {
        const char *data_buffer;
        TRITONBACKEND_RequestInputByIndex(*requests, count /* index */, &input);
        TRITONBACKEND_InputProperties(input, &name, &datatype, &shape,
                                      &dims_count, nullptr, nullptr);
        TRITONBACKEND_InputBuffer(input, 0 /* idx */,
                                  reinterpret_cast<const void **>(&data_buffer),
                                  &data_byte_size, &input_buffer_memory_type,
                                  &input_buffer_memory_type_id);
        std::string data = data_buffer;
        ParseLengthPrefixedString(data_buffer, data);
        if (!strcmp(name, "key")) {
            key = data;
        } else if (!strcmp(name, "engine")) {
            engine = data;
        } else {
            printf("get error input name :%s\n", name);
        }
    }
    std::string results = GetModelInfoFromRepository(model_state, key, engine);
    if (key == "model_info_list") {
#ifdef KYML_ENABLED
        printf("befor CheckDeviceSupport results:%s\n", results.c_str());
        results = CheckDeviceSupport(results, model_state->devices_,
                                     model_state->relationships_);
        printf("after CheckDeviceSupport results:%s\n", results.c_str());
#else
        auto root = nlohmann::json::parse(results);
        for (auto &modelObj : root["models"]) {
            modelObj["available"] = true;
        }
        results = root.dump();
#endif
    }
    std::vector<int64_t> output_batch_shape;
    bool supports_first_dim_batching;
    RESPOND_ALL_AND_SET_NULL_IF_ERROR(
        responses, request_count,
        model_state->SupportsFirstDimBatching(&supports_first_dim_batching));
    if (supports_first_dim_batching) {
        output_batch_shape.push_back(-1);
    }
    output_batch_shape.push_back(results.size());

    BackendOutputResponder responder(
        requests, request_count, &responses, model_state->TritonMemoryManager(),
        supports_first_dim_batching, false /* pinned_enabled */,
        nullptr /* stream*/);
    std::vector<char> bytes = StringToTritonBytes(results);

    OutputBytes(responses[0], "value", results, output_batch_shape);

    const bool need_cuda_output_sync = responder.Finalize();
    if (need_cuda_output_sync) {
        LOG_MESSAGE(TRITONSERVER_LOG_ERROR,
                    "'modelbank' backend: unexpected CUDA sync required by "
                    "responder");
    }

    for (auto &response : responses) {
        if (response != nullptr) {
            LOG_IF_ERROR(
                TRITONBACKEND_ResponseSend(
                    response, TRITONSERVER_RESPONSE_COMPLETE_FINAL, nullptr),
                "failed to send response");
        }
    }

    for (uint32_t r = 0; r < request_count; ++r) {
        auto &request = requests[r];
        LOG_IF_ERROR(TRITONBACKEND_RequestRelease(
                         request, TRITONSERVER_REQUEST_RELEASE_ALL),
                     "failed releasing request");
    }

    return nullptr; // success
}

} // extern "C"

} // namespace modelbank
} // namespace backend
} // namespace triton

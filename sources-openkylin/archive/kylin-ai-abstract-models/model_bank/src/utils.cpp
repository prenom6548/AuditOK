#include "utils.h"

kymlDeviceType_t stringTokymlDeviceType(const std::string &str) {
    std::string strUpper = str;
    std::transform(strUpper.begin(), strUpper.end(), strUpper.begin(),
                   ::toupper);
    if (strUpper == "CPU")
        return kymlDeviceType_t::KYML_DEVICE_TYPE_CPU;
    if (strUpper == "GPU")
        return kymlDeviceType_t::KYML_DEVICE_TYPE_GPU;
    if (strUpper == "NPU")
        return kymlDeviceType_t::KYML_DEVICE_TYPE_NPU;
    if (strUpper == "ACCELERATOR")
        return kymlDeviceType_t::KYML_DEVICE_TYPE_ACCELERATOR;
    if (strUpper == "ALL")
        return kymlDeviceType_t::CL_DEVICE_TYPE_ALL;
    printf("unknow device type:%s, return ALL\n", str.c_str());

    return kymlDeviceType_t::CL_DEVICE_TYPE_ALL;
}

std::vector<ModelDeviceBinding>
loadModelDeviceBindings(const std::string &bindings_file_path) {
    std::vector<ModelDeviceBinding> relationships;
    try {
        // 打开文件
        std::ifstream f(bindings_file_path);
        if (!f.is_open()) {
            std::cerr << "Error: Cannot open file: " << bindings_file_path
                      << "\n";
            return {};
        }

        // 解析 JSON
        nlohmann::json j;
        f >> j;

        // 遍历 bindings 数组
        if (!j.contains("bindings") || !j["bindings"].is_array()) {
            std::cerr << "Error: 'bindings' array not found in JSON.\n";
            return {};
        }

        for (const auto &binding : j["bindings"]) {
            ModelDeviceBinding entry;

            // 解析 model
            if (binding.contains("model_name")) {
                entry.modelName = binding["model_name"];
            } else {
                std::cerr
                    << "Warning: Missing 'model_name' in a binding entry.\n";
                continue;
            }

            // 解析 devices
            if (binding.contains("devices") && binding["devices"].is_array()) {
                for (const auto &dev_json : binding["devices"]) {
                    KymlDevice dev;
                    dev.type =
                        stringTokymlDeviceType(dev_json.value("type", ""));
                    dev.vendor = dev_json.value("vendor", "");
                    dev.chipName = dev_json.value("chip_name", "");
                    entry.devices.push_back(dev);
                }
            }

            relationships.push_back(entry);
        }

        return relationships;

    } catch (const nlohmann::json::parse_error &e) {
        std::cerr << "JSON parse error at byte " << e.byte << ": " << e.what()
                  << "\n";
        return {};
    } catch (const std::exception &e) {
        std::cerr << "Error loading model-device binding: " << e.what() << "\n";
        return {};
    }
}

std::vector<KymlDevice> getKymlDevicesList() {
    std::vector<KymlDevice> devices;
#ifndef KYML_ENABLED
    printf("KYML support is disabled at compile time, no devices enumerated.\n");
    return devices;
#else
    int dev_count = 0;
    kymlReturn_t ret = kymlInitDevs(&dev_count);

    if (ret != KYML_SUCCESS) {
        printf("Failed to initialize KYML devices, error code: %d\n",
               static_cast<int>(ret));
        return devices; // 返回空列表
    }

    printf("kymlInitDevs reported %d device(s)\n", dev_count);

    // 安全限制：避免异常大值（防御性编程）
    if (dev_count <= 0 || dev_count > 16) {
        printf("Warning: dev_count = %d, skipping enumeration.\n", dev_count);
        return devices;
    }

    for (int i = 0; i < dev_count; ++i) {
        KymlDevice dev;
        dev.index = i;

        char buf[64] = {0}; // 每次清零

        // 获取 vendor
        ret = kymlGetDeviceVendor(buf, sizeof(buf), i);
        if (ret != KYML_SUCCESS) {
            printf("Warning: failed to get vendor for device %d, error: %d\n",
                   i, static_cast<int>(ret));
            dev.vendor = "unknown";
        } else {
            dev.vendor = buf;
        }
        printf("Device %d vendor: %s\n", i, dev.vendor.c_str());

        // 获取 chipName (model)
        buf[0] = '\0';
        ret = kymlGetDeviceModel(buf, sizeof(buf), i);
        if (ret != KYML_SUCCESS) {
            printf("Warning: failed to get model for device %d, error: %d\n", i,
                   static_cast<int>(ret));
            dev.chipName = "unknown";
        } else {
            dev.chipName = buf;
        }
        printf("Device %d chipModel: %s\n", i, dev.chipName.c_str());

        // 获取 type
        kymlDeviceType_t type = static_cast<kymlDeviceType_t>(-1);
        ret = kymlGetDeviceType(reinterpret_cast<unsigned int *>(&type), i);
        if (ret != KYML_SUCCESS) {
            printf("Warning: failed to get type for device %d, error: %d\n", i,
                   static_cast<int>(ret));
            dev.type = static_cast<kymlDeviceType_t>(0); // 或默认值
        } else {
            dev.type = type;
        }
        printf("Device %d type: %d\n", i, static_cast<int>(dev.type));

        devices.push_back(std::move(dev));
    }

    return devices;
#endif
}

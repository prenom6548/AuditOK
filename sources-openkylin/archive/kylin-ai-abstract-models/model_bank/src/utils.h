
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#ifdef KYML_ENABLED
extern "C" {
#include <kyml/kyml.h>
}
#else
typedef enum {
    KYML_DEVICE_TYPE_CPU = 0,
    KYML_DEVICE_TYPE_GPU = 1,
    KYML_DEVICE_TYPE_NPU = 2,
    KYML_DEVICE_TYPE_ACCELERATOR = 3,
    CL_DEVICE_TYPE_ALL = 4
} kymlDeviceType_t;

typedef enum {
    KYML_SUCCESS = 0,
    KYML_ERROR_INVALID_PARAMETER = 1,
    KYML_ERROR_OUT_OF_MEMORY = 2,
    KYML_ERROR_DEVICE_NOT_FOUND = 3
} kymlReturn_t;
#endif

struct KymlDevice {
    int index;
    std::string vendor;
    std::string
        chipName; // 芯片型号，原名为model，易与模型名混淆，此处改为chipName
    kymlDeviceType_t type;

    bool operator==(const KymlDevice &other) const {
        return vendor == other.vendor && chipName == other.chipName &&
               type == other.type;
    }
};

struct ModelDeviceBinding {
    std::string modelName;
    std::vector<KymlDevice> devices;
};

kymlDeviceType_t stringTokymlDeviceType(const std::string &str);

std::vector<ModelDeviceBinding>
loadModelDeviceBindings(const std::string &bindings_file_path);

std::vector<KymlDevice> getKymlDevicesList();

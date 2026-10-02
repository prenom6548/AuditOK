#include "protocolversion.h"

#include <gio/gio.h>

#include <array>
#include <atomic>
#include <cctype>
#include <limits>
#include <string>
#include <string_view>

#include "corevisionserviceglue.h"

namespace kyai::core::vision {
namespace {
constexpr ProtocolVersion kDefaultSdkProtocolVersion{1, 0, 0};
constexpr std::string_view kSdkVersionAnnotation = "SdkVersion";

constexpr int encode(const ProtocolVersion &version) {
    return version.major * 10000 + version.minor * 100 + version.patch;
}

ProtocolVersion decode(int value) {
    ProtocolVersion version{};
    if (value <= 0) {
        return version;
    }
    version.major = value / 10000;
    version.minor = (value / 100) % 100;
    version.patch = value % 100;
    return version;
}

ProtocolVersion parseProtocolVersionInternal(std::string_view text) {
    ProtocolVersion parsed{};

    int partIndex = 0;
    int currentValue = 0;
    bool hasDigit = false;
    for (char ch : text) {
        if (std::isdigit(static_cast<unsigned char>(ch))) {
            hasDigit = true;
            currentValue = currentValue * 10 + (ch - '0');
        } else if (ch == '.') {
            if (!hasDigit || partIndex >= 2) {
                return {};  // 无效：连续点、超过两处点、或空段
            }
            if (partIndex == 0)
                parsed.major = currentValue;
            else if (partIndex == 1)
                parsed.minor = currentValue;
            partIndex++;
            currentValue = 0;
            hasDigit = false;
        } else {
            return {};  // 非法字符
        }
    }

    if (!hasDigit) {
        return {};
    }
    if (partIndex == 0) {
        parsed.major = currentValue;
    } else if (partIndex == 1) {
        parsed.minor = currentValue;
    } else {
        parsed.patch = currentValue;
    }
    return parsed;
}

ProtocolVersion readSdkProtocolVersionFromAnnotation() {
    GDBusInterfaceInfo *interfaceInfo = ai_runtime_core_vision_service_interface_info();
    if (interfaceInfo == nullptr || interfaceInfo->annotations == nullptr) {
        return {};
    }

    for (GDBusAnnotationInfo **annotation = interfaceInfo->annotations; annotation && *annotation; ++annotation) {
        if ((*annotation)->key == nullptr || (*annotation)->value == nullptr) {
            continue;
        }
        if (g_strcmp0((*annotation)->key, kSdkVersionAnnotation.data()) == 0) {
            return parseProtocolVersionInternal((*annotation)->value);
        }
    }
    return {};
}

}  // namespace

ProtocolVersion parseProtocolVersion(const char *versionText) {
    if (versionText == nullptr) {
        return {};
    }
    return parseProtocolVersionInternal(versionText);
}

ProtocolVersion getSdkProtocolVersion() {
    static const ProtocolVersion sdkVersion = [] {
        const ProtocolVersion parsed = readSdkProtocolVersionFromAnnotation();
        if (parsed.major == 0 && parsed.minor == 0 && parsed.patch == 0) {
            return kDefaultSdkProtocolVersion;
        }
        return parsed;
    }();
    return sdkVersion;
}

std::atomic<int> runtimeProtocolVersion{encode(ProtocolVersion{1, 0, 0})};

const std::array<ApiProtocolRequirement, 15> kApiProtocolRequirements = {{
    {"init", {ProtocolVersion{1, 0, 0}, kProtocolVersionMax}},
    {"destroy", {ProtocolVersion{1, 0, 0}, kProtocolVersionMax}},
    {"recognize_text_from_image_file", {ProtocolVersion{1, 0, 0}, kProtocolVersionMax}},
    {"recognize_text_from_image_file_with_request_id", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"recognize_text_from_image_data", {ProtocolVersion{1, 0, 0}, kProtocolVersionMax}},
    {"recognize_text_from_image_data_with_request_id", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"init_subject_segmentation", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"destroy_subject_segmentation", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"subject_segmentation_from_image_file_async", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"subject_segmentation_from_image_data_async", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"init_segmentation", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"encode_segmentation_from_base64_image_data", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"encode_segmentation_from_image_file", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"run_segmentation", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
    {"clear_segmentation_cache", {ProtocolVersion{1, 3, 0}, kProtocolVersionMax}},
}};

void setRuntimeProtocolVersion(const ProtocolVersion &version) {
    if (version.major < 0 || version.minor < 0 || version.patch < 0) {
        return;
    }
    runtimeProtocolVersion.store(encode(version), std::memory_order_release);
}

void setRuntimeProtocolVersion(int major, int minor, int patch) {
    setRuntimeProtocolVersion(ProtocolVersion{major, minor, patch});
}

ProtocolVersion getRuntimeProtocolVersion() { return decode(runtimeProtocolVersion.load(std::memory_order_acquire)); }

bool isSdkAheadOfRuntime() { return encode(getRuntimeProtocolVersion()) < encode(getSdkProtocolVersion()); }

bool ensureProtocolSupported(const ProtocolRequirement &requirement, int *errorCode) {
    if (errorCode) {
        *errorCode = AI_COMMON_SUCCESS;
    }

    const int runtimeVersion = encode(getRuntimeProtocolVersion());
    const int minRequired = encode(requirement.minVersion);
    const int maxRequired = encode(requirement.maxVersion);

    if (encode(getSdkProtocolVersion()) < minRequired || runtimeVersion < minRequired) {
        if (errorCode) {
            *errorCode = AI_COMMON_RUNTIME_OUTDATED;
        }
        return false;
    }

    if (runtimeVersion > maxRequired) {
        if (errorCode) {
            *errorCode = AI_COMMON_RUNTIME_OUTDATED;
        }
        return false;
    }

    return true;
}

const ProtocolRequirement *findApiRequirement(const char *apiName) {
    if (apiName == nullptr) {
        return nullptr;
    }

    for (const auto &requirement : kApiProtocolRequirements) {
        if (std::string_view{apiName} == requirement.name) {
            return &requirement.requirement;
        }
    }
    return nullptr;
}

bool ensureApiSupported(const char *apiName, int *errorCode) {
    const ProtocolRequirement *requirement = findApiRequirement(apiName);
    if (requirement == nullptr) {
        // 未登记的接口按默认全兼容处理
        ProtocolRequirement defaultRequirement{ProtocolVersion{1, 0, 0}, kProtocolVersionMax};
        return ensureProtocolSupported(defaultRequirement, errorCode);
    }
    return ensureProtocolSupported(*requirement, errorCode);
}

std::string versionToString(const ProtocolVersion &version) {
    return std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch);
}

}  // namespace kyai::core::vision

#ifndef GENAI_NLP_PROTOCOL_VERSION_H
#define GENAI_NLP_PROTOCOL_VERSION_H

#include <kylin-ai/common/error.h>

#include <string>

namespace kyai::genai::nlp {

struct ProtocolVersion {
    int major{0};
    int minor{0};
    int patch{0};
};

inline constexpr ProtocolVersion kProtocolVersionMax{9999, 99, 99};

ProtocolVersion getSdkProtocolVersion();

ProtocolVersion parseProtocolVersion(const char *versionText);

void setRuntimeProtocolVersion(const ProtocolVersion &version);
void setRuntimeProtocolVersion(int major, int minor, int patch);

ProtocolVersion getRuntimeProtocolVersion();

struct ProtocolRequirement {
    ProtocolVersion minVersion;
    ProtocolVersion maxVersion;
};

bool isSdkAheadOfRuntime();

bool ensureProtocolSupported(const ProtocolRequirement &requirement, int *errorCode = nullptr);

struct ApiProtocolRequirement {
    const char *name;
    ProtocolRequirement requirement;
};

const ProtocolRequirement *findApiRequirement(const char *apiName);

bool ensureApiSupported(const char *apiName, int *errorCode = nullptr);

std::string versionToString(const ProtocolVersion &version);

}  // namespace kyai::genai::nlp

#endif  // GENAI_NLP_PROTOCOL_VERSION_H

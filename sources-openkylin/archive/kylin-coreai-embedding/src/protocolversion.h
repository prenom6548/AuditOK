#ifndef COREAI_EMBEDDING_PROTOCLO_VERSION_H
#define COREAI_EMBEDDING_PROTOCLO_VERSION_H

#include <kylin-ai/common/error.h>

#include <string>

namespace kyai::coreai::embedding {

struct ProtocolVersion {
    int major{0};
    int minor{0};
    int patch{0};
};

inline constexpr ProtocolVersion kProtocolVersionMax{9999, 99, 99};

ProtocolVersion getSdkProtocolVersion(bool isTextEmbeddingApi = true);

ProtocolVersion parseProtocolVersion(const char *versionText);

void setRuntimeProtocolVersion(const ProtocolVersion &version);
void setRuntimeProtocolVersion(int major, int minor, int patch);

ProtocolVersion getRuntimeProtocolVersion();

struct ProtocolRequirement {
    ProtocolVersion minVersion;
    ProtocolVersion maxVersion;
};

bool isSdkAheadOfRuntime(bool isTextEmbeddingApi = true);

bool ensureProtocolSupported(const ProtocolRequirement &requirement,
                             int *errorCode = nullptr,
                             bool isTextEmbeddingApi = true);

struct ApiProtocolRequirement {
    const char *name;
    ProtocolRequirement requirement;
};

const ProtocolRequirement *findApiRequirement(const char *apiName,
                                              bool isTextEmbeddingApi = false);

bool ensureApiSupported(const char *apiName, int *errorCode = nullptr,
                        bool isTextEmbeddingApi = true);

std::string versionToString(const ProtocolVersion &version);

}  // namespace kyai::coreai::embedding

#endif  // COREAI_EMBEDDING_PROTOCLO_VERSION_H

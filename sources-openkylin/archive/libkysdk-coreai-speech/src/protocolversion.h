/*
 * Copyright 2026 KylinSoft Co., Ltd.
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

#ifndef KYAI_CORE_SPEECH_PROTOCOL_VERSION_H
#define KYAI_CORE_SPEECH_PROTOCOL_VERSION_H

#include <kylin-ai/common/error.h>

#include <string>

namespace kyai::core::speech {

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

}  // namespace kyai::core::speech

#endif  // KYAI_CORE_SPEECH_PROTOCOL_VERSION_H

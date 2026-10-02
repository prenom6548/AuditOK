/*
 * Copyright 2024 KylinSoft Co., Ltd.
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

#include "textembeddingserver.h"

#include "logger.h"

namespace kyai::coreai::embedding {

TextEmbeddingServer::TextEmbeddingServer()
    : Server("unix:path=/tmp/.kylin-ai-runtime-unix/" +
                 std::to_string(getuid()) + "/core-textembedding.sock",
             "/com/kylin/AiRuntime/CoreTextEmbeddingService") {}

TextEmbeddingServer *TextEmbeddingServer::getInstance() {
    static TextEmbeddingServer instance;
    return &instance;
}

GDBusProxy *TextEmbeddingServer::createProxy(GDBusConnection *connection,
                                             GError **error) {
    AiRuntimeCoreTextEmbeddingService *proxy =
        ai_runtime_core_text_embedding_service_proxy_new_sync(
            connection, G_DBUS_PROXY_FLAGS_NONE, nullptr,
            getObjectPath().c_str(), nullptr, error);

    return G_DBUS_PROXY(proxy);
}

void TextEmbeddingServer::refreshProtocolVersion(GDBusProxy *proxy) {
    if (proxy == nullptr) {
        return;
    }

    AiRuntimeCoreTextEmbeddingService *textProxy =
        AI_RUNTIME_CORE_TEXT_EMBEDDING_SERVICE(proxy);
    const gchar *versionRaw =
        ai_runtime_core_text_embedding_service_get_runtime_version(textProxy);
    const ProtocolVersion parsedVersion = parseProtocolVersion(versionRaw);
    if (parsedVersion.major == 0 && parsedVersion.minor == 0 &&
        parsedVersion.patch == 0) {
        // 读不到runtime版本，set runtime版本是1.0.0
        const ProtocolVersion fallbackVersion{1, 0, 0};
        LOGW("Runtime protocol version is invalid:",
             versionRaw ? versionRaw : "null", ", fallback to ",
             versionToString(fallbackVersion));
        setRuntimeProtocolVersion(fallbackVersion);
        return;
    }

    setRuntimeProtocolVersion(parsedVersion);
    if (isSdkAheadOfRuntime()) {
        LOGW("SDK protocol", versionToString(getSdkProtocolVersion()),
             "is newer than ", versionToString(parsedVersion),
             "some APIs may be disabled based on requirements.");
    } else {
        LOGD("Runtime protocol", versionToString(parsedVersion),
             "is compatible with SDK",
             versionToString(getSdkProtocolVersion()));
    }
}

AiRuntimeCoreTextEmbeddingService *
TextEmbeddingServer::getTextEmbeddingProxy() {
    GDBusProxy *proxy = getProxy();
    if (proxy == nullptr) {
        // 尝试获取连接（会触发重连）
        if (getConnection() != nullptr) {
            proxy = getProxy();
        }
    }
    return proxy ? AI_RUNTIME_CORE_TEXT_EMBEDDING_SERVICE(proxy) : nullptr;
}

}  // namespace kyai::coreai::embedding

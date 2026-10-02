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

#include "genainlpserver.h"

#include <cstdlib>

namespace kyai::genai::nlp {

namespace {
constexpr const char kGenAiNlpAddressEnv[] = "KYLIN_GENAI_NLP_DBUS_ADDRESS";

std::string buildDefaultServerAddress() {
    return "unix:path=/tmp/.kylin-ai-runtime-unix/" + std::to_string(getuid()) + "/genai-nlp.sock";
}

std::string resolveServerAddress() {
    const char* env = std::getenv(kGenAiNlpAddressEnv);
    if (env != nullptr && *env != '\0') {
        return env;
    }
    static const std::string defaultAddress = buildDefaultServerAddress();
    return defaultAddress;
}
}  // namespace

GenAiNlpServer& GenAiNlpServer::getInstance() {
    static GenAiNlpServer instance{};
    return instance;
}

GenAiNlpServer::~GenAiNlpServer() { destroy(); }

GenAiNlpServer::GenAiNlpServer() {}

void GenAiNlpServer::init() { init(resolveServerAddress()); }

void GenAiNlpServer::init(const std::string& unixPath) {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr && !g_dbus_connection_is_closed(connection_)) {
        return;
    }
    GError* error{nullptr};
    connection_ =
        g_dbus_connection_new_for_address_sync(unixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
                                               nullptr,  // GDBusAuthObserver
                                               nullptr,  // GCancellable
                                               &error);
    if (connection_ == nullptr) {
        g_printerr("Error connecting to D-Bus address %s: %s\n", unixPath.c_str(), error->message);
        g_error_free(error);
    }
}

void GenAiNlpServer::destroy() const {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr) {
        g_object_unref(connection_);
    }
}

}  // namespace kyai::genai::nlp

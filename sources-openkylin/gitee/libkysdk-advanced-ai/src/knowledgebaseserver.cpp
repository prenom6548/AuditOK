#include "knowledgebaseserver.h"

namespace kyai::advancedai::knowledgebase {
const std::string& serverUnixPath =
    "unix:abstract=/tmp/.kylin-ai-business-unix/" + std::to_string(getuid()) +
    "/KnowledgeBaseService.sock";

KnowledgeBaseServer& KnowledgeBaseServer::getInstance() {
    static KnowledgeBaseServer instance{};
    return instance;
}

KnowledgeBaseServer::~KnowledgeBaseServer() { destroy(); }

KnowledgeBaseServer::KnowledgeBaseServer() {}

void KnowledgeBaseServer::init() { init(serverUnixPath); }

void KnowledgeBaseServer::init(const std::string& unixPath) {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr && !g_dbus_connection_is_closed(connection_)) {
        return;
    }
    GError* error{nullptr};
    connection_ = g_dbus_connection_new_for_address_sync(
        unixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT,
        nullptr,  // GDBusAuthObserver
        nullptr,  // GCancellable
        &error);
    if (connection_ == nullptr) {
        g_printerr("Error connecting to D-Bus address %s: %s\n",
                   unixPath.c_str(), error->message);
        g_error_free(error);
    }
}

void KnowledgeBaseServer::destroy() const {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr) {
        g_object_unref(connection_);
    }
}

bool KnowledgeBaseServer::isConnectionActive() const {
    std::lock_guard<std::mutex> locker(mutex_);
    return connection_ != nullptr && !g_dbus_connection_is_closed(connection_);
}
}  // namespace kyai::advancedai::knowledgebase
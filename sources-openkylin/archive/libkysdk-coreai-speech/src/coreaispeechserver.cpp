#include "coreaispeechserver.h"

#include <logger.h>

namespace kyai::core::speech {

const std::string serverUnixPath =
    "unix:path=/tmp/.kylin-ai-runtime-unix/" + std::to_string(getuid()) + "/core-speech.sock";

CoreAiSpeechServer &CoreAiSpeechServer::getInstance() {
    static CoreAiSpeechServer instance{};
    return instance;
}

CoreAiSpeechServer::CoreAiSpeechServer() {}

CoreAiSpeechServer::~CoreAiSpeechServer() { destroy(); }

void CoreAiSpeechServer::init() { init(serverUnixPath); }

void CoreAiSpeechServer::init(const std::string &unixPath) {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr && !g_dbus_connection_is_closed(connection_)) {
        return;
    }
    GError *error = nullptr;
    connection_ = g_dbus_connection_new_for_address_sync(
        unixPath.c_str(), G_DBUS_CONNECTION_FLAGS_AUTHENTICATION_CLIENT, nullptr, /* GDBusAuthObserver */
        nullptr,                                                                  /* GCancellable */
        &error);
    if (connection_ == nullptr) {
        g_printerr("Error connecting to D-Bus address %s: %s\n", unixPath.c_str(), error->message);
        g_error_free(error);
        return;
    }
    handlerId_ = g_signal_connect(connection_, "closed", G_CALLBACK(onConnectionClosed), this);
}

void CoreAiSpeechServer::destroy() {
    std::lock_guard<std::mutex> locker(mutex_);
    if (connection_ != nullptr) {
        // 断开信号连接
        if (handlerId_ != 0) {
            g_signal_handler_disconnect(connection_, handlerId_);
        }
        // 关闭并释放连接
        g_dbus_connection_close_sync(connection_, nullptr, nullptr);
        g_object_unref(connection_);
    }
    onConnectionClosed_ = nullptr;
}

void CoreAiSpeechServer::onConnectionClosed(GDBusConnection *connection, gboolean remotePeerVanished, GError *error,
                                            gpointer userData) {
    LOGE("On connection closed.");
    CoreAiSpeechServer *server = (CoreAiSpeechServer *)(userData);
    if (server->onConnectionClosed_) {
        server->onConnectionClosed_();
    }
}

}  // namespace kyai::core::speech

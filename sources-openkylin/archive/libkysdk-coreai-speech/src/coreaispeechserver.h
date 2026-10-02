#ifndef SRC_CORE_AI_SPEECH_SERVER_H
#define SRC_CORE_AI_SPEECH_SERVER_H
#include <gio/gio.h>

#include <functional>
#include <mutex>
#include <string>

namespace kyai::core::speech {

using OnConnectionClosed = std::function<void()>;

class CoreAiSpeechServer {
public:
    static CoreAiSpeechServer &getInstance();

    ~CoreAiSpeechServer();

    void init();

    bool available() const { return connection_ != nullptr; }

    GDBusConnection *getConnection() const { return connection_; }

    void setOnConnectionClosed(const OnConnectionClosed &onConnectionClosed) {
        onConnectionClosed_ = onConnectionClosed;
    }

private:
    CoreAiSpeechServer();

    void init(const std::string &unixPath);

    void destroy();

    static void onConnectionClosed(GDBusConnection *connection, gboolean remotePeerVanished, GError *error,
                                   gpointer userData);

private:
    GDBusConnection *connection_{nullptr};

    gulong handlerId_{};

    OnConnectionClosed onConnectionClosed_;
    mutable std::mutex mutex_;
};

}  // namespace kyai::core::speech

#endif  // SRC_CORE_AI_SPEECH_SERVER_H

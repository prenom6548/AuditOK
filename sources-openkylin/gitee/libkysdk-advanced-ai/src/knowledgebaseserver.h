#ifndef ADVANCEDAI_SERVER_H
#define ADVANCEDAI_SERVER_H

#include <mutex>
#include <string>

#include "advaiknowledgebaseserviceglue.h"

namespace kyai::advancedai::knowledgebase {
class KnowledgeBaseServer {
public:
    static KnowledgeBaseServer &getInstance();

    ~KnowledgeBaseServer();

    void init();
    [[nodiscard]] inline bool available() const {
        return connection_ != nullptr;
    }

    [[nodiscard]] inline GDBusConnection *getConnection() const {
        return connection_;
    }

    bool isConnectionActive() const;

private:
    KnowledgeBaseServer();
    void init(const std::string &unixPath);
    void destroy() const;

    GDBusConnection *connection_{nullptr};
    mutable std::mutex mutex_;
};
}  // namespace kyai::advancedai::knowledgebase

#endif
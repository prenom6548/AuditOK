#pragma once

#include <chrono>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace mcp {

struct Session {
    std::string id;
    std::chrono::steady_clock::time_point created_at;
    std::chrono::steady_clock::time_point expires_at;
    std::unordered_map<std::string, std::string> attributes;
};

class SessionManager {
public:
    using Clock = std::chrono::steady_clock;
    using Duration = Clock::duration;
    static constexpr Duration kDefaultTtl = std::chrono::minutes(30);

    SessionManager();

    template <typename Rep, typename Period>
    explicit SessionManager(const std::chrono::duration<Rep, Period>& ttl) : SessionManager() {
        SetSessionTtl(ttl);
    }

    std::string CreateSession(const std::unordered_map<std::string, std::string>& attributes = {},
                              const std::optional<std::string>& requested_id = std::nullopt);
    bool ValidateSession(const std::string& session_id);
    std::optional<std::unordered_map<std::string, std::string>> GetSessionAttributes(const std::string& session_id);
    bool UpdateSessionAttributes(const std::string& session_id, const std::unordered_map<std::string, std::string>& attributes);
    bool UpdateSessionAttribute(const std::string& session_id, const std::string& key, const std::string& value);
    bool AddSession(const std::string& session_id, const Session& session);
    bool RemoveSession(const std::string& session_id);
    void CleanupExpiredSessions();

    void SetSessionTtl(Duration ttl);

    template <typename Rep, typename Period>
    void SetSessionTtl(const std::chrono::duration<Rep, Period>& ttl) {
        SetSessionTtl(std::chrono::duration_cast<Duration>(ttl));
    }
    Duration GetSessionTtl() const;

private:
    bool IsExpired(const Session& session, const Clock::time_point now) const;
    void RefreshSession(Session& session, const Clock::time_point now);
    std::string GenerateSessionId() const;

    Duration ttl_;
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, Session> sessions_;
};

}  // namespace mcp



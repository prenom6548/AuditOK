#include "session_manager.h"

#include <mutex>
#include <random>

namespace mcp {

namespace {
constexpr char kSessionIdAlphabet[] =
    "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
constexpr std::size_t kSessionIdLength = 32;
}  // namespace

SessionManager::SessionManager() : ttl_(kDefaultTtl) {}

std::string SessionManager::CreateSession(
    const std::unordered_map<std::string, std::string>& attributes,
    const std::optional<std::string>& requested_id) {
    const auto now = Clock::now();
    std::string session_id = requested_id.value_or(GenerateSessionId());

    std::unique_lock lock(mutex_);

    // TODO: handle collision
    while (sessions_.find(session_id) != sessions_.end()) {
        if (requested_id && session_id == *requested_id) {
            // Requested ID already exists, fall back to generated IDs.
            session_id = GenerateSessionId();
            continue;
        }
        session_id = GenerateSessionId();
    }

    Session session;
    session.id = session_id;
    session.created_at = now;
    session.expires_at = now + ttl_;
    session.attributes = attributes;
    sessions_.emplace(session.id, std::move(session));

    return session_id;
}

bool SessionManager::ValidateSession(const std::string& session_id) {
    const auto now = Clock::now();

    std::unique_lock lock(mutex_);
    const auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return false;
    }

    if (IsExpired(it->second, now)) {
        sessions_.erase(it);
        return false;
    }

    RefreshSession(it->second, now);
    return true;
}

std::optional<std::unordered_map<std::string, std::string>>
SessionManager::GetSessionAttributes(const std::string& session_id) {
    const auto now = Clock::now();

    std::unique_lock lock(mutex_);
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return std::nullopt;
    }

    if (IsExpired(it->second, now)) {
        sessions_.erase(it);
        return std::nullopt;
    }

    auto attributes = it->second.attributes;
    RefreshSession(it->second, now);
    return attributes;
}

bool SessionManager::UpdateSessionAttributes(
    const std::string& session_id,
    const std::unordered_map<std::string, std::string>& attributes) {
    const auto now = Clock::now();

    std::unique_lock lock(mutex_);
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return false;
    }

    if (IsExpired(it->second, now)) {
        sessions_.erase(it);
        return false;
    }

    it->second.attributes = attributes;
    RefreshSession(it->second, now);
    return true;
}

bool SessionManager::UpdateSessionAttribute(const std::string& session_id,
                                            const std::string& key,
                                            const std::string& value) {
    const auto now = Clock::now();

    std::unique_lock lock(mutex_);
    auto it = sessions_.find(session_id);
    if (it == sessions_.end()) {
        return false;
    }

    if (IsExpired(it->second, now)) {
        sessions_.erase(it);
        return false;
    }

    it->second.attributes[key] = value;
    RefreshSession(it->second, now);
    return true;
}

bool SessionManager::AddSession(const std::string& session_id,
                                const Session& session) {
    std::unique_lock lock(mutex_);

    // Ensure session_id matches incoming session.id
    Session session_to_add = session;
    session_to_add.id = session_id;

    // If session has no expiration, use now + TTL
    const auto now = Clock::now();
    if (session_to_add.expires_at <= session_to_add.created_at) {
        session_to_add.expires_at = now + ttl_;
    }
    if (session_to_add.created_at == Clock::time_point{}) {
        session_to_add.created_at = now;
    }

    sessions_[session_id] = std::move(session_to_add);
    return true;
}

bool SessionManager::RemoveSession(const std::string& session_id) {
    std::unique_lock lock(mutex_);
    return sessions_.erase(session_id) > 0;
}

void SessionManager::CleanupExpiredSessions() {
    const auto now = Clock::now();
    std::unique_lock lock(mutex_);

    for (auto it = sessions_.begin(); it != sessions_.end();) {
        if (IsExpired(it->second, now)) {
            it = sessions_.erase(it);
        } else {
            ++it;
        }
    }
}

void SessionManager::SetSessionTtl(Duration ttl) {
    std::unique_lock lock(mutex_);
    ttl_ = ttl;
}

SessionManager::Duration SessionManager::GetSessionTtl() const {
    std::shared_lock lock(mutex_);
    return ttl_;
}

bool SessionManager::IsExpired(const Session& session,
                               const Clock::time_point now) const {
    return session.expires_at <= now;
}

void SessionManager::RefreshSession(Session& session,
                                    const Clock::time_point now) {
    session.expires_at = now + ttl_;
}

std::string SessionManager::GenerateSessionId() const {
    thread_local std::random_device rd;
    thread_local std::mt19937 generator(rd());
    thread_local std::uniform_int_distribution<std::size_t> distribution(
        0, sizeof(kSessionIdAlphabet) - 2);

    std::string session_id;
    session_id.reserve(kSessionIdLength);
    for (std::size_t i = 0; i < kSessionIdLength; ++i) {
        session_id.push_back(kSessionIdAlphabet[distribution(generator)]);
    }
    return session_id;
}

}  // namespace mcp

/**
 * @file message.h
 * @brief Core definitions for the Model Context Protocol (MCP) framework
 *
 * This file contains the core structures and definitions for the MCP protocol.
 * Implements the 2024-11-05 and 2025-11-25 protocol specifications.
 * Supports backward compatibility between versions.
 */

#ifndef KYSDK_MCP_MESSAGE_COMMON_H
#define KYSDK_MCP_MESSAGE_COMMON_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <stdexcept>
#include <string>

// Include the JSON library for parsing and generating JSON
#include "json.hpp"

namespace mcp {

// Use the nlohmann json library
using json = nlohmann::ordered_json;

// MCP version constants
constexpr const char* MCP_VERSION_2024_11_05 = "2024-11-05";
constexpr const char* MCP_VERSION_2025_11_25 = "2025-11-25";
constexpr const char* MCP_VERSION =
    MCP_VERSION_2025_11_25;  // Default version for backward compatibility

/**
 * @brief MCP error codes
 *
 * Follows JSON-RPC 2.0 standard error codes with extensions
 * in the -32000 to -32099 range for server-defined errors.
 */
enum class ErrorCode {
    kParseError = -32700,        ///< Invalid JSON
    kInvalidRequest = -32600,    ///< Invalid Request object
    kMethodNotFound = -32601,    ///< Method not found
    kInvalidParams = -32602,     ///< Invalid method parameters
    kInternalError = -32603,     ///< Internal JSON-RPC error
    kServerErrorStart = -32000,  ///< Server error start
    kServerErrorEnd = -32099     ///< Server error end
};

/**
 * @brief MCP exception class
 *
 * Thrown when MCP operations fail due to protocol errors,
 * invalid parameters, or internal failures.
 */
class mcp_exception : public std::runtime_error {
public:
    mcp_exception(ErrorCode code, const std::string& message)
        : std::runtime_error(message), code_(code) {}

    ErrorCode code() const { return code_; }

private:
    ErrorCode code_;
};

/**
 * @brief JSON-RPC 2.0 Request structure
 *
 * Represents a JSON-RPC 2.0 request object that may or may not
 * expect a response (notification).
 */
struct request {
    std::string jsonrpc = "2.0";  ///< JSON-RPC version (must be "2.0")
    json id;                     ///< Request identifier (null for notifications)
    std::string method;          ///< Method name to invoke
    json params;                 ///< Method parameters

    /**
     * @brief Create a new request with auto-generated ID
     * @param method Method name to invoke
     * @param params Optional method parameters (default: empty object)
     * @return New request with unique ID
     */
    static request create(const std::string& method,
                          const json& params = json::object()) {
        request req;
        req.jsonrpc = "2.0";
        req.id = generateId();
        req.method = method;
        req.params = params;
        return req;
    }

    /**
     * @brief Create a request with a specific string ID
     * @param id Request identifier
     * @param method Method name to invoke
     * @param params Optional method parameters
     * @return New request with specified ID
     */
    static request createWithId(const std::string& id,
                                const std::string& method,
                                const json& params = json::object()) {
        request req;
        req.jsonrpc = "2.0";
        req.id = json::string_t(id);
        req.method = method;
        req.params = params;
        return req;
    }

    /**
     * @brief Create a notification (request without ID)
     * @param method Notification method name
     * @param params Optional method parameters
     * @return New notification request (id will be null)
     */
    static request createNotification(const std::string& method,
                                      const json& params = json::object()) {
        request req;
        req.jsonrpc = "2.0";
        req.id = nullptr;
        req.method = method;
        req.params = params;
        return req;
    }

    /**
     * @brief Check if this request is a notification
     * @return true if no response is expected (id is null)
     */
    bool isNotification() const { return id.is_null(); }

    /**
     * @brief Convert request to JSON
     * @return JSON representation of the request
     */
    json toJson() const {
        json j = {{"jsonrpc", jsonrpc}, {"method", method}};

        if (!params.empty()) {
            j["params"] = params;
        }

        if (!isNotification()) {
            j["id"] = id;
        }

        return j;
    }

    /**
     * @brief Parse request from JSON
     * @param j JSON object to parse
     * @return Parsed request structure
     */
    static request fromJson(const json& j) {
        request req;
        req.jsonrpc = j["jsonrpc"].get<std::string>();
        if (j.contains("id")) {
            req.id = j["id"];
        } else {
            req.id = nullptr;
        }
        req.method = j["method"].get<std::string>();

        if (j.contains("params")) {
            req.params = j["params"];
        }

        return req;
    }

private:
    /// Generate a unique ID for requests
    static json generateId() {
        static std::atomic<uint64_t> next_id{1};
        const std::uint64_t seq =
            next_id.fetch_add(1, std::memory_order_relaxed);
        const auto now_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                                std::chrono::steady_clock::now().time_since_epoch())
                                .count();
        return "req-" + std::to_string(now_ns) + "-" + std::to_string(seq);
    }
};

/**
 * @brief JSON-RPC 2.0 Response structure
 *
 * Represents a JSON-RPC 2.0 response, either success or error.
 */
struct response {
    std::string jsonrpc = "2.0";  ///< JSON-RPC version (must be "2.0")
    json id;                     ///< Request identifier (matches request ID)
    json result;                 ///< Result data (present on success)
    json error;                  ///< Error information (present on failure)

    /**
     * @brief Create a success response
     * @param req_id Request ID to respond to
     * @param result_data Result payload
     * @return Success response
     */
    static response createSuccess(const json& req_id,
                                  const json& result_data = json::object()) {
        response res;
        res.jsonrpc = "2.0";
        res.id = req_id;
        res.result = result_data;
        return res;
    }

    /**
     * @brief Create an error response (backward compatible with 2024-11-05)
     * @param req_id Request ID to respond to
     * @param code Error code
     * @param message Human-readable error message
     * @param data Optional additional error data
     * @return Error response
     */
    static response createError(const json& req_id, ErrorCode code,
                                const std::string& message,
                                const json& data = json::object()) {
        response res;
        res.jsonrpc = "2.0";
        res.id = req_id;  // Always include id for backward compatibility
        res.error = {{"code", static_cast<int>(code)}, {"message", message}};

        if (!data.empty()) {
            res.error["data"] = data;
        }

        return res;
    }

    /**
     * @brief Create an error response for 2025-11-25 (id is optional)
     * @param req_id Request ID (can be null)
     * @param code Error code
     * @param message Human-readable error message
     * @param data Optional additional error data
     * @return Error response
     */
    static response createError2025(const json& req_id, ErrorCode code,
                                    const std::string& message,
                                    const json& data = json::object()) {
        response res;
        res.jsonrpc = "2.0";
        res.id = req_id;  // Can be null for notifications
        res.error = {{"code", static_cast<int>(code)}, {"message", message}};

        if (!data.empty()) {
            res.error["data"] = data;
        }

        return res;
    }

    /**
     * @brief Create an error response without id (for notifications in 2025-11-25)
     * @param code Error code
     * @param message Human-readable error message
     * @param data Optional additional error data
     * @return Error response without ID
     */
    static response createErrorWithoutId(ErrorCode code,
                                         const std::string& message,
                                         const json& data = json::object()) {
        response res;
        res.jsonrpc = "2.0";
        res.id = nullptr;  // id is optional in error responses per 2025-11-25 spec
        res.error = {{"code", static_cast<int>(code)}, {"message", message}};

        if (!data.empty()) {
            res.error["data"] = data;
        }

        return res;
    }

    /**
     * @brief Check if this is an error response
     * @return true if response contains an error
     */
    bool isError() const { return !error.empty(); }

    /**
     * @brief Convert response to JSON
     * @param protocol_version Protocol version for compatibility
     * @return JSON representation
     */
    json toJson(
        const std::string& protocol_version = MCP_VERSION_2024_11_05) const {
        json j = {{"jsonrpc", jsonrpc}};

        if (isError()) {
            j["error"] = error;
            if (protocol_version == MCP_VERSION_2025_11_25 && id.is_null()) {
                // Don't include id for 2025-11-25 if it's null
            } else {
                j["id"] = id;
            }
        } else {
            j["id"] = id;
            j["result"] = result;
        }

        return j;
    }

    /**
     * @brief Parse response from JSON
     * @param j JSON object to parse
     * @return Parsed response structure
     */
    static response fromJson(const json& j) {
        response res;
        res.jsonrpc = j["jsonrpc"].get<std::string>();

        if (j.contains("result")) {
            res.id = j["id"];
            res.result = j["result"];
        } else if (j.contains("error")) {
            // In 2025-11-25, id is optional in error responses
            if (j.contains("id")) {
                res.id = j["id"];
            } else {
                res.id = nullptr;  // id is optional in error responses
            }
            res.error = j["error"];
        }

        return res;
    }
};

}  // namespace mcp

#endif  // KYSDK_MCP_MESSAGE_COMMON_H

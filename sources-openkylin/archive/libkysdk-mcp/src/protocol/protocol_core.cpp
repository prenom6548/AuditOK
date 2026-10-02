/**
 * @file protocol_core.cpp
 * @brief MCP Protocol Core Implementation
 *
 * This file provides the implementation of core protocol utilities
 * and data structure serialization/deserialization.
 */

#include "common/protocol.h"

#include <chrono>
#include <random>
#include <sstream>

// Logger is declared locally when needed

namespace mcp {

// Utility functions implementation
namespace utils {

ClientCapabilities createDefaultClientCapabilities() {
    ClientCapabilities caps;
    // Keep historical defaults for backward compatibility.
    caps.prompts = {{"listChanged", true}};
    caps.resources = {{"subscribe", true}};
    caps.tools = {{"listChanged", true}};
    caps.sampling = {{"maxTokens", 4096}, {"temperature", 0.7}};
    // 2025-11-25 client features.
    caps.elicitation = json{{"form", json::object()}};
    caps.tasks = json{{"requests",
                       json{{"sampling", json{{"createMessage", json::object()}}},
                            {"elicitation", json{{"create", json::object()}}}}}};
    caps.roots = {{"listChanged", true}};
    return caps;
}

ServerCapabilities createDefaultServerCapabilities() {
    ServerCapabilities caps;
    caps.prompts = {{"listChanged", false}};
    caps.resources = {{"listChanged", false}, {"subscribe", false}};
    caps.tools = {{"listChanged", false}};
    // Keep historical defaults for backward compatibility.
    caps.logging = {{"setLevel", true}};
    caps.sampling = {{"createMessage", true},
                     {"maxTokens", 2048},
                     {"temperature", 0.5}};
    caps.completions = json::object();
    caps.tasks = json{{"list", json::object()},
                      {"cancel", json::object()}};
    return caps;
}

bool isValidProtocolVersion(const std::string& version) {
    return version == "2024-11-05" || version == "2025-03-26" ||
           version == "2025-06-18" || version == "2025-11-25";
}

Tool createToolFromSchema(const std::string& name,
                          const std::string& description, const json& schema) {
    Tool tool;
    tool.name = name;
    tool.description = description;
    tool.inputSchema = schema;
    return tool;
}

Resource createResourceFromUri(const std::string& uri, const std::string& name,
                               const std::string& description) {
    Resource resource;
    resource.uri = uri;
    resource.name = name.empty() ? uri : name;
    resource.description = description;
    resource.mimeType = "text/plain";
    return resource;
}

}  // namespace utils

}  // namespace mcp

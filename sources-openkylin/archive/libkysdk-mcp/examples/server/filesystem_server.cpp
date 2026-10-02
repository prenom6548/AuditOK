/**
 * @file filesystem_server.cpp
 * @brief MCP FileSystem Server Implementation
 *
 * This file provides the implementation of the MCP file system server.
 */

#include "filesystem_server.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>

#include "logger.h"

namespace mcp {

MCPFileSystemServer::MCPFileSystemServer(const std::string& root_path,
                                         bool enable_write)
    : MCPServer(), root_path_(root_path), enable_write_(enable_write) {
    // 设置服务器信息
    setServerInfo("MCP FileSystem Server", "1.0.0");

    // 设置服务器能力
    ServerCapabilities capabilities;
    capabilities.tools = json{{"listChanged", true}};
    capabilities.resources = json{{"subscribe", true}, {"listChanged", true}};
    capabilities.prompts = json{{"listChanged", true}};
    setCapabilities(capabilities);

    // 设置文件系统特定的工具和资源
    registerDefaultTools();
    registerDefaultResources();

    mcp_logger::getModuleLogger("mcp_filesystem_server")
        ->info("MCPFileSystemServer created with root path: " + root_path_);
}

void MCPFileSystemServer::setRootPath(const std::string& root_path) {
    root_path_ = root_path;
    mcp_logger::getModuleLogger("mcp_filesystem_server")
        ->info("Root path changed to: " + root_path_);
}

const std::string& MCPFileSystemServer::getRootPath() const {
    return root_path_;
}

void MCPFileSystemServer::setWriteEnabled(bool enable) {
    enable_write_ = enable;
    mcp_logger::getModuleLogger("mcp_filesystem_server")
        ->info("Write operations " +
               std::string(enable ? "enabled" : "disabled"));
}

bool MCPFileSystemServer::isWriteEnabled() const { return enable_write_; }

void MCPFileSystemServer::addFileSystemResource(
    const std::string& path, const std::string& name,
    const std::string& description) {
    if (!validatePath(path)) {
        mcp_logger::getModuleLogger("mcp_filesystem_server")
            ->error("Invalid path for resource: " + path);
        return;
    }

    Resource resource;
    resource.uri = "file://" + path;
    resource.name =
        name.empty() ? std::filesystem::path(path).filename().string() : name;
    resource.description =
        description.empty() ? "File system resource: " + path : description;
    resource.mimeType =
        std::filesystem::is_directory(path) ? "application/json" : "text/plain";

    if (!registerResource(resource, [this](const std::string& uri) {
            return readFileSystemResource(uri);
        })) {
        mcp_logger::getModuleLogger("mcp_filesystem_server")
            ->warning("Resource already registered: " + resource.uri);
        return;
    }

    // 添加到文件系统资源列表
    if (std::find(fs_resources_.begin(), fs_resources_.end(), path) ==
        fs_resources_.end()) {
        fs_resources_.push_back(path);
    }

    mcp_logger::getModuleLogger("mcp_filesystem_server")
        ->info("Added file system resource: " + path);
}

void MCPFileSystemServer::removeFileSystemResource(const std::string& path) {
    std::string uri = "file://" + path;
    unregisterResource(uri);

    // 从文件系统资源列表中移除
    fs_resources_.erase(
        std::remove(fs_resources_.begin(), fs_resources_.end(), path),
        fs_resources_.end());

    mcp_logger::getModuleLogger("mcp_filesystem_server")
        ->info("Removed file system resource: " + path);
}

std::vector<std::string> MCPFileSystemServer::listFileSystemResources() const {
    return fs_resources_;
}

bool MCPFileSystemServer::validatePath(const std::string& path) const {
    try {
        std::filesystem::path fs_path(path);
        std::filesystem::path root_fs_path(root_path_);

        // 如果路径是相对的，将其解析为相对于根路径
        if (!fs_path.is_absolute()) {
            fs_path = root_fs_path / fs_path;
        }

        // weakly_canonical keeps the path usable even when the target does not
        // exist yet.
        fs_path = std::filesystem::weakly_canonical(fs_path);
        root_fs_path = std::filesystem::weakly_canonical(root_fs_path);

        // 检查路径是否在根路径内
        std::string path_str = fs_path.string();
        std::string root_str = root_fs_path.string();

        return path_str.find(root_str) == 0;
    } catch (const std::exception& e) {
        mcp_logger::getModuleLogger("mcp_filesystem_server")
            ->error("Path validation error: " + std::string(e.what()));
        return false;
    }
}

std::string MCPFileSystemServer::resolvePath(const std::string& path) const {
    try {
        std::filesystem::path fs_path(path);
        std::filesystem::path root_fs_path(root_path_);

        // 如果路径是绝对的，直接使用
        if (fs_path.is_absolute()) {
            return std::filesystem::weakly_canonical(fs_path).string();
        }

        // 否则相对于根路径解析
        return std::filesystem::weakly_canonical(root_fs_path / fs_path)
            .string();
    } catch (const std::exception& e) {
        mcp_logger::getModuleLogger("mcp_filesystem_server")
            ->error("Path resolution error: " + std::string(e.what()));
        return path;  // 返回原始路径作为fallback
    }
}

void MCPFileSystemServer::registerDefaultTools() {
    auto logger = mcp_logger::getModuleLogger("mcp_filesystem_server");
    logger->info("Setting up file system tools");

    // 文件读取工具
    Tool readFileTool;
    readFileTool.name = "read_file";
    readFileTool.description = "Read the contents of a file";
    readFileTool.inputSchema = {
        {"type", "object"},
        {"properties",
         {{"path",
           {{"type", "string"},
            {"description",
             "Path to the file to read (relative to root path)"}}}}},
        {"required", json::array({"path"})}};
    registerTool(readFileTool, [this](const ToolCallParams& params) {
        return handleFileSystemToolCall(params);
    });

    // 文件写入工具（如果启用写操作）
    if (enable_write_) {
        Tool writeFileTool;
        writeFileTool.name = "write_file";
        writeFileTool.description = "Write content to a file";
        writeFileTool.inputSchema = {
            {"type", "object"},
            {"properties",
             {{"path",
               {{"type", "string"},
                {"description",
                 "Path to the file to write (relative to root path)"}}},
              {"content",
               {{"type", "string"},
                {"description", "Content to write to the file"}}}}},
            {"required", json::array({"path", "content"})}};
        registerTool(writeFileTool, [this](const ToolCallParams& params) {
            return handleFileSystemToolCall(params);
        });
    }

    // 目录列表工具
    Tool listDirectoryTool;
    listDirectoryTool.name = "list_directory";
    listDirectoryTool.description = "List the contents of a directory";
    listDirectoryTool.inputSchema = {
        {"type", "object"},
        {"properties",
         {{"path",
           {{"type", "string"},
            {"description",
             "Path to the directory to list (relative to root path, default: "
             "root)"}}}}},
        {"required", json::array()}};
    registerTool(listDirectoryTool, [this](const ToolCallParams& params) {
        return handleFileSystemToolCall(params);
    });

    // 目录创建工具（如果启用写操作）
    if (enable_write_) {
        Tool createDirectoryTool;
        createDirectoryTool.name = "create_directory";
        createDirectoryTool.description = "Create a new directory";
        createDirectoryTool.inputSchema = {
            {"type", "object"},
            {"properties",
             {{"path",
               {{"type", "string"},
                {"description",
                 "Path of the directory to create (relative to root path)"}}}}},
            {"required", json::array({"path"})}};
        registerTool(createDirectoryTool, [this](const ToolCallParams& params) {
            return handleFileSystemToolCall(params);
        });
    }

    // 文件删除工具（如果启用写操作）
    if (enable_write_) {
        Tool deleteFileTool;
        deleteFileTool.name = "delete_file";
        deleteFileTool.description = "Delete a file or directory";
        deleteFileTool.inputSchema = {
            {"type", "object"},
            {"properties",
             {{"path",
               {{"type", "string"},
                {"description",
                 "Path of the file or directory to delete (relative to root "
                 "path)"}}}}},
            {"required", json::array({"path"})}};
        registerTool(deleteFileTool, [this](const ToolCallParams& params) {
            return handleFileSystemToolCall(params);
        });
    }

    // 文件信息工具
    Tool fileInfoTool;
    fileInfoTool.name = "file_info";
    fileInfoTool.description = "Get information about a file or directory";
    fileInfoTool.inputSchema = {
        {"type", "object"},
        {"properties",
         {{"path",
           {{"type", "string"},
            {"description",
             "Path to the file or directory (relative to root path)"}}}}},
        {"required", json::array({"path"})}};
    registerTool(fileInfoTool, [this](const ToolCallParams& params) {
        return handleFileSystemToolCall(params);
    });

    logger->info("File system tools setup completed");
}

void MCPFileSystemServer::registerDefaultResources() {
    auto logger = mcp_logger::getModuleLogger("mcp_filesystem_server");
    logger->info("Setting up file system resources");

    // 添加根目录作为资源
    addFileSystemResource(root_path_, "Root Directory",
                          "The root directory for file system operations");

    // 如果根目录存在且有子目录，添加一些常见的子目录作为资源
    try {
        std::filesystem::path root_fs_path(root_path_);
        if (std::filesystem::exists(root_fs_path) &&
            std::filesystem::is_directory(root_fs_path)) {
            for (const auto& entry :
                 std::filesystem::directory_iterator(root_fs_path)) {
                if (entry.is_directory()) {
                    std::string relative_path =
                        std::filesystem::relative(entry.path(), root_fs_path)
                            .string();
                    addFileSystemResource(
                        relative_path, entry.path().filename().string(),
                        "Directory: " + entry.path().filename().string());
                }
            }
        }
    } catch (const std::exception& e) {
        logger->error("Error setting up default resources: " +
                      std::string(e.what()));
    }

    logger->info("File system resources setup completed");
}

ReadResourceResult MCPFileSystemServer::readFileSystemResource(
    const std::string& uri) const {
    if (uri.rfind("file://", 0) != 0) {
        throw mcp_exception(ErrorCode::kInternalError,
                            "Unsupported resource URI: " + uri);
    }

    std::string path = uri.substr(7);
    std::string resolved_path = resolvePath(path);
    if (!validatePath(resolved_path)) {
        throw mcp_exception(ErrorCode::kInternalError,
                            "Invalid resource path: " + path);
    }

    std::filesystem::path fs_path(resolved_path);
    if (!std::filesystem::exists(fs_path)) {
        throw mcp_exception(ErrorCode::kInternalError,
                            "Resource not found: " + uri);
    }

    ReadResourceResult result;
    ResourceContent content;
    content.uri = uri;

    if (std::filesystem::is_directory(fs_path)) {
        json payload;
        payload["type"] = "directory";
        payload["path"] = path;
        payload["uri"] = uri;

        json files = json::array();
        for (const auto& entry : std::filesystem::directory_iterator(fs_path)) {
            json file_info;
            file_info["name"] = entry.path().filename().string();
            file_info["path"] = entry.path().string();
            file_info["is_directory"] = entry.is_directory();
            file_info["size"] = entry.is_regular_file() ? entry.file_size() : 0;
            files.push_back(file_info);
        }
        payload["files"] = files;
        content.mimeType = "application/json";
        content.text = payload.dump(2);
        result.contents.push_back(content);
        return result;
    }

    if (std::filesystem::is_regular_file(fs_path)) {
        std::ifstream file(resolved_path);
        if (!file.is_open()) {
            throw mcp_exception(ErrorCode::kInternalError,
                                "Cannot open file: " + path);
        }

        std::string file_content((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
        content.mimeType = "text/plain";
        content.text = file_content;
        result.contents.push_back(content);
        return result;
    }

    throw mcp_exception(ErrorCode::kInternalError,
                        "Unsupported resource type: " + uri);
}

ToolCallResult MCPFileSystemServer::handleFileSystemToolCall(
    const ToolCallParams& params) {
    auto logger = mcp_logger::getModuleLogger("mcp_filesystem_server");
    logger->info("Handling file system tool call: " + params.name);

    ToolCallResult result;

    try {
        if (params.name == "read_file") {
            std::string path = params.arguments.value("path", "");
            if (path.empty()) {
                result.isError = true;
                result.errorMessage = "Path parameter is required";
                return result;
            }

            std::string resolved_path = resolvePath(path);
            if (!validatePath(resolved_path)) {
                result.isError = true;
                result.errorMessage = "Invalid path: " + path;
                return result;
            }

            std::ifstream file(resolved_path);
            if (!file.is_open()) {
                result.isError = true;
                result.errorMessage = "Cannot open file: " + path;
                return result;
            }

            std::string content((std::istreambuf_iterator<char>(file)),
                                std::istreambuf_iterator<char>());

            // 根据 MCP 协议规范，content 应该是一个数组，每个元素是 TextContent
            // 或 ImageContent 将文件内容包装为符合 MCP 协议规范的格式 创建符合
            // MCP 协议规范的 content 数组
            result.content =
                json::array({json{{"type", "text"}, {"text", content}}});

            return result;
        } else if (params.name == "write_file" && enable_write_) {
            std::string path = params.arguments.value("path", "");
            std::string content = params.arguments.value("content", "");

            if (path.empty()) {
                result.isError = true;
                result.errorMessage = "Path parameter is required";
                return result;
            }

            std::string resolved_path = resolvePath(path);
            if (!validatePath(resolved_path)) {
                result.isError = true;
                result.errorMessage = "Invalid path: " + path;
                return result;
            }

            // 确保目录存在
            std::filesystem::path file_path(resolved_path);
            std::filesystem::create_directories(file_path.parent_path());

            std::ofstream file(resolved_path);
            if (!file.is_open()) {
                result.isError = true;
                result.errorMessage = "Cannot create file: " + path;
                return result;
            }

            file << content;
            file.close();

            // 根据 MCP 协议规范，content 应该是一个数组
            std::string message = "File written successfully\n";
            message += "Path: " + path + "\n";
            message += "Resolved path: " + resolved_path + "\n";
            message += "Size: " + std::to_string(content.size()) + " bytes";

            result.content =
                json::array({json{{"type", "text"}, {"text", message}}});
            return result;
        } else if (params.name == "list_directory") {
            std::string path = params.arguments.value("path", "");
            if (path.empty()) {
                path = ".";  // 默认列出根目录
            }

            std::string resolved_path = resolvePath(path);
            if (!validatePath(resolved_path)) {
                result.isError = true;
                result.errorMessage = "Invalid path: " + path;
                return result;
            }

            std::filesystem::path dir_path(resolved_path);
            if (!std::filesystem::exists(dir_path)) {
                result.isError = true;
                result.errorMessage = "Directory does not exist: " + path;
                return result;
            }

            if (!std::filesystem::is_directory(dir_path)) {
                result.isError = true;
                result.errorMessage = "Path is not a directory: " + path;
                return result;
            }

            json files = json::array();
            for (const auto& entry :
                 std::filesystem::directory_iterator(dir_path)) {
                json file_info;
                file_info["name"] = entry.path().filename().string();
                file_info["path"] =
                    std::filesystem::relative(entry.path(),
                                              std::filesystem::path(root_path_))
                        .string();
                file_info["is_directory"] = entry.is_directory();
                file_info["size"] =
                    entry.is_regular_file() ? entry.file_size() : 0;

                // 添加更多文件信息
                auto status = entry.status();
                if (std::filesystem::is_regular_file(status)) {
                    file_info["type"] = "file";
                } else if (std::filesystem::is_directory(status)) {
                    file_info["type"] = "directory";
                } else {
                    file_info["type"] = "other";
                }

                files.push_back(file_info);
            }

            // 根据 MCP 协议规范，content 应该是一个数组
            // 将目录列表格式化为文本
            std::ostringstream dir_list;
            dir_list << "Directory listing for: " << path << "\n";
            dir_list << "Resolved path: " << resolved_path << "\n";
            dir_list << "Count: " << files.size() << "\n\n";

            for (const auto& file : files) {
                dir_list << (file.value("is_directory", false) ? "[DIR] "
                                                               : "[FILE] ");
                dir_list << file.value("name", "") << " ";
                if (!file.value("is_directory", false)) {
                    dir_list << "(" << file.value("size", 0) << " bytes)";
                }
                dir_list << "\n";
            }

            // 也可以将结构化数据作为 JSON 字符串附加
            dir_list << "\n--- Structured Data (JSON) ---\n";
            dir_list << json{{"files", files},
                             {"path", path},
                             {"resolved_path", resolved_path},
                             {"count", files.size()}}
                            .dump(2);

            result.content =
                json::array({json{{"type", "text"}, {"text", dir_list.str()}}});
            return result;
        } else if (params.name == "create_directory" && enable_write_) {
            std::string path = params.arguments.value("path", "");
            if (path.empty()) {
                result.isError = true;
                result.errorMessage = "Path parameter is required";
                return result;
            }

            std::string resolved_path = resolvePath(path);
            if (!validatePath(resolved_path)) {
                result.isError = true;
                result.errorMessage = "Invalid path: " + path;
                return result;
            }

            std::filesystem::path dir_path(resolved_path);
            if (std::filesystem::exists(dir_path)) {
                result.isError = true;
                result.errorMessage = "Directory already exists: " + path;
                return result;
            }

            if (!std::filesystem::create_directories(dir_path)) {
                result.isError = true;
                result.errorMessage = "Failed to create directory: " + path;
                return result;
            }

            std::string message = "Directory created successfully\n";
            message += "Path: " + path + "\n";
            message += "Resolved path: " + resolved_path;

            result.content =
                json::array({json{{"type", "text"}, {"text", message}}});
            return result;
        } else if (params.name == "delete_file" && enable_write_) {
            std::string path = params.arguments.value("path", "");
            if (path.empty()) {
                result.isError = true;
                result.errorMessage = "Path parameter is required";
                return result;
            }

            std::string resolved_path = resolvePath(path);
            if (!validatePath(resolved_path)) {
                result.isError = true;
                result.errorMessage = "Invalid path: " + path;
                return result;
            }

            std::filesystem::path file_path(resolved_path);
            if (!std::filesystem::exists(file_path)) {
                result.isError = true;
                result.errorMessage =
                    "File or directory does not exist: " + path;
                return result;
            }

            std::error_code ec;
            bool removed = std::filesystem::remove_all(file_path, ec);
            if (!removed || ec) {
                result.isError = true;
                result.errorMessage =
                    "Failed to delete: " + path + " (" + ec.message() + ")";
                return result;
            }

            std::string message = "File or directory deleted successfully\n";
            message += "Path: " + path + "\n";
            message += "Resolved path: " + resolved_path;

            result.content =
                json::array({json{{"type", "text"}, {"text", message}}});
            return result;
        } else if (params.name == "file_info") {
            std::string path = params.arguments.value("path", "");
            if (path.empty()) {
                result.isError = true;
                result.errorMessage = "Path parameter is required";
                return result;
            }

            std::string resolved_path = resolvePath(path);
            if (!validatePath(resolved_path)) {
                result.isError = true;
                result.errorMessage = "Invalid path: " + path;
                return result;
            }

            std::filesystem::path file_path(resolved_path);
            if (!std::filesystem::exists(file_path)) {
                result.isError = true;
                result.errorMessage =
                    "File or directory does not exist: " + path;
                return result;
            }

            auto status = std::filesystem::status(file_path);
            json file_info;
            file_info["path"] = path;
            file_info["resolved_path"] = resolved_path;
            file_info["name"] = file_path.filename().string();
            file_info["exists"] = true;
            file_info["is_directory"] = std::filesystem::is_directory(status);
            file_info["is_regular_file"] =
                std::filesystem::is_regular_file(status);
            file_info["is_symlink"] = std::filesystem::is_symlink(status);

            if (std::filesystem::is_regular_file(status)) {
                file_info["size"] = std::filesystem::file_size(file_path);
                file_info["type"] = "file";
            } else if (std::filesystem::is_directory(status)) {
                file_info["type"] = "directory";
                // 计算目录中的文件数量
                int file_count = 0;
                int dir_count = 0;
                for (const auto& entry :
                     std::filesystem::directory_iterator(file_path)) {
                    if (entry.is_directory()) {
                        dir_count++;
                    } else {
                        file_count++;
                    }
                }
                file_info["file_count"] = file_count;
                file_info["directory_count"] = dir_count;
            } else {
                file_info["type"] = "other";
            }

            // 根据 MCP 协议规范，content 应该是一个数组
            std::string info_text = "File Information:\n";
            info_text += file_info.dump(2);

            result.content =
                json::array({json{{"type", "text"}, {"text", info_text}}});
            return result;
        } else {
            result.isError = true;
            result.errorMessage = "Unknown tool: " + params.name;
            return result;
        }
    } catch (const std::exception& e) {
        result.isError = true;
        result.errorMessage = "Tool execution error: " + std::string(e.what());
        logger->error("Tool call failed: " + result.errorMessage);
        return result;
    }
}

}  // namespace mcp

#pragma once

#include <chrono>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>

// Undefine possible DEBUG macro to avoid conflict with LogLevel::DEBUG
#ifdef DEBUG
#undef DEBUG
#endif
#ifdef INFO
#undef INFO
#endif

namespace mcp_logger {

// Log level enum
enum class LogLevel {
    DEBUG = 0,
    DEBUG_LEVEL = 0,  // Backward compatible alias
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
};

// Log output target
enum class LogTarget {
    CONSOLE = 0,  // Console output
    FILE = 1,     // File output
    BOTH = 2      // Output to both console and file
};

// Log config struct
struct LogConfig {
    LogLevel level = LogLevel::INFO;
    LogTarget target = LogTarget::CONSOLE;
    std::string file_path = "";
    bool enable_timestamp = true;
    bool enable_thread_id = true;
    bool enable_file_location = true;
    bool auto_flush = true;
    size_t max_file_size = 10 * 1024 * 1024;  // 10MB
    int max_files = 5;                        // Keep at most 5 log files
    std::string format =
        "[{timestamp}] [{level}] [{thread_id}] {file_location} - {message}";
};

// Global log manager
class Logger {
public:
    // Singleton
    static Logger& getInstance();

    // Get default standard log format
    static std::string getDefaultStandardFormat();

    // Initialize logging system
    void initialize(const LogConfig& config);

    // Set global config
    void setLevel(LogLevel level);
    void setTarget(LogTarget target);
    void setLogFile(const std::string& file_path);
    void enableTimestamp(bool enable);
    void enableThreadId(bool enable);
    void enableFileLocation(bool enable);
    void setFormat(const std::string& format);
    // Unified log format settings (global)
    void setUnifiedFormat(const std::string& format);

    // Apply default standard format
    void applyDefaultStandardFormat();

    // Logging methods
    void log(LogLevel level, const std::string& message,
             const std::string& file = "", int line = 0,
             const std::string& function = "");

    // Convenience methods
    void debug(const std::string& message, const std::string& file = "",
               int line = 0, const std::string& function = "");
    void info(const std::string& message, const std::string& file = "",
              int line = 0, const std::string& function = "");
    void warning(const std::string& message, const std::string& file = "",
                 int line = 0, const std::string& function = "");
    void error(const std::string& message, const std::string& file = "",
               int line = 0, const std::string& function = "");
    void fatal(const std::string& message, const std::string& file = "",
               int line = 0, const std::string& function = "");

    // Modular logger
    class ModuleLogger {
    public:
        ModuleLogger(const std::string& module_name, Logger* parent);

        void debug(const std::string& message, const std::string& file = "",
                   int line = 0, const std::string& function = "");
        void info(const std::string& message, const std::string& file = "",
                  int line = 0, const std::string& function = "");
        void warning(const std::string& message, const std::string& file = "",
                     int line = 0, const std::string& function = "");
        void error(const std::string& message, const std::string& file = "",
                   int line = 0, const std::string& function = "");
        void fatal(const std::string& message, const std::string& file = "",
                   int line = 0, const std::string& function = "");

    private:
        std::string module_name_;
        Logger* parent_;
    };

    // Get module logger
    std::shared_ptr<ModuleLogger> getModuleLogger(
        const std::string& module_name);

    // Cleanup resources
    void shutdown();

    // Get current config
    LogConfig getConfig() const { return config_; }

private:
    Logger() = default;
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Internal methods
    std::string formatMessage(LogLevel level, const std::string& message,
                              const std::string& file, int line,
                              const std::string& function) const;
    std::string getTimestamp() const;
    std::string getThreadId() const;
    std::string levelToString(LogLevel level) const;
    std::string getFileLocation(const std::string& file, int line,
                                const std::string& function) const;

    // File management
    void rotateLogFile();
    void openLogFile();
    void closeLogFile();

    // Member fields
    mutable std::mutex mutex_;
    LogConfig config_;
    std::ofstream log_file_;
    std::map<std::string, std::shared_ptr<ModuleLogger>> module_loggers_;
    bool initialized_ = false;
    // If set, apply this format when creating ModuleLogger
    std::string unified_format_;
};

// Global convenience macros
#define MCP_LOG_DEBUG(msg)                                           \
    mcp_logger::Logger::getInstance().debug(msg, __FILE__, __LINE__, \
                                            __FUNCTION__)
#define MCP_LOG_INFO(msg)                                           \
    mcp_logger::Logger::getInstance().info(msg, __FILE__, __LINE__, \
                                           __FUNCTION__)
#define MCP_LOG_WARNING(msg)                                           \
    mcp_logger::Logger::getInstance().warning(msg, __FILE__, __LINE__, \
                                              __FUNCTION__)
#define MCP_LOG_ERROR(msg)                                           \
    mcp_logger::Logger::getInstance().error(msg, __FILE__, __LINE__, \
                                            __FUNCTION__)
#define MCP_LOG_FATAL(msg)                                           \
    mcp_logger::Logger::getInstance().fatal(msg, __FILE__, __LINE__, \
                                            __FUNCTION__)

// Modular logging macros
#define MCP_MODULE_LOG_DEBUG(module, msg)                             \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->debug( \
        msg, __FILE__, __LINE__, __FUNCTION__)
#define MCP_MODULE_LOG_INFO(module, msg)                             \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->info( \
        msg, __FILE__, __LINE__, __FUNCTION__)
#define MCP_MODULE_LOG_WARNING(module, msg)                             \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->warning( \
        msg, __FILE__, __LINE__, __FUNCTION__)
#define MCP_MODULE_LOG_ERROR(module, msg)                             \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->error( \
        msg, __FILE__, __LINE__, __FUNCTION__)
#define MCP_MODULE_LOG_FATAL(module, msg)                             \
    mcp_logger::Logger::getInstance().getModuleLogger(module)->fatal( \
        msg, __FILE__, __LINE__, __FUNCTION__)

// Global convenience functions
// Get module logger (supports module name parameter)
std::shared_ptr<Logger::ModuleLogger> getModuleLogger(
    const std::string& module_name);

// Initialize logging system (convenience)
void initializeLogger(const LogConfig& config);

}  // namespace mcp_logger

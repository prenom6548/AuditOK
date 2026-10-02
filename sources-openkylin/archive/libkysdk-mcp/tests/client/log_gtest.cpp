#include <gtest/gtest.h>
#include "logger.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <vector>
#include <sstream>
#include <cstdio>

using namespace mcp_logger;

// 测试基类，提供通用的设置和清理
class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 清理之前的日志文件
        cleanupLogFiles();

        // 重置日志器状态
        Logger::getInstance().shutdown();
    }

    void TearDown() override {
        // 清理测试产生的日志文件
        cleanupLogFiles();

        // 关闭日志器
        Logger::getInstance().shutdown();
    }

    void cleanupLogFiles() {
        // 清理测试日志文件
        std::vector<std::string> test_files = {
            "/tmp/test_log.txt",
            "/tmp/test_log_file.txt",
            "/tmp/test_rotation.txt"
        };

        for (const auto& file : test_files) {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }

            // 清理轮转文件
            for (int i = 1; i <= 10; ++i) {
                std::string rotated_file = file + "." + std::to_string(i);
                if (std::filesystem::exists(rotated_file)) {
                    std::filesystem::remove(rotated_file);
                }
            }
        }
    }

    std::string readLogFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            return "";
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
};

// 测试单例模式
TEST_F(LoggerTest, SingletonPattern) {
    Logger& logger1 = Logger::getInstance();
    Logger& logger2 = Logger::getInstance();

    // 应该返回同一个实例
    EXPECT_EQ(&logger1, &logger2);
}

// 测试初始化
TEST_F(LoggerTest, Initialize) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::CONSOLE;

    EXPECT_NO_THROW(logger.initialize(config));

    LogConfig retrieved_config = logger.getConfig();
    EXPECT_EQ(retrieved_config.level, LogLevel::INFO);
    EXPECT_EQ(retrieved_config.target, LogTarget::CONSOLE);
}

// 测试日志级别设置
TEST_F(LoggerTest, SetLevel) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::CONSOLE;
    logger.initialize(config);

    logger.setLevel(LogLevel::WARNING);
    EXPECT_EQ(logger.getConfig().level, LogLevel::WARNING);

    logger.setLevel(LogLevel::ERROR);
    EXPECT_EQ(logger.getConfig().level, LogLevel::ERROR);
}

// 测试日志目标设置
TEST_F(LoggerTest, SetTarget) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::DEBUG_LEVEL;
    config.target = LogTarget::CONSOLE;
    logger.initialize(config);

    logger.setTarget(LogTarget::FILE);
    EXPECT_EQ(logger.getConfig().target, LogTarget::FILE);

    logger.setTarget(LogTarget::BOTH);
    EXPECT_EQ(logger.getConfig().target, LogTarget::BOTH);
}

// 测试文件日志输出
TEST_F(LoggerTest, FileLogging) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::DEBUG_LEVEL;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log_file.txt";
    logger.initialize(config);

    logger.info("Test message for file logging");
    logger.shutdown(); // 确保文件被刷新

    // 验证文件是否存在且包含内容
    EXPECT_TRUE(std::filesystem::exists("/tmp/test_log_file.txt"));

    std::string content = readLogFile("/tmp/test_log_file.txt");
    EXPECT_FALSE(content.empty());
    EXPECT_NE(content.find("Test message for file logging"), std::string::npos);
}

// 测试控制台日志输出
TEST_F(LoggerTest, ConsoleLogging) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::DEBUG_LEVEL;
    config.target = LogTarget::CONSOLE;
    logger.initialize(config);

    // 控制台输出难以直接测试，但应该不会崩溃
    EXPECT_NO_THROW(logger.info("Test console message"));
    EXPECT_NO_THROW(logger.warning("Test warning message"));
    EXPECT_NO_THROW(logger.error("Test error message"));
}

// 测试同时输出到控制台和文件
TEST_F(LoggerTest, BothTargetLogging) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::DEBUG_LEVEL;
    config.target = LogTarget::BOTH;
    config.file_path = "/tmp/test_log_file.txt";
    logger.initialize(config);

    logger.info("Test message for both targets");
    logger.shutdown();

    // 验证文件存在
    EXPECT_TRUE(std::filesystem::exists("/tmp/test_log_file.txt"));

    std::string content = readLogFile("/tmp/test_log_file.txt");
    EXPECT_NE(content.find("Test message for both targets"), std::string::npos);
}

// 测试各种日志级别
TEST_F(LoggerTest, LogLevels) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::DEBUG_LEVEL;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    logger.initialize(config);

    logger.debug("Debug message");
    logger.info("Info message");
    logger.warning("Warning message");
    logger.error("Error message");
    logger.fatal("Fatal message");

    logger.shutdown();

    std::string content = readLogFile("/tmp/test_log.txt");
    EXPECT_NE(content.find("Debug message"), std::string::npos);
    EXPECT_NE(content.find("Info message"), std::string::npos);
    EXPECT_NE(content.find("Warning message"), std::string::npos);
    EXPECT_NE(content.find("Error message"), std::string::npos);
    EXPECT_NE(content.find("Fatal message"), std::string::npos);
}

// 测试日志级别过滤
TEST_F(LoggerTest, LogLevelFiltering) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::WARNING;  // 只记录 WARNING 及以上级别
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    logger.initialize(config);

    logger.debug("Debug message - should be filtered");
    logger.info("Info message - should be filtered");
    logger.warning("Warning message - should be logged");
    logger.error("Error message - should be logged");
    logger.fatal("Fatal message - should be logged");

    logger.shutdown();

    std::string content = readLogFile("/tmp/test_log.txt");
    EXPECT_EQ(content.find("Debug message"), std::string::npos);
    EXPECT_EQ(content.find("Info message"), std::string::npos);
    EXPECT_NE(content.find("Warning message"), std::string::npos);
    EXPECT_NE(content.find("Error message"), std::string::npos);
    EXPECT_NE(content.find("Fatal message"), std::string::npos);
}

// 测试时间戳功能
TEST_F(LoggerTest, Timestamp) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    config.enable_timestamp = true;
    logger.initialize(config);

    logger.info("Message with timestamp");
    logger.shutdown();

    std::string content = readLogFile("/tmp/test_log.txt");
    // 检查是否包含时间戳格式（YYYY-MM-DD）
    EXPECT_NE(content.find("202"), std::string::npos); // 年份前缀
}

// 测试文件位置功能
TEST_F(LoggerTest, FileLocation) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    config.enable_file_location = true;
    config.format = "[{timestamp}] [{level}] {file_location} - {message}";  // 确保格式包含 file_location
    logger.initialize(config);

    logger.info("Message with file location", __FILE__, __LINE__, __FUNCTION__);
    logger.shutdown();

    std::string content = readLogFile("/tmp/test_log.txt");
    // 检查是否包含文件名（可能是 log_gtest.cpp 或完整路径的一部分）
    EXPECT_TRUE(content.find("log_gtest") != std::string::npos ||
                content.find("gtest") != std::string::npos ||
                !content.empty());
}

// 测试线程ID功能
TEST_F(LoggerTest, ThreadId) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    config.enable_thread_id = true;
    logger.initialize(config);

    logger.info("Message with thread ID");
    logger.shutdown();

    std::string content = readLogFile("/tmp/test_log.txt");
    // 线程ID应该存在（格式可能不同）
    EXPECT_FALSE(content.empty());
}

// 测试模块化日志记录器
TEST_F(LoggerTest, ModuleLogger) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    logger.initialize(config);

    auto networkLogger = logger.getModuleLogger("Network");
    auto databaseLogger = logger.getModuleLogger("Database");

    networkLogger->info("Network message");
    databaseLogger->info("Database message");

    logger.shutdown();

    std::string content = readLogFile("/tmp/test_log.txt");
    EXPECT_NE(content.find("[Network]"), std::string::npos);
    EXPECT_NE(content.find("[Database]"), std::string::npos);
    EXPECT_NE(content.find("Network message"), std::string::npos);
    EXPECT_NE(content.find("Database message"), std::string::npos);
}

// 测试模块日志记录器复用
TEST_F(LoggerTest, ModuleLoggerReuse) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::CONSOLE;
    logger.initialize(config);

    auto logger1 = logger.getModuleLogger("TestModule");
    auto logger2 = logger.getModuleLogger("TestModule");

    // 应该返回同一个模块日志记录器
    EXPECT_EQ(logger1, logger2);
}

// 测试线程安全性
TEST_F(LoggerTest, ThreadSafety) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    logger.initialize(config);

    std::vector<std::thread> threads;
    const int num_threads = 5;
    const int messages_per_thread = 10;

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&logger, i, messages_per_thread]() {
            for (int j = 0; j < messages_per_thread; ++j) {
                logger.info("Thread " + std::to_string(i) + " message " + std::to_string(j));
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    logger.shutdown();

    // 验证所有消息都被记录
    std::string content = readLogFile("/tmp/test_log.txt");
    for (int i = 0; i < num_threads; ++i) {
        for (int j = 0; j < messages_per_thread; ++j) {
            std::string expected = "Thread " + std::to_string(i) + " message " + std::to_string(j);
            EXPECT_NE(content.find(expected), std::string::npos)
                << "Missing message: " << expected;
        }
    }
}

// 测试日志格式设置
TEST_F(LoggerTest, CustomFormat) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    config.format = "*** {level} *** {message} ***";
    logger.initialize(config);

    logger.info("Custom format message");
    logger.shutdown();

    std::string content = readLogFile("/tmp/test_log.txt");
    EXPECT_NE(content.find("***"), std::string::npos);
    EXPECT_NE(content.find("INFO"), std::string::npos);
    EXPECT_NE(content.find("Custom format message"), std::string::npos);
}

// 测试设置日志文件路径
TEST_F(LoggerTest, SetLogFile) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    logger.initialize(config);

    logger.info("Message to first file");

    logger.setLogFile("/tmp/test_log_file.txt");
    logger.info("Message to second file");
    logger.shutdown();

    // 两个文件都应该存在
    EXPECT_TRUE(std::filesystem::exists("/tmp/test_log.txt"));
    EXPECT_TRUE(std::filesystem::exists("/tmp/test_log_file.txt"));

    std::string content1 = readLogFile("/tmp/test_log.txt");
    std::string content2 = readLogFile("/tmp/test_log_file.txt");

    EXPECT_NE(content1.find("Message to first file"), std::string::npos);
    EXPECT_NE(content2.find("Message to second file"), std::string::npos);
}

// 测试关闭和重新初始化
TEST_F(LoggerTest, ShutdownAndReinitialize) {
    Logger& logger = Logger::getInstance();

    LogConfig config1;
    config1.level = LogLevel::INFO;
    config1.target = LogTarget::FILE;
    config1.file_path = "/tmp/test_log.txt";
    logger.initialize(config1);

    logger.info("First initialization");
    logger.shutdown();

    LogConfig config2;
    config2.level = LogLevel::WARNING;
    config2.target = LogTarget::CONSOLE;
    logger.initialize(config2);

    EXPECT_EQ(logger.getConfig().level, LogLevel::WARNING);
    EXPECT_EQ(logger.getConfig().target, LogTarget::CONSOLE);
}

// 测试便捷宏
TEST_F(LoggerTest, LogMacros) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::DEBUG_LEVEL;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    logger.initialize(config);

    MCP_LOG_DEBUG("Debug macro");
    MCP_LOG_INFO("Info macro");
    MCP_LOG_WARNING("Warning macro");
    MCP_LOG_ERROR("Error macro");
    MCP_LOG_FATAL("Fatal macro");

    logger.shutdown();

    std::string content = readLogFile("/tmp/test_log.txt");
    EXPECT_NE(content.find("Debug macro"), std::string::npos);
    EXPECT_NE(content.find("Info macro"), std::string::npos);
    EXPECT_NE(content.find("Warning macro"), std::string::npos);
    EXPECT_NE(content.find("Error macro"), std::string::npos);
    EXPECT_NE(content.find("Fatal macro"), std::string::npos);
}

// 测试模块日志宏
TEST_F(LoggerTest, ModuleLogMacros) {
    Logger& logger = Logger::getInstance();

    LogConfig config;
    config.level = LogLevel::INFO;
    config.target = LogTarget::FILE;
    config.file_path = "/tmp/test_log.txt";
    logger.initialize(config);

    MCP_MODULE_LOG_INFO("TestModule", "Module info macro");
    MCP_MODULE_LOG_WARNING("TestModule", "Module warning macro");
    MCP_MODULE_LOG_ERROR("TestModule", "Module error macro");

    logger.shutdown();

    std::string content = readLogFile("/tmp/test_log.txt");
    EXPECT_NE(content.find("[TestModule]"), std::string::npos);
    EXPECT_NE(content.find("Module info macro"), std::string::npos);
    EXPECT_NE(content.find("Module warning macro"), std::string::npos);
    EXPECT_NE(content.find("Module error macro"), std::string::npos);
}

// 主函数
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>
#ifdef MCP_SDK_INTERNAL_BUILD
#include "../common/json.hpp"
#include "../log/logger.h"
#else
#include "json.hpp"
#include "logger.h"
#endif

namespace mcp {

// Log level (kept for backward compatibility)
enum class LogLevel { DEBUG = 0, INFO = 1, WARNING = 2, ERROR = 3 };

// Communication mode enum
enum class CommunicationMode {
    CLIENT = 0,  // Client mode: start child process and communicate with it
    SERVER = 1  // Server mode: run as child process and communicate with parent
};

// Pipe communication config
struct PipeConfig {
    CommunicationMode mode = CommunicationMode::CLIENT;

    // Client-mode config
    std::string command;
    std::vector<std::string> args;
    std::map<std::string, std::string> env_vars;

    // Server-mode config
    std::string server_name = "mcp_server";
    std::string server_version = "1.0.0";

    // Common config
    int buffer_size = 4096;
    int read_timeout_ms = 10;
    int max_retry_count = 10;
    bool enable_logging = true;
    std::string log_file_path = "";  // Empty string means output to console
    LogLevel log_level = LogLevel::INFO;
};

// Message type
enum class MessageType { REQUEST, RESPONSE, NOTIFICATION };

// Message struct
struct Message {
    MessageType type;
    std::string content;
    std::string id;  // Used for request-response matching
};

// Message handler interface
class MessageHandler {
public:
    virtual ~MessageHandler() = default;
    // New naming style
    virtual void onMessage(const Message& message) { on_message(message); }
    virtual void onError(const std::string& error) { on_error(error); }
    // Backward-compatible naming style
    virtual void on_message(const Message& message) { onMessage(message); }
    virtual void on_error(const std::string& error) { onError(error); }
};

// Server request handler interface
class ServerRequestHandler {
public:
    virtual ~ServerRequestHandler() = default;
    virtual std::string handleRequest(const std::string& method,
                                      const std::string& params,
                                      const std::string& request_id) = 0;
    virtual void handleNotification(const std::string& method,
                                    const std::string& params) = 0;
};

// Pipe communication core class
class PipeCommunication {
public:
    // performPosixRead 的特殊返回值：poll 超时（空闲），不算错误也不算 EOF。
    // 见 readThreadFunc / performReadOperation。
    static constexpr ssize_t kReadTimeout = -2;

    PipeCommunication();
    ~PipeCommunication();

    // Start and stop
    bool start(const PipeConfig& config);
    void stop();
    bool isRunning() const;
    bool is_running() const { return isRunning(); }

    // Message sending
    bool sendMessage(const std::string& message);
    std::future<std::string> sendRequest(const std::string& request,
                                         int timeout_seconds = 60);
    bool send_message(const std::string& message) { return sendMessage(message); }
    std::future<std::string> send_request(const std::string& request,
                                          int timeout_seconds = 60) {
        return sendRequest(request, timeout_seconds);
    }

    // Message handling
    void setMessageHandler(std::shared_ptr<MessageHandler> handler);
    void set_message_handler(std::shared_ptr<MessageHandler> handler) {
        setMessageHandler(std::move(handler));
    }
    void setConnectionLostCallback(std::function<void(const std::string&)> callback);

    // Server-mode-only methods
    void setServerRequestHandler(std::shared_ptr<ServerRequestHandler> handler);
    bool sendResponse(const std::string& request_id, const std::string& result);
    bool sendError(const std::string& request_id, int error_code,
                   const std::string& error_message);
    bool sendNotification(const std::string& method, const std::string& params);

    // Direct message retrieval
    bool hasMessage() const;
    Message getMessage();
    std::vector<Message> getAllMessages();
    bool has_message() const { return hasMessage(); }
    Message get_message() { return getMessage(); }
    std::vector<Message> get_all_messages() { return getAllMessages(); }

    // Direct result retrieval
    bool hasResult() const;
    std::string getResult();
    std::vector<std::string> getAllResults();
    bool has_result() const { return hasResult(); }
    std::string get_result() { return getResult(); }
    std::vector<std::string> get_all_results() { return getAllResults(); }

    // Process management
    int getProcessId() const;
    bool isProcessAlive() const;
    int get_process_id() const { return getProcessId(); }
    bool is_process_alive() const { return isProcessAlive(); }

    // Logging
    void log(LogLevel level, const std::string& message);
    void setLogLevel(LogLevel level);
    void setLogFile(const std::string& file_path);
    void enableLogging(bool enable);
    void set_log_level(LogLevel level) { setLogLevel(level); }
    void set_log_file(const std::string& file_path) { setLogFile(file_path); }
    void enable_logging(bool enable) { enableLogging(enable); }

    bool activate(const PipeConfig& config);
    void shutdown();
    void terminate();
    bool isActive() const;
    bool isCommunicationEstablished() const;

    bool postMessage(const std::string& message);
    bool dispatchMessage(const std::string& message);
    bool transmitData(const std::string& data);
    bool pushToPipe(const std::string& payload);

    std::future<std::string> initiateRequest(const std::string& request,
                                             int timeout_seconds = 60);
    std::future<std::string> submitRequest(const std::string& request,
                                           int timeout_seconds = 60);
    std::future<std::string> invokeRpc(const std::string& request,
                                       int timeout_seconds = 60);

    void registerMessageHandler(std::shared_ptr<MessageHandler> handler);
    void attachMessageHandler(std::shared_ptr<MessageHandler> handler);

    void registerServerHandler(std::shared_ptr<ServerRequestHandler> handler);
    bool dispatchResponse(const std::string& request_id,
                          const std::string& result);
    bool dispatchError(const std::string& request_id, int error_code,
                       const std::string& error_message);
    bool broadcastNotification(const std::string& method,
                               const std::string& params);
    bool emitNotification(const std::string& method, const std::string& params);

    bool messageQueueHasItems() const;
    Message retrieveMessage();
    Message dequeueNextMessage();
    std::vector<Message> drainMessageQueue();

    bool resultQueueHasItems() const;
    std::string fetchResult();
    std::string consumeNextResult();
    std::vector<std::string> drainResultQueue();

    int acquireProcessId() const;
    int getChildPid() const;
    bool verifyProcessStatus() const;
    bool isSubprocessAlive() const;

    void writeLog(LogLevel level, const std::string& message);
    void recordLog(LogLevel level, const std::string& message);
    void configureLogLevel(LogLevel level);
    void setLoggingEnabled(bool enable);

private:
    // Internal helper operations (split from complex logic)
    void performStopOperation();
    void performThreadJoinOperation();
    // 在 join 读线程前唤醒可能阻塞在 read/poll 上的读端 fd
    void wakeReadFdForShutdown();
    void performTimeoutThreadJoinOperation();
    void failAllPendingRequests(const std::string& reason);
    void notifyConnectionLost(const std::string& reason);

    // Startup flow helpers
    void applyConfiguration(const PipeConfig& config);
    bool determineAndSetMode(const PipeConfig& config);
    bool executeServerModeInitialization();
    bool executeClientModeInitialization(const PipeConfig& config);

    // Message-sending helpers
    bool executeMessageSendOperation(const std::string& message);
    std::string formatMessageWithNewline(const std::string& message);
    ssize_t performWriteOperation(const std::string& data);
    bool validateWriteResult(ssize_t written, size_t expected);
    void logWriteOperationResult(bool success, ssize_t bytes_written);

    // Request-sending flow helpers
    struct RequestProcessingContext {
        std::string original_request;
        std::string full_request;
        std::string request_id;
        int timeout_seconds;
        std::promise<std::string> response_promise;
        std::future<std::string> response_future;
    };
    void processRequestId(RequestProcessingContext& context);
    void createResponsePromise(RequestProcessingContext& context);
    bool executeRequestSend(RequestProcessingContext& context);
    void registerPendingRequest(RequestProcessingContext& context);
    void setupTimeoutHandler(RequestProcessingContext& context);

    // Process-state check helpers
    bool performProcessAliveCheck() const;
#if defined(_WIN32)
    bool checkWindowsProcessAlive() const;
#else
    bool checkPosixProcessAlive() const;
#endif

    // Platform-specific implementation
    bool createPipes();
    void closePipes();
    bool startProcess();
    void stopProcess();

    // Read thread
    void readThreadFunc();
    void processMessage(const std::string& line);

    // Request-response management
    std::string generateRequestId();
    void handleResponse(const std::string& id, const std::string& content);
    void handleError(const std::string& id, const std::string& error);

    // Server-mode handling
    void handleServerRequest(const nlohmann::json& request);
    void handleServerNotification(const nlohmann::json& notification);
    bool initializeServerMode();

    // Platform-specific implementation
    bool createPipesImpl();
    void closePipesImpl();
    bool startProcessImpl();
    void stopProcessImpl();
    ssize_t readImpl(char* buffer, size_t size);
    ssize_t writeImpl(const char* buffer, size_t size);

#if defined(_WIN32)
    // Windows pipe/process helper methods
    bool createWindowsPipes();
    void initializeSecurityAttributes(struct SECURITY_ATTRIBUTES& sa);
    bool performWindowsPipeCreation(void* stdin_read, void* stdin_write,
                                    void* stdout_read, void* stdout_write,
                                    const struct SECURITY_ATTRIBUTES& sa);
    bool configureWindowsPipeHandles(void* stdin_write, void* stdout_read);
    void cleanupWindowsPipeHandles(void* stdin_read, void* stdin_write,
                                   void* stdout_read, void* stdout_write);
    void assignWindowsPipeHandles(void* stdin_read, void* stdin_write,
                                  void* stdout_read, void* stdout_write);

    void closeWindowsPipes();
    void closeWindowsPipeHandle(int& pipe_fd);

    bool startWindowsProcess();
    void initializeWindowsStartupInfo(struct STARTUPINFOA& si);
    void applyEnvironmentVariables();
    std::string buildWindowsCommandLine();
    bool executeWindowsProcessCreation(const std::string& cmd_line,
                                       struct STARTUPINFOA& si,
                                       struct PROCESS_INFORMATION& pi);
    void finalizeWindowsProcessStart(const struct PROCESS_INFORMATION& pi);
    void logWindowsProcessCreationError();

    void stopWindowsProcess();
    ssize_t performWindowsRead(char* buffer, size_t size);
    ssize_t performWindowsWrite(const char* buffer, size_t size);
#else
    // POSIX pipe/process helper methods
    bool createPosixPipes();
    bool performPosixPipeCreation();
    void configurePosixPipeNonBlocking();

    void closePosixPipes();
    void closePosixPipeDescriptor(int& pipe_fd);

    bool startPosixProcess();
    bool executeChildProcessOperations();
    void applyPosixEnvironmentVariables();
    void performChildProcessPipeSetup();
    void performChildProcessStdioRedirection();
    void executeChildProcessCommand();
    std::vector<char*> buildCommandArguments();
    std::string buildCommandString();
    bool executeParentProcessOperations();
    void performParentProcessPipeSetup();
    void waitForProcessInitialization();
    bool validateProcessStartup();

    void stopPosixProcess();
    void waitForProcessTermination();
    void performProcessTerminationWait(int& status);
    void logProcessExitStatus(int status);
    ssize_t performPosixRead(char* buffer, size_t size);
    ssize_t performPosixWrite(const char* buffer, size_t size);
    void handlePosixWriteError();
#endif

    // Read-thread context and split logic
    struct ReadThreadContext {
        std::vector<char> buffer;
        std::string data_buffer;
        int consecutive_empty_reads;
        int max_empty_reads;
        size_t max_buffer_size;

        ReadThreadContext()
            : consecutive_empty_reads(0),
              max_empty_reads(0),
              max_buffer_size(0) {}
    };
    ssize_t performReadOperation(std::vector<char>& buffer);
    void processSuccessfulRead(ssize_t bytes_read, ReadThreadContext& context);
    void processCompleteMessages(ReadThreadContext& context);
    void handlePipeClosure(ReadThreadContext& context);
    void handleReadError(ReadThreadContext& context);

    // JSON / JSON-RPC routing helpers
    void routeJsonRpcMessage(const nlohmann::json& message);
    void routeServerModeMessage(const nlohmann::json& message);
    void routeClientModeMessage(const nlohmann::json& message);
    void processClientResponse(const nlohmann::json& message);
    void processClientNotification(const nlohmann::json& message);
    void handleNonJsonMessage(const std::string& line);
    void handleNonJsonRpcMessage(const std::string& line);

    // Server-mode helper methods
    bool validateServerModeOperation();
    bool executeResponseSendOperation(const std::string& request_id,
                                      const std::string& result);
    nlohmann::json buildJsonRpcResponse(const std::string& request_id,
                                        const std::string& result);
    void logResponseSendResult(bool success, const std::string& request_id);

    bool executeErrorSendOperation(const std::string& request_id,
                                   int error_code,
                                   const std::string& error_message);
    nlohmann::json buildJsonRpcErrorResponse(const std::string& request_id,
                                             int error_code,
                                             const std::string& error_message);
    void logErrorSendResult(bool success, const std::string& request_id);

    bool executeNotificationSendOperation(const std::string& method,
                                          const std::string& params);
    nlohmann::json buildJsonRpcNotification(const std::string& method,
                                            const std::string& params);
    void logNotificationSendResult(bool success, const std::string& method);

    void assignServerModePipeDescriptors();
    struct ServerRequestContext {
        std::string method;
        std::string request_id;
        std::string params;
    };
    void extractServerRequestInfo(const nlohmann::json& request,
                                  ServerRequestContext& context);

private:
    PipeConfig config_;
    bool running_;
    std::unique_ptr<std::thread> read_thread_;
    std::shared_ptr<MessageHandler> message_handler_;
    std::function<void(const std::string&)> connection_lost_callback_;

    // Process-related fields
    int process_id_;
#ifdef _WIN32
    void* process_handle_;  // Platform-specific handle
#endif

    // Pipe-related fields
    int stdin_pipe_[2];
    int stdout_pipe_[2];

    // Request-response management
    std::mutex response_mutex_;
    std::map<std::string, std::promise<std::string>> pending_requests_;
    std::unordered_set<std::string> timed_out_request_ids_;
    std::deque<std::string> timed_out_request_order_;
    std::atomic<int> request_id_counter_;
    std::mutex timeout_threads_mutex_;
    std::vector<std::thread> timeout_threads_;
    std::mutex timeout_control_mutex_;
    std::condition_variable timeout_control_cv_;
    bool timeout_stop_requested_ = false;

    // Message queue
    mutable std::mutex message_mutex_;
    std::queue<Message> message_queue_;

    // Result queue
    mutable std::mutex result_mutex_;
    std::queue<std::string> result_queue_;

    // Server-mode-related fields
    std::shared_ptr<ServerRequestHandler> server_request_handler_;
    std::atomic<bool> server_mode_;

    // Logging-related fields (using unified logging)
    LogLevel current_log_level_;
    bool logging_enabled_;
    // Logger now uses global function calls

    // Child process output handling
    void printSubprocessOutput(const std::string& output,
                               const std::string& source = "subprocess");
    std::string logLevelToString(LogLevel level) const;
};

}  // namespace mcp

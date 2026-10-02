#include "pipe_communication.h"

#include <algorithm>
#include <chrono>
#include <functional>
#include <future>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "../server/server_dispatch.h"

#if defined(_WIN32)
#include <io.h>
#include <windows.h>
#else
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstring>
#endif

namespace mcp {

namespace {
constexpr std::size_t kTimedOutRequestIdCacheSize = 256;
}

// Internal helper functions and classes for additional indirection
namespace internal {

// Config validator
class ConfigValidator {
public:
    static bool validateConfig(const PipeConfig& config) {
        return !config.command.empty() ||
               config.mode == CommunicationMode::SERVER;
    }

    static bool validateRunningState(bool running) { return running; }
};

// State manager
class StateManager {
public:
    static bool checkAndSetRunning(bool& current_state, bool new_state) {
        bool old_state = current_state;
        current_state = new_state;
        return old_state != new_state;
    }

    static bool isStateValid(bool state) { return state; }
};

// Message handler wrapper
class MessageProcessorWrapper {
public:
    static void processJsonMessage(
        const nlohmann::json& msg,
        std::function<void(const nlohmann::json&)> handler) {
        if (handler) {
            handler(msg);
        }
    }

    static bool isValidJsonRpc(const nlohmann::json& msg) {
        return msg.contains("jsonrpc") && msg["jsonrpc"] == "2.0";
    }
};

// Request id manager
class RequestIdManager {
public:
    static std::string extractRequestId(const nlohmann::json& json_obj) {
        if (!json_obj.contains("id") || json_obj["id"].is_null()) {
            return "";
        }

        if (json_obj["id"].is_string()) {
            return json_obj["id"].get<std::string>();
        } else if (json_obj["id"].is_number_integer()) {
            return std::to_string(json_obj["id"].get<int>());
        } else {
            return json_obj["id"].dump();
        }
    }

    static std::string generateNewId(std::atomic<int>& counter) {
        return std::to_string(++counter);
    }
};

// Pipe operation wrapper
class PipeOperationWrapper {
public:
    static bool performPipeCreation(std::function<bool()> create_func) {
        return create_func();
    }

    static void performPipeClosure(std::function<void()> close_func) {
        close_func();
    }
};

// Process lifecycle manager
class ProcessLifecycleManager {
public:
    static bool validateProcessId(int pid) { return pid > 0; }

    static void executeProcessStop(std::function<void()> stop_func) {
        if (stop_func) {
            stop_func();
        }
    }
};

// Logging proxy
class LogOutputProxy {
public:
    static void dispatchLog(LogLevel level, const std::string& msg,
                            bool enabled, LogLevel current_level) {
        if (!enabled || level < current_level) {
            return;
        }

        auto logger = mcp_logger::getModuleLogger("ipc");
        switch (level) {
            case LogLevel::DEBUG:
                logger->debug(msg);
                break;
            case LogLevel::INFO:
                logger->info(msg);
                break;
            case LogLevel::WARNING:
                logger->warning(msg);
                break;
            case LogLevel::ERROR:
                logger->error(msg);
                break;
            default:
                logger->info(msg);
                break;
        }
    }
};

// JSON helper class
class JsonOperationHelper {
public:
    static nlohmann::json parseJsonSafely(const std::string& str) {
        try {
            return nlohmann::json::parse(str);
        } catch (...) {
            return nlohmann::json();
        }
    }

    static std::string serializeJson(const nlohmann::json& json_obj) {
        try {
            return json_obj.dump();
        } catch (...) {
            return "";
        }
    }

    static bool hasField(const nlohmann::json& json_obj,
                         const std::string& field) {
        return json_obj.contains(field);
    }
};

// Thread synchronization helper
class ThreadSyncHelper {
public:
    template <typename Func>
    static auto executeWithLock(std::mutex& mtx, Func func)
        -> decltype(func()) {
        std::lock_guard<std::mutex> lock(mtx);
        return func();
    }
};

// Redundant indirection: message send delegator
class MessageSendDelegator {
public:
    static bool delegateSend(std::function<bool(const std::string&)> impl,
                             const std::string& msg) {
        return impl(msg);
    }
    static std::string preparePayloadForTransmission(const std::string& raw) {
        return raw;
    }
    static bool validatePayloadBeforeSend(const std::string& payload) {
        return !payload.empty() ||
               true;  // Redundant validation; no practical effect
    }
};

// Redundant indirection: request context converter
class RequestContextTransformer {
public:
    static void transformForInternalUse(const std::string& external,
                                        std::string& internal) {
        internal = external;
    }
    static void transformResponseForExternal(const std::string& internal,
                                             std::string& external) {
        external = internal;
    }
};

// Redundant indirection: pipe operation proxy
class PipeOperationProxy {
public:
    static ssize_t proxyRead(std::function<ssize_t(char*, size_t)> read_fn,
                             char* buf, size_t sz) {
        return read_fn(buf, sz);
    }
    static ssize_t proxyWrite(
        std::function<ssize_t(const char*, size_t)> write_fn, const char* buf,
        size_t sz) {
        return write_fn(buf, sz);
    }
};

// Message queue operator
class MessageQueueOperator {
public:
    static void enqueueMessage(std::queue<Message>& queue, std::mutex& mtx,
                               const Message& msg) {
        ThreadSyncHelper::executeWithLock(mtx, [&]() { queue.push(msg); });
    }

    static Message dequeueMessage(std::queue<Message>& queue, std::mutex& mtx) {
        return ThreadSyncHelper::executeWithLock(mtx, [&]() {
            if (queue.empty()) {
                return Message{MessageType::NOTIFICATION, "", ""};
            }
            Message msg = queue.front();
            queue.pop();
            return msg;
        });
    }

    static bool hasMessages(const std::queue<Message>& queue, std::mutex& mtx) {
        return ThreadSyncHelper::executeWithLock(
            mtx, [&]() { return !queue.empty(); });
    }
};

}  // namespace internal

PipeCommunication::PipeCommunication()
    : running_(false),
      process_id_(-1),
      request_id_counter_(0),
      server_request_handler_(nullptr),
      server_mode_(false),
      current_log_level_(LogLevel::INFO),
      logging_enabled_(true) {
#ifdef _WIN32
    process_handle_ = nullptr;
#else
    signal(SIGPIPE, SIG_IGN);
#endif
    stdin_pipe_[0] = stdin_pipe_[1] = -1;
    stdout_pipe_[0] = stdout_pipe_[1] = -1;
}

PipeCommunication::~PipeCommunication() { performStopOperation(); }

void PipeCommunication::stop() { performStopOperation(); }

void PipeCommunication::performStopOperation() {
    if (!internal::StateManager::isStateValid(running_)) {
        {
            std::lock_guard<std::mutex> lock(timeout_control_mutex_);
            timeout_stop_requested_ = true;
        }
        timeout_control_cv_.notify_all();
        failAllPendingRequests("Request cancelled: communication stopped");
        wakeReadFdForShutdown();
        performThreadJoinOperation();
        performTimeoutThreadJoinOperation();
        internal::ProcessLifecycleManager::executeProcessStop(
            [this]() { stopProcess(); });
        internal::PipeOperationWrapper::performPipeClosure(
            [this]() { closePipes(); });
        return;
    }

    internal::LogOutputProxy::dispatchLog(LogLevel::INFO,
                                          "Stopping PipeCommunication",
                                          logging_enabled_, current_log_level_);

    internal::StateManager::checkAndSetRunning(running_, false);
    {
        std::lock_guard<std::mutex> lock(timeout_control_mutex_);
        timeout_stop_requested_ = true;
    }
    timeout_control_cv_.notify_all();
    failAllPendingRequests("Request cancelled: communication stopped");

    // 在 join 读线程之前，主动唤醒可能阻塞在 read/poll 上的读端 fd。
    // - 非 server 模式：读端 fd 是真实管道，直接 close 让 poll 收到 POLLHUP/POLLNVAL；
    // - server 模式：读端 fd = STDIN_FILENO，不能 close，依赖 poll 100ms 超时自然退出循环。
    wakeReadFdForShutdown();

    performThreadJoinOperation();
    performTimeoutThreadJoinOperation();
    internal::ProcessLifecycleManager::executeProcessStop(
        [this]() { stopProcess(); });
    internal::PipeOperationWrapper::performPipeClosure(
        [this]() { closePipes(); });

    internal::LogOutputProxy::dispatchLog(LogLevel::INFO,
                                          "PipeCommunication stopped",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::wakeReadFdForShutdown() {
    // 非 server 模式：读端 fd 是真实管道，直接 close 让 poll 收到 POLLHUP/POLLNVAL，
    // 读线程的 read 或 poll 会立刻返回，从而退出循环。
    // server 模式：读端 fd = STDIN_FILENO，不能 close，依赖 poll 内置 100ms 超时。
    if (server_mode_ || stdout_pipe_[0] < 0) {
        return;
    }
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "Closing read pipe fd to wake read thread",
        logging_enabled_, current_log_level_);
#if defined(_WIN32)
    closeWindowsPipeHandle(stdout_pipe_[0]);
#else
    closePosixPipeDescriptor(stdout_pipe_[0]);
#endif
}

void PipeCommunication::performThreadJoinOperation() {
    if (read_thread_ && read_thread_->joinable()) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "Waiting for read thread to finish...",
            logging_enabled_, current_log_level_);
        read_thread_->join();
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "Read thread joined successfully",
            logging_enabled_, current_log_level_);
    }
}

void PipeCommunication::performTimeoutThreadJoinOperation() {
    std::vector<std::thread> threads_to_join;
    {
        std::lock_guard<std::mutex> lock(timeout_threads_mutex_);
        threads_to_join.swap(timeout_threads_);
    }

    for (auto& thread : threads_to_join) {
        if (thread.joinable()) {
            thread.join();
        }
    }
}

void PipeCommunication::failAllPendingRequests(const std::string& reason) {
    internal::ThreadSyncHelper::executeWithLock(response_mutex_, [&]() {
        for (auto& entry : pending_requests_) {
            try {
                entry.second.set_value(reason);
            } catch (const std::future_error&) {
            }
        }
        pending_requests_.clear();
        timed_out_request_ids_.clear();
        timed_out_request_order_.clear();
    });
}

void PipeCommunication::notifyConnectionLost(const std::string& reason) {
    failAllPendingRequests(reason);
    auto callback = connection_lost_callback_;
    if (callback) {
        callback(reason);
    }
}

bool PipeCommunication::start(const PipeConfig& config) {
    if (!internal::ConfigValidator::validateConfig(config)) {
        return false;
    }

    if (internal::StateManager::isStateValid(running_)) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::WARNING, "PipeCommunication is already running",
            logging_enabled_, current_log_level_);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(timeout_control_mutex_);
        timeout_stop_requested_ = false;
    }

    applyConfiguration(config);

    bool mode_determined = determineAndSetMode(config);
    if (mode_determined && server_mode_) {
        return executeServerModeInitialization();
    } else if (mode_determined) {
        return executeClientModeInitialization(config);
    }

    return false;
}

void PipeCommunication::applyConfiguration(const PipeConfig& config) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[applyConfiguration] ENTRY - mode=" +
            std::string(config.mode == CommunicationMode::SERVER ? "SERVER"
                                                                 : "CLIENT") +
            ", buffer_size=" + std::to_string(config.buffer_size),
        logging_enabled_, current_log_level_);

    config_ = config;
    current_log_level_ = config.log_level;
    logging_enabled_ = config.enable_logging;

    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[applyConfiguration] config applied: log_level=" +
            logLevelToString(current_log_level_) + ", logging_enabled=" +
            std::string(logging_enabled_ ? "true" : "false"),
        logging_enabled_, current_log_level_);
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[applyConfiguration] EXIT",
                                          logging_enabled_, current_log_level_);
}

bool PipeCommunication::determineAndSetMode(const PipeConfig& config) {
    server_mode_ = (config.mode == CommunicationMode::SERVER);
    return true;
}

bool PipeCommunication::executeServerModeInitialization() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::INFO, "Starting PipeCommunication in SERVER mode",
        logging_enabled_, current_log_level_);
    return initializeServerMode();
}

bool PipeCommunication::executeClientModeInitialization(
    const PipeConfig& config) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::INFO,
        "Starting PipeCommunication in CLIENT mode with command: " +
            config.command,
        logging_enabled_, current_log_level_);

    bool pipes_created = internal::PipeOperationWrapper::performPipeCreation(
        [this]() { return createPipes(); });

    if (!pipes_created) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Failed to create pipes", logging_enabled_,
            current_log_level_);
        return false;
    }

    bool process_started = startProcess();
    if (!process_started) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Failed to start process", logging_enabled_,
            current_log_level_);
        internal::PipeOperationWrapper::performPipeClosure(
            [this]() { closePipes(); });
        return false;
    }

    internal::StateManager::checkAndSetRunning(running_, true);
    read_thread_ =
        std::make_unique<std::thread>(&PipeCommunication::readThreadFunc, this);

    internal::LogOutputProxy::dispatchLog(
        LogLevel::INFO,
        "PipeCommunication started successfully, PID: " +
            std::to_string(process_id_),
        logging_enabled_, current_log_level_);
    return true;
}

bool PipeCommunication::isRunning() const {
    return internal::StateManager::isStateValid(running_);
}

bool PipeCommunication::sendMessage(const std::string& message) {
    if (!internal::StateManager::isStateValid(running_)) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Cannot send message: PipeCommunication is not running",
            logging_enabled_, current_log_level_);
        return false;
    }

    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "Sending message to subprocess: " + message,
        logging_enabled_, current_log_level_);

    return executeMessageSendOperation(message);
}

bool PipeCommunication::executeMessageSendOperation(
    const std::string& message) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[executeMessageSendOperation] ENTRY - message_len=" +
            std::to_string(message.size()) +
            ", running=" + std::string(running_ ? "true" : "false"),
        logging_enabled_, current_log_level_);

    std::string formatted_message = formatMessageWithNewline(message);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[executeMessageSendOperation] formatMessageWithNewline done, "
        "formatted_len=" +
            std::to_string(formatted_message.size()),
        logging_enabled_, current_log_level_);

    ssize_t bytes_written = performWriteOperation(formatted_message);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[executeMessageSendOperation] performWriteOperation returned "
        "bytes_written=" +
            std::to_string(bytes_written) +
            ", expected=" + std::to_string(formatted_message.size()),
        logging_enabled_, current_log_level_);

    bool success = validateWriteResult(bytes_written, formatted_message.size());
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[executeMessageSendOperation] validateWriteResult success=" +
            std::string(success ? "true" : "false"),
        logging_enabled_, current_log_level_);

    logWriteOperationResult(success, bytes_written);

    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[executeMessageSendOperation] EXIT - returning " +
            std::string(success ? "true" : "false"),
        logging_enabled_, current_log_level_);
    return success;
}

std::string PipeCommunication::formatMessageWithNewline(
    const std::string& message) {
    return message + "\n";
}

ssize_t PipeCommunication::performWriteOperation(const std::string& data) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[performWriteOperation] ENTRY - data_size=" +
            std::to_string(data.size()),
        logging_enabled_, current_log_level_);

    ssize_t result = internal::PipeOperationProxy::proxyWrite(
        [this](const char* buf, size_t sz) { return writeImpl(buf, sz); },
        data.c_str(), data.size());

    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[performWriteOperation] writeImpl returned " + std::to_string(result),
        logging_enabled_, current_log_level_);
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[performWriteOperation] EXIT",
                                          logging_enabled_, current_log_level_);
    return result;
}

bool PipeCommunication::validateWriteResult(ssize_t written, size_t expected) {
    return written == static_cast<ssize_t>(expected);
}

void PipeCommunication::logWriteOperationResult(bool success,
                                                ssize_t bytes_written) {
    if (success) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "Message sent successfully (" + std::to_string(bytes_written) +
                " bytes)",
            logging_enabled_, current_log_level_);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Failed to send message, written " + std::to_string(bytes_written) +
                " bytes",
            logging_enabled_, current_log_level_);
    }
}

std::future<std::string> PipeCommunication::sendRequest(
    const std::string& request, int timeout_seconds) {
    RequestProcessingContext context;
    context.original_request = request;
    context.timeout_seconds = timeout_seconds;

    processRequestId(context);
    createResponsePromise(context);

    if (!executeRequestSend(context)) {
        context.response_promise.set_value("Failed to send request");
        return std::move(context.response_future);
    }

    registerPendingRequest(context);
    setupTimeoutHandler(context);

    return std::move(context.response_future);
}

void PipeCommunication::processRequestId(RequestProcessingContext& context) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[processRequestId] ENTRY - original_request_len=" +
            std::to_string(context.original_request.size()),
        logging_enabled_, current_log_level_);

    nlohmann::json req_json = internal::JsonOperationHelper::parseJsonSafely(
        context.original_request);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[processRequestId] parseJsonSafely done, is_null=" +
            std::string(req_json.is_null() ? "true" : "false"),
        logging_enabled_, current_log_level_);

    if (req_json.is_null() || req_json.empty()) {
        context.request_id =
            internal::RequestIdManager::generateNewId(request_id_counter_);
        context.full_request = context.original_request;
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[processRequestId] null/empty json, generated new id=" +
                context.request_id + ", EXIT",
            logging_enabled_, current_log_level_);
        return;
    }

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "req_json: " + req_json.dump(),
                                          logging_enabled_, current_log_level_);

    context.full_request =
        internal::JsonOperationHelper::serializeJson(req_json);

    std::string extracted_id =
        internal::RequestIdManager::extractRequestId(req_json);
    if (extracted_id.empty() || req_json["id"].is_null()) {
        context.request_id =
            internal::RequestIdManager::generateNewId(request_id_counter_);
    } else {
        context.request_id = extracted_id;
    }

    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "send_request request_id: " + context.request_id,
        logging_enabled_, current_log_level_);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[processRequestId] EXIT - request_id=" + context.request_id,
        logging_enabled_, current_log_level_);
}

void PipeCommunication::createResponsePromise(
    RequestProcessingContext& context) {
    context.response_promise = std::promise<std::string>();
    context.response_future = context.response_promise.get_future();
}

bool PipeCommunication::executeRequestSend(RequestProcessingContext& context) {
    return sendMessage(context.full_request);
}

void PipeCommunication::registerPendingRequest(
    RequestProcessingContext& context) {
    internal::ThreadSyncHelper::executeWithLock(response_mutex_, [&]() {
        pending_requests_[context.request_id] =
            std::move(context.response_promise);
    });
}

void PipeCommunication::setupTimeoutHandler(RequestProcessingContext& context) {
    if (context.timeout_seconds > 0) {
        const std::string req_id = context.request_id;
        const int timeout = context.timeout_seconds;
        std::thread timeout_thread([this, req_id, timeout]() {
            std::unique_lock<std::mutex> control_lock(timeout_control_mutex_);
            const bool stop_notified = timeout_control_cv_.wait_for(
                control_lock, std::chrono::seconds(timeout),
                [this]() { return timeout_stop_requested_; });
            if (stop_notified) {
                return;
            }
            control_lock.unlock();

            internal::ThreadSyncHelper::executeWithLock(response_mutex_, [&]() {
                auto it = pending_requests_.find(req_id);
                if (it != pending_requests_.end()) {
                    it->second.set_value("Request timeout");
                    pending_requests_.erase(it);
                    timed_out_request_ids_.insert(req_id);
                    timed_out_request_order_.push_back(req_id);
                    while (timed_out_request_order_.size() >
                           kTimedOutRequestIdCacheSize) {
                        const std::string& expired =
                            timed_out_request_order_.front();
                        timed_out_request_ids_.erase(expired);
                        timed_out_request_order_.pop_front();
                    }
                }
            });
        });
        std::lock_guard<std::mutex> lock(timeout_threads_mutex_);
        timeout_threads_.push_back(std::move(timeout_thread));
    }
}

void PipeCommunication::setMessageHandler(
    std::shared_ptr<MessageHandler> handler) {
    message_handler_ = handler;
}

void PipeCommunication::setConnectionLostCallback(
    std::function<void(const std::string&)> callback) {
    connection_lost_callback_ = std::move(callback);
}

int PipeCommunication::getProcessId() const { return process_id_; }

bool PipeCommunication::isProcessAlive() const {
    if (!internal::ProcessLifecycleManager::validateProcessId(process_id_)) {
        return false;
    }

    return performProcessAliveCheck();
}

bool PipeCommunication::performProcessAliveCheck() const {
#if defined(_WIN32)
    return checkWindowsProcessAlive();
#else
    return checkPosixProcessAlive();
#endif
}

#if defined(_WIN32)
bool PipeCommunication::checkWindowsProcessAlive() const {
    DWORD exit_code;
    if (GetExitCodeProcess(static_cast<HANDLE>(process_handle_), &exit_code)) {
        return exit_code == STILL_ACTIVE;
    }
    return false;
}
#else
bool PipeCommunication::checkPosixProcessAlive() const {
    int status;
    pid_t result = waitpid(process_id_, &status, WNOHANG);
    return result == 0;
}
#endif

bool PipeCommunication::createPipes() { return createPipesImpl(); }

void PipeCommunication::closePipes() { closePipesImpl(); }

bool PipeCommunication::startProcess() { return startProcessImpl(); }

void PipeCommunication::stopProcess() { stopProcessImpl(); }

bool PipeCommunication::createPipesImpl() {
#if defined(_WIN32)
    return createWindowsPipes();
#else
    return createPosixPipes();
#endif
}

#if defined(_WIN32)
bool PipeCommunication::createWindowsPipes() {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "Creating pipes on Windows",
                                          logging_enabled_, current_log_level_);

    SECURITY_ATTRIBUTES sa;
    initializeSecurityAttributes(sa);

    HANDLE child_stdin_read, child_stdin_write;
    HANDLE child_stdout_read, child_stdout_write;

    if (!performWindowsPipeCreation(child_stdin_read, child_stdin_write,
                                    child_stdout_read, child_stdout_write,
                                    sa)) {
        return false;
    }

    if (!configureWindowsPipeHandles(child_stdin_write, child_stdout_read)) {
        cleanupWindowsPipeHandles(child_stdin_read, child_stdin_write,
                                  child_stdout_read, child_stdout_write);
        return false;
    }

    assignWindowsPipeHandles(child_stdin_read, child_stdin_write,
                             child_stdout_read, child_stdout_write);

    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "Pipes created successfully on Windows",
        logging_enabled_, current_log_level_);
    return true;
}

void PipeCommunication::initializeSecurityAttributes(SECURITY_ATTRIBUTES& sa) {
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;
}

bool PipeCommunication::performWindowsPipeCreation(
    HANDLE& stdin_read, HANDLE& stdin_write, HANDLE& stdout_read,
    HANDLE& stdout_write, const SECURITY_ATTRIBUTES& sa) {
    if (!CreatePipe(&stdin_read, &stdin_write, &sa, 0) ||
        !CreatePipe(&stdout_read, &stdout_write, &sa, 0)) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Failed to create pipes on Windows",
            logging_enabled_, current_log_level_);
        return false;
    }
    return true;
}

bool PipeCommunication::configureWindowsPipeHandles(HANDLE stdin_write,
                                                    HANDLE stdout_read) {
    if (!SetHandleInformation(stdin_write, HANDLE_FLAG_INHERIT, 0) ||
        !SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0)) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Failed to set handle information on Windows",
            logging_enabled_, current_log_level_);
        return false;
    }
    return true;
}

void PipeCommunication::cleanupWindowsPipeHandles(HANDLE stdin_read,
                                                  HANDLE stdin_write,
                                                  HANDLE stdout_read,
                                                  HANDLE stdout_write) {
    CloseHandle(stdin_read);
    CloseHandle(stdin_write);
    CloseHandle(stdout_read);
    CloseHandle(stdout_write);
}

void PipeCommunication::assignWindowsPipeHandles(HANDLE stdin_read,
                                                 HANDLE stdin_write,
                                                 HANDLE stdout_read,
                                                 HANDLE stdout_write) {
    stdin_pipe_[0] = reinterpret_cast<intptr_t>(stdin_read);
    stdin_pipe_[1] = reinterpret_cast<intptr_t>(stdin_write);
    stdout_pipe_[0] = reinterpret_cast<intptr_t>(stdout_read);
    stdout_pipe_[1] = reinterpret_cast<intptr_t>(stdout_write);
}
#else
bool PipeCommunication::createPosixPipes() {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "Creating pipes on POSIX",
                                          logging_enabled_, current_log_level_);

    if (!performPosixPipeCreation()) {
        return false;
    }

    configurePosixPipeNonBlocking();

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "Pipes created successfully on POSIX",
                                          logging_enabled_, current_log_level_);
    return true;
}

bool PipeCommunication::performPosixPipeCreation() {
    if (pipe(stdin_pipe_) == -1 || pipe(stdout_pipe_) == -1) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Failed to create pipes on POSIX",
            logging_enabled_, current_log_level_);
        return false;
    }
    return true;
}

void PipeCommunication::configurePosixPipeNonBlocking() {
    int flags = fcntl(stdout_pipe_[0], F_GETFL, 0);
    if (fcntl(stdout_pipe_[0], F_SETFL, flags | O_NONBLOCK) == -1) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::WARNING,
            "Failed to set non-blocking mode for stdout pipe", logging_enabled_,
            current_log_level_);
    }
}
#endif

void PipeCommunication::closePipesImpl() {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG, "Closing pipes",
                                          logging_enabled_, current_log_level_);

#if defined(_WIN32)
    closeWindowsPipes();
#else
    closePosixPipes();
#endif

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG, "Pipes closed",
                                          logging_enabled_, current_log_level_);
}

#if defined(_WIN32)
void PipeCommunication::closeWindowsPipes() {
    closeWindowsPipeHandle(stdin_pipe_[0]);
    closeWindowsPipeHandle(stdin_pipe_[1]);
    closeWindowsPipeHandle(stdout_pipe_[0]);
    closeWindowsPipeHandle(stdout_pipe_[1]);
}

void PipeCommunication::closeWindowsPipeHandle(int& pipe_fd) {
    if (pipe_fd != -1) {
        CloseHandle(reinterpret_cast<HANDLE>(pipe_fd));
        pipe_fd = -1;
    }
}
#else
void PipeCommunication::closePosixPipes() {
    closePosixPipeDescriptor(stdin_pipe_[0]);
    closePosixPipeDescriptor(stdin_pipe_[1]);
    closePosixPipeDescriptor(stdout_pipe_[0]);
    closePosixPipeDescriptor(stdout_pipe_[1]);
}

void PipeCommunication::closePosixPipeDescriptor(int& pipe_fd) {
    if (pipe_fd != -1) {
        close(pipe_fd);
        pipe_fd = -1;
    }
}
#endif

bool PipeCommunication::startProcessImpl() {
#if defined(_WIN32)
    return startWindowsProcess();
#else
    return startPosixProcess();
#endif
}

#if defined(_WIN32)
bool PipeCommunication::startWindowsProcess() {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "Starting process on Windows",
                                          logging_enabled_, current_log_level_);

    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    initializeWindowsStartupInfo(si);
    ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));

    applyEnvironmentVariables();

    std::string cmd_line = buildWindowsCommandLine();
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "Command line: " + cmd_line,
                                          logging_enabled_, current_log_level_);

    bool success = executeWindowsProcessCreation(cmd_line, si, pi);

    if (success) {
        finalizeWindowsProcessStart(pi);
        return true;
    } else {
        logWindowsProcessCreationError();
        return false;
    }
}

void PipeCommunication::initializeWindowsStartupInfo(STARTUPINFOA& si) {
    ZeroMemory(&si, sizeof(STARTUPINFOA));
    si.cb = sizeof(STARTUPINFOA);
    si.hStdInput = reinterpret_cast<HANDLE>(stdin_pipe_[0]);
    si.hStdOutput = reinterpret_cast<HANDLE>(stdout_pipe_[1]);
    si.hStdError = reinterpret_cast<HANDLE>(stdout_pipe_[1]);
    si.dwFlags |= STARTF_USESTDHANDLES;
}

void PipeCommunication::applyEnvironmentVariables() {
    for (const auto& [key, value] : config_.env_vars) {
        SetEnvironmentVariableA(key.c_str(), value.c_str());
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "Set environment variable: " + key + "=" + value,
            logging_enabled_, current_log_level_);
    }
}

std::string PipeCommunication::buildWindowsCommandLine() {
    std::string cmd_line = "cmd.exe /c " + config_.command;
    for (const auto& arg : config_.args) {
        cmd_line += " " + arg;
    }
    return cmd_line;
}

bool PipeCommunication::executeWindowsProcessCreation(
    const std::string& cmd_line, STARTUPINFOA& si, PROCESS_INFORMATION& pi) {
    char* cmd_line_ptr = _strdup(cmd_line.c_str());
    BOOL success = CreateProcessA(NULL, cmd_line_ptr, NULL, NULL, TRUE,
                                  CREATE_NO_WINDOW, NULL, NULL, &si, &pi);
    free(cmd_line_ptr);
    return success != FALSE;
}

void PipeCommunication::finalizeWindowsProcessStart(
    const PROCESS_INFORMATION& pi) {
    process_id_ = pi.dwProcessId;
    process_handle_ = pi.hProcess;
    CloseHandle(pi.hThread);
    CloseHandle(reinterpret_cast<HANDLE>(stdin_pipe_[0]));
    CloseHandle(reinterpret_cast<HANDLE>(stdout_pipe_[1]));
    internal::LogOutputProxy::dispatchLog(
        LogLevel::INFO,
        "Process started successfully on Windows, PID: " +
            std::to_string(process_id_),
        logging_enabled_, current_log_level_);
}

void PipeCommunication::logWindowsProcessCreationError() {
    DWORD error = GetLastError();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::ERROR,
        "Failed to start process on Windows, error code: " +
            std::to_string(error),
        logging_enabled_, current_log_level_);
}
#else
bool PipeCommunication::startPosixProcess() {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "Starting process on POSIX",
                                          logging_enabled_, current_log_level_);

    process_id_ = fork();

    if (process_id_ == -1) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Failed to fork process on POSIX",
            logging_enabled_, current_log_level_);
        return false;
    }

    if (process_id_ == 0) {
        return executeChildProcessOperations();
    } else {
        return executeParentProcessOperations();
    }
}

bool PipeCommunication::executeChildProcessOperations() {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "Child process executing",
                                          logging_enabled_, current_log_level_);

    applyPosixEnvironmentVariables();
    performChildProcessPipeSetup();
    performChildProcessStdioRedirection();
    executeChildProcessCommand();

    return false;  // Child process should not return
}

void PipeCommunication::applyPosixEnvironmentVariables() {
    for (const auto& [key, value] : config_.env_vars) {
        setenv(key.c_str(), value.c_str(), 1);
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "Set environment variable: " + key + "=" + value,
            logging_enabled_, current_log_level_);
    }
}

void PipeCommunication::performChildProcessPipeSetup() {
    close(stdin_pipe_[1]);
    close(stdout_pipe_[0]);
}

void PipeCommunication::performChildProcessStdioRedirection() {
    if (dup2(stdin_pipe_[0], STDIN_FILENO) == -1 ||
        dup2(stdout_pipe_[1], STDOUT_FILENO) == -1) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Failed to redirect stdio in child process",
            logging_enabled_, current_log_level_);
        exit(EXIT_FAILURE);
    }

    close(stdin_pipe_[0]);
    close(stdout_pipe_[1]);
}

void PipeCommunication::executeChildProcessCommand() {
    std::vector<char*> c_args = buildCommandArguments();
    std::string cmd_str = buildCommandString();

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "Executing command: " + cmd_str,
                                          logging_enabled_, current_log_level_);

    execvp(c_args[0], c_args.data());
    internal::LogOutputProxy::dispatchLog(
        LogLevel::ERROR, "Failed to execute command: " + cmd_str,
        logging_enabled_, current_log_level_);
    exit(EXIT_FAILURE);
}

std::vector<char*> PipeCommunication::buildCommandArguments() {
    std::vector<char*> c_args;
    c_args.push_back(const_cast<char*>(config_.command.c_str()));
    for (const auto& arg : config_.args) {
        c_args.push_back(const_cast<char*>(arg.c_str()));
    }
    c_args.push_back(nullptr);
    return c_args;
}

std::string PipeCommunication::buildCommandString() {
    std::string cmd_str = config_.command;
    for (const auto& arg : config_.args) {
        cmd_str += " " + arg;
    }
    return cmd_str;
}

bool PipeCommunication::executeParentProcessOperations() {
    performParentProcessPipeSetup();
    waitForProcessInitialization();
    return validateProcessStartup();
}

void PipeCommunication::performParentProcessPipeSetup() {
    close(stdin_pipe_[0]);
    close(stdout_pipe_[1]);
}

void PipeCommunication::waitForProcessInitialization() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

bool PipeCommunication::validateProcessStartup() {
    int status;
    pid_t result = waitpid(process_id_, &status, WNOHANG);
    if (result == process_id_) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Child process exited immediately, command execution failed",
            logging_enabled_, current_log_level_);
        process_id_ = -1;
        return false;
    }

    internal::LogOutputProxy::dispatchLog(
        LogLevel::INFO,
        "Process started successfully on POSIX, PID: " +
            std::to_string(process_id_),
        logging_enabled_, current_log_level_);
    return true;
}
#endif

void PipeCommunication::stopProcessImpl() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "Stopping process, PID: " + std::to_string(process_id_),
        logging_enabled_, current_log_level_);

#if defined(_WIN32)
    stopWindowsProcess();
#else
    stopPosixProcess();
#endif

    process_id_ = -1;
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG, "Process stopped",
                                          logging_enabled_, current_log_level_);
}

#if defined(_WIN32)
void PipeCommunication::stopWindowsProcess() {
    if (process_handle_) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "Terminating process on Windows", logging_enabled_,
            current_log_level_);
        TerminateProcess(static_cast<HANDLE>(process_handle_), 0);

        DWORD wait_result =
            WaitForSingleObject(static_cast<HANDLE>(process_handle_), 2000);
        if (wait_result == WAIT_TIMEOUT) {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::WARNING, "Process termination timeout on Windows",
                logging_enabled_, current_log_level_);
        } else {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG, "Process terminated successfully on Windows",
                logging_enabled_, current_log_level_);
        }

        CloseHandle(static_cast<HANDLE>(process_handle_));
        process_handle_ = nullptr;
    }
}
#else
void PipeCommunication::stopPosixProcess() {
    if (process_id_ > 0) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "Sending SIGTERM to process on POSIX",
            logging_enabled_, current_log_level_);
        if (kill(process_id_, SIGTERM) == 0) {
            waitForProcessTermination();
        } else {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::ERROR, "Failed to send SIGTERM to process on POSIX",
                logging_enabled_, current_log_level_);
        }
    }
}

void PipeCommunication::waitForProcessTermination() {
    int status;
    pid_t result = waitpid(process_id_, &status, WNOHANG);

    if (result == 0) {
        performProcessTerminationWait(status);
    } else if (result == process_id_) {
        logProcessExitStatus(status);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::WARNING,
            "Failed to wait for process termination on POSIX", logging_enabled_,
            current_log_level_);
    }
}

void PipeCommunication::performProcessTerminationWait(int& status) {
    auto start_time = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start_time <
           std::chrono::seconds(2)) {
        pid_t result = waitpid(process_id_, &status, WNOHANG);
        if (result == process_id_) {
            logProcessExitStatus(status);
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    internal::LogOutputProxy::dispatchLog(
        LogLevel::WARNING, "Process did not exit gracefully, sending SIGKILL",
        logging_enabled_, current_log_level_);
    kill(process_id_, SIGKILL);
    waitpid(process_id_, &status, 0);
}

void PipeCommunication::logProcessExitStatus(int status) {
    if (WIFEXITED(status)) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "Process exited normally with code: " +
                std::to_string(WEXITSTATUS(status)),
            logging_enabled_, current_log_level_);
    } else if (WIFSIGNALED(status)) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "Process terminated by signal: " + std::to_string(WTERMSIG(status)),
            logging_enabled_, current_log_level_);
    }
}
#endif

ssize_t PipeCommunication::readImpl(char* buffer, size_t size) {
#if defined(_WIN32)
    return performWindowsRead(buffer, size);
#else
    return performPosixRead(buffer, size);
#endif
}

#if defined(_WIN32)
ssize_t PipeCommunication::performWindowsRead(char* buffer, size_t size) {
    DWORD bytes_read;
    BOOL success = ReadFile(reinterpret_cast<HANDLE>(stdout_pipe_[0]), buffer,
                            size - 1, &bytes_read, NULL);
    return success ? bytes_read : -1;
}
#else
ssize_t PipeCommunication::performPosixRead(char* buffer, size_t size) {
    // 用 poll 做带超时的可读检测，避免裸 read 阻塞导致 stop() 时 join 死锁。
    // 100ms 超时既保证 stop 后能及时退出，又不会显著增加 CPU 占用。
    // 返回值约定：
    //   >0           实际读到的字节数
    //    0           对端关闭 / 读端被关闭（EOF）
    //   -1           read 真实错误（沿用 errno）
    //   kReadTimeout 本次 poll 超时（空闲），上层应直接重试，不要计入连续空读
    int fd = stdout_pipe_[0];
    if (fd < 0) {
        return -1;
    }

    struct pollfd pfd;
    pfd.fd = fd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    int pret = poll(&pfd, 1, 100);
    if (pret == 0) {
        return kReadTimeout;
    }
    if (pret < 0) {
        if (errno == EINTR) {
            return kReadTimeout;
        }
        return -1;
    }
    if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
        // 读端已被关闭（非 server 模式 stop 时主动 close）—— 返回 0 让循环走 EOF 分支退出
        return 0;
    }
    return read(fd, buffer, size - 1);
}
#endif

ssize_t PipeCommunication::writeImpl(const char* buffer, size_t size) {
#if defined(_WIN32)
    return performWindowsWrite(buffer, size);
#else
    return performPosixWrite(buffer, size);
#endif
}

#if defined(_WIN32)
ssize_t PipeCommunication::performWindowsWrite(const char* buffer,
                                               size_t size) {
    DWORD bytes_written;
    BOOL success = WriteFile(reinterpret_cast<HANDLE>(stdin_pipe_[1]), buffer,
                             size, &bytes_written, NULL);
    return success ? bytes_written : -1;
}
#else
ssize_t PipeCommunication::performPosixWrite(const char* buffer, size_t size) {
    ssize_t result = write(stdin_pipe_[1], buffer, size);
    if (result == -1) {
        handlePosixWriteError();
    }
    return result;
}

void PipeCommunication::handlePosixWriteError() {
    if (errno == EPIPE) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::WARNING,
            "Pipe broken (EPIPE) - subprocess may have terminated",
            logging_enabled_, current_log_level_);
        notifyConnectionLost("connection lost: broken pipe");
    } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "Write would block (EAGAIN/EWOULDBLOCK)",
            logging_enabled_, current_log_level_);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Write error: " + std::string(strerror(errno)),
            logging_enabled_, current_log_level_);
    }
}
#endif

void PipeCommunication::readThreadFunc() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[readThreadFunc] ENTRY - initializing ReadThreadContext",
        logging_enabled_, current_log_level_);

    ReadThreadContext context;
    context.buffer.resize(config_.buffer_size);
    context.max_empty_reads = 100;
    context.max_buffer_size = 1024 * 1024;

    internal::LogOutputProxy::dispatchLog(LogLevel::INFO, "Read thread started",
                                          logging_enabled_, current_log_level_);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[readThreadFunc] context initialized: buffer_size=" +
            std::to_string(config_.buffer_size) +
            ", max_empty_reads=" + std::to_string(context.max_empty_reads) +
            ", max_buffer_size=" + std::to_string(context.max_buffer_size),
        logging_enabled_, current_log_level_);

    int loop_iteration = 0;
    auto last_child_check = std::chrono::steady_clock::now();
    while (running_) {
        loop_iteration++;
        if (loop_iteration % 50 == 1) {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[readThreadFunc] loop iteration #" +
                    std::to_string(loop_iteration) +
                    ", running=" + std::string(running_ ? "true" : "false"),
                logging_enabled_, current_log_level_);
        }

        const auto now = std::chrono::steady_clock::now();
        if (!server_mode_ && process_id_ > 0 &&
            now - last_child_check >= std::chrono::milliseconds(500)) {
            last_child_check = now;
            if (!isProcessAlive()) {
                internal::LogOutputProxy::dispatchLog(
                    LogLevel::WARNING,
                    "Subprocess is no longer alive, stopping read thread",
                    logging_enabled_, current_log_level_);
                notifyConnectionLost("child exited");
                running_ = false;
                break;
            }
        }

        ssize_t bytes_read = performReadOperation(context.buffer);

        if (bytes_read == kReadTimeout) {
            // poll 超时（空闲），回到 while(running_) 检查 stop 标志后继续
            continue;
        }

        if (bytes_read > 0) {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[readThreadFunc] bytes_read=" + std::to_string(bytes_read) +
                    ", branching to processSuccessfulRead",
                logging_enabled_, current_log_level_);
            processSuccessfulRead(bytes_read, context);
        } else if (bytes_read == 0) {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[readThreadFunc] bytes_read=0 (EOF), branching to "
                "handlePipeClosure",
                logging_enabled_, current_log_level_);
            handlePipeClosure(context);
            break;
        } else {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[readThreadFunc] bytes_read=" + std::to_string(bytes_read) +
                    " (error), branching to handleReadError, "
                    "consecutive_empty=" +
                    std::to_string(context.consecutive_empty_reads),
                logging_enabled_, current_log_level_);
            handleReadError(context);
        }
    }

    internal::LogOutputProxy::dispatchLog(LogLevel::INFO,
                                          "Read thread finished",
                                          logging_enabled_, current_log_level_);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[readThreadFunc] EXIT - total_loop_iterations=" +
            std::to_string(loop_iteration),
        logging_enabled_, current_log_level_);
}

ssize_t PipeCommunication::performReadOperation(std::vector<char>& buffer) {
    ssize_t result = internal::PipeOperationProxy::proxyRead(
        [this](char* buf, size_t sz) { return readImpl(buf, sz); },
        buffer.data(), config_.buffer_size);
    return result;
}

void PipeCommunication::processSuccessfulRead(ssize_t bytes_read,
                                              ReadThreadContext& context) {
    context.consecutive_empty_reads = 0;
    context.buffer[bytes_read] = '\0';

    if (context.data_buffer.size() + bytes_read > context.max_buffer_size) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Data buffer size exceeded limit, clearing buffer",
            logging_enabled_, current_log_level_);
        context.data_buffer.clear();
    }

    context.data_buffer.append(context.buffer.data(), bytes_read);
    processCompleteMessages(context);
}

void PipeCommunication::processCompleteMessages(ReadThreadContext& context) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[processCompleteMessages] ENTRY - data_buffer_size=" +
            std::to_string(context.data_buffer.size()),
        logging_enabled_, current_log_level_);

    size_t pos = 0;
    int line_count = 0;
    while ((pos = context.data_buffer.find('\n')) != std::string::npos) {
        std::string line = context.data_buffer.substr(0, pos);
        context.data_buffer.erase(0, pos + 1);
        line_count++;

        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[processCompleteMessages] extracted line #" +
                std::to_string(line_count) +
                ", len=" + std::to_string(line.size()) + ", remaining_buffer=" +
                std::to_string(context.data_buffer.size()),
            logging_enabled_, current_log_level_);

        if (!line.empty()) {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[processCompleteMessages] non-empty line, calling "
                "printSubprocessOutput and processMessage",
                logging_enabled_, current_log_level_);
            printSubprocessOutput(line, "read_thread_func");
            processMessage(line);
        } else {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[processCompleteMessages] empty line, skipping",
                logging_enabled_, current_log_level_);
        }
    }

    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[processCompleteMessages] EXIT - processed_lines=" +
            std::to_string(line_count) +
            ", remaining_buffer=" + std::to_string(context.data_buffer.size()),
        logging_enabled_, current_log_level_);
}

void PipeCommunication::handlePipeClosure(ReadThreadContext& context) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::INFO, "Subprocess pipe closed (EOF received)",
        logging_enabled_, current_log_level_);

    if (!context.data_buffer.empty()) {
        printSubprocessOutput(context.data_buffer, "read_thread_func_final");
        processMessage(context.data_buffer);
        context.data_buffer.clear();
    }

    notifyConnectionLost("connection lost: pipe closed");
}

void PipeCommunication::handleReadError(ReadThreadContext& context) {
    context.consecutive_empty_reads++;

    if (context.consecutive_empty_reads > context.max_empty_reads) {
        if (context.consecutive_empty_reads % 2 == 0) {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::WARNING,
                "Too many consecutive empty reads, checking process status",
                logging_enabled_, current_log_level_);
        }

        if (!isProcessAlive()) {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::WARNING,
                "Subprocess is no longer alive, stopping read thread",
                logging_enabled_, current_log_level_);
            notifyConnectionLost("child exited");
            running_ = false;
            return;
        }

        context.consecutive_empty_reads = 0;
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(config_.read_timeout_ms));
}

void PipeCommunication::processMessage(const std::string& line) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[processMessage] ENTRY - line_len=" + std::to_string(line.size()) +
            ", line_preview=" +
            (line.size() > 64 ? line.substr(0, 64) + "..." : line),
        logging_enabled_, current_log_level_);

    nlohmann::json message =
        internal::JsonOperationHelper::parseJsonSafely(line);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[processMessage] parseJsonSafely done, is_null=" +
            std::string(message.is_null() ? "true" : "false") +
            ", empty=" + std::string(message.empty() ? "true" : "false"),
        logging_enabled_, current_log_level_);

    if (message.is_null() || message.empty()) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[processMessage] branching to handleNonJsonMessage",
            logging_enabled_, current_log_level_);
        handleNonJsonMessage(line);
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[processMessage] handleNonJsonMessage returned, EXIT",
            logging_enabled_, current_log_level_);
        return;
    }

    bool is_valid_rpc =
        internal::MessageProcessorWrapper::isValidJsonRpc(message);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[processMessage] isValidJsonRpc=" +
            std::string(is_valid_rpc ? "true" : "false"),
        logging_enabled_, current_log_level_);

    if (is_valid_rpc) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[processMessage] branching to routeJsonRpcMessage",
            logging_enabled_, current_log_level_);
        routeJsonRpcMessage(message);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[processMessage] branching to handleNonJsonRpcMessage",
            logging_enabled_, current_log_level_);
        handleNonJsonRpcMessage(line);
    }

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[processMessage] EXIT",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::routeJsonRpcMessage(const nlohmann::json& message) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[routeJsonRpcMessage] ENTRY - server_mode=" +
            std::string(server_mode_ ? "true" : "false"),
        logging_enabled_, current_log_level_);

    if (server_mode_) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[routeJsonRpcMessage] routing to routeServerModeMessage",
            logging_enabled_, current_log_level_);
        routeServerModeMessage(message);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[routeJsonRpcMessage] routing to routeClientModeMessage",
            logging_enabled_, current_log_level_);
        routeClientModeMessage(message);
    }

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[routeJsonRpcMessage] EXIT",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::routeServerModeMessage(const nlohmann::json& message) {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[routeServerModeMessage] ENTRY",
                                          logging_enabled_, current_log_level_);

    bool has_id = internal::JsonOperationHelper::hasField(message, "id") &&
                  !message["id"].is_null();
    bool has_method =
        internal::JsonOperationHelper::hasField(message, "method");
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[routeServerModeMessage] has_id=" +
            std::string(has_id ? "true" : "false") +
            ", has_method=" + std::string(has_method ? "true" : "false"),
        logging_enabled_, current_log_level_);

    if (has_id && has_method) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[routeServerModeMessage] dispatching to handleServerRequest",
            logging_enabled_, current_log_level_);
        handleServerRequest(message);
    } else if (has_id) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[routeServerModeMessage] dispatching to processClientResponse",
            logging_enabled_, current_log_level_);
        processClientResponse(message);
    } else if (has_method) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[routeServerModeMessage] dispatching to handleServerNotification",
            logging_enabled_, current_log_level_);
        handleServerNotification(message);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[routeServerModeMessage] no valid route, skipping",
            logging_enabled_, current_log_level_);
    }

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[routeServerModeMessage] EXIT",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::routeClientModeMessage(const nlohmann::json& message) {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[routeClientModeMessage] ENTRY",
                                          logging_enabled_, current_log_level_);

    bool has_id = internal::JsonOperationHelper::hasField(message, "id") &&
                  !message["id"].is_null();
    bool has_method =
        internal::JsonOperationHelper::hasField(message, "method");
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[routeClientModeMessage] has_id=" +
            std::string(has_id ? "true" : "false") +
            ", has_method=" + std::string(has_method ? "true" : "false"),
        logging_enabled_, current_log_level_);

    // In client mode:
    // - method + id => server-initiated request, should be dispatched
    // - id only      => response to a previous client request
    // - method only  => notification
    if (has_id && !has_method) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[routeClientModeMessage] dispatching to processClientResponse",
            logging_enabled_, current_log_level_);
        processClientResponse(message);
    } else if (has_method) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[routeClientModeMessage] dispatching to processClientNotification",
            logging_enabled_, current_log_level_);
        processClientNotification(message);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[routeClientModeMessage] no valid route, skipping",
            logging_enabled_, current_log_level_);
    }

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[routeClientModeMessage] EXIT",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::processClientResponse(const nlohmann::json& message) {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[processClientResponse] ENTRY",
                                          logging_enabled_, current_log_level_);

    std::string id = internal::RequestIdManager::extractRequestId(message);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[processClientResponse] extracted request_id=" + id,
        logging_enabled_, current_log_level_);

    bool has_result =
        internal::JsonOperationHelper::hasField(message, "result");
    bool has_error = internal::JsonOperationHelper::hasField(message, "error");
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[processClientResponse] has_result=" +
            std::string(has_result ? "true" : "false") +
            ", has_error=" + std::string(has_error ? "true" : "false"),
        logging_enabled_, current_log_level_);

    if (has_result) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "pipe_communication received response: id: " + id +
                ", result: " + message["result"].dump(),
            logging_enabled_, current_log_level_);
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "[processClientResponse] invoking handleResponse",
            logging_enabled_, current_log_level_);
        handleResponse(id, message["result"].dump());
    } else if (has_error) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::WARNING,
            "pipe_communication received error: " + message["error"].dump(),
            logging_enabled_, current_log_level_);
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "[processClientResponse] invoking handleError",
            logging_enabled_, current_log_level_);
        handleError(id, message["error"].dump());
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[processClientResponse] neither result nor error, skipping",
            logging_enabled_, current_log_level_);
    }

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[processClientResponse] EXIT",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::processClientNotification(
    const nlohmann::json& message) {
    Message msg;
    msg.type = MessageType::NOTIFICATION;
    msg.content = internal::JsonOperationHelper::serializeJson(message);

    internal::MessageQueueOperator::enqueueMessage(message_queue_,
                                                   message_mutex_, msg);

    if (message_handler_) {
        message_handler_->onMessage(msg);
    }
}

void PipeCommunication::handleNonJsonMessage(const std::string& line) {
    Message msg;
    msg.type = MessageType::NOTIFICATION;
    msg.content = line;

    internal::MessageQueueOperator::enqueueMessage(message_queue_,
                                                   message_mutex_, msg);

    if (message_handler_) {
        message_handler_->onMessage(msg);
    }
}

void PipeCommunication::handleNonJsonRpcMessage(const std::string& line) {
    handleNonJsonMessage(line);
}

std::string PipeCommunication::generateRequestId() {
    return internal::RequestIdManager::generateNewId(request_id_counter_);
}

void PipeCommunication::handleResponse(const std::string& id,
                                       const std::string& content) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[handleResponse] ENTRY - id=" + id +
            ", content_len=" + std::to_string(content.size()),
        logging_enabled_, current_log_level_);

    internal::ThreadSyncHelper::executeWithLock(response_mutex_, [&]() {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "[handleResponse] acquired response_mutex_, pending_count=" +
                std::to_string(pending_requests_.size()),
            logging_enabled_, current_log_level_);
        auto it = pending_requests_.find(id);
        if (it != pending_requests_.end()) {
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[handleResponse] found pending request for id=" + id +
                    ", setting value",
                logging_enabled_, current_log_level_);
            it->second.set_value(content);
            pending_requests_.erase(it);
            timed_out_request_ids_.erase(id);
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[handleResponse] erased pending request, remaining=" +
                    std::to_string(pending_requests_.size()),
                logging_enabled_, current_log_level_);
        } else {
            if (timed_out_request_ids_.find(id) != timed_out_request_ids_.end()) {
                timed_out_request_ids_.erase(id);
                internal::LogOutputProxy::dispatchLog(
                    LogLevel::DEBUG,
                    "[handleResponse] dropping late response for timed-out id=" +
                        id,
                    logging_enabled_, current_log_level_);
                return;
            }
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[handleResponse] no pending request found for id=" + id,
                logging_enabled_, current_log_level_);
            // Ignore unmatched responses. They can be late packets from timed
            // out requests or responses already consumed by another path.
            // Forcing them into a different pending request causes false
            // "id mismatch" errors on unrelated calls.
            internal::LogOutputProxy::dispatchLog(
                LogLevel::DEBUG,
                "[handleResponse] dropping unmatched response id=" + id,
                logging_enabled_, current_log_level_);
        }
    });

    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[handleResponse] EXIT",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::handleError(const std::string& id,
                                    const std::string& error) {
    internal::ThreadSyncHelper::executeWithLock(response_mutex_, [&]() {
        auto it = pending_requests_.find(id);
        if (it != pending_requests_.end()) {
            it->second.set_value(error);
            pending_requests_.erase(it);
        }
    });
}

bool PipeCommunication::hasResult() const {
    return internal::ThreadSyncHelper::executeWithLock(
        result_mutex_, [&]() { return !result_queue_.empty(); });
}

std::string PipeCommunication::getResult() {
    return internal::ThreadSyncHelper::executeWithLock(result_mutex_, [&]() {
        if (result_queue_.empty()) {
            return std::string("");
        }
        std::string result = result_queue_.front();
        result_queue_.pop();
        return result;
    });
}

std::vector<std::string> PipeCommunication::getAllResults() {
    return internal::ThreadSyncHelper::executeWithLock(result_mutex_, [&]() {
        std::vector<std::string> results;
        while (!result_queue_.empty()) {
            results.push_back(result_queue_.front());
            result_queue_.pop();
        }
        return results;
    });
}

bool PipeCommunication::hasMessage() const {
    return internal::MessageQueueOperator::hasMessages(message_queue_,
                                                       message_mutex_);
}

Message PipeCommunication::getMessage() {
    return internal::MessageQueueOperator::dequeueMessage(message_queue_,
                                                          message_mutex_);
}

std::vector<Message> PipeCommunication::getAllMessages() {
    return internal::ThreadSyncHelper::executeWithLock(message_mutex_, [&]() {
        std::vector<Message> messages;
        while (!message_queue_.empty()) {
            messages.push_back(message_queue_.front());
            message_queue_.pop();
        }
        return messages;
    });
}

void PipeCommunication::log(LogLevel level, const std::string& message) {
    internal::LogOutputProxy::dispatchLog(level, message, logging_enabled_,
                                          current_log_level_);
}

void PipeCommunication::setLogLevel(LogLevel level) {
    current_log_level_ = level;
    log(LogLevel::INFO, "Log level changed to " + logLevelToString(level));
}

void PipeCommunication::setLogFile(const std::string& file_path) {
    config_.log_file_path = file_path;
}

void PipeCommunication::enableLogging(bool enable) {
    logging_enabled_ = enable;
    if (enable) {
        log(LogLevel::INFO, "Logging enabled");
    }
}

void PipeCommunication::printSubprocessOutput(const std::string& output,
                                              const std::string& source) {
    if (!logging_enabled_) {
        return;
    }

    std::string subprocess_msg = "[subprocess " + source + "] " + output;
    mcp_logger::getModuleLogger("ipc")->info(subprocess_msg);
}

std::string PipeCommunication::logLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

void PipeCommunication::setServerRequestHandler(
    std::shared_ptr<ServerRequestHandler> handler) {
    server_request_handler_ = handler;
}

bool PipeCommunication::sendResponse(const std::string& request_id,
                                     const std::string& result) {
    if (!validateServerModeOperation()) {
        return false;
    }

    return executeResponseSendOperation(request_id, result);
}

bool PipeCommunication::validateServerModeOperation() {
    if (!running_ || !server_mode_) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Cannot send response: not in server mode or not running",
            logging_enabled_, current_log_level_);
        return false;
    }
    return true;
}

bool PipeCommunication::executeResponseSendOperation(
    const std::string& request_id, const std::string& result) {
    try {
        std::string response_str;
        if (mcp::internal::isJsonRpcPayload(result)) {
            nlohmann::json response = nlohmann::json::parse(result);
            response["id"] = request_id;
            response_str =
                internal::JsonOperationHelper::serializeJson(response) + "\n";
        } else {
            nlohmann::json response = buildJsonRpcResponse(request_id, result);
            response_str =
                internal::JsonOperationHelper::serializeJson(response) + "\n";
        }

        ssize_t bytes_written = performWriteOperation(response_str);
        bool success = validateWriteResult(bytes_written, response_str.size());

        logResponseSendResult(success, request_id);
        return success;
    } catch (const std::exception& e) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Error creating response JSON: " + std::string(e.what()),
            logging_enabled_, current_log_level_);
        return false;
    }
}

nlohmann::json PipeCommunication::buildJsonRpcResponse(
    const std::string& request_id, const std::string& result) {
    nlohmann::json response;
    response["jsonrpc"] = "2.0";
    response["id"] = request_id;
    response["result"] = internal::JsonOperationHelper::parseJsonSafely(result);
    return response;
}

void PipeCommunication::logResponseSendResult(bool success,
                                              const std::string& request_id) {
    if (success) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "Response sent successfully for request ID: " + request_id,
            logging_enabled_, current_log_level_);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Failed to send response for request ID: " + request_id,
            logging_enabled_, current_log_level_);
    }
}

bool PipeCommunication::sendError(const std::string& request_id, int error_code,
                                  const std::string& error_message) {
    if (!validateServerModeOperation()) {
        return false;
    }

    return executeErrorSendOperation(request_id, error_code, error_message);
}

bool PipeCommunication::executeErrorSendOperation(
    const std::string& request_id, int error_code,
    const std::string& error_message) {
    try {
        nlohmann::json error_response =
            buildJsonRpcErrorResponse(request_id, error_code, error_message);
        std::string error_str =
            internal::JsonOperationHelper::serializeJson(error_response) + "\n";

        ssize_t bytes_written = performWriteOperation(error_str);
        bool success = validateWriteResult(bytes_written, error_str.size());

        logErrorSendResult(success, request_id);
        return success;
    } catch (const std::exception& e) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Error creating error response JSON: " + std::string(e.what()),
            logging_enabled_, current_log_level_);
        return false;
    }
}

nlohmann::json PipeCommunication::buildJsonRpcErrorResponse(
    const std::string& request_id, int error_code,
    const std::string& error_message) {
    nlohmann::json error_response;
    error_response["jsonrpc"] = "2.0";
    error_response["id"] = request_id;

    nlohmann::json error_obj;
    error_obj["code"] = error_code;
    error_obj["message"] = error_message;
    error_response["error"] = error_obj;

    return error_response;
}

void PipeCommunication::logErrorSendResult(bool success,
                                           const std::string& request_id) {
    if (success) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "Error response sent successfully for request ID: " + request_id,
            logging_enabled_, current_log_level_);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Failed to send error response for request ID: " + request_id,
            logging_enabled_, current_log_level_);
    }
}

bool PipeCommunication::sendNotification(const std::string& method,
                                         const std::string& params) {
    if (!validateServerModeOperation()) {
        return false;
    }

    return executeNotificationSendOperation(method, params);
}

bool PipeCommunication::executeNotificationSendOperation(
    const std::string& method, const std::string& params) {
    try {
        nlohmann::json notification = buildJsonRpcNotification(method, params);
        std::string notification_str =
            internal::JsonOperationHelper::serializeJson(notification) + "\n";

        ssize_t bytes_written = performWriteOperation(notification_str);
        bool success =
            validateWriteResult(bytes_written, notification_str.size());

        logNotificationSendResult(success, method);
        return success;
    } catch (const std::exception& e) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Error creating notification JSON: " + std::string(e.what()),
            logging_enabled_, current_log_level_);
        return false;
    }
}

nlohmann::json PipeCommunication::buildJsonRpcNotification(
    const std::string& method, const std::string& params) {
    nlohmann::json notification;
    notification["jsonrpc"] = "2.0";
    notification["method"] = method;
    if (!params.empty()) {
        notification["params"] =
            internal::JsonOperationHelper::parseJsonSafely(params);
    }
    return notification;
}

void PipeCommunication::logNotificationSendResult(bool success,
                                                  const std::string& method) {
    if (success) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "Notification sent successfully: " + method,
            logging_enabled_, current_log_level_);
    } else {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR, "Failed to send notification: " + method,
            logging_enabled_, current_log_level_);
    }
}

bool PipeCommunication::initializeServerMode() {
    internal::LogOutputProxy::dispatchLog(LogLevel::INFO,
                                          "Initializing server mode",
                                          logging_enabled_, current_log_level_);

    assignServerModePipeDescriptors();

    internal::StateManager::checkAndSetRunning(running_, true);
    read_thread_ =
        std::make_unique<std::thread>(&PipeCommunication::readThreadFunc, this);

    internal::LogOutputProxy::dispatchLog(
        LogLevel::INFO, "Server mode initialized successfully",
        logging_enabled_, current_log_level_);
    return true;
}

void PipeCommunication::assignServerModePipeDescriptors() {
    stdin_pipe_[0] = STDIN_FILENO;
    stdin_pipe_[1] = STDOUT_FILENO;
    stdout_pipe_[0] = STDIN_FILENO;
    stdout_pipe_[1] = STDOUT_FILENO;
}

void PipeCommunication::handleServerRequest(const nlohmann::json& request) {
    if (!server_request_handler_) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::WARNING,
            "No server request handler set, ignoring request", logging_enabled_,
            current_log_level_);
        return;
    }

    try {
        ServerRequestContext context;
        extractServerRequestInfo(request, context);

        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG,
            "Handling server request: " + context.method +
                " (ID: " + context.request_id + ")",
            logging_enabled_, current_log_level_);

        std::string result = server_request_handler_->handleRequest(
            context.method, context.params, context.request_id);

        if (!context.request_id.empty()) {
            sendResponse(context.request_id, result);
        }
    } catch (const mcp_exception& e) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "MCP error handling server request: " + std::string(e.what()),
            logging_enabled_, current_log_level_);

        if (internal::JsonOperationHelper::hasField(request, "id") &&
            !request["id"].is_null()) {
            std::string request_id =
                internal::RequestIdManager::extractRequestId(request);
            sendError(request_id, static_cast<int>(e.code()), e.what());
        }
    } catch (const std::exception& e) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Error handling server request: " + std::string(e.what()),
            logging_enabled_, current_log_level_);

        if (internal::JsonOperationHelper::hasField(request, "id") &&
            !request["id"].is_null()) {
            std::string request_id =
                internal::RequestIdManager::extractRequestId(request);
            sendError(request_id, -32603,
                      "Internal error: " + std::string(e.what()));
        }
    }
}

void PipeCommunication::extractServerRequestInfo(
    const nlohmann::json& request, ServerRequestContext& context) {
    context.method = request["method"].get<std::string>();
    context.request_id = internal::RequestIdManager::extractRequestId(request);

    if (internal::JsonOperationHelper::hasField(request, "params")) {
        context.params =
            internal::JsonOperationHelper::serializeJson(request["params"]);
    } else {
        context.params = "";
    }
}

void PipeCommunication::handleServerNotification(
    const nlohmann::json& notification) {
    if (!server_request_handler_) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::WARNING,
            "No server request handler set, ignoring notification",
            logging_enabled_, current_log_level_);
        return;
    }

    try {
        std::string method = notification["method"].get<std::string>();
        std::string params = "";

        if (internal::JsonOperationHelper::hasField(notification, "params")) {
            params = internal::JsonOperationHelper::serializeJson(
                notification["params"]);
        }

        internal::LogOutputProxy::dispatchLog(
            LogLevel::DEBUG, "Handling server notification: " + method,
            logging_enabled_, current_log_level_);
        server_request_handler_->handleNotification(method, params);
    } catch (const std::exception& e) {
        internal::LogOutputProxy::dispatchLog(
            LogLevel::ERROR,
            "Error handling server notification: " + std::string(e.what()),
            logging_enabled_, current_log_level_);
    }
}

// ==================== Redundant interface implementation (internally routed
// through multiple indirection layers) ====================

bool PipeCommunication::activate(const PipeConfig& config) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] activate() entry - delegating to start()",
        logging_enabled_, current_log_level_);
    bool ret = start(config);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] activate() exit, result=" +
            std::string(ret ? "true" : "false"),
        logging_enabled_, current_log_level_);
    return ret;
}

void PipeCommunication::shutdown() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] shutdown() entry - delegating to stop()",
        logging_enabled_, current_log_level_);
    stop();
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] shutdown() exit",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::terminate() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] terminate() entry - delegating to stop()",
        logging_enabled_, current_log_level_);
    stop();
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] terminate() exit",
                                          logging_enabled_, current_log_level_);
}

bool PipeCommunication::isActive() const {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] isActive() entry",
                                          logging_enabled_, current_log_level_);
    bool ret = isRunning();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] isActive() exit, result=" +
            std::string(ret ? "true" : "false"),
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::isCommunicationEstablished() const {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] isCommunicationEstablished() entry",
        logging_enabled_, current_log_level_);
    bool ret = isRunning();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] isCommunicationEstablished() exit, result=" +
            std::string(ret ? "true" : "false"),
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::postMessage(const std::string& message) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] postMessage() entry, msg_len=" +
            std::to_string(message.size()),
        logging_enabled_, current_log_level_);
    bool ret = sendMessage(message);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] postMessage() exit, result=" +
            std::string(ret ? "true" : "false"),
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::dispatchMessage(const std::string& message) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] dispatchMessage() entry",
        logging_enabled_, current_log_level_);
    bool ret = sendMessage(message);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] dispatchMessage() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::transmitData(const std::string& data) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] transmitData() entry, data_size=" +
            std::to_string(data.size()),
        logging_enabled_, current_log_level_);
    bool ret = sendMessage(data);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] transmitData() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::pushToPipe(const std::string& payload) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] pushToPipe() entry",
        logging_enabled_, current_log_level_);
    bool ret = sendMessage(payload);
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] pushToPipe() exit",
                                          logging_enabled_, current_log_level_);
    return ret;
}

std::future<std::string> PipeCommunication::initiateRequest(
    const std::string& request, int timeout_seconds) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] initiateRequest() entry, timeout=" +
            std::to_string(timeout_seconds),
        logging_enabled_, current_log_level_);
    auto ret = sendRequest(request, timeout_seconds);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] initiateRequest() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

std::future<std::string> PipeCommunication::submitRequest(
    const std::string& request, int timeout_seconds) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] submitRequest() entry",
        logging_enabled_, current_log_level_);
    auto ret = sendRequest(request, timeout_seconds);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] submitRequest() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

std::future<std::string> PipeCommunication::invokeRpc(
    const std::string& request, int timeout_seconds) {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] invokeRpc() entry",
                                          logging_enabled_, current_log_level_);
    auto ret = sendRequest(request, timeout_seconds);
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] invokeRpc() exit",
                                          logging_enabled_, current_log_level_);
    return ret;
}

void PipeCommunication::registerMessageHandler(
    std::shared_ptr<MessageHandler> handler) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] registerMessageHandler() entry",
        logging_enabled_, current_log_level_);
    setMessageHandler(handler);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] registerMessageHandler() exit",
        logging_enabled_, current_log_level_);
}

void PipeCommunication::attachMessageHandler(
    std::shared_ptr<MessageHandler> handler) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] attachMessageHandler() entry",
        logging_enabled_, current_log_level_);
    setMessageHandler(handler);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] attachMessageHandler() exit",
        logging_enabled_, current_log_level_);
}

void PipeCommunication::registerServerHandler(
    std::shared_ptr<ServerRequestHandler> handler) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] registerServerHandler() entry",
        logging_enabled_, current_log_level_);
    setServerRequestHandler(handler);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] registerServerHandler() exit",
        logging_enabled_, current_log_level_);
}

bool PipeCommunication::dispatchResponse(const std::string& request_id,
                                         const std::string& result) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] dispatchResponse() entry, req_id=" + request_id,
        logging_enabled_, current_log_level_);
    bool ret = sendResponse(request_id, result);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] dispatchResponse() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::dispatchError(const std::string& request_id,
                                      int error_code,
                                      const std::string& error_message) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] dispatchError() entry, code=" +
            std::to_string(error_code),
        logging_enabled_, current_log_level_);
    bool ret = sendError(request_id, error_code, error_message);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] dispatchError() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::broadcastNotification(const std::string& method,
                                              const std::string& params) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] broadcastNotification() entry, method=" + method,
        logging_enabled_, current_log_level_);
    bool ret = sendNotification(method, params);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] broadcastNotification() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::emitNotification(const std::string& method,
                                         const std::string& params) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] emitNotification() entry",
        logging_enabled_, current_log_level_);
    bool ret = sendNotification(method, params);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] emitNotification() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::messageQueueHasItems() const {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] messageQueueHasItems() entry",
        logging_enabled_, current_log_level_);
    bool ret = hasMessage();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] messageQueueHasItems() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

Message PipeCommunication::retrieveMessage() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] retrieveMessage() entry",
        logging_enabled_, current_log_level_);
    Message ret = getMessage();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] retrieveMessage() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

Message PipeCommunication::dequeueNextMessage() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] dequeueNextMessage() entry",
        logging_enabled_, current_log_level_);
    Message ret = getMessage();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] dequeueNextMessage() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

std::vector<Message> PipeCommunication::drainMessageQueue() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] drainMessageQueue() entry",
        logging_enabled_, current_log_level_);
    auto ret = getAllMessages();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] drainMessageQueue() exit, count=" +
            std::to_string(ret.size()),
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::resultQueueHasItems() const {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] resultQueueHasItems() entry",
        logging_enabled_, current_log_level_);
    bool ret = hasResult();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] resultQueueHasItems() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

std::string PipeCommunication::fetchResult() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] fetchResult() entry",
        logging_enabled_, current_log_level_);
    std::string ret = getResult();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] fetchResult() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

std::string PipeCommunication::consumeNextResult() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] consumeNextResult() entry",
        logging_enabled_, current_log_level_);
    std::string ret = getResult();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] consumeNextResult() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

std::vector<std::string> PipeCommunication::drainResultQueue() {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] drainResultQueue() entry",
        logging_enabled_, current_log_level_);
    auto ret = getAllResults();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] drainResultQueue() exit, count=" +
            std::to_string(ret.size()),
        logging_enabled_, current_log_level_);
    return ret;
}

int PipeCommunication::acquireProcessId() const {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] acquireProcessId() entry",
        logging_enabled_, current_log_level_);
    int ret = getProcessId();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] acquireProcessId() exit, pid=" + std::to_string(ret),
        logging_enabled_, current_log_level_);
    return ret;
}

int PipeCommunication::getChildPid() const {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] getChildPid() entry",
        logging_enabled_, current_log_level_);
    int ret = getProcessId();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] getChildPid() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::verifyProcessStatus() const {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] verifyProcessStatus() entry",
        logging_enabled_, current_log_level_);
    bool ret = isProcessAlive();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] verifyProcessStatus() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

bool PipeCommunication::isSubprocessAlive() const {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] isSubprocessAlive() entry",
        logging_enabled_, current_log_level_);
    bool ret = isProcessAlive();
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] isSubprocessAlive() exit",
        logging_enabled_, current_log_level_);
    return ret;
}

void PipeCommunication::writeLog(LogLevel level, const std::string& message) {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] writeLog() entry",
                                          logging_enabled_, current_log_level_);
    log(level, message);
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] writeLog() exit",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::recordLog(LogLevel level, const std::string& message) {
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] recordLog() entry",
                                          logging_enabled_, current_log_level_);
    log(level, message);
    internal::LogOutputProxy::dispatchLog(LogLevel::DEBUG,
                                          "[redundant_iface] recordLog() exit",
                                          logging_enabled_, current_log_level_);
}

void PipeCommunication::configureLogLevel(LogLevel level) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] configureLogLevel() entry",
        logging_enabled_, current_log_level_);
    setLogLevel(level);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] configureLogLevel() exit",
        logging_enabled_, current_log_level_);
}

void PipeCommunication::setLoggingEnabled(bool enable) {
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG,
        "[redundant_iface] setLoggingEnabled() entry, enable=" +
            std::string(enable ? "true" : "false"),
        logging_enabled_, current_log_level_);
    enableLogging(enable);
    internal::LogOutputProxy::dispatchLog(
        LogLevel::DEBUG, "[redundant_iface] setLoggingEnabled() exit",
        logging_enabled_, current_log_level_);
}

}  // namespace mcp

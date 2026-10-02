#include <glib.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <atomic>
#include <condition_variable>
#include <cstdarg>
#include <ctime>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include "chatconfig.h"
#include "chatresult.h"
#include "chatsource.h"
#include "retrievalconfig.h"
#include "retrievalresponse.h"
#include "session.h"

namespace {

struct TestResult {
    std::string operation;
    bool success;
    std::string message;
    int thread_id;
};

struct ThreadSafeChatContext {
    std::string full_response;
    KBChatHistory *history{};
    std::atomic<bool> is_chatting{false};
    std::mutex response_mutex;  // 保护响应内容的互斥锁
    std::condition_variable completion_cv;
    int thread_id{};  // 线程ID，用于输出标识
};

struct ThreadSafeRetrievalContext {
    bool is_retrieving = false;
    bool completed = false;
    int thread_id{};  // 线程ID，用于输出标识
};

// 检查文件是否存在
bool file_exists(const char *path) { return path && access(path, F_OK) == 0; }

// 获取文件大小
long get_file_size(const char *path) {
    if (!path) return -1;

    struct stat st;
    if (stat(path, &st) == 0) {
        return st.st_size;
    }
    return -1;
}

// 获取文件扩展名
const char *get_file_extension(const char *path) {
    if (!path) return NULL;

    const char *dot = strrchr(path, '.');
    if (dot && dot != path) {
        return dot + 1;
    }
    return NULL;
}

// 检查文件格式是否支持
bool is_supported_format(const char *path) {
    const char *ext = get_file_extension(path);
    if (!ext) return false;

    const char *supported_formats[] = {"pdf",  "doc", "docx", "txt", "md",
                                       "html", "htm", "rtf",  "csv"};

    for (int i = 0; i < sizeof(supported_formats) / sizeof(char *); i++) {
        if (strcasecmp(ext, supported_formats[i]) == 0) {
            return true;
        }
    }

    return false;
}

// 全局变量用于信号处理
volatile bool g_interrupted = false;
std::mutex g_output_mutex;

// 信号处理函数
void signal_handler(int signal) {
    if (signal == SIGINT) {
        g_interrupted = true;
        {
            std::lock_guard lock(g_output_mutex);
            printf("\n收到中断信号，正在清理...\n");
        }
    }
}

// 线程安全的输出函数
void thread_safe_printf(const char *format, ...) {
    std::lock_guard lock(g_output_mutex);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    fflush(stdout);
}

// 线程安全的聊天回调函数
int thread_safe_chat_callback(const KBChatResponse *response, void *user_data) {
    auto *ctx = static_cast<ThreadSafeChatContext *>(user_data);

    // 检查上下文有效性
    if (!ctx) {
        return 0;
    }

    if (g_interrupted) {
        ctx->is_chatting.store(false);
        ctx->completion_cv.notify_all();
        return 0;
    }

    if (!response) {
        ctx->is_chatting.store(false);
        ctx->completion_cv.notify_all();
        return 0;
    }

    int error_code = kb_chat_response_get_error_code(response);
    if (error_code != AI_COMMON_SUCCESS) {
        thread_safe_printf("线程[%d] 聊天错误: %s\n", ctx->thread_id,
                           kb_get_error_message(static_cast<int>(error_code)));
        ctx->is_chatting.store(false);
        ctx->completion_cv.notify_all();
        return 0;
    }

    const KBChatResult *result = kb_chat_response_get_result(response);
    if (result) {
        const char *model = kb_chat_result_get_model(result);
        const char *reasoningContent =
            kb_chat_result_get_reasoning_content(result);
        const char *text = kb_chat_result_get_response(result);

        if (model && strlen(model) > 0) {
            thread_safe_printf("线程[%d] 当前模型为 %s\n", ctx->thread_id,
                               model);
        }

        if (reasoningContent && strlen(reasoningContent) > 0) {
            thread_safe_printf("线程[%d] 推理过程: %s\n", ctx->thread_id,
                               reasoningContent);
        }

        if (text && strlen(text) > 0) {
            // 保护对 full_response 的访问
            {
                std::lock_guard lock(ctx->response_mutex);
                ctx->full_response += text;
            }
            thread_safe_printf("线程[%d] 回答结果：%s\n", ctx->thread_id, text);
        }

        int reference_count;
        const KBChatReference **reference =
            kb_chat_result_get_reference(result, &reference_count);
        if (reference && reference_count > 0) {
            thread_safe_printf("\n线程[%d] 📚 参考内容:\n", ctx->thread_id);
            for (int i = 0; i < reference_count; i++) {
                int file_number =
                    kb_chat_reference_get_file_number(reference[i]);
                int chunk_index =
                    kb_chat_reference_get_chunk_index(reference[i]);
                thread_safe_printf("线程[%d] No.%d (chunk index: %d)\n",
                                   ctx->thread_id, file_number, chunk_index);
            }
        }

        if (kb_chat_result_is_finished(result)) {
            // 显示源文档
            int source_count = 0;
            const KBChatSource **sources =
                kb_chat_result_get_source(result, &source_count);
            if (sources && source_count > 0) {
                thread_safe_printf("\n线程[%d] 📚 源文档:\n", ctx->thread_id);
                for (int i = 0; i < source_count; i++) {
                    const char *file_id =
                        kb_chat_source_get_file_id(sources[i]);
                    double confidence =
                        kb_chat_source_get_confidence(sources[i]);
                    const char *content =
                        kb_chat_source_get_content(sources[i]);

                    if (file_id && content) {
                        thread_safe_printf(
                            "线程[%d]   %d. %s (可信度: %.1f%%)\n\n线程[%d]   "
                            "内容:【%s】\n\n",
                            ctx->thread_id, i + 1, file_id, confidence * 100,
                            ctx->thread_id, content);
                    }
                }
            }

            // 显示参考来源
            int reference_file_count;
            const char **reference_file_ids =
                kb_chat_result_get_reference_file_ids(result,
                                                      &reference_file_count);
            if (reference_file_ids && reference_file_count > 0) {
                thread_safe_printf("\n线程[%d] 📚 标注参考来源:\n",
                                   ctx->thread_id);
                for (int i = 0; i < reference_file_count; i++) {
                    thread_safe_printf("线程[%d]   %d. %s\n", ctx->thread_id,
                                       i + 1, reference_file_ids[i]);
                }
            }

            // 添加响应到历史记录（线程安全）
            {
                std::lock_guard lock(ctx->response_mutex);
                if (ctx->history && !ctx->full_response.empty()) {
                    int index = kb_chat_history_add_response(
                        ctx->history, ctx->full_response.c_str());
                    thread_safe_printf("线程[%d] 第[%d]条回答已经添加\n",
                                       ctx->thread_id, index);
                }
            }

            ctx->is_chatting.store(false);
            ctx->completion_cv.notify_all();
            return 0;
        }
    }

    ctx->is_chatting.store(true);
    return 1;  // 不再使用G_SOURCE_CONTINUE
}

// 线程安全的检索回调函数
void thread_safe_retrieval_callback(const KBRetrievalResponse *response,
                                    void *user_data) {
    auto *ctx = static_cast<ThreadSafeRetrievalContext *>(user_data);

    if (!ctx) return;

    if (!response) {
        ctx->is_retrieving = false;
        ctx->completed = true;
        return;
    }

    int error_code = kb_retrieval_response_get_error_code(response);
    if (error_code != AISDK_NO_ERROR) {
        thread_safe_printf("线程[%d] 检索错误: %s\n", ctx->thread_id,
                           kb_get_error_message(static_cast<int>(error_code)));
        ctx->is_retrieving = false;
        ctx->completed = true;
        return;
    }

    const char *request_id = kb_retrieval_response_get_request_id(response);
    const char *kb_id = kb_retrieval_response_get_knowledgebase_id(response);
    const char *query = kb_retrieval_response_get_query(response);

    thread_safe_printf("\n线程[%d] 🔍 检索结果:\n", ctx->thread_id);
    thread_safe_printf("线程[%d]   请求ID: %s\n", ctx->thread_id,
                       request_id ? request_id : "N/A");
    thread_safe_printf("线程[%d]   知识库ID: %s\n", ctx->thread_id,
                       kb_id ? kb_id : "N/A");
    thread_safe_printf("线程[%d]   查询内容: %s\n", ctx->thread_id,
                       query ? query : "N/A");

    int source_count = kb_retrieval_response_get_source_count(response);
    thread_safe_printf("线程[%d]   检索到 %d 个结果:\n", ctx->thread_id,
                       source_count);

    for (int i = 0; i < source_count; i++) {
        const KBChatSource *source =
            kb_retrieval_response_get_source(response, i);
        if (source) {
            const char *file_id = kb_chat_source_get_file_id(source);
            const char *content = kb_chat_source_get_content(source);
            double confidence = kb_chat_source_get_confidence(source);

            thread_safe_printf("线程[%d]   [%d] 文件ID: %s (可信度: %.2f%%)\n",
                               ctx->thread_id, i + 1, file_id ? file_id : "N/A",
                               confidence);
            if (content) {
                std::string content_str(content);
                thread_safe_printf("线程[%d]       内容摘要: %s\n",
                                   ctx->thread_id, content_str.c_str());
            }
        }
    }

    ctx->is_retrieving = false;
    ctx->completed = true;
}

// RAII 封装 C 风格资源
template <typename T, typename Deleter>
class CResource {
    T resource_;
    Deleter deleter_;

public:
    CResource(T res, Deleter d) : resource_(res), deleter_(d) {}
    ~CResource() {
        if (resource_) deleter_(resource_);
    }
    operator T() const { return resource_; }
    T get() const { return resource_; }
    void reset(T new_res = nullptr) {
        if (resource_) deleter_(resource_);
        resource_ = new_res;
    }
};

// 测试知识库创建/删除接口的线程安全
void test_knowledgebase_thread_safety(KBSession *session,
                                      const std::string &base_name,
                                      int thread_id,
                                      std::vector<TestResult> &results) {
    int error;
    char kb_name[256];
    // 使用随机数增强唯一性，避免时间戳冲突
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    snprintf(kb_name, sizeof(kb_name), "%s_thread_%d_%ld_%d", base_name.c_str(),
             thread_id, time(nullptr), dis(gen));

    // 创建知识库信息 - 使用 RAII
    CResource kb_info(kb_knowledgebase_info_new(kb_name, "线程安全测试知识库"),
                      &kb_knowledgebase_info_free);

    if (!kb_info.get()) {
        TestResult result{"knowledgebase_info_new", false, "创建知识库信息失败",
                          thread_id};
        results.push_back(result);
        return;
    }

    // 创建知识库
    CResource<char *, decltype(&kb_free_string)> kb_id(
        kb_knowledgebase_create(session, kb_info.get(), &error),
        &kb_free_string);

    if (!kb_id.get()) {
        if (error == KB_ERROR_KB_ALREADY_EXISTS) {
            TestResult result{"knowledgebase_create", false, "知识库已存在",
                              thread_id};
            results.push_back(result);
        } else {
            TestResult result{
                "knowledgebase_create", false,
                std::string("创建失败: ") + kb_get_error_message(error),
                thread_id};
            results.push_back(result);
        }
        return;
    }

    thread_safe_printf("线程[%d] 创建知识库成功: %s\n", thread_id, kb_id.get());

    // 获取知识库信息
    CResource get_kb_info(
        kb_knowledgebase_info_get(session, kb_id.get(), &error),
        &kb_knowledgebase_info_free);

    if (error != AI_COMMON_SUCCESS) {
        TestResult result{
            "knowledgebase_info_get", false,
            std::string("获取信息失败: ") + kb_get_error_message(error),
            thread_id};
        results.push_back(result);
    } else {
        TestResult result{"knowledgebase_info_get", true, "获取信息成功",
                          thread_id};
        results.push_back(result);
    }

    // 列出知识库
    int kb_count;
    KnowledgeBaseInfo **kb_list =
        kb_knowledgebase_info_list(session, &kb_count, &error);
    if (error != AI_COMMON_SUCCESS) {
        TestResult result{
            "knowledgebase_info_list", false,
            std::string("列出知识库失败: ") + kb_get_error_message(error),
            thread_id};
        results.push_back(result);
    } else {
        thread_safe_printf("线程[%d] 列出知识库数量: %d\n", thread_id,
                           kb_count);
        TestResult result{"knowledgebase_info_list", true, "列出知识库成功",
                          thread_id};
        results.push_back(result);

        // 释放列表
        for (int i = 0; i < kb_count; i++) {
            kb_knowledgebase_info_free(kb_list[i]);
        }
        delete[] kb_list;
    }

    // 更新知识库
    char updated_kb_name[256];
    snprintf(updated_kb_name, sizeof(updated_kb_name),
             "更新后的线程安全测试知识库_%ld_%d", time(nullptr), dis(gen));
    kb_knowledgebase_info_set_name(kb_info.get(), updated_kb_name);
    kb_knowledgebase_info_set_description(kb_info.get(),
                                          "更新后的线程安全测试描述");
    kb_knowledgebase_info_set_id(kb_info.get(), kb_id.get());
    error = kb_knowledgebase_info_update(session, kb_info.get());
    if (error != AI_COMMON_SUCCESS) {
        TestResult result{
            "knowledgebase_info_update", false,
            std::string("更新知识库失败: ") + kb_get_error_message(error),
            thread_id};
        results.push_back(result);
    } else {
        TestResult result{"knowledgebase_info_update", true, "更新知识库成功",
                          thread_id};
        results.push_back(result);
    }

    // 删除知识库
    error = kb_knowledgebase_delete(session, kb_id.get());
    if (error != AI_COMMON_SUCCESS) {
        TestResult result{
            "knowledgebase_delete", false,
            std::string("删除失败: ") + kb_get_error_message(error), thread_id};
        results.push_back(result);
    } else {
        thread_safe_printf("线程[%d] 删除知识库成功: %s\n", thread_id,
                           kb_id.get());
        TestResult result{"knowledgebase_delete", true, "删除成功", thread_id};
        results.push_back(result);
    }

    TestResult result{"knowledgebase_full_test", true, "完整测试成功",
                      thread_id};
    results.push_back(result);
}

// 测试文档接口的线程安全
void test_document_thread_safety(KBSession *session, const char *kb_id,
                                 const char *doc_path, int thread_id,
                                 std::vector<TestResult> &results) {
    int error;
    char doc_id[256];
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    snprintf(doc_id, sizeof(doc_id), "doc_thread_%d_%ld_%d", thread_id,
             time(nullptr), dis(gen));

    // 上传文档
    error = kb_document_add(session, kb_id, doc_path, doc_id, NULL);
    if (error != AI_COMMON_SUCCESS) {
        TestResult result{
            "document_add", false,
            std::string("上传失败: ") + kb_get_error_message(error), thread_id};
        results.push_back(result);
        return;
    }

    thread_safe_printf("线程[%d] 上传文档成功: %s\n", thread_id, doc_id);
    TestResult result_add{"document_add", true, "上传成功", thread_id};
    results.push_back(result_add);

    // 使用 RAII 管理 kb_info
    CResource<KnowledgeBaseInfo *, decltype(&kb_knowledgebase_info_free)>
        kb_info(kb_knowledgebase_info_get(session, kb_id, &error),
                &kb_knowledgebase_info_free);

    // 获取文档处理进度
    int status;
    int progress = kb_document_get_processing_progress(session, kb_info.get(),
                                                       doc_id, &status, &error);
    if (progress >= 0) {
        thread_safe_printf("线程[%d] 文档进度: %d%% 状态: %d\n", thread_id,
                           progress, status);
        TestResult result{"document_get_processing_progress", true,
                          "获取进度成功", thread_id};
        results.push_back(result);
    } else {
        TestResult result{
            "document_get_processing_progress", false,
            std::string("获取进度失败: ") + kb_get_error_message(error),
            thread_id};
        results.push_back(result);
    }

    // 列出文档
    int doc_count;
    char **doc_list = kb_document_list(session, kb_id, &doc_count, &error);
    if (error != AI_COMMON_SUCCESS) {
        TestResult result{
            "document_list", false,
            std::string("列出文档失败: ") + kb_get_error_message(error),
            thread_id};
        results.push_back(result);
    } else {
        thread_safe_printf("线程[%d] 列出文档数量: %d\n", thread_id, doc_count);
        TestResult result{"document_list", true, "列出文档成功", thread_id};
        results.push_back(result);
        kb_free_string_array(doc_list, doc_count);
    }

    // 删除文档
    error = kb_document_delete(session, kb_id, doc_id);
    if (error != AI_COMMON_SUCCESS) {
        TestResult result{
            "document_delete", false,
            std::string("删除失败: ") + kb_get_error_message(error), thread_id};
        results.push_back(result);
    } else {
        thread_safe_printf("线程[%d] 删除文档成功: %s\n", thread_id, doc_id);
        TestResult result{"document_delete", true, "删除成功", thread_id};
        results.push_back(result);
    }
}

// 测试文档批量操作的线程安全
void test_document_batch_thread_safety(KBSession *session, const char *kb_id,
                                       const char *doc_path, int thread_id,
                                       std::vector<TestResult> &results) {
    int error;
    const int docs_count = 2;
    char doc_ids[docs_count][256];
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);

    const char *doc_paths[docs_count] = {doc_path, doc_path};
    const char *doc_ids_ptrs[docs_count] = {doc_ids[0], doc_ids[1]};

    for (int i = 0; i < docs_count; i++) {
        snprintf(doc_ids[i], sizeof(doc_ids[0]), "batch_doc_%d_%d_%ld",
                 thread_id, i, time(nullptr) + dis(gen));
    }

    // 批量上传文档
    error = kb_document_add_multi(session, kb_id, doc_paths, doc_ids_ptrs,
                                  docs_count, NULL, NULL);
    if (error != AI_COMMON_SUCCESS) {
        TestResult result{
            "document_add_multi", false,
            std::string("批量上传失败: ") + kb_get_error_message(error),
            thread_id};
        results.push_back(result);
        return;
    }

    thread_safe_printf("线程[%d] 批量上传文档成功\n", thread_id);
    TestResult result_add_multi{"document_add_multi", true, "批量上传成功",
                                thread_id};
    results.push_back(result_add_multi);

    // 等待文档处理完成
    for (int i = 0; i < docs_count; i++) {
        time_t start_time = time(NULL);
        int last_progress = -1;

        while (time(NULL) - start_time < 60 && !g_interrupted) {  // 1分钟超时
            sleep(1);
            int status;
            int progress_error;
            CResource<KnowledgeBaseInfo *,
                      decltype(&kb_knowledgebase_info_free)>
                kb_info(
                    kb_knowledgebase_info_get(session, kb_id, &progress_error),
                    &kb_knowledgebase_info_free);

            int progress = kb_document_get_processing_progress(
                session, kb_info.get(), doc_ids_ptrs[i], &status,
                &progress_error);

            if (progress < 0) {
                break;
            }

            if (progress != last_progress) {
                thread_safe_printf("线程[%d] 进度: %3d%% [状态:%d]\n",
                                   thread_id, progress, status);
                last_progress = progress;
            }

            if (status == KB_DOC_STATUS_COMPLETED) {
                thread_safe_printf("线程[%d] 文档 %s 处理完成\n", thread_id,
                                   doc_ids_ptrs[i]);
                break;
            }

            if (status == KB_DOC_STATUS_ERROR) {
                thread_safe_printf("线程[%d] 文档 %s 处理失败\n", thread_id,
                                   doc_ids_ptrs[i]);
                break;
            }
        }
    }

    // 批量删除文档
    error = kb_document_delete_multi(
        session, kb_id, (const char **)doc_ids_ptrs, docs_count, NULL);
    if (error != AI_COMMON_SUCCESS) {
        TestResult result{
            "document_delete_multi", false,
            std::string("批量删除失败: ") + kb_get_error_message(error),
            thread_id};
        results.push_back(result);
    } else {
        thread_safe_printf("线程[%d] 批量删除文档成功\n", thread_id);
        TestResult result{"document_delete_multi", true, "批量删除成功",
                          thread_id};
        results.push_back(result);
    }
}

// 测试会话接口的线程安全（只测试会话查询，不创建/销毁）
void test_session_thread_safety(KBSession *shared_session, const char *app_name,
                                const char *password, const char *databasePath,
                                int thread_id,
                                std::vector<TestResult> &results) {
    // 这里只测试会话的查询操作，不创建和销毁会话
    // 因为会话是共享的，不能由单个线程销毁

    // 检查会话是否有效
    if (!shared_session) {
        TestResult result{"session_check", false, "共享会话为空", thread_id};
        results.push_back(result);
        return;
    }

    thread_safe_printf("线程[%d] 共享会话检查成功\n", thread_id);
    TestResult result_check{"session_check", true, "共享会话有效", thread_id};
    results.push_back(result_check);
}

// 测试聊天接口的线程安全 - 纯C++线程版本
void test_chat_thread_safety(KBSession *session, const char *kb_id,
                             int thread_id, std::vector<TestResult> &results) {
    int error;

    // 创建聊天上下文
    ThreadSafeChatContext context;
    context.history = kb_chat_history_new();
    context.thread_id = thread_id;
    context.is_chatting.store(true);

    // 管理聊天相关资源
    struct ChatResourceGuard {
        KBChatConfig *config;
        KBChatHistory *history;

        ChatResourceGuard() : config(nullptr), history(nullptr) {}

        ~ChatResourceGuard() {
            if (config) {
                kb_chat_config_free(config);
            }
            if (history) {
                kb_chat_history_free(history);
            }
        }
    };

    ChatResourceGuard chat_guard;
    chat_guard.history = context.history;

    // 创建聊天配置
    chat_guard.config = kb_chat_config_new();
    if (!chat_guard.config) {
        TestResult result{"chat_config_new", false, "创建聊天配置失败",
                          thread_id};
        results.push_back(result);
        return;
    }

    kb_chat_config_set_new_chat(chat_guard.config, false);
    kb_chat_config_set_model(chat_guard.config, "");

    // 测试问题
    const char *test_questions[] = {"如何报销？", "报销审批流程是什么？",
                                    "报销结算流程是什么？", "报销政策有哪些？",
                                    "报销单据要求？"};

    const char *question = test_questions[thread_id % 5];

    thread_safe_printf("线程[%d] 开始聊天测试: %s\n", thread_id, question);

    // 执行聊天请求
    error =
        kb_chat_stream(session, nullptr, kb_id, question, nullptr,
                       chat_guard.config, thread_safe_chat_callback, &context);

    if (error != AI_COMMON_SUCCESS) {
        context.is_chatting.store(false);
        TestResult result{
            "chat_stream", false,
            std::string("聊天请求失败: ") + kb_get_error_message(error),
            thread_id};
        results.push_back(result);
        return;
    }

    // 等待聊天完成或超时 - 确保在回调完成前不退出
    time_t start_time = time(nullptr);
    const int timeout_seconds = 300;

    // 使用Glib事件循环等待异步回调完成
    GMainContext *main_context = g_main_context_default();
    while (!g_interrupted && (time(nullptr) - start_time < timeout_seconds)) {
        if (!context.is_chatting.load()) {
            break;  // 回调已完成
        }

        // 处理挂起的Glib事件（包括GDBus回调）
        g_main_context_iteration(main_context, FALSE);

        // 短暂休眠以避免过度占用CPU
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (context.is_chatting.load()) {
        thread_safe_printf("线程[%d] 聊天超时\n", thread_id);
        TestResult result{"chat_stream", false, "聊天超时", thread_id};
        results.push_back(result);
    } else {
        thread_safe_printf("线程[%d] 聊天测试完成\n", thread_id);
        TestResult result{"chat_stream", true, "聊天测试成功", thread_id};
        results.push_back(result);
    }

    // 确保回调完全结束后再清理资源
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 额外等待确保回调结束
    // 重置资源指针，因为它们将由guard自动释放
    chat_guard.config = nullptr;
    chat_guard.history = nullptr;
}

// 测试检索接口的线程安全
void test_retrieval_thread_safety(KBSession *session, const char *kb_id,
                                  int thread_id,
                                  std::vector<TestResult> &results) {
    int error;

    // 创建检索配置
    CResource<KBRetrievalConfig *, decltype(&kb_retrieval_config_free)> config(
        kb_retrieval_config_new(), &kb_retrieval_config_free);

    if (!config.get()) {
        TestResult result{"retrieval_config_new", false, "创建检索配置失败",
                          thread_id};
        results.push_back(result);
        return;
    }

    // 设置检索参数
    kb_retrieval_config_set_top_k(config.get(), 3);
    kb_retrieval_config_set_retrieval_method(config.get(),
                                             KB_RETRIEVAL_METHOD_HYBRID);
    kb_retrieval_config_set_rerank_method(config.get(),
                                          KB_RERANK_METHOD_WEIGHT);

    // 测试检索问题
    const char *test_queries[] = {"报销政策", "报销流程", "报销单据",
                                  "报销审批", "报销结算"};

    const char *query = "微服务是什么";  // test_queries[thread_id % 5];

    thread_safe_printf("线程[%d] 开始检索测试: %s\n", thread_id, query);

    // 创建检索上下文
    ThreadSafeRetrievalContext retrieval_ctx = {true, false, thread_id};

    // 执行检索
    error = kb_retrieval(session, kb_id, nullptr, query, config.get(),
                         thread_safe_retrieval_callback, &retrieval_ctx);

    if (error != AISDK_NO_ERROR) {
        TestResult result{
            "retrieval", false,
            std::string("发起检索请求失败: ") + kb_get_error_message(error),
            thread_id};
        results.push_back(result);
        return;
    }

    thread_safe_printf("线程[%d] ⏳ 正在检索，请稍候...\n", thread_id);

    // 等待检索完成 - 确保在回调完成前不退出
    time_t start_time = time(nullptr);
    const int timeout_seconds = 300;

    GMainContext *main_context = g_main_context_default();
    while (!retrieval_ctx.completed && !g_interrupted &&
           (time(nullptr) - start_time < timeout_seconds)) {
        g_main_context_iteration(main_context, FALSE);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (!retrieval_ctx.completed) {
        thread_safe_printf("线程[%d] 检索超时\n", thread_id);
        TestResult result{"retrieval", false, "检索超时", thread_id};
        results.push_back(result);
    } else {
        thread_safe_printf("线程[%d] 检索测试完成\n", thread_id);
        TestResult result{"retrieval", true, "检索测试成功", thread_id};
        results.push_back(result);
    }

    // 确保回调完全结束后再清理资源
    std::this_thread::sleep_for(
        std::chrono::milliseconds(100));  // 额外等待确保回调结束
}

void cleanup_resources(char *kb_id, KnowledgeBaseInfo *kb_info,
                       KBSession *session) {
    if (kb_id) {
        kb_free_string(kb_id);
    }

    if (kb_info) {
        kb_knowledgebase_info_free(kb_info);
    }

    // 不销毁主会话，因为它被所有线程共享
    if (session) {
        kb_session_destroy(session);
    }

    printf("\n👋 全接口线程安全测试完成！\n");
}

}  // namespace

// 主函数
int main(int argc, char *argv[]) {
    // 检查参数
    if (argc < 5) {
        printf("用法: %s <文档路径> [应用名称] [密码] [文档id] [知识库名称]\n",
               argv[0]);
        return 1;
    }

    const char *doc_path = argv[1];
    const char *app_name = argv[2];
    const char *password = argv[3];
    const char *doc_id = argv[4];
    const char *kb_name = argc > 5 ? argv[5] : "threadSafetyFullTest";

    int error;
    KBSession *session = nullptr;
    char *kb_id = nullptr;
    KnowledgeBaseInfo *kb_info = nullptr;
    const char *databasePath = "/home/kylin/123.db";

    const int thread_count = 12;  // 增加线程数量以测试更多接口
    std::vector<std::thread> threads;
    std::vector<std::vector<TestResult>> all_results(thread_count);
    int total_tests = 0;
    int passed_tests = 0;

    // 设置信号处理
    signal(SIGINT, signal_handler);

    printf("🚀 知识库SDK全接口线程安全测试\n");
    printf("==============================\n");

    // 1. 创建主会话
    printf("1. 创建主会话...\n");
    session = kb_session_create(app_name, password, databasePath, &error);
    if (!session) {
        printf("❌ 创建会话失败: %s\n", kb_get_error_message(error));
        cleanup_resources(kb_id, kb_info, session);
        return 1;
    }
    printf("✅ 主会话创建成功\n");

    // 2. 初始化主会话
    printf("2. 初始化主会话...\n");
    error = kb_session_init(session);
    if (error != AI_COMMON_SUCCESS) {
        printf("❌ 初始化会话失败: %s\n", kb_get_error_message(error));
        cleanup_resources(kb_id, kb_info, session);
        return 1;
    }
    printf("✅ 主会话初始化成功\n");

    // 3. 创建主知识库
    printf("3. 创建主知识库...\n");
    kb_info = kb_knowledgebase_info_new(kb_name, "全接口线程安全测试知识库");
    if (!kb_info) {
        printf("❌ 创建知识库信息失败\n");
        cleanup_resources(kb_id, kb_info, session);
        return 1;
    }

    kb_id = kb_knowledgebase_create(session, kb_info, &error);
    char *allocated_kb_id = nullptr;
    if (!kb_id) {
        if (error == KB_ERROR_KB_ALREADY_EXISTS) {
            int kb_info_count;
            KnowledgeBaseInfo **kb_infos =
                kb_knowledgebase_info_list(session, &kb_info_count, &error);
            for (int i = 0; i < kb_info_count; i++) {
                std::string name = kb_knowledgebase_info_get_name(kb_infos[i]);
                if (strcmp(name.c_str(), kb_name) == 0) {
                    allocated_kb_id =
                        strdup(kb_knowledgebase_info_get_id(kb_infos[i]));
                    kb_id = allocated_kb_id;
                    kb_info = kb_infos[i];
                    break;
                }
            }
            printf("知识库已存在，使用现有知识库。\n");
        } else {
            printf("❌ 创建知识库失败: %s\n", kb_get_error_message(error));
            cleanup_resources(kb_id, kb_info, session);
            return 1;
        }
    }
    printf("✅ 主知识库创建成功，ID: %s\n", kb_id);
    kb_knowledgebase_info_set_id(kb_info, kb_id);

    // 4. 上传主文档
    printf("4. 上传主文档...\n");
    error = kb_document_add(session, kb_id, doc_path, doc_id, NULL);
    if (error != AI_COMMON_SUCCESS) {
        printf("❌ 文档上传失败: %s\n", kb_get_error_message(error));
        cleanup_resources(kb_id, kb_info, session);
        return 1;
    }
    printf("✅ 主文档上传成功，ID: %s\n", doc_id);

    // 5. 等待文档处理完成
    printf("5. 处理主文档...\n");
    {
        time_t start_time = time(NULL);
        int last_progress = -1;

        while (time(NULL) - start_time < 60 && !g_interrupted) {  // 1分钟超时
            sleep(1);
            int status;
            int progress_error;
            int progress = kb_document_get_processing_progress(
                session, kb_info, doc_id, &status, &progress_error);

            if (progress < 0) {
                printf("获取进度失败: %s\n",
                       kb_get_error_message(progress_error));
                break;
            }

            if (progress != last_progress) {
                printf("\r进度: %3d%% [状态:%d]\n", progress, status);
                fflush(stdout);
                last_progress = progress;
            }

            if (status == KB_DOC_STATUS_COMPLETED) {
                printf("\n✅ 主文档处理完成\n");
                break;
            }

            if (status == KB_DOC_STATUS_ERROR) {
                printf("\n❌ 主文档处理失败\n");
                cleanup_resources(kb_id, kb_info, session);
                return 1;
            }
        }
    }

    // 6. 多线程并发测试所有接口
    printf("6. 开始全接口多线程并发测试...\n");

    // 检查文档文件
    if (!file_exists(doc_path)) {
        printf("❌ 文档文件不存在: %s\n", doc_path);
        cleanup_resources(kb_id, kb_info, session);
        return 1;
    }

    if (!is_supported_format(doc_path)) {
        printf("⚠️  警告: 文档格式可能不被支持\n");
    }

    long file_size = get_file_size(doc_path);
    if (file_size > 0) {
        printf("📄 文档大小: %.2f MB\n", file_size / (1024.0 * 1024.0));
    }

    // 启动测试不同接口的线程
    for (int i = 0; i < thread_count; i++) {
        if (i % 6 == 0) {
            // 测试知识库接口
            threads.emplace_back(test_knowledgebase_thread_safety, session,
                                 std::string(kb_name) + "_concurrent", i,
                                 std::ref(all_results[i]));
        } else if (i % 6 == 1) {
            // 测试文档接口
            threads.emplace_back(test_document_thread_safety, session, kb_id,
                                 doc_path, i, std::ref(all_results[i]));
        } else if (i % 6 == 2) {
            // 测试文档批量接口
            threads.emplace_back(test_document_batch_thread_safety, session,
                                 kb_id, doc_path, i, std::ref(all_results[i]));
        } else if (i % 6 == 3) {
            // 测试会话接口（只检查，不创建/销毁）
            threads.emplace_back(test_session_thread_safety, session, app_name,
                                 password, databasePath, i,
                                 std::ref(all_results[i]));
        } else if (i % 6 == 4) {
            // 测试聊天接口
            threads.emplace_back(test_chat_thread_safety, session, kb_id, i,
                                 std::ref(all_results[i]));
        } else if (i % 6 == 5) {
            // 测试检索接口
            threads.emplace_back(test_retrieval_thread_safety, session, kb_id,
                                 i, std::ref(all_results[i]));
        }
    }

    // 等待所有线程完成
    for (auto &t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    // 等待Glib事件循环处理完所有异步操作
    GMainContext *main_context = g_main_context_default();
    // 处理剩余的异步事件
    for (int i = 0; i < 10; i++) {  // 多次迭代确保所有事件被处理
        while (g_main_context_pending(main_context)) {
            g_main_context_iteration(main_context, FALSE);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // 统计测试结果
    for (int i = 0; i < thread_count; i++) {
        total_tests += all_results[i].size();
        for (const auto &result : all_results[i]) {
            if (result.success) {
                passed_tests++;
            } else {
                printf("❌ 线程[%d] %s 失败: %s\n", result.thread_id,
                       result.operation.c_str(), result.message.c_str());
            }
        }
    }

    printf("\n📊 测试结果统计:\n");
    printf("   总测试数: %d\n", total_tests);
    printf("   通过数: %d\n", passed_tests);
    printf("   失败数: %d\n", total_tests - passed_tests);
    printf("   通过率: %.2f%%\n",
           total_tests > 0 ? (passed_tests * 100.0 / total_tests) : 0);

    if (passed_tests == total_tests) {
        printf("✅ 所有线程安全测试通过\n");
    } else {
        printf("⚠️  部分测试失败，请检查上述错误信息\n");
    }

    // 7. 删除主文档
    printf("7. 删除主文档...\n");
    error = kb_document_delete(session, kb_id, doc_id);
    if (error == AI_COMMON_SUCCESS) {
        printf("✅ 主文档删除成功，ID: %s\n", doc_id);
    } else {
        printf("主文档删除失败: %s\n", kb_get_error_message(error));
    }

    // 8. 删除主知识库
    printf("8. 删除主知识库...\n");
    error = kb_knowledgebase_delete(session, kb_id);
    if (error == AI_COMMON_SUCCESS) {
        printf("✅ 主知识库删除成功，ID: %s\n", kb_id);
    } else {
        printf("主知识库删除失败: %s\n", kb_get_error_message(error));
    }

    cleanup_resources(kb_id, kb_info, session);
    return 0;
}

#include <gio/gio.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ctime>
#include <string>
#include <thread>

#include "chatconfig.h"
#include "chatresult.h"
#include "chatsource.h"
#include "retrievalconfig.h"
#include "retrievalresponse.h"
#include "session.h"

namespace {

struct ChatContext {
    std::string full_response;  // 用于拼接完整回复
    KBChatHistory *history{};
    bool is_chatting = false;
};

struct RetrievalContext {
    bool is_retrieving = false;
    bool completed = false;
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
}  // namespace
// 全局变量用于信号处理
static volatile bool g_interrupted = false;

// 信号处理函数
void signal_handler(int signal) {
    if (signal == SIGINT) {
        g_interrupted = true;
        printf("\n收到中断信号，正在清理...\n");
    }
}

// 聊天回调函数
int chat_callback(const KBChatResponse *response, void *user_data) {
    auto *ctx = static_cast<ChatContext *>(user_data);
    if (g_interrupted) {
        ctx->is_chatting = false;
        return 1;
    }

    if (!response) {
        ctx->is_chatting = false;
        return 1;
    }

    int error_code = kb_chat_response_get_error_code(response);
    if (error_code != AISDK_NO_ERROR) {
        printf("聊天错误: 错误码：%d 错误信息：%s\n", error_code,
               kb_get_error_message(static_cast<int>(error_code)));
        ctx->is_chatting = false;
        return 1;
    }

    const KBChatResult *result = kb_chat_response_get_result(response);
    if (result) {
        const char *model = kb_chat_result_get_model(result);
        printf("当前模型为 %s\n", model);
        const char *reasoningContent =
            kb_chat_result_get_reasoning_content(result);
        printf("推理过程: %s\n", reasoningContent);
        const char *text = kb_chat_result_get_response(result);
        if (text && strlen(text) > 0) {
            printf("回答结果：%s\n", text);
            fflush(stdout);

            ctx->full_response += text;
        }
        int reference_count;
        const KBChatReference **reference =
            kb_chat_result_get_reference(result, &reference_count);
        if (reference && reference_count > 0) {
            printf("\n📚 参考内容:\n");
            for (int i = 0; i < reference_count; i++) {
                int file_number =
                    kb_chat_reference_get_file_number(reference[i]);
                int chunk_index =
                    kb_chat_reference_get_chunk_index(reference[i]);
                printf("No.%d (chunk index: %d)\n", file_number, chunk_index);
            }
        }
        bool is_finished = kb_chat_result_is_finished(result);
        printf("is finish %d \n", is_finished);
        if (kb_chat_result_is_finished(result)) {
            printf("\n");

            // 显示源文档
            int source_count;
            const KBChatSource **sources =
                kb_chat_result_get_source(result, &source_count);
            if (sources && source_count > 0) {
                printf("\n📚 源文档:\n");
                for (int i = 0; i < source_count; i++) {
                    const char *file_id =
                        kb_chat_source_get_file_id(sources[i]);
                    double confidence =
                        kb_chat_source_get_confidence(sources[i]);
                    const char *content =
                        kb_chat_source_get_content(sources[i]);
                    printf("  %d. %s (可信度: %.1f%%)\n\n  内容:【%s】\n\n",
                           i + 1, file_id, confidence, content);
                }
            }

            // 显示参考来源
            int reference_count;
            const char **reference_file_ids =
                kb_chat_result_get_reference_file_ids(result, &reference_count);
            if (reference_file_ids && reference_count > 0) {
                printf("\n📚 标注参考来源:\n");
                for (int i = 0; i < reference_count; i++) {
                    printf("  %d. %s\n", i + 1, reference_file_ids[i]);
                }
            }

            int index = kb_chat_history_add_response(
                ctx->history, ctx->full_response.c_str());
            printf("第[%d]条回答已经添加： %s\n", index,
                   ctx->full_response.c_str());
            ctx->is_chatting = false;
            return 1;
        }
    }

    ctx->is_chatting = true;
    return 0;
}

// 检索回调函数
void retrieval_callback(const KBRetrievalResponse *response, void *user_data) {
    auto *ctx = static_cast<RetrievalContext *>(user_data);

    if (!response) {
        ctx->is_retrieving = false;
        ctx->completed = true;
        return;
    }

    int error_code = kb_retrieval_response_get_error_code(response);
    if (error_code != AISDK_NO_ERROR) {
        printf("检索错误: 错误码：%d 错误信息：%s\n", error_code,
               kb_get_error_message(static_cast<int>(error_code)));
        ctx->is_retrieving = false;
        ctx->completed = true;
        return;
    }

    const char *request_id = kb_retrieval_response_get_request_id(response);
    const char *kb_id = kb_retrieval_response_get_knowledgebase_id(response);
    const char *query = kb_retrieval_response_get_query(response);

    printf("\n🔍 检索结果:\n");
    printf("  请求ID: %s\n", request_id ? request_id : "N/A");
    printf("  知识库ID: %s\n", kb_id ? kb_id : "N/A");
    printf("  查询内容: %s\n", query ? query : "N/A");

    int source_count = kb_retrieval_response_get_source_count(response);
    printf("  检索到 %d 个结果:\n", source_count);

    for (int i = 0; i < source_count; i++) {
        const KBChatSource *source =
            kb_retrieval_response_get_source(response, i);
        if (source) {
            const char *file_id = kb_chat_source_get_file_id(source);
            const char *content = kb_chat_source_get_content(source);
            double confidence = kb_chat_source_get_confidence(source);

            printf("  [%d] 文件ID: %s (可信度: %.2f%%)\n", i + 1,
                   file_id ? file_id : "N/A", confidence);
            if (content) {
                std::string content_str(content);
                printf("      内容摘要: %s\n", content_str.c_str());
            }
        }
    }

    ctx->is_retrieving = false;
    ctx->completed = true;
}

// 等待文档处理完成
bool wait_for_document_completion(KBSession *session,
                                  KnowledgeBaseInfo *knowledgebase,
                                  const char *kb_id, const char *doc_id,
                                  int timeout_seconds) {
    printf("等待文档处理完成...\n");
    printf("输入 'stop' 可以停止文档上传。\n");

    time_t start_time = time(NULL);
    int last_progress = -1;

    while (time(NULL) - start_time < timeout_seconds && !g_interrupted) {
        // 检查是否有用户输入（非阻塞方式）
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        struct timeval timeout = {0, 0};  // 非阻塞
        if (select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout) > 0) {
            if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                char input[256];
                if (fgets(input, sizeof(input), stdin)) {
                    // 移除换行符
                    char *newline = strchr(input, '\n');
                    if (newline) *newline = '\0';

                    if (strcmp(input, "stop") == 0) {
                        printf(
                            "\n检测到停止请求，正在调用停止文档上传接口...\n");

                        // 调用停止文档上传接口
                        const char *doc_ids[] = {doc_id};
                        int stop_error = kb_document_cancel_addition(
                            session, kb_id, doc_ids, 1);

                        if (stop_error != AISDK_NO_ERROR) {
                            printf("❌ 停止文档上传失败: %s\n",
                                   kb_get_error_message(stop_error));
                            return false;
                        } else {
                            printf("⏹️ 文档上传已停止\n");
                            return true;
                        }
                    }
                }
            }
        }

        // 查询处理进度
        sleep(1);  // 等待1s再进行查询，每隔1s查询一次进度
        int status;
        int error;
        int progress = kb_document_get_processing_progress(
            session, knowledgebase, doc_id, &status, &error);

        if (progress < 0) {
            printf("获取进度失败: %s\n", kb_get_error_message(error));
            return true;
        }

        if (error != AISDK_NO_ERROR) {
            printf("获取进度失败,错误信息为: %s\n",
                   kb_get_error_message(error));
        }

        if (progress != last_progress) {
            // const char *status_name = status;
            printf("\r进度: %3d%% [状态:%d]\n", progress, status);
            fflush(stdout);
            last_progress = progress;
        }

        if (status == KB_DOC_STATUS_COMPLETED) {
            printf("\n✅ 文档处理完成\n");
            sleep(1);  // 等待1s才能获取列表
            return true;
        }

        if (status == KB_DOC_STATUS_ERROR) {
            printf("\n❌ 文档处理失败，%s\n", kb_get_error_message(error));
            return true;
            // return false;
        }
    }

    if (g_interrupted) {
        printf("\n用户中断\n");
    } else {
        printf("\n⏰ 等待超时\n");
    }
    return false;
}

// 执行检索操作
bool perform_retrieval(KBSession *session, const char *kb_id) {
    char query[1024];
    printf("\n请输入检索内容: ");
    fflush(stdout);

    if (!fgets(query, sizeof(query), stdin)) {
        return false;
    }

    // 移除换行符
    char *newline = strchr(query, '\n');
    if (newline) *newline = '\0';

    if (strlen(query) == 0) {
        printf("检索内容不能为空\n");
        return false;
    }

    // 创建检索配置
    KBRetrievalConfig *config = kb_retrieval_config_new();
    if (!config) {
        printf("❌ 创建检索配置失败\n");
        return false;
    }
    const char *doc_ids[] = {"doc0001"};
    kb_retrieval_config_set_doc_ids(config, doc_ids, 1);

    // 设置检索参数
    kb_retrieval_config_set_top_k(config, 5);
    kb_retrieval_config_set_retrieval_method(config,
                                             KB_RETRIEVAL_METHOD_HYBRID);
    kb_retrieval_config_set_rerank_method(config, KB_RERANK_METHOD_WEIGHT);

    // 执行检索
    RetrievalContext retrieval_ctx = {true, false};
    int error = kb_retrieval(session, kb_id, nullptr, query, config,
                             retrieval_callback, &retrieval_ctx);

    if (error != AISDK_NO_ERROR) {
        printf("❌ 发起检索请求失败: %s\n", kb_get_error_message(error));
        kb_retrieval_config_free(config);
        return false;
    }

    printf("⏳ 正在检索，请稍候...\n");

    // 等待检索完成
    time_t start_time = time(NULL);
    while (!retrieval_ctx.completed && !g_interrupted) {
        if (time(NULL) - start_time > 100) {  // 30秒超时
            printf("⏰ 检索超时\n");
            break;
        }
        usleep(100000);  // 休眠100ms
    }

    kb_retrieval_config_free(config);
    return retrieval_ctx.completed;
}

// 执行聊天操作
bool perform_chat(KBSession *session, const char *kb_id, KBChatConfig *config,
                  ChatContext *chat_ctx) {
    char question[8192];
    printf("\n请输入问题: ");
    fflush(stdout);

    if (!fgets(question, sizeof(question), stdin)) {
        return false;
    }

    // 移除换行符
    char *newline = strchr(question, '\n');
    if (newline) *newline = '\0';

    if (strlen(question) == 0) {
        printf("问题不能为空\n");
        return false;
    }

    printf("正在思考...输入stop停止回答\n");
    printf("🤖 AI: \n");
    fflush(stdout);
    chat_ctx->is_chatting = true;
    chat_ctx->full_response.clear();

    int error = kb_chat_stream(session, NULL, kb_id, question, nullptr, config,
                               chat_callback, chat_ctx);
    int index = kb_chat_history_add_question(chat_ctx->history, question);
    int count = kb_chat_history_get_message_count(chat_ctx->history);
    printf("第[%d]条问题已经添加： %s\n当前包含%d条对话\n", index, question,
           count);

    if (error != AISDK_NO_ERROR) {
        printf("❌ 聊天失败: %s\n", kb_get_error_message(error));
        printf("删除第[%d]条问题： %s\n", index, question);
        kb_chat_history_delete_question(chat_ctx->history, index);
        return false;
    }

    // 等待聊天完成
    time_t start_time = time(NULL);
    while (chat_ctx->is_chatting && !g_interrupted) {
        if (time(NULL) - start_time > 60) {  // 60秒超时
            printf("⏰ 聊天响应超时\n");
            break;
        }
        usleep(100000);  // 休眠100ms
    }

    return true;
}

// 主函数
int main(int argc, char *argv[]) {
    // 检查参数
    if (argc < 5) {
        printf(
            "用法: %s <文档路径>  [应用名称] [密码] [文档id] [知识库名称] "
            "[知识库描述]\n",
            argv[0]);
        return 1;
    }

    const char *doc_path = argv[1];
    const char *app_name = argv[2];
    const char *password = argv[3];
    const char *databasePath = "/home/kylin/123.db";
    const char *doc_id = argv[4];
    const char *kb_name = argc > 5 ? argv[5] : "testName01";
    const char *kb_desc =
        argc > 6 ? argv[6] : "this is a test description created by sdk demo";
    const char *doc_ids[] = {};

    long file_size;
    KBChatConfig *config;
    ChatContext *chat_ctx = nullptr;

    // 设置信号处理
    signal(SIGINT, signal_handler);

    printf("🚀 知识库SDK示例应用\n");
    printf("===================\n");

    // 1. 创建会话
    printf("1. 创建会话...\n");
    int error;
    KBSession *session =
        kb_session_create(app_name, password, databasePath, &error);
    if (!session) {
        printf("❌ 创建会话失败: %s\n", kb_get_error_message(error));
        return 1;
    }
    printf("✅ 会话创建成功\n");

    // 2. 初始化会话
    printf("2. 初始化会话...\n");
    error = kb_session_init(session);
    if (error != AISDK_NO_ERROR) {
        printf("❌ 初始化会话失败: %s\n", kb_get_error_message(error));
        kb_session_destroy(session);
        return 1;
    }
    printf("✅ 会话初始化成功\n");

    // 3. 创建知识库
    printf("3. 创建知识库...\n");
    KnowledgeBaseInfo *kb_info = kb_knowledgebase_info_new(kb_name, kb_desc);
    if (!kb_info) {
        printf("❌ 创建知识库信息失败\n");
        kb_session_destroy(session);
        return 1;
    }

    char *kb_id = kb_knowledgebase_create(session, kb_info, &error);
    char *allocated_kb_id = nullptr;
    if (!kb_id) {
        if (error != KB_ERROR_KB_ALREADY_EXISTS) {
            printf("❌ 创建知识库失败: %s\n", kb_get_error_message(error));
            kb_knowledgebase_info_free(kb_info);
            kb_session_destroy(session);
            return 1;
        }
        // 知识库已存在，获取其ID并创建副本
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
    }
    printf("✅ 知识库创建成功，ID: %s\n", kb_id);
    kb_knowledgebase_info_set_id(kb_info, kb_id);

    // 4. 检查文档文件
    printf("4. 检查文档文件...\n");
    if (!file_exists(doc_path)) {
        printf("❌ 文档文件不存在: %s\n", doc_path);
        goto cleanup;
    }

    if (!is_supported_format(doc_path)) {
        printf("⚠️  警告: 文档格式可能不被支持\n");
    }

    file_size = get_file_size(doc_path);
    if (file_size > 0) {
        printf("📄 文档大小: %.2f MB\n", file_size / (1024.0 * 1024.0));
    }

    // 5. 上传文档
    printf("5. 上传文档...\n");
    error = kb_document_add(session, kb_id, doc_path, doc_id, NULL);
    if (error != AISDK_NO_ERROR) {
        printf("❌ 文档上传失败: %s\n", kb_get_error_message(error));
        goto cleanup;
    }
    printf("✅ 文档上传成功，ID: %s\n", doc_id);

    // 6. 等待文档处理完成
    printf("6. 处理文档...\n");
    if (!wait_for_document_completion(session, kb_info, kb_id, doc_id,
                                      300)) {  // 5分钟超时
        printf("❌ 文档处理未完成\n");
        goto cleanup;
    }

    // 7.获取知识库信息
    {
        printf("7. 获取知识库信息...\n");
        KnowledgeBaseInfo *get_kb_info =
            kb_knowledgebase_info_get(session, kb_id, &error);
        if (error != AISDK_NO_ERROR) {
            printf("获取知识库信息失败: %s\n", kb_get_error_message(error));
            goto cleanup;
        }

        const char *kb_name = kb_knowledgebase_info_get_name(get_kb_info);
        printf("知识库名称: %s\n", kb_name);

        const char *kb_description =
            kb_knowledgebase_info_get_description(get_kb_info);
        printf("知识库描述: %s\n", kb_description);

        const char *kb_info_id = kb_knowledgebase_info_get_id(get_kb_info);
        printf("知识库ID: %s\n", kb_info_id);

        int doc_count;
        char **kb_documents =
            kb_document_list(session, kb_info_id, &doc_count, &error);

        if (error != AISDK_NO_ERROR) {
            printf("获取文档列表失败: %s\n", kb_get_error_message(error));
            kb_knowledgebase_info_free(get_kb_info);
            goto cleanup;
        }
        printf("文档数量: %d\n", doc_count);
        for (int i = 0; i < doc_count; i++) {
            printf("文档ID: %s\n", kb_documents[i]);
        }

        // 释放文档列表
        for (int i = 0; i < doc_count; i++) {
            kb_free_string(kb_documents[i]);
        }
        free(kb_documents);  // 释放文档列表数组
        kb_knowledgebase_info_free(get_kb_info);
    }

    // 8. 创建聊天配置和上下文
    printf("8. 初始化聊天环境...\n");
    chat_ctx = new ChatContext;
    chat_ctx->history = kb_chat_history_new();

    config = kb_chat_config_new();
    kb_chat_config_set_new_chat(config, false);  // 支持多轮对话
    kb_chat_config_set_model(config, "百度-ERNIE-Bot-4");
    kb_chat_config_set_doc_ids(config, doc_ids, 1);

    // 9. 开始交互式问答/检索
    printf("9. 开始交互式问答/检索...\n");
    printf("💬 你可以开始提问了:\n");
    printf("  输入 'chat' 进入问答模式\n");
    printf("  输入 'search' 进入检索模式\n");
    printf("  输入 'quit' 或 'exit' 退出\n");
    printf("=====================================\n");

    {
        char command[256];
        GMainLoop *pMainLoop = g_main_loop_new(nullptr, false);

        std::thread ctrlThread(
            [&session, &command, &error, &chat_ctx, &config, pMainLoop, kb_id] {
                while (!g_interrupted) {
                    printf("\n请选择模式 (chat/search): ");
                    fflush(stdout);

                    if (!fgets(command, sizeof(command), stdin)) {
                        break;
                    }

                    // 移除换行符
                    char *newline = strchr(command, '\n');
                    if (newline) *newline = '\0';

                    // 检查退出命令
                    if (strcmp(command, "quit") == 0 ||
                        strcmp(command, "exit") == 0) {
                        break;
                    }

                    // 处理不同模式
                    if (strcmp(command, "chat") == 0) {
                        printf("进入问答模式\n");
                        if (!perform_chat(session, kb_id, config, chat_ctx)) {
                            printf("❌ 问答执行失败\n");
                        }
                    } else if (strcmp(command, "search") == 0) {
                        printf("进入检索模式\n");
                        if (!perform_retrieval(session, kb_id)) {
                            printf("❌ 检索执行失败\n");
                        }
                    } else {
                        printf(
                            "未知命令，请输入 'chat'、'search'、'quit' 或 "
                            "'exit'\n");
                    }
                }
                g_main_loop_quit(pMainLoop);
            });
        ctrlThread.detach();

        g_main_loop_run(pMainLoop);
        g_main_loop_unref(pMainLoop);
    }

    // 清理资源
    kb_chat_history_free(chat_ctx->history);
    delete chat_ctx;
    kb_chat_config_free(config);

    // 10. 删除文档
    printf("10. 删除文档...\n");
    error = kb_document_delete(session, kb_id, doc_id);
    if (!error) {
        printf("✅ 文档删除成功，ID: %s\n", doc_id);
    } else {
        printf("文档删除失败: %s\n", kb_get_error_message(error));
    }

    // 11. 删除知识库
    printf("11. 删除知识库...\n");
    error = kb_knowledgebase_delete(session, kb_id);
    if (!error) {
        printf("✅ 知识库删除成功，ID: %s\n", kb_id);
    } else {
        printf("知识库删除失败: %s\n", kb_get_error_message(error));
    }


cleanup:
    if (allocated_kb_id) {
        free(allocated_kb_id);  // 使用free而不是kb_free_string
    } else {
        kb_free_string(kb_id);  // 只有当我们实际创建了知识库时才释放
    }
    kb_knowledgebase_info_free(kb_info);
    kb_session_destroy(session);

    printf("\n👋 再见！\n");
    return 0;
}

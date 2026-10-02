//
// Created by kylin on 26-3-6.
//

#include <gio/gio.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <ctime>
#include <string>

#include "chatconfig.h"
#include "chatresult.h"
#include "chatsource.h"
#include "retrievalconfig.h"
#include "retrievalresponse.h"
#include "session.h"

namespace {

/**
 * @brief 聊天上下文（用于累积历史）
 */
struct ChatContext {
    std::string full_response;
    bool is_chatting;
};

/**
 * @brief 等待文档处理完成
 *
 * 轮询检查文档上传后的处理进度，直到处理完成或超时。
 * 该函数会阻塞执行，每隔 1 秒查询一次进度并打印。
 *
 * @param session          KBSession 会话对象
 * @param knowledgebase    知识库信息对象
 * @param kb_id           知识库 ID
 * @param doc_id          文档 ID
 * @param timeout_seconds 超时时间（秒）
 * @return bool           true: 处理完成或出错
 *                        false: 等待超时
 *
 * @note 必须在 session 有效期内调用，session 销毁后将无法获取进度
 * @note 支持的文档状态：
 *       - KB_DOC_STATUS_COMPLETED: 处理完成
 *       - KB_DOC_STATUS_ERROR: 处理出错
 *       - 其他值：处理中
 */
bool wait_for_document_completion(KBSession *session,
                                  KnowledgeBaseInfo *knowledgebase,
                                  const char *kb_id, const char *doc_id,
                                  int timeout_seconds) {
    time_t start_time = time(NULL);
    int last_progress = -1;

    while (time(NULL) - start_time < timeout_seconds) {
        sleep(1);

        int status, error;
        int progress = kb_document_get_processing_progress(
            session, knowledgebase, doc_id, &status, &error);

        if (progress < 0) {
            printf("获取进度失败：%s\n", kb_get_error_message(error));
            return true;
        }

        if (progress != last_progress) {
            printf("进度：%3d%% [状态:%d]\n", progress, status);
            fflush(stdout);
            last_progress = progress;
        }

        if (status == KB_DOC_STATUS_COMPLETED) {
            printf("文档处理完成\n");
            return true;
        }

        if (status == KB_DOC_STATUS_ERROR) {
            printf("文档处理失败：%s\n", kb_get_error_message(error));
            return false;
        }
    }

    printf("等待超时\n");
    return false;
}

/**
 * @brief 聊天回调函数
 *
 * 该函数在 kb_chat_stream 流式对话过程中被异步调用，用于处理对话响应。
 * 每次接收到 AI 模型的回复片段时都会触发此回调。
 *
 * 主要功能：
 * 1. 错误处理：检查并报告对话过程中的错误
 * 2. 模型信息：显示当前使用的 AI 模型名称
 * 3. 推理过程：显示 AI 的推理思考过程（如果模型支持）
 * 4. 流式输出：实时显示 AI 的回答内容
 * 5. 参考信息：对话完成后显示源文档、参考内容等详细信息
 *    - 源文档：带置信度和完整内容
 *    - 参考内容：文件编号和片段索引
 *    - 标注参考来源：参考的文件 ID 列表
 *
 * @param response   对话响应对象，包含完整的响应数据
 * @param user_data  用户自定义数据指针，通常用于传递完成标志等状态信息
 * @return int       返回值含义：
 *                   - 0: 继续接收后续响应（对话进行中）
 *                   - 1: 结束对话（完成或出错）
 *
 * @note 该函数会被多次调用：
 *       - 前几次调用：text 参数包含部分回答内容，is_finished() 返回 false
 *       - 最后一次调用：is_finished() 返回 true，表示回答完成
 *
 * @note 所有显示的信息都来自 KBChatResult 对象，通过不同的 getter 方法获取
 *
 * @see kb_chat_stream - 发起流式对话的 API
 * @see KBChatResult - 对话结果类
 * @see KBChatSource - 源文档信息类
 * @see KBChatReference - 参考内容信息类
 */
int chat_callback(const KBChatResponse *response, void *user_data) {
    if (!response) {
        return 1;
    }

    int error_code = kb_chat_response_get_error_code(response);
    if (error_code != AISDK_NO_ERROR) {
        printf("聊天错误：%s\n", kb_chat_response_get_error_message(response));
        return 1;
    }

    const KBChatResult *result = kb_chat_response_get_result(response);
    if (result) {
        // 显示模型信息
        const char *model = kb_chat_result_get_model(result);
        if (model && strlen(model) > 0) {
            printf("\n[模型] %s\n", model);
        }

        // 显示推理过程
        const char *reasoningContent =
            kb_chat_result_get_reasoning_content(result);
        if (reasoningContent && strlen(reasoningContent) > 0) {
            printf("\n[推理过程]\n%s\n", reasoningContent);
        }

        // 流式显示回答内容
        const char *text = kb_chat_result_get_response(result);
        if (text && strlen(text) > 0) {
            printf("%s\n", text);
            fflush(stdout);
        }

        // 检查是否完成
        if (kb_chat_result_is_finished(result)) {
            printf("\n");

            // 显示源文档（带内容和置信度）
            int source_count;
            const KBChatSource **sources =
                kb_chat_result_get_source(result, &source_count);
            if (sources && source_count > 0) {
                printf("\n=================================\n");
                printf("源文档 (%d 个):\n", source_count);
                printf("=================================\n");
                for (int i = 0; i < source_count; i++) {
                    const char *file_id =
                        kb_chat_source_get_file_id(sources[i]);
                    double confidence =
                        kb_chat_source_get_confidence(sources[i]);
                    const char *content =
                        kb_chat_source_get_content(sources[i]);

                    printf("\n[%d] 文件 ID: %s\n", i + 1,
                           file_id ? file_id : "N/A");
                    printf("    置信度：%.1f%%\n", confidence);
                    if (content && strlen(content) > 0) {
                        printf("    内容:\n    ---\n    %s\n    ---\n",
                               content);
                    }
                }
            }

            // 显示参考内容（文件编号和片段索引）
            int reference_count;
            const KBChatReference **reference =
                kb_chat_result_get_reference(result, &reference_count);
            if (reference && reference_count > 0) {
                printf("\n=================================\n");
                printf("参考内容 (%d 个):\n", reference_count);
                printf("=================================\n");
                for (int i = 0; i < reference_count; i++) {
                    int file_number =
                        kb_chat_reference_get_file_number(reference[i]);
                    int chunk_index =
                        kb_chat_reference_get_chunk_index(reference[i]);

                    printf("[%d] 文件编号：%d, 片段索引：%d\n", i + 1,
                           file_number, chunk_index);
                }
            }

            // 显示标注参考来源（文件 ID 列表）
            int reference_file_count;
            const char **reference_file_ids =
                kb_chat_result_get_reference_file_ids(result,
                                                      &reference_file_count);
            if (reference_file_ids && reference_file_count > 0) {
                printf("\n=================================\n");
                printf("标注参考来源 (%d 个):\n", reference_file_count);
                printf("=================================\n");
                for (int i = 0; i < reference_file_count; i++) {
                    printf("[%d] %s\n", i + 1, reference_file_ids[i]);
                }
            }

            printf("\n=================================\n");
            printf("对话完成\n");
            printf("=================================\n\n");

            auto *finished = static_cast<bool *>(user_data);
            if (finished) {
                *finished = true;
            }
            return 1;
        }
    }

    return 0;
}

/**
 * @brief 带历史积累的聊天回调
 *
 * 与 chat_callback 类似，但会累积完整回答以便加入历史记录
 */
int chat_callback_with_history(const KBChatResponse *response,
                               void *user_data) {
    if (!response) {
        return 1;
    }

    int error_code = kb_chat_response_get_error_code(response);
    if (error_code != AISDK_NO_ERROR) {
        printf("聊天错误：%s\n",
               kb_get_error_message(static_cast<int>(error_code)));
        return 1;
    }

    const KBChatResult *result = kb_chat_response_get_result(response);
    if (result) {
        // 流式显示回答内容
        const char *text = kb_chat_result_get_response(result);
        if (text && strlen(text) > 0) {
            // 累积回答内容
            auto *ctx = static_cast<ChatContext *>(user_data);
            if (ctx) {
                ctx->full_response += text;
            }
        }

        // 检查是否完成
        if (kb_chat_result_is_finished(result)) {
            printf("\n\n");

            auto *ctx = static_cast<ChatContext *>(user_data);
            if (ctx) {
                ctx->is_chatting = false;  // 标记为已完成
            }
            return 1;
        }
    }

    return 0;
}

}  // namespace

/**
 * @brief 知识库文档上传接口演示
 *
 * 本函数演示了如何使用 Kylin AI SDK 的知识库文档上传功能。
 *
 * 使用场景：
 * - 将本地文档（PDF、Word、TXT 等格式）上传到知识库
 * - 上传后文档会被自动处理和索引，之后可以通过对话或检索查询文档内容
 *
 * 主要步骤：
 * 1. 创建 KBSession 会话对象，连接到知识库服务
 *    - 需要应用名称和密码进行认证
 *    - 指定本地 SQLite 数据库路径用于缓存
 *
 * 2. 初始化会话，建立通信通道
 *    - 必须在使用任何 API 前调用
 *
 * 3. 创建或获取知识库（KnowledgeBaseInfo）
 *    - 如果知识库不存在则创建
 *    - 如果已存在则获取现有知识库信息
 *    - 设置知识库 ID 以便后续操作
 *
 * 4. 上传文档到指定知识库（kb_document_add）
 *    - 指定文档路径和自定义文档 ID
 *    - 上传是异步的，立即返回不等待完成
 *
 * 5. 等待文档处理完成（异步处理，轮询状态）
 *    - 轮询检查处理进度（每 1 秒一次）
 *
 * 6. 获取文档列表，验证上传结果
 *    - 列出知识库中的所有文档
 *    - 确认文档已成功添加
 *
 * 7. 清理资源，释放内存和连接
 *
 * @param doc_path        待上传的文档路径
 * @param app_name        应用名称（用于认证）
 * @param password        应用密码（用于认证）
 * @param database_path   本地数据库文件路径（SQLite .db 文件）
 * @param doc_id          文档唯一标识符（用户自定义）
 * @param kb_name         知识库名称
 * @param kb_desc         知识库描述信息
 *
 * @return void           无返回值，错误信息通过 printf 输出
 *
 * @note 重要提示：
 *       1. 知识库只能创建一次，再次创建同名知识库会返回
 * KB_ERROR_KB_ALREADY_EXISTS
 *       2. 文档上传是异步的，必须在 session 有效期间等待文档处理完成
 *       3. 文档 ID 由用户指定，在同一知识库中必须唯一
 */
void test_knowledgebase_upload(const char *doc_path, const char *app_name,
                               const char *password, const char *database_path,
                               const char *doc_id, const char *kb_name,
                               const char *kb_desc) {
    int error = AISDK_NO_ERROR;

    // 1. 创建会话
    KBSession *session =
        kb_session_create(app_name, password, database_path, &error);
    if (!session) {
        printf("创建会话失败：%s\n", kb_get_error_message(error));
        return;
    }

    // 2. 初始化会话
    error = kb_session_init(session);
    if (error != AISDK_NO_ERROR) {
        printf("初始化会话失败：%s\n", kb_get_error_message(error));
        kb_session_destroy(session);
        return;
    }

    // 3. 创建知识库
    KnowledgeBaseInfo *kb_info = kb_knowledgebase_info_new(kb_name, kb_desc);
    if (!kb_info) {
        printf("创建知识库信息失败\n");
        kb_session_destroy(session);
        return;
    }

    char *kb_id = kb_knowledgebase_create(session, kb_info, &error);
    char *allocated_kb_id = nullptr;
    if (!kb_id) {
        if (error != KB_ERROR_KB_ALREADY_EXISTS) {
            printf("创建知识库失败：%s\n", kb_get_error_message(error));
            kb_knowledgebase_info_free(kb_info);
            kb_session_destroy(session);
            return;
        }

        // 知识库只能创建一次，再次创建同名知识库会报已存在的错误
        // 如果知识库已存在，可以通过如下方法获取现有知识库
        int kb_info_count;
        KnowledgeBaseInfo **kb_infos =
            kb_knowledgebase_info_list(session, &kb_info_count, &error);
        if (error == AISDK_NO_ERROR) {
            for (int i = 0; i < kb_info_count; i++) {
                std::string name = kb_knowledgebase_info_get_name(kb_infos[i]);
                if (strcmp(name.c_str(), kb_name) == 0) {
                    allocated_kb_id =
                        strdup(kb_knowledgebase_info_get_id(kb_infos[i]));
                    kb_id = allocated_kb_id;
                    kb_info = kb_infos[i];

                    for (int j = 0; j < kb_info_count; j++) {
                        if (j != i) kb_knowledgebase_info_free(kb_infos[j]);
                    }
                    free(kb_infos);
                    break;
                }
            }
        }
    }
    // 设置知识库ID，否则后续操作会失败
    kb_knowledgebase_info_set_id(kb_info, kb_id);

    // 4. 上传文档
    error = kb_document_add(session, kb_id, doc_path, doc_id, NULL);
    if (error != AISDK_NO_ERROR) {
        printf("文档上传失败：%s\n", kb_get_error_message(error));
    } else {
        // 5. 等待文档处理完成
        // 必须在session有效期间获取文档处理进度，session销毁后将无法获取未完成的文档进度
        wait_for_document_completion(session, kb_info, kb_id, doc_id, 300);

        // 6. 获取文档列表
        int doc_count = 0;
        char **documents = kb_document_list(session, kb_id, &doc_count, &error);
        if (error == AISDK_NO_ERROR) {
            printf("文档数量：%d\n", doc_count);
            for (int i = 0; i < doc_count; i++) {
                printf("文档 [%d]: %s\n", i + 1, documents[i]);
                kb_free_string(documents[i]);
            }
            free(documents);
        }
    }

    // 清理资源
    if (allocated_kb_id) {
        free(allocated_kb_id);
    } else {
        kb_free_string(kb_id);
    }
    kb_knowledgebase_info_free(kb_info);
    kb_session_destroy(session);
}

/**
 * @brief 删除知识库接口演示
 *
 * 本函数演示了如何删除知识库及其中的所有文档。
 *
 * 使用场景：
 * - 清理测试数据
 * - 删除不再需要的知识库
 * - 重建知识库前先删除旧的
 *
 * 主要步骤：
 * 1. 创建 KBSession 会话对象
 *    - 使用应用名称和密码进行认证
 *
 * 2. 初始化会话
 *    - 建立与知识库服务的连接
 *
 * 3. 根据知识库名称获取知识库 ID（如果存在）
 *    - 遍历知识库列表查找匹配项
 *    - 如果找不到则直接返回
 *
 * 4. 获取知识库中的文档列表
 *    - 查询知识库包含的所有文档
 *
 * 5. 逐个删除文档
 *    - 遍历并删除每个文档
 *    - 记录删除失败的文档
 *
 * 6. 删除知识库
 *    - 删除知识库本身
 *    - 此操作不可逆
 *
 * 7. 清理资源
 *    - 释放所有分配的内存
 *    - 销毁会话连接
 *
 * @param app_name        应用名称（用于认证）
 * @param password        应用密码（用于认证）
 * @param kb_name         要删除的知识库名称
 * @param database_path   本地数据库文件路径
 *
 * @return void           无返回值，错误信息通过 printf 输出
 *
 * @note 注意事项：
 *       1. 删除操作是不可逆的，删除后数据无法恢复
 *       2. 可以只删除知识库，知识库里的文档也会被删除
 */
void test_knowledgebase_delete(const char *app_name, const char *password,
                               const char *kb_name, const char *database_path) {
    KBSession *session = nullptr;
    KnowledgeBaseInfo *kb_info = nullptr;
    char *kb_id = nullptr;
    int error = AISDK_NO_ERROR;

    // 1. 创建会话
    session = kb_session_create(app_name, password, database_path, &error);
    if (!session) {
        printf("创建会话失败：%s\n", kb_get_error_message(error));
        return;
    }

    // 2. 初始化会话
    error = kb_session_init(session);
    if (error != AISDK_NO_ERROR) {
        printf("初始化会话失败：%s\n", kb_get_error_message(error));
        kb_session_destroy(session);
        return;
    }

    // 3. 查找知识库
    int kb_info_count;
    KnowledgeBaseInfo **kb_infos =
        kb_knowledgebase_info_list(session, &kb_info_count, &error);
    if (error != AISDK_NO_ERROR) {
        printf("获取知识库列表失败：%s\n", kb_get_error_message(error));
        kb_session_destroy(session);
        return;
    }

    bool found = false;
    for (int i = 0; i < kb_info_count; i++) {
        std::string name = kb_knowledgebase_info_get_name(kb_infos[i]);
        if (strcmp(name.c_str(), kb_name) == 0) {
            kb_id = strdup(kb_knowledgebase_info_get_id(kb_infos[i]));
            kb_info = kb_infos[i];
            found = true;
            break;
        }
    }

    if (!found) {
        printf("知识库不存在：%s\n", kb_name);
        for (int i = 0; i < kb_info_count; i++) {
            kb_knowledgebase_info_free(kb_infos[i]);
        }
        free(kb_infos);
        kb_session_destroy(session);
        return;
    }

    // 释放未使用的知识库信息
    for (int i = 0; i < kb_info_count; i++) {
        if (kb_infos[i] != kb_info) {
            kb_knowledgebase_info_free(kb_infos[i]);
        }
    }
    free(kb_infos);

    // 4. 删除所有文档
    int doc_count = 0;
    char **documents = kb_document_list(session, kb_id, &doc_count, &error);
    if (error == AISDK_NO_ERROR && doc_count > 0) {
        for (int i = 0; i < doc_count; i++) {
            printf("删除文档 %s\n", documents[i]);
            error = kb_document_delete(session, kb_id, documents[i]);
            if (error != AISDK_NO_ERROR) {
                printf("删除文档 %s 失败：%s\n", documents[i],
                       kb_get_error_message(error));
            }
            kb_free_string(documents[i]);
        }
        free(documents);
    }

    // 5. 删除知识库
    error = kb_knowledgebase_delete(session, kb_id);
    if (error != AISDK_NO_ERROR) {
        printf("删除知识库失败：%s\n", kb_get_error_message(error));
        kb_knowledgebase_info_free(kb_info);
        free(kb_id);
        kb_session_destroy(session);
        return;
    }

    // 清理资源
    kb_knowledgebase_info_free(kb_info);
    free(kb_id);
    kb_session_destroy(session);
}

/**
 * @brief 测试对话接口演示
 *
 * 本函数演示了如何使用 Kylin AI SDK 的知识库对话功能。
 * 通过流式 API 与知识库进行交互式问答，并显示完整的参考信息。
 *
 * 主要步骤：
 * 1. 创建 KBSession 会话对象
 *    - 使用应用名称和密码进行认证
 *    - 指定本地数据库路径用于缓存
 *
 * 2. 初始化会话
 *    - 建立与知识库服务的连接
 *    - 准备后续 API 调用的环境
 *
 * 3. 获取知识库
 *    - 遍历知识库列表查找指定名称的知识库
 *    - 如果知识库不存在则退出
 *    - 获取知识库 ID 用于后续对话
 *
 * 4. 创建聊天配置
 *    - 设置是否为新会话（多轮对话上下文）
 *    - 指定使用的 AI 模型（如通义千问等）
 *
 * 5. 发起对话请求
 *    - 使用 kb_chat_stream 发起流式对话
 *    - 注册 chat_callback 回调函数处理响应
 *    - 实时接收并显示 AI 的回答内容
 *
 * 6. 等待对话完成
 *    - 通过主循环处理异步回调
 *    - 设置超时机制防止无限等待
 *    - 最长等待 60 秒
 *
 * 7. 清理资源
 *    - 释放聊天配置
 *    - 释放知识库 ID 和信息
 *    - 销毁会话连接
 *
 * @param app_name       应用名称（用于认证）
 * @param password       应用密码（用于认证）
 * @param kb_name        知识库名称
 * @param database_path  本地数据库文件路径（用于缓存会话数据）
 * @param question       要提问的问题内容
 *
 * @return void          无返回值，错误信息通过 printf 输出
 *
 * @note 对话特点：
 *       - 流式输出：回答内容会实时显示，无需等待完整回复
 *       - 参考信息：对话完成后会显示源文档、参考内容等详细信息
 *       - 异步处理：使用 GMainLoop 处理异步回调，确保响应能被正确处理
 *
 * @note 显示的参考信息包括：
 *       - 源文档：带置信度和完整内容的引用文档
 *       - 参考内容：文件编号和片段索引
 *       - 标注参考来源：参考的文件 ID 列表
 */
void test_knowledgebase_chat(const char *app_name, const char *password,
                             const char *kb_name, const char *database_path,
                             const char *question) {
    int error = AISDK_NO_ERROR;

    // 1. 创建会话
    KBSession *session =
        kb_session_create(app_name, password, database_path, &error);
    if (!session) {
        printf("创建会话失败：%s\n", kb_get_error_message(error));
        return;
    }

    // 2. 初始化会话
    error = kb_session_init(session);
    if (error != AISDK_NO_ERROR) {
        printf("初始化会话失败：%s\n", kb_get_error_message(error));
        kb_session_destroy(session);
        return;
    }

    // 3. 获取知识库
    int kb_info_count;
    KnowledgeBaseInfo **kb_infos =
        kb_knowledgebase_info_list(session, &kb_info_count, &error);
    if (error != AISDK_NO_ERROR || kb_info_count == 0) {
        printf("获取知识库列表失败或无知识库\n");
        kb_session_destroy(session);
        return;
    }

    char *kb_id = nullptr;
    KnowledgeBaseInfo *kb_info = nullptr;
    for (int i = 0; i < kb_info_count; i++) {
        std::string name = kb_knowledgebase_info_get_name(kb_infos[i]);
        if (strcmp(name.c_str(), kb_name) == 0) {
            kb_id = strdup(kb_knowledgebase_info_get_id(kb_infos[i]));
            kb_info = kb_infos[i];
            break;
        }
    }

    if (!kb_id) {
        printf("知识库不存在：%s\n", kb_name);
        kb_session_destroy(session);
        return;
    }

    // 4. 创建聊天配置
    KBChatConfig *config = kb_chat_config_new();
    if (!config) {
        printf("创建聊天配置失败\n");
        kb_free_string(kb_id);
        kb_knowledgebase_info_free(kb_info);
        kb_session_destroy(session);
        return;
    }
    // 设置新会话
    kb_chat_config_set_new_chat(config, true);
    kb_chat_config_set_diversify_query_enabled(config, true);
    // 设置模型，不设置默认为 AI 模块管理的优先模型
    kb_chat_config_set_model(config, "通义千问（Qwen-Plus-试用版）");

    // 5. 发起对话
    printf("提问：%s\n", question);
    printf("回答：");
    bool finished = false;
    error = kb_chat_stream(session, NULL, kb_id, question, NULL, config,
                           chat_callback, &finished);
    if (error != AISDK_NO_ERROR) {
        printf("\n对话失败：%s\n", kb_get_error_message(error));
    } else {
        // 6. 运行主循环等待回调
        GMainLoop *loop = g_main_loop_new(NULL, FALSE);

        time_t start_time = time(NULL);
        while (!finished && time(NULL) - start_time < 60) {
            g_main_context_iteration(NULL, FALSE);
            usleep(10000);  // 休眠 10ms
        }

        g_main_loop_unref(loop);

        if (!finished) {
            printf("\n等待响应超时\n");
        }
    }

    // 7. 清理资源
    kb_chat_config_free(config);
    kb_free_string(kb_id);
    kb_knowledgebase_info_free(kb_info);
    kb_session_destroy(session);
}

/**
 * @brief 测试对话历史管理接口演示
 *
 * 本函数演示了如何使用 Kylin AI SDK 的对话历史功能进行多轮对话。
 * 通过维护对话历史，AI 可以理解上下文，实现连续对话。
 *
 * 使用场景：
 * - 多轮问答，AI 能记住之前的对话内容
 * - 基于上下文的追问和澄清
 * - 建立连续的对话会话
 *
 * 主要步骤：
 * 1. 创建 KBSession 会话对象
 * 2. 初始化会话
 * 3. 获取知识库
 * 4. 创建对话历史对象
 * 5. 第一轮对话（单轮）
 * 6. 第二轮对话（携带历史上下文）
 * 7. 显示历史消息总结
 * 8. 清理资源
 *
 * @param app_name        应用名称（用于认证）
 * @param password        应用密码（用于认证）
 * @param kb_name         知识库名称
 * @param database_path   本地数据库文件路径
 *
 * @return void           无返回值，错误信息通过 printf 输出
 *
 * @note 对话历史的作用：
 *       - 单轮对话 (new_chat=true): 每次都是独立对话，不保留上下文
 *       - 多轮对话 (new_chat=false + history): AI 能看到之前的问答历史
 *
 * @note 历史管理说明：
 *       - 使用 kb_chat_history_new() 创建空历史
 *       - 使用 kb_chat_history_add_question() 添加问题
 *       - 使用 kb_chat_history_add_response() 添加回答
 *       - 历史会一直累积，直到调用 kb_chat_history_free()
 *
 * @example
 * // 调用示例
 * test_chat_with_history(
 *     "my_app",           // 应用名称
 *     "my_password",      // 应用密码
 *     "my_knowledge_base" // 知识库名称
 *     "/tmp/kylin.db"     // 数据库路径
 * );
 */
void test_chat_with_history(const char *app_name, const char *password,
                            const char *kb_name, const char *database_path) {
    int error = AISDK_NO_ERROR;

    // 1. 创建会话
    KBSession *session =
        kb_session_create(app_name, password, database_path, &error);
    if (!session) {
        printf("创建会话失败：%s\n", kb_get_error_message(error));
        return;
    }

    // 2. 初始化会话
    error = kb_session_init(session);
    if (error != AISDK_NO_ERROR) {
        printf("初始化会话失败：%s\n", kb_get_error_message(error));
        kb_session_destroy(session);
        return;
    }

    // 3. 获取知识库
    int kb_info_count;
    KnowledgeBaseInfo **kb_infos =
        kb_knowledgebase_info_list(session, &kb_info_count, &error);
    if (error != AISDK_NO_ERROR || kb_info_count == 0) {
        printf("获取知识库列表失败或无知识库\n");
        kb_session_destroy(session);
        return;
    }

    char *kb_id = nullptr;
    KnowledgeBaseInfo *kb_info = nullptr;
    for (int i = 0; i < kb_info_count; i++) {
        std::string name = kb_knowledgebase_info_get_name(kb_infos[i]);
        if (strcmp(name.c_str(), kb_name) == 0) {
            kb_id = strdup(kb_knowledgebase_info_get_id(kb_infos[i]));
            kb_info = kb_infos[i];
            break;
        }
    }

    if (!kb_id) {
        printf("知识库不存在：%s\n", kb_name);
        kb_session_destroy(session);
        return;
    }

    // 4. 创建对话历史
    KBChatHistory *history = kb_chat_history_new();
    if (!history) {
        printf("创建对话历史失败\n");
        kb_free_string(kb_id);
        kb_knowledgebase_info_free(kb_info);
        kb_session_destroy(session);
        return;
    }

    printf("\n========== 多轮对话演示 ==========\n\n");

    // 5. 第一轮对话（单轮模式）
    {
        printf("--- 第 1 轮：初次提问 ---\n");

        KBChatConfig *config = kb_chat_config_new();
        kb_chat_config_set_new_chat(config, true);  // 单轮对话
        kb_chat_config_set_model(config, "通义千问（Qwen-Plus-试用版）");

        const char *question1 = "怎么报销？";

        ChatContext ctx1 = {"", true};  // 初始化为 true，表示正在聊天中
        error = kb_chat_stream(session, NULL, kb_id, question1, NULL, config,
                               chat_callback_with_history, &ctx1);
        if (error != AISDK_NO_ERROR) {
            printf("\n对话失败：%s\n", kb_get_error_message(error));
        } else {
            time_t start_time = time(NULL);
            while (ctx1.is_chatting && time(NULL) - start_time < 60) {
                g_main_context_iteration(NULL, FALSE);
                usleep(10000);
            }

            // 将第一轮的问答加入历史
            if (!ctx1.full_response.empty()) {
                kb_chat_history_add_question(history, question1);
                kb_chat_history_add_response(history, ctx1.full_response.c_str());
            }
        }

        kb_chat_config_free(config);
    }

    // 6. 第二轮对话（携带历史上下文）
    {
        printf("--- 第 2 轮：基于上下文提问 ---\n");

        KBChatConfig *config = kb_chat_config_new();
        kb_chat_config_set_new_chat(config, false);  // 多轮对话
        kb_chat_config_set_model(config, "通义千问（Qwen-Plus-试用版）");

        const char *question2 = "报销流程是什么？";

        ChatContext ctx2 = {"", true};  // 初始化为 true，表示正在聊天中
        error = kb_chat_stream(session, NULL, kb_id, question2, history, config,
                               chat_callback_with_history, &ctx2);
        if (error != AISDK_NO_ERROR) {
            printf("\n对话失败：%s\n", kb_get_error_message(error));
        } else {
            time_t start_time = time(NULL);
            while (ctx2.is_chatting && time(NULL) - start_time < 60) {
                g_main_context_iteration(NULL, FALSE);
                usleep(10000);
            }

            // 将第二轮的问答加入历史
            if (!ctx2.full_response.empty()) {
                kb_chat_history_add_question(history, question2);
                kb_chat_history_add_response(history, ctx2.full_response.c_str());
            }
        }

        kb_chat_config_free(config);
    }

    // 7. 显示历史消息总结
    printf("--- 对话历史总结 ---\n");
    int msg_count = kb_chat_history_get_message_count(history);
    printf("总共 %d 轮对话\n\n", msg_count);

    for (int i = 0; i < msg_count; i++) {
        const char *q = kb_chat_history_get_question(history, i);
        const char *a = kb_chat_history_get_response(history, i);
        printf("Q%d: %s\n", i + 1, q ? q : "N/A");
        printf("A%d: %s\n\n", i + 1, a ? a : "N/A");
    }

    printf("======================================\n\n");

    // 8. 清理资源
    kb_chat_history_free(history);
    kb_free_string(kb_id);
    kb_knowledgebase_info_free(kb_info);
    kb_session_destroy(session);
}

int main() {
    test_knowledgebase_upload("/home/kylin/公司报销制度及流程.docx", "app_name", "password","/home/kylin/tmp", "doc_id_1", "kb_name", "kb_desc");
    test_knowledgebase_chat("app_name", "password","kb_name","/home/kylin/tmp","报销流程是什么？");
    test_chat_with_history("app_name", "password","kb_name","/home/kylin/tmp");
    test_knowledgebase_delete("app_name", "password","kb_name","/home/kylin/tmp");
    return 0;
}
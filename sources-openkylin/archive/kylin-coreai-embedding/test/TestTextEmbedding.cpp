#include <condition_variable>
#include <mutex>
#include <nlohmann/json.hpp>
#include <thread>

#include "mocks/CoreaiEmbeddingMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

void textEmbeddingCallback(EmbeddingResult *result, void *callback_user_data) {
    std::cout << "Start textEmbeddingCallback: " << std::endl;
    EXPECT_EQ(embedding_result_get_error_code(result), 0);

    float *vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    std::cout << "vector_length: " << vector_length << std::endl;
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 768);
}

void textCallback(EmbeddingResult *result, void *callback_user_data) {
    std::cout << "Start callback: " << std::endl;
}

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Success_Basic_1) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, const gchar *arg_model,
                     gint *out_errorCode, GCancellable *cancellable,
                     GError **error) {
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, gchar **out_jsonOutput,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json modelList = {
                {"models",
                 {{{"dim", 1024},
                   {"displayName", "cn-clip_512-uint8"},
                   {"model",
                    "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-"
                    "uint8-image"},
                   {"modelName", "kylin-cn-clip_512-uint8-image"},
                   {"model_capability", "IMAGE"},
                   {"ondevice", true},
                   {"priority", 1}},
                  {{"dim", 768},
                   {"displayName", "gte-base_uint8"},
                   {"model", "ensemble-embd_gte-base_uint8-text"},
                   {"modelName", "kylin-ensemble-embd_gte-base_uint8-text"},
                   {"model_capability", "TEXT"},
                   {"ondevice", true},
                   {"priority", 1}}}}};

            *out_jsonOutput = g_strdup(modelList.dump().c_str());
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextEmbedding(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, gchar **out_json_output,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 768; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    text_embedding_init_session_no_connect(session_);
#endif

    const char *modeName = "ensemble-embd_gte-base_uint8-text";
    text_embedding_init_model(session_, modeName);

    std::string input_text = "公司报销制度及流程.";
    EmbeddingResult *result = nullptr;
    EXPECT_TRUE(text_embedding(session_, input_text.c_str(), &result));
    EXPECT_NE(result, nullptr);

    // 验证结果没有错误
    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    EXPECT_STREQ(embedding_result_get_error_message(result), "Success");

    // 验证向量数据
    float *vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 768);

    // 清理资源
    embedding_result_destroy(&result);
}

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Success_Basic_2) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    text_embedding_init_session_no_connect(session_);
#endif

    // 验证模型信息
    char *model_info = nullptr;
    EXPECT_TRUE(text_embedding_get_model_info(session_, &model_info));
    EXPECT_NE(model_info, nullptr);
    std::string modelInfo = model_info;
    std::cout << "modelInfo: " << modelInfo << std::endl;
    // 清理资源
    embedding_model_info_destroy(model_info);
}

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Success_Basic_3) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, const gchar *arg_model,
                     gint *out_errorCode, GCancellable *cancellable,
                     GError **error) {
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, gchar **out_jsonOutput,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json modelList = {
                {"models",
                 {{{"dim", 1024},
                   {"displayName", "cn-clip_512-uint8"},
                   {"model",
                    "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-"
                    "uint8-image"},
                   {"modelName", "kylin-cn-clip_512-uint8-image"},
                   {"model_capability", "IMAGE"},
                   {"ondevice", true},
                   {"priority", 1}},
                  {{"dim", 768},
                   {"displayName", "gte-base_uint8"},
                   {"model", "ensemble-embd_gte-base_uint8-text"},
                   {"modelName", "kylin-ensemble-embd_gte-base_uint8-text"},
                   {"model_capability", "TEXT"},
                   {"ondevice", true},
                   {"priority", 1}}}}};

            *out_jsonOutput = g_strdup(modelList.dump().c_str());
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextEmbeddingAsync(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, GCancellable *cancellable,
                     GAsyncReadyCallback callback, gpointer user_data) {
            GTask *task = g_task_new(proxy, cancellable, callback, user_data);

            // 立即完成任务（模拟异步操作立即完成）
            g_task_return_boolean(task, TRUE);
            g_object_unref(task);

            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextEmbeddingAsyncFinish(_, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gchar **out_json_output, GAsyncResult *res,
                     GError **error) {
            // 创建模拟的响应JSON
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 768; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });
    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    text_embedding_init_session_no_connect(session_);
#endif

    const char *modeName = "ensemble-embd_gte-base_uint8-text";
    text_embedding_init_model(session_, modeName);

    text_embedding_async(session_, "hello world!", textEmbeddingCallback,
                         nullptr);
    auto *loop = g_main_loop_new(nullptr, false);
    std::thread Thread([loop] {
        // 等待5s后退出
        sleep(5);
        g_main_loop_quit(loop);
    });
    Thread.detach();

    g_main_loop_run(loop);
    std::cout << "loop stop" << std ::endl;
}

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Success_Basic_4) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, const gchar *arg_model,
                     gint *out_errorCode, GCancellable *cancellable,
                     GError **error) {
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, gchar **out_jsonOutput,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json modelList = {
                {"models",
                 {{{"dim", 1024},
                   {"displayName", "cn-clip_512-uint8"},
                   {"model",
                    "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-"
                    "uint8-image"},
                   {"modelName", "kylin-cn-clip_512-uint8-image"},
                   {"model_capability", "IMAGE"},
                   {"ondevice", true},
                   {"priority", 1}},
                  {{"dim", 768},
                   {"displayName", "gte-base_uint8"},
                   {"model", "ensemble-embd_gte-base_uint8-text"},
                   {"modelName", "kylin-ensemble-embd_gte-base_uint8-text"},
                   {"model_capability", "TEXT"},
                   {"ondevice", true},
                   {"priority", 1}}}}};

            *out_jsonOutput = g_strdup(modelList.dump().c_str());
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextEmbeddingAsync(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, GCancellable *cancellable,
                     GAsyncReadyCallback callback, gpointer user_data) {
            GTask *task = g_task_new(proxy, cancellable, callback, user_data);

            // 立即完成任务（模拟异步操作立即完成）
            g_task_return_boolean(task, TRUE);
            g_object_unref(task);

            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextEmbeddingAsyncFinish(_, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gchar **out_json_output, GAsyncResult *res,
                     GError **error) {
            // 创建模拟的响应JSON
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 768; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });
    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    text_embedding_init_session_no_connect(session_);
#endif

    const char *modeName = "ensemble-embd_gte-base_uint8-text";
    text_embedding_init_model(session_, modeName);

    // 开启内部事件循环
    text_embedding_enable_internal_event_loop(session_, true);
    text_embedding_async(session_, "hello world!", textEmbeddingCallback,
                         nullptr);
    // 等待结果
    sleep(5);
    // 关闭事件循环
    text_embedding_enable_internal_event_loop(session_, false);
    std::cout << "loop stop" << std ::endl;
}

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Success_Basic_5) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, const gchar *arg_model,
                     gint *out_errorCode, GCancellable *cancellable,
                     GError **error) {
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, gchar **out_jsonOutput,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json modelList = {
                {"models",
                 {{{"dim", 1024},
                   {"displayName", "cn-clip_512-uint8"},
                   {"model",
                    "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-"
                    "uint8-image"},
                   {"modelName", "kylin-cn-clip_512-uint8-image"},
                   {"model_capability", "IMAGE"},
                   {"ondevice", true},
                   {"priority", 1}},
                  {{"dim", 768},
                   {"displayName", "gte-base_uint8"},
                   {"model", "ensemble-embd_gte-base_uint8-text"},
                   {"modelName", "kylin-ensemble-embd_gte-base_uint8-text"},
                   {"model_capability", "TEXT"},
                   {"ondevice", true},
                   {"priority", 1}}}}};

            *out_jsonOutput = g_strdup(modelList.dump().c_str());
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextEmbedding(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, gchar **out_json_output,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 768; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    text_embedding_init_session_no_connect(session_);
#endif

    const EmbeddingModelList *modelList = nullptr;
    modelList = text_embedding_get_model_list(session_, nullptr);
    int errorCode;
    int num = embedding_model_list_get_count(modelList, &errorCode);

    const _EmbeddingModelInfo *modelInfo =
        embedding_model_list_get_model(modelList, 0, nullptr);
    const char *modeName =
        embedding_model_info_get_model_name(modelInfo, nullptr);
    EXPECT_STREQ(modeName, "ensemble-embd_gte-base_uint8-text");
    int dim = embedding_model_info_get_model_dim(modelInfo, nullptr);
    EXPECT_EQ(dim, 768);
    text_embedding_init_model(session_, modeName);

    std::string input_text = "Hello, world!";
    EmbeddingResult *result = nullptr;
    EXPECT_TRUE(text_embedding(session_, input_text.c_str(), &result));
    EXPECT_NE(result, nullptr);

    // 验证结果没有错误
    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    EXPECT_STREQ(embedding_result_get_error_message(result), "Success");

    // 验证向量数据
    float *vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 768);

    // 清理资源
    embedding_result_destroy(&result);
}
TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Success_Basic_6) {
    const EmbeddingModelList *modelList = nullptr;
    modelList = text_embedding_get_model_list(session_, nullptr);
    int errorCode;
    int num = embedding_model_list_get_count(modelList, &errorCode);

    const _EmbeddingModelInfo *modelInfo =
        embedding_model_list_get_model(modelList, 0, nullptr);
    const char *modeName =
        embedding_model_info_get_model_name(modelInfo, nullptr);
    EXPECT_STREQ(modeName, "ensemble-embd_gte-base_uint8-text");
    int dim = embedding_model_info_get_model_dim(modelInfo, nullptr);
    EXPECT_EQ(dim, 768);
    text_embedding_init_model(session_, modeName);

    text_embedding_async(session_, "hello world!", textEmbeddingCallback,
                         nullptr);

    auto *loop = g_main_loop_new(nullptr, false);
    std::thread Thread([loop] {
        // 等待5s后退出
        sleep(5);
        g_main_loop_quit(loop);
    });
    Thread.detach();

    g_main_loop_run(loop);
    std::cout << "loop stop" << std ::endl;
}

// ==================== 错误场景测试 ====================

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Error_Null_Session) {
    std::string input_text = "Hello, world!";
    EmbeddingResult *result = nullptr;

    // 测试空session
    EXPECT_FALSE(text_embedding(nullptr, input_text.c_str(), &result));
}

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Error_Null_Text) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });
    text_embedding_init_session_no_connect(session_);
#endif

    EmbeddingResult *result = nullptr;

    // 测试空文本
    EXPECT_FALSE(text_embedding(session_, nullptr, &result));
    EXPECT_EQ(result, nullptr);

    embedding_result_destroy(&result);
}

#ifdef COREAI_EMBEDDING_ENABLE_MOCK
TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Error_Service_Failure) {
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, const gchar *arg_model,
                     gint *out_errorCode, GCancellable *cancellable,
                     GError **error) {
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, gchar **out_jsonOutput,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json modelList = {
                {"models",
                 {{{"dim", 1024},
                   {"displayName", "cn-clip_512-uint8"},
                   {"model",
                    "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-"
                    "uint8-image"},
                   {"modelName", "kylin-cn-clip_512-uint8-image"},
                   {"model_capability", "IMAGE"},
                   {"ondevice", true},
                   {"priority", 1}},
                  {{"dim", 768},
                   {"displayName", "gte-base_uint8"},
                   {"model", "ensemble-embd_gte-base_uint8-text"},
                   {"modelName", "kylin-ensemble-embd_gte-base_uint8-text"},
                   {"model_capability", "TEXT"},
                   {"ondevice", true},
                   {"priority", 1}}}}};

            *out_jsonOutput = g_strdup(modelList.dump().c_str());
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextEmbedding(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, gchar **out_json_output,
                     GCancellable *cancellable, GError **error) {
            // 模拟服务返回错误
            nlohmann::json response_json;
            response_json["errorCode"] = 1001;
            response_json["errorMessage"] = "Service internal error";
            response_json["vectorResult"] = std::vector<float>();

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    text_embedding_init_session_no_connect(session_);

    std::string input_text = "Hello, world!";
    EmbeddingResult *result = nullptr;

    EXPECT_TRUE(text_embedding(session_, input_text.c_str(), &result));
    EXPECT_NE(result, nullptr);

    // 验证错误码和错误信息
    EXPECT_NE(embedding_result_get_error_code(result), 0);
    EXPECT_STRNE(embedding_result_get_error_message(result), "Success");

    // 验证向量数据为空
    float *vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    EXPECT_EQ(vector_length, 0);

    embedding_result_destroy(&result);
}
#endif

// ==================== 边界条件测试 ====================

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Empty_Text) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, const gchar *arg_model,
                     gint *out_errorCode, GCancellable *cancellable,
                     GError **error) {
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, gchar **out_jsonOutput,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json modelList = {
                {"models",
                 {{{"dim", 1024},
                   {"displayName", "cn-clip_512-uint8"},
                   {"model",
                    "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-"
                    "uint8-image"},
                   {"modelName", "kylin-cn-clip_512-uint8-image"},
                   {"model_capability", "IMAGE"},
                   {"ondevice", true},
                   {"priority", 1}},
                  {{"dim", 768},
                   {"displayName", "gte-base_uint8"},
                   {"model", "ensemble-embd_gte-base_uint8-text"},
                   {"modelName", "kylin-ensemble-embd_gte-base_uint8-text"},
                   {"model_capability", "TEXT"},
                   {"ondevice", true},
                   {"priority", 1}}}}};

            *out_jsonOutput = g_strdup(modelList.dump().c_str());
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextEmbedding(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, gchar **out_json_output,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            std::vector<float> vectorResult;
            for (int i = 0; i < 768; i++) {
                vectorResult.push_back(0.0f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    text_embedding_init_session_no_connect(session_);
#endif

    std::string empty_text = "";
    EmbeddingResult *result = nullptr;
    const char *modeName = "ensemble-embd_gte-base_uint8-text";
    text_embedding_init_model(session_, modeName);

    EXPECT_TRUE(text_embedding(session_, empty_text.c_str(), &result));
    EXPECT_NE(result, nullptr);

    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    EXPECT_EQ(embedding_result_get_vector_length(result), 768);

    embedding_result_destroy(&result);
}

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Long_Text) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, const gchar *arg_model,
                     gint *out_errorCode, GCancellable *cancellable,
                     GError **error) {
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, gchar **out_jsonOutput,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json modelList = {
                {"models",
                 {{{"dim", 1024},
                   {"displayName", "cn-clip_512-uint8"},
                   {"model",
                    "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-"
                    "uint8-image"},
                   {"modelName", "kylin-cn-clip_512-uint8-image"},
                   {"model_capability", "IMAGE"},
                   {"ondevice", true},
                   {"priority", 1}},
                  {{"dim", 768},
                   {"displayName", "gte-base_uint8"},
                   {"model", "ensemble-embd_gte-base_uint8-text"},
                   {"modelName", "kylin-ensemble-embd_gte-base_uint8-text"},
                   {"model_capability", "TEXT"},
                   {"ondevice", true},
                   {"priority", 1}}}}};

            *out_jsonOutput = g_strdup(modelList.dump().c_str());
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextEmbedding(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, gchar **out_json_output,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            std::vector<float> vectorResult;
            for (int i = 0; i < 768; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    text_embedding_init_session_no_connect(session_);
#endif

    // 测试长文本
    std::string long_text(1000, 'x');  // 1000个字符的长文本
    EmbeddingResult *result = nullptr;

    const char *modeName = "ensemble-embd_gte-base_uint8-text";
    text_embedding_init_model(session_, modeName);

    EXPECT_TRUE(text_embedding(session_, long_text.c_str(), &result));
    EXPECT_NE(result, nullptr);

    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    EXPECT_EQ(embedding_result_get_vector_length(result), 768);

    embedding_result_destroy(&result);
}

// ==================== 会话管理测试 ====================

TEST_F(CoreaiTextEmbeddingMockedTest, Session_Destroy_Null) {
    // 测试销毁空session
    TextEmbeddingSession *null_session = nullptr;
    text_embedding_destroy_session(&null_session);
    EXPECT_EQ(null_session, nullptr);
}

// ==================== 模型信息测试 ====================

TEST_F(CoreaiTextEmbeddingMockedTest, Model_Info_Null_Session) {
    char *model_info = nullptr;

    // 测试空session获取模型信息
    EXPECT_FALSE(text_embedding_get_model_info(nullptr, &model_info));
    EXPECT_EQ(model_info, nullptr);
}

// ==================== 资源清理测试 ====================

TEST_F(CoreaiTextEmbeddingMockedTest, Resource_Cleanup_Null_Result) {
    // 测试销毁空结果
    EmbeddingResult *null_result = nullptr;
    embedding_result_destroy(&null_result);
    EXPECT_EQ(null_result, nullptr);
}

TEST_F(CoreaiTextEmbeddingMockedTest, Resource_Cleanup_Null_Model_Info) {
    // 测试销毁空模型信息
    embedding_model_info_destroy(nullptr);
}

// ==================== 关键异常分支覆盖测试 ====================

#ifdef COREAI_EMBEDDING_ENABLE_MOCK
TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Cover_All_Error_Paths) {
    // 这个测试用例覆盖多个错误路径

    // 1. 测试初始化时的DBus错误
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, const gchar *arg_model,
                     gint *out_errorCode, GCancellable *cancellable,
                     GError **error) {
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, gchar **out_jsonOutput,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json modelList = {
                {"models",
                 {{{"dim", 1024},
                   {"displayName", "cn-clip_512-uint8"},
                   {"model",
                    "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-"
                    "uint8-image"},
                   {"modelName", "kylin-cn-clip_512-uint8-image"},
                   {"model_capability", "IMAGE"},
                   {"ondevice", true},
                   {"priority", 1}},
                  {{"dim", 768},
                   {"displayName", "gte-base_uint8"},
                   {"model", "ensemble-embd_gte-base_uint8-text"},
                   {"modelName", "kylin-ensemble-embd_gte-base_uint8-text"},
                   {"model_capability", "TEXT"},
                   {"ondevice", true},
                   {"priority", 1}}}}};

            *out_jsonOutput = g_strdup(modelList.dump().c_str());
            return true;
        });
    // 2. 测试同步调用DBus错误和空结果
    EXPECT_CALL(mock_, CoreaiTextEmbedding(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, gchar **out_json_output,
                     GCancellable *cancellable, GError **error) {
            // 第一次调用：DBus错误
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED, "DBus error");
            return false;
        })
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, gchar **out_json_output,
                     GCancellable *cancellable, GError **error) {
            // 第二次调用：空结果
            *out_json_output = nullptr;
            return true;
        })
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     const gchar *arg_json_input, gchar **out_json_output,
                     GCancellable *cancellable, GError **error) {
            // 第三次调用：无效JSON
            *out_json_output = g_strdup("invalid json {");
            return true;
        });

    text_embedding_init_session_no_connect(session_);

    // 测试DBus错误
    EmbeddingResult *result1 = nullptr;
    EXPECT_TRUE(text_embedding(session_, "test1", &result1));
    EXPECT_NE(result1, nullptr);
    EXPECT_NE(embedding_result_get_error_code(result1), 0);
    embedding_result_destroy(&result1);

    // 测试空结果
    EmbeddingResult *result2 = nullptr;
    EXPECT_TRUE(text_embedding(session_, "test2", &result2));
    EXPECT_NE(result2, nullptr);
    EXPECT_NE(embedding_result_get_error_code(result2), 0);
    embedding_result_destroy(&result2);

    // 测试无效JSON
    EmbeddingResult *result3 = nullptr;
    EXPECT_TRUE(text_embedding(session_, "test3", &result3));
    EXPECT_NE(result3, nullptr);
    EXPECT_NE(embedding_result_get_error_code(result3), 0);
    embedding_result_destroy(&result3);
}

TEST_F(CoreaiTextEmbeddingMockedTest, Text_Embedding_Async_Cover_Error_Paths) {
    // 覆盖异步调用的错误路径
    EXPECT_CALL(mock_, CoreaiTextInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint *out_sessionId, gint *out_errorCode,
                     GCancellable *cancellable, GError **error) {
            *out_sessionId = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiTextInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, const gchar *arg_model,
                     gint *out_errorCode, GCancellable *cancellable,
                     GError **error) {
            *out_errorCode = 0;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                     gint arg_sessionId, gchar **out_jsonOutput,
                     GCancellable *cancellable, GError **error) {
            nlohmann::json modelList = {
                {"models",
                 {{{"dim", 1024},
                   {"displayName", "cn-clip_512-uint8"},
                   {"model",
                    "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-"
                    "uint8-image"},
                   {"modelName", "kylin-cn-clip_512-uint8-image"},
                   {"model_capability", "IMAGE"},
                   {"ondevice", true},
                   {"priority", 1}},
                  {{"dim", 768},
                   {"displayName", "gte-base_uint8"},
                   {"model", "ensemble-embd_gte-base_uint8-text"},
                   {"modelName", "kylin-ensemble-embd_gte-base_uint8-text"},
                   {"model_capability", "TEXT"},
                   {"ondevice", true},
                   {"priority", 1}}}}};

            *out_jsonOutput = g_strdup(modelList.dump().c_str());
            return true;
        });

    text_embedding_init_session_no_connect(session_);

    // 测试第一个异步调用：DBus错误
    {
        EXPECT_CALL(mock_, CoreaiTextEmbeddingAsync(_, _, _, _, _))
            .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                         const gchar *arg_json_input, GCancellable *cancellable,
                         GAsyncReadyCallback callback, gpointer user_data) {
                GTask *task =
                    g_task_new(proxy, cancellable, callback, user_data);
                g_task_return_boolean(task, TRUE);
                g_object_unref(task);
                return true;
            });

        EXPECT_CALL(mock_, CoreaiTextEmbeddingAsyncFinish(_, _, _, _))
            .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                         gchar **out_json_output, GAsyncResult *res,
                         GError **error) {
                // 模拟异步DBus错误
                *error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED,
                                     "Async DBus error");
                return false;
            });

        struct CallbackContext {
            bool called = false;
            int error_code = 0;
            GMainLoop *loop;
        } context1;

        auto error_callback = [](EmbeddingResult *result, void *user_data) {
            auto *context = static_cast<CallbackContext *>(user_data);
            context->called = true;
            context->error_code = embedding_result_get_error_code(result);
            if (context->loop) {
                g_main_loop_quit(context->loop);
            }
        };

        GMainLoop *loop1 = g_main_loop_new(nullptr, FALSE);
        context1.loop = loop1;
        context1.called = false;

        text_embedding_async(session_, "test async dbus error", error_callback,
                             &context1);

        // 运行主循环等待回调
        g_main_loop_run(loop1);
        g_main_loop_unref(loop1);
        context1.loop = nullptr;

        EXPECT_TRUE(context1.called);
    }

    // 测试第二个异步调用：空结果
    {
        EXPECT_CALL(mock_, CoreaiTextEmbeddingAsync(_, _, _, _, _))
            .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                         const gchar *arg_json_input, GCancellable *cancellable,
                         GAsyncReadyCallback callback, gpointer user_data) {
                GTask *task =
                    g_task_new(proxy, cancellable, callback, user_data);
                g_task_return_boolean(task, TRUE);
                g_object_unref(task);
                return true;
            });

        EXPECT_CALL(mock_, CoreaiTextEmbeddingAsyncFinish(_, _, _, _))
            .WillOnce([](AiRuntimeCoreTextEmbeddingService *proxy,
                         gchar **out_json_output, GAsyncResult *res,
                         GError **error) {
                // 模拟异步空结果
                *out_json_output = nullptr;
                return true;
            });

        struct CallbackContext {
            bool called = false;
            int error_code = 0;
            GMainLoop *loop;
        } context2;

        auto error_callback = [](EmbeddingResult *result, void *user_data) {
            auto *context = static_cast<CallbackContext *>(user_data);
            context->called = true;
            context->error_code = embedding_result_get_error_code(result);
            if (context->loop) {
                g_main_loop_quit(context->loop);
            }
        };

        GMainLoop *loop2 = g_main_loop_new(nullptr, FALSE);
        context2.loop = loop2;
        context2.called = false;

        text_embedding_async(session_, "test async null result", error_callback,
                             &context2);

        // 运行主循环等待回调
        g_main_loop_run(loop2);
        g_main_loop_unref(loop2);
        context2.loop = nullptr;

        EXPECT_TRUE(context2.called);
    }
}

#endif
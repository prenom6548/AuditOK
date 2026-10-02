#include <filesystem>
#include <nlohmann/json.hpp>
#include <thread>

#include "mocks/CoreaiEmbeddingMockedTest.h"

using ::testing::_;

// ==================== 正常功能测试 (Happy Path) ====================

void textEmbeddingByImageModelCallback(EmbeddingResult* result,
                                       void* callback_user_data) {
    std::cout << "Start textEmbeddingByImageModelCallback: " << std::endl;
    EXPECT_EQ(embedding_result_get_error_code(result), 0);

    float* vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    std::cout << "vector_length: " << vector_length << std::endl;
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 1024);
}
void imageEmbeddingCallback(EmbeddingResult* result, void* callback_user_data) {
    std::cout << "Start imageEmbeddingCallback: " << std::endl;
    EXPECT_EQ(embedding_result_get_error_code(result), 0);

    float* vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    std::cout << "vector_length: " << vector_length << std::endl;
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 1024);
}
void imageEmbeddingByBase64Callback(EmbeddingResult* result,
                                    void* callback_user_data) {
    std::cout << "Start imageEmbeddingByBase64Callback: " << std::endl;
    EXPECT_EQ(embedding_result_get_error_code(result), 0);

    float* vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    std::cout << "vector_length: " << vector_length << std::endl;
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 1024);
}

void imageCallback(EmbeddingResult* result, void* callback_user_data) {
    std::cout << "Start callback: " << std::endl;
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_1) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    std::string input_text = "Hello, world!";
    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(
        text_embedding_by_image_model(session_, input_text.c_str(), &result));
    EXPECT_NE(result, nullptr);

    // 验证结果没有错误
    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    EXPECT_STREQ(embedding_result_get_error_message(result), "Success");

    // 验证向量数据
    float* vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 1024);

    // 清理资源
    embedding_result_destroy(&result);
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_2) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiImageEmbedding(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "conf/aiassistant_20240709_191804.png";
    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(image_embedding_by_image_file(session_, path.string().c_str(),
                                              &result));
    EXPECT_NE(result, nullptr);

    // 验证结果没有错误
    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    EXPECT_STREQ(embedding_result_get_error_message(result), "Success");

    // 验证向量数据
    float* vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 1024);

    // 清理资源
    embedding_result_destroy(&result);
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_3) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiImageEmbeddingByBase64(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "conf/aiassistant_20240709_191804.png";

    std::vector<uint8_t> image_data = readFile(path.string());
    std::string base64_data = base64Encode(image_data);
    unsigned int length = base64_data.length();
    const unsigned char* uchar_ptr =
        reinterpret_cast<const unsigned char*>(base64_data.c_str());

    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(image_embedding_by_base64_image_data(session_, uchar_ptr,
                                                     length, &result));

    EXPECT_NE(result, nullptr);

    // 验证结果没有错误
    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    EXPECT_STREQ(embedding_result_get_error_message(result), "Success");

    // 验证向量数据
    float* vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 1024);

    // 清理资源
    embedding_result_destroy(&result);
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_4) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    // 验证模型信息
    char* model_info = nullptr;
    EXPECT_TRUE(image_embedding_get_model_info(session_, &model_info));
    EXPECT_NE(model_info, nullptr);
    std::string modelInfo = model_info;
    std::cout << "modelInfo: " << modelInfo << std::endl;
    // 清理资源
    embedding_model_info_destroy(model_info);
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_5) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModelAsync(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, GCancellable* cancellable,
                     GAsyncReadyCallback callback, gpointer user_data) {
            GTask* task = g_task_new(proxy, cancellable, callback, user_data);

            // 立即完成任务（模拟异步操作立即完成）
            g_task_return_boolean(task, TRUE);
            g_object_unref(task);

            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModelAsyncFinish(_, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gchar** out_json_output, GAsyncResult* res,
                     GError** error) {
            // 创建模拟的响应JSON
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    text_embedding_by_image_model_async(
        session_, "hello world!", textEmbeddingByImageModelCallback, nullptr);
    auto* loop = g_main_loop_new(nullptr, false);
    std::thread Thread([loop] {
        // 等待5s后退出
        sleep(5);
        g_main_loop_quit(loop);
    });
    Thread.detach();

    g_main_loop_run(loop);
    std::cout << "loop stop" << std ::endl;
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_6) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiImageEmbeddingAsync(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, GCancellable* cancellable,
                     GAsyncReadyCallback callback, gpointer user_data) {
            GTask* task = g_task_new(proxy, cancellable, callback, user_data);

            // 立即完成任务（模拟异步操作立即完成）
            g_task_return_boolean(task, TRUE);
            g_object_unref(task);

            return true;
        });

    EXPECT_CALL(mock_, CoreaiImageEmbeddingAsyncFinish(_, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gchar** out_json_output, GAsyncResult* res,
                     GError** error) {
            // 创建模拟的响应JSON
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "conf/aiassistant_20240709_191804.png";
    image_embedding_from_by_file_async(session_, path.c_str(),
                                       imageEmbeddingCallback, nullptr);

    auto* loop = g_main_loop_new(nullptr, false);
    std::thread Thread([loop] {
        // 等待5s后退出
        sleep(5);
        g_main_loop_quit(loop);
    });
    Thread.detach();

    g_main_loop_run(loop);
    std::cout << "loop stop" << std ::endl;
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_7) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiImageEmbeddingByBase64Async(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, GCancellable* cancellable,
                     GAsyncReadyCallback callback, gpointer user_data) {
            GTask* task = g_task_new(proxy, cancellable, callback, user_data);

            // 立即完成任务（模拟异步操作立即完成）
            g_task_return_boolean(task, TRUE);
            g_object_unref(task);

            return true;
        });

    EXPECT_CALL(mock_, CoreaiImageEmbeddingByBase64AsyncFinish(_, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gchar** out_json_output, GAsyncResult* res,
                     GError** error) {
            // 创建模拟的响应JSON
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "conf/aiassistant_20240709_191804.png";

    std::vector<uint8_t> image_data = readFile(path.string());
    std::string base64_data = base64Encode(image_data);
    unsigned int length = base64_data.length();
    const unsigned char* uchar_ptr =
        reinterpret_cast<const unsigned char*>(base64_data.c_str());

    image_embedding_by_base64_image_data_async(
        session_, uchar_ptr, length, imageEmbeddingByBase64Callback, nullptr);

    auto* loop = g_main_loop_new(nullptr, false);
    std::thread Thread([loop] {
        // 等待5s后退出
        sleep(5);
        g_main_loop_quit(loop);
    });
    Thread.detach();

    g_main_loop_run(loop);
    std::cout << "loop stop" << std ::endl;
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_8) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModelAsync(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, GCancellable* cancellable,
                     GAsyncReadyCallback callback, gpointer user_data) {
            GTask* task = g_task_new(proxy, cancellable, callback, user_data);

            // 立即完成任务（模拟异步操作立即完成）
            g_task_return_boolean(task, TRUE);
            g_object_unref(task);

            return true;
        });

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModelAsyncFinish(_, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gchar** out_json_output, GAsyncResult* res,
                     GError** error) {
            // 创建模拟的响应JSON
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    image_embedding_enable_internal_event_loop(session_, true);
    text_embedding_by_image_model_async(
        session_, "hello world!", textEmbeddingByImageModelCallback, nullptr);
    sleep(5);
    image_embedding_enable_internal_event_loop(session_, false);
    std::cout << "loop stop" << std ::endl;
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_9) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiImageEmbeddingAsync(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, GCancellable* cancellable,
                     GAsyncReadyCallback callback, gpointer user_data) {
            GTask* task = g_task_new(proxy, cancellable, callback, user_data);

            // 立即完成任务（模拟异步操作立即完成）
            g_task_return_boolean(task, TRUE);
            g_object_unref(task);

            return true;
        });

    EXPECT_CALL(mock_, CoreaiImageEmbeddingAsyncFinish(_, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gchar** out_json_output, GAsyncResult* res,
                     GError** error) {
            // 创建模拟的响应JSON
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "conf/aiassistant_20240709_191804.png";
    image_embedding_enable_internal_event_loop(session_, true);
    image_embedding_from_by_file_async(session_, path.c_str(),
                                       imageEmbeddingCallback, nullptr);
    sleep(5);
    image_embedding_enable_internal_event_loop(session_, false);
    std::cout << "loop stop" << std ::endl;
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_10) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiImageEmbeddingByBase64Async(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, GCancellable* cancellable,
                     GAsyncReadyCallback callback, gpointer user_data) {
            GTask* task = g_task_new(proxy, cancellable, callback, user_data);

            // 立即完成任务（模拟异步操作立即完成）
            g_task_return_boolean(task, TRUE);
            g_object_unref(task);

            return true;
        });

    EXPECT_CALL(mock_, CoreaiImageEmbeddingByBase64AsyncFinish(_, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gchar** out_json_output, GAsyncResult* res,
                     GError** error) {
            // 创建模拟的响应JSON
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建768维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "conf/aiassistant_20240709_191804.png";

    std::vector<uint8_t> image_data = readFile(path.string());
    std::string base64_data = base64Encode(image_data);
    unsigned int length = base64_data.length();
    const unsigned char* uchar_ptr =
        reinterpret_cast<const unsigned char*>(base64_data.c_str());

    image_embedding_enable_internal_event_loop(session_, true);
    image_embedding_by_base64_image_data_async(
        session_, uchar_ptr, length, imageEmbeddingByBase64Callback, nullptr);
    sleep(5);
    image_embedding_enable_internal_event_loop(session_, false);
    std::cout << "loop stop" << std ::endl;
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Success_Basic_11) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            // 创建1024维向量
            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });
    // sdk原来的init接口会先连接，在调用init，如果开启mock开关，就不会连接服务了
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const EmbeddingModelList* modelList = nullptr;
    modelList = image_embedding_get_model_list(session_, nullptr);
    int num = embedding_model_list_get_count(modelList, nullptr);
    std::cout << "model num:" << num << std::endl;
    const _EmbeddingModelInfo* modelInfo =
        embedding_model_list_get_model(modelList, 0, nullptr);
    const char* modeName =
        embedding_model_info_get_model_name(modelInfo, nullptr);
    std::cout << "model name:" << modeName << std::endl;
    EXPECT_STREQ(
        modeName,
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image");
    int dim = embedding_model_info_get_model_dim(modelInfo, nullptr);
    EXPECT_EQ(dim, 1024);
    image_embedding_init_model(session_, modeName);

    std::string input_text = "Hello, world!";
    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(
        text_embedding_by_image_model(session_, input_text.c_str(), &result));
    EXPECT_NE(result, nullptr);

    // 验证结果没有错误
    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    EXPECT_STREQ(embedding_result_get_error_message(result), "Success");

    // 验证向量数据
    float* vector_data = embedding_result_get_vector_data(result);
    int vector_length = embedding_result_get_vector_length(result);
    EXPECT_NE(vector_data, nullptr);
    EXPECT_EQ(vector_length, 1024);

    // 清理资源
    embedding_result_destroy(&result);
}

// ==================== 边界情况测试 (Boundary Cases) ====================

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Empty_Text) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    // 测试空文本
    std::string empty_text = "";
    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(
        text_embedding_by_image_model(session_, empty_text.c_str(), &result));

    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    embedding_result_destroy(&result);
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Long_Text) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    // 测试长文本
    std::string long_text(10000, 'x');  // 10000个字符的长文本
    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(
        text_embedding_by_image_model(session_, long_text.c_str(), &result));
    EXPECT_NE(result, nullptr);

    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    embedding_result_destroy(&result);
}

#ifdef COREAI_EMBEDDING_ENABLE_MOCK
TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Nonexistent_File) {
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiImageEmbedding(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = -1;  // 模拟文件不存在的错误
            response_json["errorMessage"] = "File not found";
            response_json["vectorResult"] = std::vector<float>();

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);

    // 测试不存在的文件
    std::string nonexistent_file = "/path/to/nonexistent/image.jpg";
    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(image_embedding_by_image_file(
        session_, nonexistent_file.c_str(), &result));
    EXPECT_NE(result, nullptr);

    // 验证返回了错误
    EXPECT_NE(embedding_result_get_error_code(result), 0);
    EXPECT_NE(embedding_result_get_error_message(result), nullptr);

    embedding_result_destroy(&result);
}
#endif

#ifdef COREAI_EMBEDDING_ENABLE_MOCK
TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Invalid_Base64_Data) {
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiImageEmbeddingByBase64(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = -2;  // 模拟无效base64数据的错误
            response_json["errorMessage"] = "Invalid base64 data";
            response_json["vectorResult"] = std::vector<float>();

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);

    // 测试无效的base64数据
    std::string invalid_base64 = "!!!invalid_base64_data!!!";
    const unsigned char* invalid_data =
        reinterpret_cast<const unsigned char*>(invalid_base64.c_str());
    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(image_embedding_by_base64_image_data(
        session_, invalid_data, invalid_base64.length(), &result));
    EXPECT_NE(result, nullptr);

    // 验证返回了错误
    EXPECT_NE(embedding_result_get_error_code(result), 0);
    EXPECT_NE(embedding_result_get_error_message(result), nullptr);

    embedding_result_destroy(&result);
}
#endif

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Null_Parameters) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);
#endif

    // 测试空指针参数
    EmbeddingResult* result = nullptr;

    // 测试空session（应该失败）
    EXPECT_FALSE(text_embedding_by_image_model(nullptr, "test", &result));

    // 测试空text
    EXPECT_FALSE(text_embedding_by_image_model(session_, nullptr, &result));
}

// ==================== 资源管理测试 (Resource Management) ====================

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Multiple_Sessions) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .Times(3)  // 期望被调用3次
        .WillRepeatedly([](AiRuntimeCoreImageEmbeddingService* proxy,
                           gint* out_session_Id, gint* out_errorCode,
                           GCancellable* cancellable, GError** error) {
            static int session_counter = 10001;
            *out_session_Id = session_counter++;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .Times(3)
        .WillRepeatedly([](AiRuntimeCoreImageEmbeddingService* proxy,
                           gint arg_sessionId, const gchar* arg_model,
                           gint* out_errorCode, GCancellable* cancellable,
                           GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .Times(3)
        .WillRepeatedly([](AiRuntimeCoreImageEmbeddingService* proxy,
                           gint arg_sessionId, gchar** out_jsonOutput,
                           GCancellable* cancellable, GError** error) {
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
    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .Times(3)
        .WillRepeatedly([](AiRuntimeCoreImageEmbeddingService* proxy,
                           const gchar* arg_json_input, gchar** out_json_output,
                           GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });
#endif

    // 创建多个session并测试
    ImageEmbeddingSession* session1 = image_embedding_create_session();
    ImageEmbeddingSession* session2 = image_embedding_create_session();
    ImageEmbeddingSession* session3 = image_embedding_create_session();

    EXPECT_NE(session1, nullptr);
    EXPECT_NE(session2, nullptr);
    EXPECT_NE(session3, nullptr);

#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session1);
    image_embedding_init_session_no_connect(session2);
    image_embedding_init_session_no_connect(session3);
#else
    image_embedding_init_session(session1);
    image_embedding_init_session(session2);
    image_embedding_init_session(session3);
#endif

    // 在每个session上执行操作
    EmbeddingResult* result1 = nullptr;
    EmbeddingResult* result2 = nullptr;
    EmbeddingResult* result3 = nullptr;

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session1, modelName);
    image_embedding_init_model(session2, modelName);
    image_embedding_init_model(session3, modelName);

    EXPECT_TRUE(text_embedding_by_image_model(session1, "text1", &result1));
    EXPECT_TRUE(text_embedding_by_image_model(session2, "text2", &result2));
    EXPECT_TRUE(text_embedding_by_image_model(session3, "text3", &result3));

    // 验证结果
    EXPECT_NE(result1, nullptr);
    EXPECT_NE(result2, nullptr);
    EXPECT_NE(result3, nullptr);

    // 清理资源
    embedding_result_destroy(&result1);
    embedding_result_destroy(&result2);
    embedding_result_destroy(&result3);

    image_embedding_destroy_session(&session1);
    image_embedding_destroy_session(&session2);
    image_embedding_destroy_session(&session3);

    EXPECT_EQ(session1, nullptr);
    EXPECT_EQ(session2, nullptr);
    EXPECT_EQ(session3, nullptr);
}

// ==================== 错误处理测试 (Error Handling) ====================

#ifdef COREAI_EMBEDDING_ENABLE_MOCK
TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Service_Error) {
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            static int session_counter = 10001;
            *out_session_Id = session_counter++;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            // 模拟服务端错误
            nlohmann::json response_json;
            response_json["errorCode"] = 4;
            response_json["errorMessage"] = "Internal server error";
            response_json["vectorResult"] = std::vector<float>();

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });
    std::cout << "Start create session no connect..." << std::endl;
    image_embedding_init_session_no_connect(session_);

    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(text_embedding_by_image_model(session_, "test error", &result));
    EXPECT_NE(result, nullptr);

    // 验证错误码和错误信息
    EXPECT_NE(embedding_result_get_error_code(result), 0);
    EXPECT_STREQ(embedding_result_get_error_message(result),
                 "Internal server error");

    embedding_result_destroy(&result);
}
#endif

// ==================== 特殊字符测试 (Special Characters) ====================

TEST_F(CoreaiImageEmbeddingMockedTest,
       Image_Embedding_Special_Characters_Text) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            static int session_counter = 10001;
            *out_session_Id = session_counter++;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    // 测试包含特殊字符的文本
    std::string special_text = "Hello! @#$%^&*()_+{}[]:;\"'<>,.?/|\\~`\n\t\r";
    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(
        text_embedding_by_image_model(session_, special_text.c_str(), &result));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    embedding_result_destroy(&result);
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Unicode_Text) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            static int session_counter = 10001;
            *out_session_Id = session_counter++;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            response_json["errorMessage"] = "Success";

            std::vector<float> vectorResult;
            for (int i = 0; i < 1024; i++) {
                vectorResult.push_back(static_cast<float>(i) * 0.01f - 3.84f);
            }
            response_json["vectorResult"] = vectorResult;

            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    image_embedding_init_session_no_connect(session_);
#endif

    const char* modelName =
        "ensemble-embd_cn-clip_512-uint8-text|embd_cn-clip_512-uint8-image";
    image_embedding_init_model(session_, modelName);

    // 测试Unicode文本
    std::string unicode_text = "中文测试 🚀 Hello 世界 🌍 测试 ✅";
    EmbeddingResult* result = nullptr;
    EXPECT_TRUE(
        text_embedding_by_image_model(session_, unicode_text.c_str(), &result));
    EXPECT_NE(result, nullptr);
    EXPECT_EQ(embedding_result_get_error_code(result), 0);
    embedding_result_destroy(&result);
}

// ==================== 补充 ====================
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_True_Call) {
#ifdef COREAI_EMBEDDING_ENABLE_MOCK
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = -1;
            *out_errorCode = -1;
            return true;
        });

#endif

    // 开启mock开关时的真实调用，服务不存在时 会连接不上
    EXPECT_NE(image_embedding_init_session(session_), 0);
    EmbeddingResult* result = nullptr;
    EXPECT_FALSE(image_embedding_by_image_file(nullptr, nullptr, &result));

    namespace fs = std::filesystem;
    fs::path dir = fs::path(__FILE__).parent_path();
    fs::path path = dir / "conf/aiassistant_20240709_191804.png";

    std::vector<uint8_t> image_data = readFile(path.string());
    std::string base64_data = base64Encode(image_data);
    unsigned int length = base64_data.length();
    const unsigned char* uchar_ptr =
        reinterpret_cast<const unsigned char*>(base64_data.c_str());

    EXPECT_FALSE(image_embedding_by_base64_image_data(nullptr, uchar_ptr,
                                                      length, &result));

    text_embedding_by_image_model_async(nullptr, "hello world!", imageCallback,
                                        nullptr);

    image_embedding_from_by_file_async(nullptr, path.c_str(), imageCallback,
                                       nullptr);

    image_embedding_by_base64_image_data_async(nullptr, uchar_ptr, length,
                                               imageCallback, nullptr);
}
#endif

// ==================== 关键异常分支覆盖测试 ====================

#ifdef COREAI_EMBEDDING_ENABLE_MOCK
TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_Cover_All_Error_Paths) {
    // 这个测试用例覆盖多个关键错误路径
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    // 1. 测试同步调用的DBus错误和空结果
    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            // DBus错误
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED, "DBus error");
            return false;
        })
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            // 空结果
            *out_json_output = nullptr;
            return true;
        });

    EXPECT_CALL(mock_, CoreaiImageEmbedding(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            // DBus错误
            *error = g_error_new(G_IO_ERROR, G_IO_ERROR_FAILED, "DBus error");
            return false;
        });

    EXPECT_CALL(mock_, CoreaiImageEmbeddingByBase64(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            // 空结果
            *out_json_output = nullptr;
            return true;
        });

    image_embedding_init_session_no_connect(session_);

    // 测试文本向量化的DBus错误
    EmbeddingResult* result1 = nullptr;
    EXPECT_TRUE(text_embedding_by_image_model(session_, "test1", &result1));
    EXPECT_NE(result1, nullptr);
    embedding_result_destroy(&result1);

    // 测试文本向量化的空结果
    EmbeddingResult* result2 = nullptr;
    EXPECT_TRUE(text_embedding_by_image_model(session_, "test2", &result2));
    EXPECT_NE(result2, nullptr);
    embedding_result_destroy(&result2);

    // 测试图像向量化的DBus错误
    EmbeddingResult* result3 = nullptr;
    EXPECT_TRUE(image_embedding_by_image_file(session_, "test.jpg", &result3));
    EXPECT_NE(result3, nullptr);
    embedding_result_destroy(&result3);

    // 测试base64图像向量化的空结果
    std::string base64_data = "test_base64_data";
    const unsigned char* uchar_ptr =
        reinterpret_cast<const unsigned char*>(base64_data.c_str());
    EmbeddingResult* result4 = nullptr;
    EXPECT_TRUE(image_embedding_by_base64_image_data(
        session_, uchar_ptr, base64_data.length(), &result4));
    EXPECT_NE(result4, nullptr);
    embedding_result_destroy(&result4);
}

TEST_F(CoreaiImageEmbeddingMockedTest, Image_Embedding_JSON_Parse_Error) {
    // 覆盖JSON解析错误路径
    EXPECT_CALL(mock_, CoreaiImageInit(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint* out_session_Id, gint* out_errorCode,
                     GCancellable* cancellable, GError** error) {
            *out_session_Id = 12345;
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageInitModel(_, _, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, const gchar* arg_model,
                     gint* out_errorCode, GCancellable* cancellable,
                     GError** error) {
            *out_errorCode = 0;
            return true;
        });
    EXPECT_CALL(mock_, CoreaiImageGetModelList(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     gint arg_sessionId, gchar** out_jsonOutput,
                     GCancellable* cancellable, GError** error) {
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

    EXPECT_CALL(mock_, CoreaiTextEmbeddingByImageModel(_, _, _, _, _))
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            // 返回无效JSON
            *out_json_output = g_strdup("invalid json {");
            return true;
        })
        .WillOnce([](AiRuntimeCoreImageEmbeddingService* proxy,
                     const gchar* arg_json_input, gchar** out_json_output,
                     GCancellable* cancellable, GError** error) {
            // 返回缺少必要字段的JSON
            nlohmann::json response_json;
            response_json["errorCode"] = 0;
            // 故意缺少 "errorMessage" 和 "vectorResult" 字段
            *out_json_output = g_strdup(response_json.dump().c_str());
            return true;
        });

    image_embedding_init_session_no_connect(session_);

    // 测试无效JSON
    EmbeddingResult* result1 = nullptr;
    EXPECT_TRUE(
        text_embedding_by_image_model(session_, "test invalid json", &result1));
    EXPECT_NE(result1, nullptr);
    embedding_result_destroy(&result1);

    // 测试缺少字段的JSON
    EmbeddingResult* result2 = nullptr;
    EXPECT_TRUE(text_embedding_by_image_model(session_, "test missing fields",
                                              &result2));
    EXPECT_NE(result2, nullptr);
    embedding_result_destroy(&result2);
}
#endif
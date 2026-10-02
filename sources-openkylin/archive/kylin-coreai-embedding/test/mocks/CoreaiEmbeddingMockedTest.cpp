#include "CoreaiEmbeddingMockedTest.h"

#include <fstream>

void CoreaiTextEmbeddingMockedTest::SetUp() {
    std::cout << "Start create session..." << std::endl;
    session_ = text_embedding_create_session();
    EXPECT_NE(session_, nullptr);

#ifdef COREAI_EMBEDDING_ENABLE_MOCK

    g_func_coreai_text_init = [&mock = this->mock_](
                                  AiRuntimeCoreTextEmbeddingService* proxy,
                                  const gchar* arg_jsonInput,
                                  gint* out_sessionId, gint* out_errorCode,
                                  GCancellable* cancellable, GError** error) {
        return mock.CoreaiTextInit(proxy, out_sessionId, out_errorCode,
                                   cancellable, error);
    };

    g_func_coreai_text_init_model =
        [&mock = this->mock_](AiRuntimeCoreTextEmbeddingService* proxy,
                              gint arg_sessionId, const gchar* arg_model,
                              gint* out_errorCode, GCancellable* cancellable,
                              GError** error) {
            return mock.CoreaiTextInitModel(proxy, arg_sessionId, arg_model,
                                            out_errorCode, cancellable, error);
        };

    g_func_coreai_text_get_model_list =
        [&mock = this->mock_](AiRuntimeCoreTextEmbeddingService* proxy,
                              gint arg_sessionId, gchar** out_jsonOutput,
                              GCancellable* cancellable, GError** error) {
            return mock.CoreaiTextGetModelList(
                proxy, arg_sessionId, out_jsonOutput, cancellable, error);
        };

    g_func_coreai_text_embedding =
        [&mock = this->mock_](AiRuntimeCoreTextEmbeddingService* proxy,
                              const gchar* arg_json_input,
                              gchar** out_json_output,
                              GCancellable* cancellable, GError** error) {
            return mock.CoreaiTextEmbedding(
                proxy, arg_json_input, out_json_output, cancellable, error);
        };

    g_func_coreai_text_embedding_async =
        [&mock = this->mock_](
            AiRuntimeCoreTextEmbeddingService* proxy,
            const gchar* arg_json_input, GCancellable* cancellable,
            GAsyncReadyCallback callback, gpointer user_data) {
            return mock.CoreaiTextEmbeddingAsync(
                proxy, arg_json_input, cancellable, callback, user_data);
        };
    g_func_coreai_text_embedding_async_finish =
        [&mock = this->mock_](AiRuntimeCoreTextEmbeddingService* proxy,
                              gchar** out_json_output, GAsyncResult* res,
                              GError** error) {
            return mock.CoreaiTextEmbeddingAsyncFinish(proxy, out_json_output,
                                                       res, error);
        };

    return;
#endif

    // 正常情况下，会先连接，再调用init 调用正常的init接口
    std::cout << "Start create true session connect..." << std::endl;
    text_embedding_init_session(session_);
}

void CoreaiTextEmbeddingMockedTest::TearDown() {
    std::cout << "Start destroy session..." << std::endl;
    text_embedding_destroy_session(&session_);
}

void CoreaiImageEmbeddingMockedTest::SetUp() {
    std::cout << "Start create session..." << std::endl;
    session_ = image_embedding_create_session();
    EXPECT_NE(session_, nullptr);

#ifdef COREAI_EMBEDDING_ENABLE_MOCK

    g_func_coreai_image_init = [&mock = this->mock_](
                                   AiRuntimeCoreImageEmbeddingService* proxy,
                                   const gchar* arg_jsonInput,
                                   gint* out_session_Id, gint* out_errorCode,
                                   GCancellable* cancellable, GError** error) {
        return mock.CoreaiImageInit(proxy, out_session_Id, out_errorCode,
                                    cancellable, error);
    };
    g_func_coreai_image_init_model =
        [&mock = this->mock_](AiRuntimeCoreImageEmbeddingService* proxy,
                              gint arg_sessionId, const gchar* arg_model,
                              gint* out_errorCode, GCancellable* cancellable,
                              GError** error) {
            return mock.CoreaiImageInitModel(proxy, arg_sessionId, arg_model,
                                             out_errorCode, cancellable, error);
        };
    g_func_coreai_image_get_model_list =
        [&mock = this->mock_](AiRuntimeCoreImageEmbeddingService* proxy,
                              gint arg_sessionId, gchar** out_jsonOutput,
                              GCancellable* cancellable, GError** error) {
            return mock.CoreaiImageGetModelList(
                proxy, arg_sessionId, out_jsonOutput, cancellable, error);
        };

    g_func_coreai_text_embedding_by_image_model =
        [&mock = this->mock_](AiRuntimeCoreImageEmbeddingService* proxy,
                              const gchar* arg_json_input,
                              gchar** out_json_output,
                              GCancellable* cancellable, GError** error) {
            return mock.CoreaiTextEmbeddingByImageModel(
                proxy, arg_json_input, out_json_output, cancellable, error);
        };
    g_func_coreai_image_embedding =
        [&mock = this->mock_](AiRuntimeCoreImageEmbeddingService* proxy,
                              const gchar* arg_json_input,
                              gchar** out_json_output,
                              GCancellable* cancellable, GError** error) {
            return mock.CoreaiImageEmbedding(
                proxy, arg_json_input, out_json_output, cancellable, error);
        };
    g_func_coreai_image_embedding_by_base64 =
        [&mock = this->mock_](AiRuntimeCoreImageEmbeddingService* proxy,
                              const gchar* arg_json_input,
                              gchar** out_json_output,
                              GCancellable* cancellable, GError** error) {
            return mock.CoreaiImageEmbeddingByBase64(
                proxy, arg_json_input, out_json_output, cancellable, error);
        };
    g_func_coreai_text_embedding_by_image_model_async =
        [&mock = this->mock_](
            AiRuntimeCoreImageEmbeddingService* proxy,
            const gchar* arg_json_input, GCancellable* cancellable,
            GAsyncReadyCallback callback, gpointer user_data) {
            return mock.CoreaiTextEmbeddingByImageModelAsync(
                proxy, arg_json_input, cancellable, callback, user_data);
        };
    g_func_coreai_text_embedding_by_image_model_async_finish =
        [&mock = this->mock_](AiRuntimeCoreImageEmbeddingService* proxy,
                              gchar** out_json_output, GAsyncResult* res,
                              GError** error) {
            return mock.CoreaiTextEmbeddingByImageModelAsyncFinish(
                proxy, out_json_output, res, error);
        };
    g_func_coreai_image_embedding_async =
        [&mock = this->mock_](
            AiRuntimeCoreImageEmbeddingService* proxy,
            const gchar* arg_json_input, GCancellable* cancellable,
            GAsyncReadyCallback callback, gpointer user_data) {
            return mock.CoreaiImageEmbeddingAsync(
                proxy, arg_json_input, cancellable, callback, user_data);
        };
    g_func_coreai_image_embedding_async_finish =
        [&mock = this->mock_](AiRuntimeCoreImageEmbeddingService* proxy,
                              gchar** out_json_output, GAsyncResult* res,
                              GError** error) {
            return mock.CoreaiImageEmbeddingAsyncFinish(proxy, out_json_output,
                                                        res, error);
        };
    g_func_coreai_image_embedding_by_base64_async =
        [&mock = this->mock_](
            AiRuntimeCoreImageEmbeddingService* proxy,
            const gchar* arg_json_input, GCancellable* cancellable,
            GAsyncReadyCallback callback, gpointer user_data) {
            return mock.CoreaiImageEmbeddingByBase64Async(
                proxy, arg_json_input, cancellable, callback, user_data);
        };
    g_func_coreai_image_embedding_by_base64_async_finish =
        [&mock = this->mock_](AiRuntimeCoreImageEmbeddingService* proxy,
                              gchar** out_json_output, GAsyncResult* res,
                              GError** error) {
            return mock.CoreaiImageEmbeddingByBase64AsyncFinish(
                proxy, out_json_output, res, error);
        };
    return;
#endif

    // 正常情况下，会先连接，再调用init 调用正常的init接口
    std::cout << "Start create true session connect..." << std::endl;
    image_embedding_init_session(session_);
}

void CoreaiImageEmbeddingMockedTest::TearDown() {
    std::cout << "Start destroy session..." << std::endl;
    image_embedding_destroy_session(&session_);
}

std::vector<uint8_t> CoreaiImageEmbeddingMockedTest::readFile(
    const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return buffer;
    } else {
        throw std::runtime_error("Failed to read file");
    }
}

std::string CoreaiImageEmbeddingMockedTest::base64Encode(
    const std::vector<uint8_t>& buffer) {
    std::string encodedData;
    int i = 0;
    uint8_t char_array_3[3];
    uint8_t char_array_4[4];

    while (i < buffer.size()) {
        char_array_3[0] = buffer[i++];
        char_array_3[1] = (i < buffer.size()) ? buffer[i++] : 0;
        char_array_3[2] = (i < buffer.size()) ? buffer[i++] : 0;

        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] =
            ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] =
            ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        char_array_4[3] = (char_array_3[2] & 0x3f);

        for (int j = 0; (j < 4); ++j) {
            encodedData += base64_chars_[char_array_4[j]];
        }
    }

    while ((encodedData.size() % 4) != 0) {
        encodedData += '=';
    }

    return encodedData;
}
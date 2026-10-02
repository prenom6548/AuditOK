#pragma once

#include <gmock/gmock.h>
#include <kylin-ai/coreai/embedding/embedding.h>

#include "CoreaiEmbeddingMockedInterface.h"

extern std::function<gboolean(AiRuntimeCoreTextEmbeddingService* proxy,
                              const gchar* arg_jsonInput, gint* out_sessionId,
                              gint* out_errorCode, GCancellable* cancellable,
                              GError** error)>
    g_func_coreai_text_init;

extern std::function<gboolean(AiRuntimeCoreTextEmbeddingService* proxy,
                              gint arg_sessionId, const gchar* arg_model,
                              gint* out_errorCode, GCancellable* cancellable,
                              GError** error)>
    g_func_coreai_text_init_model;

extern std::function<gboolean(AiRuntimeCoreTextEmbeddingService* proxy,
                              gint arg_sessionId, gchar** out_jsonOutput,
                              GCancellable* cancellable, GError** error)>
    g_func_coreai_text_get_model_list;

extern std::function<gboolean(
    AiRuntimeCoreTextEmbeddingService* proxy, const gchar* arg_json_input,
    gchar** out_json_output, GCancellable* cancellable, GError** error)>
    g_func_coreai_text_embedding;

extern std::function<void(AiRuntimeCoreTextEmbeddingService* proxy,
                          const gchar* arg_json_input,
                          GCancellable* cancellable,
                          GAsyncReadyCallback callback, gpointer user_data)>
    g_func_coreai_text_embedding_async;

extern std::function<gboolean(AiRuntimeCoreTextEmbeddingService* proxy,
                              gchar** out_json_output, GAsyncResult* res,
                              GError** error)>
    g_func_coreai_text_embedding_async_finish;

extern int text_embedding_init_session_no_connect(
    _TextEmbeddingSession* session);

extern std::function<gboolean(AiRuntimeCoreImageEmbeddingService* proxy,
                              const gchar* arg_jsonInput, gint* out_session_Id,
                              gint* out_errorCode, GCancellable* cancellable,
                              GError** error)>
    g_func_coreai_image_init;

extern std::function<gboolean(AiRuntimeCoreImageEmbeddingService* proxy,
                              gint arg_sessionId, const gchar* arg_model,
                              gint* out_errorCode, GCancellable* cancellable,
                              GError** error)>
    g_func_coreai_image_init_model;

extern std::function<gboolean(AiRuntimeCoreImageEmbeddingService* proxy,
                              gint arg_sessionId, gchar** out_jsonOutput,
                              GCancellable* cancellable, GError** error)>
    g_func_coreai_image_get_model_list;

extern std::function<gboolean(
    AiRuntimeCoreImageEmbeddingService* proxy, const gchar* arg_json_input,
    gchar** out_json_output, GCancellable* cancellable, GError** error)>
    g_func_coreai_text_embedding_by_image_model;

extern std::function<gboolean(
    AiRuntimeCoreImageEmbeddingService* proxy, const gchar* arg_json_input,
    gchar** out_json_output, GCancellable* cancellable, GError** error)>
    g_func_coreai_image_embedding;

extern std::function<gboolean(
    AiRuntimeCoreImageEmbeddingService* proxy, const gchar* arg_json_input,
    gchar** out_json_output, GCancellable* cancellable, GError** error)>
    g_func_coreai_image_embedding_by_base64;

extern std::function<void(AiRuntimeCoreImageEmbeddingService* proxy,
                          const gchar* arg_json_input,
                          GCancellable* cancellable,
                          GAsyncReadyCallback callback, gpointer user_data)>
    g_func_coreai_text_embedding_by_image_model_async;

extern std::function<gboolean(AiRuntimeCoreImageEmbeddingService* proxy,
                              gchar** out_json_output, GAsyncResult* res,
                              GError** error)>
    g_func_coreai_text_embedding_by_image_model_async_finish;

extern std::function<void(AiRuntimeCoreImageEmbeddingService* proxy,
                          const gchar* arg_json_input,
                          GCancellable* cancellable,
                          GAsyncReadyCallback callback, gpointer user_data)>
    g_func_coreai_image_embedding_async;

extern std::function<gboolean(AiRuntimeCoreImageEmbeddingService* proxy,
                              gchar** out_json_output, GAsyncResult* res,
                              GError** error)>
    g_func_coreai_image_embedding_async_finish;

extern std::function<void(AiRuntimeCoreImageEmbeddingService* proxy,
                          const gchar* arg_json_input,
                          GCancellable* cancellable,
                          GAsyncReadyCallback callback, gpointer user_data)>
    g_func_coreai_image_embedding_by_base64_async;

extern std::function<gboolean(AiRuntimeCoreImageEmbeddingService* proxy,
                              gchar** out_json_output, GAsyncResult* res,
                              GError** error)>
    g_func_coreai_image_embedding_by_base64_async_finish;

extern int image_embedding_init_session_no_connect(
    _ImageEmbeddingSession* session);

class CoreaiTextEmbeddingMockedTest : public ::testing::Test {
protected:
    friend void textEmbeddingCallback(EmbeddingResult* result,
                                      void* callback_user_data);

    void SetUp() override;

    void TearDown() override;

public:
    ::testing::StrictMock<CoreaiEmbeddingMockedInterface> mock_{};
    TextEmbeddingSession* session_ = nullptr;
};

class CoreaiImageEmbeddingMockedTest : public ::testing::Test {
protected:
    friend void textEmbeddingByImageModelCallback(EmbeddingResult* result,
                                                  void* callback_user_data);

    friend void imageEmbeddingCallback(EmbeddingResult* result,
                                       void* callback_user_data);

    friend void imageEmbeddingByBase64Callback(EmbeddingResult* result,
                                               void* callback_user_data);
    void SetUp() override;

    void TearDown() override;

    std::vector<uint8_t> readFile(const std::string& filePath);

    std::string base64Encode(const std::vector<uint8_t>& buffer);

public:
    ::testing::StrictMock<CoreaiEmbeddingMockedInterface> mock_{};
    ImageEmbeddingSession* session_ = nullptr;
    const std::string base64_chars_ =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
};

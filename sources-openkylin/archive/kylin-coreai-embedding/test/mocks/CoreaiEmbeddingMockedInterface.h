#pragma once

#include <gmock/gmock.h>

#include "coreimageembeddingserviceglue.h"
#include "coretextembeddingserviceglue.h"

class CoreaiEmbeddingMockedInterface {
public:
    MOCK_METHOD(gboolean, CoreaiTextInit,
                (AiRuntimeCoreTextEmbeddingService * proxy, gint* out_sessionId,
                 gint* out_errorCode, GCancellable* cancellable,
                 GError** error));

    MOCK_METHOD(gboolean, CoreaiTextInitModel,
                (AiRuntimeCoreTextEmbeddingService * proxy, gint arg_sessionId,
                 const gchar* arg_model, gint* out_errorCode,
                 GCancellable* cancellable, GError** error));

    MOCK_METHOD(gboolean, CoreaiTextGetModelList,
                (AiRuntimeCoreTextEmbeddingService * proxy, gint arg_sessionId,
                 gchar** out_jsonOutput, GCancellable* cancellable,
                 GError** error));

    MOCK_METHOD(gboolean, CoreaiTextEmbedding,
                (AiRuntimeCoreTextEmbeddingService * proxy,
                 const gchar* arg_json_input, gchar** out_json_output,
                 GCancellable* cancellable, GError** error));

    MOCK_METHOD(gboolean, CoreaiTextEmbeddingAsync,
                (AiRuntimeCoreTextEmbeddingService * proxy,
                 const gchar* arg_json_input, GCancellable* cancellable,
                 GAsyncReadyCallback callback, gpointer user_data));

    MOCK_METHOD(gboolean, CoreaiTextEmbeddingAsyncFinish,
                (AiRuntimeCoreTextEmbeddingService * proxy,
                 gchar** out_json_output, GAsyncResult* res, GError** error));

    MOCK_METHOD(gboolean, CoreaiImageInit,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 gint* out_session_Id, gint* out_errorCode,
                 GCancellable* cancellable, GError** error));

    MOCK_METHOD(gboolean, CoreaiImageInitModel,
                (AiRuntimeCoreImageEmbeddingService * proxy, gint arg_sessionId,
                 const gchar* arg_model, gint* out_errorCode,
                 GCancellable* cancellable, GError** error));

    MOCK_METHOD(gboolean, CoreaiImageGetModelList,
                (AiRuntimeCoreImageEmbeddingService * proxy, gint arg_sessionId,
                 gchar** out_jsonOutput, GCancellable* cancellable,
                 GError** error));

    MOCK_METHOD(gboolean, CoreaiTextEmbeddingByImageModel,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 const gchar* arg_json_input, gchar** out_json_output,
                 GCancellable* cancellable, GError** error));

    MOCK_METHOD(gboolean, CoreaiImageEmbedding,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 const gchar* arg_json_input, gchar** out_json_output,
                 GCancellable* cancellable, GError** error));

    MOCK_METHOD(gboolean, CoreaiImageEmbeddingByBase64,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 const gchar* arg_json_input, gchar** out_json_output,
                 GCancellable* cancellable, GError** error));

    MOCK_METHOD(gboolean, CoreaiTextEmbeddingByImageModelAsync,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 const gchar* arg_json_input, GCancellable* cancellable,
                 GAsyncReadyCallback callback, gpointer user_data));

    MOCK_METHOD(gboolean, CoreaiTextEmbeddingByImageModelAsyncFinish,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 gchar** out_json_output, GAsyncResult* res, GError** error));

    MOCK_METHOD(gboolean, CoreaiImageEmbeddingAsync,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 const gchar* arg_json_input, GCancellable* cancellable,
                 GAsyncReadyCallback callback, gpointer user_data));

    MOCK_METHOD(gboolean, CoreaiImageEmbeddingAsyncFinish,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 gchar** out_json_output, GAsyncResult* res, GError** error));

    MOCK_METHOD(gboolean, CoreaiImageEmbeddingByBase64Async,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 const gchar* arg_json_input, GCancellable* cancellable,
                 GAsyncReadyCallback callback, gpointer user_data));

    MOCK_METHOD(gboolean, CoreaiImageEmbeddingByBase64AsyncFinish,
                (AiRuntimeCoreImageEmbeddingService * proxy,
                 gchar** out_json_output, GAsyncResult* res, GError** error));
};

/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COREAIIMAGEEMBEDDINGDBUSFUNCS_H
#define COREAIIMAGEEMBEDDINGDBUSFUNCS_H

#include "coreimageembeddingserviceglue.h"

std::function<gboolean(AiRuntimeCoreImageEmbeddingService *proxy,
                       const gchar *arg_jsonInput, gint *out_sessionId,
                       gint *out_errorCode, GCancellable *cancellable,
                       GError **error)>
    g_func_coreai_image_init =
        ai_runtime_core_image_embedding_service_call_init_sync;

std::function<gboolean(AiRuntimeCoreImageEmbeddingService *proxy,
                       gint arg_sessionId, const gchar *arg_model,
                       gint *out_errorCode, GCancellable *cancellable,
                       GError **error)>
    g_func_coreai_image_init_model =
        ai_runtime_core_image_embedding_service_call_init_model_sync;

std::function<gboolean(AiRuntimeCoreImageEmbeddingService *proxy,
                       gint arg_sessionId, gchar **out_jsonOutput,
                       GCancellable *cancellable, GError **error)>
    g_func_coreai_image_get_model_list =
        ai_runtime_core_image_embedding_service_call_get_model_list_sync;

std::function<gboolean(AiRuntimeCoreImageEmbeddingService *proxy,
                       const gchar *arg_json_input, gchar **out_json_output,
                       GCancellable *cancellable, GError **error)>
    g_func_coreai_text_embedding_by_image_model =
        ai_runtime_core_image_embedding_service_call_embedding_text_sync;

std::function<gboolean(AiRuntimeCoreImageEmbeddingService *proxy,
                       const gchar *arg_json_input, gchar **out_json_output,
                       GCancellable *cancellable, GError **error)>
    g_func_coreai_image_embedding =
        ai_runtime_core_image_embedding_service_call_embedding_image_file_sync;

std::function<gboolean(AiRuntimeCoreImageEmbeddingService *proxy,
                       const gchar *arg_json_input, gchar **out_json_output,
                       GCancellable *cancellable, GError **error)>
    g_func_coreai_image_embedding_by_base64 =
        ai_runtime_core_image_embedding_service_call_embedding_base64_image_sync;

std::function<void(AiRuntimeCoreImageEmbeddingService *proxy,
                   const gchar *arg_json_input, GCancellable *cancellable,
                   GAsyncReadyCallback callback, gpointer user_data)>
    g_func_coreai_text_embedding_by_image_model_async =
        ai_runtime_core_image_embedding_service_call_embedding_text;

std::function<gboolean(AiRuntimeCoreImageEmbeddingService *proxy,
                       gchar **out_json_output, GAsyncResult *res,
                       GError **error)>
    g_func_coreai_text_embedding_by_image_model_async_finish =
        ai_runtime_core_image_embedding_service_call_embedding_text_finish;

std::function<void(AiRuntimeCoreImageEmbeddingService *proxy,
                   const gchar *arg_json_input, GCancellable *cancellable,
                   GAsyncReadyCallback callback, gpointer user_data)>
    g_func_coreai_image_embedding_async =
        ai_runtime_core_image_embedding_service_call_embedding_image_file;

std::function<gboolean(AiRuntimeCoreImageEmbeddingService *proxy,
                       gchar **out_json_output, GAsyncResult *res,
                       GError **error)>
    g_func_coreai_image_embedding_async_finish =
        ai_runtime_core_image_embedding_service_call_embedding_image_file_finish;

std::function<void(AiRuntimeCoreImageEmbeddingService *proxy,
                   const gchar *arg_json_input, GCancellable *cancellable,
                   GAsyncReadyCallback callback, gpointer user_data)>
    g_func_coreai_image_embedding_by_base64_async =
        ai_runtime_core_image_embedding_service_call_embedding_base64_image;

std::function<gboolean(AiRuntimeCoreImageEmbeddingService *proxy,
                       gchar **out_json_output, GAsyncResult *res,
                       GError **error)>
    g_func_coreai_image_embedding_by_base64_async_finish =
        ai_runtime_core_image_embedding_service_call_embedding_base64_image_finish;

#endif
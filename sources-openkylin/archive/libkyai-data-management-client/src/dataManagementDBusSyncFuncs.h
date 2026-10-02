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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DATAMANAGEMENTDBUSSYNCFUNCS_H
#define DATAMANAGEMENTDBUSSYNCFUNCS_H

#include <functional>

#include "datamanagementserviceglue.h"

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_similarity_search = ai_business_data_management_call_similarity_search_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_add_text_files = ai_business_data_management_call_add_text_files_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_add_image_files = ai_business_data_management_call_add_image_files_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_delete_files = ai_business_data_management_call_delete_files_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_update_files_name = ai_business_data_management_call_update_files_name_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_update_files_content_data = ai_business_data_management_call_update_files_content_data_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_all_file_infos = ai_business_data_management_call_get_all_file_infos_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_feature_status = ai_business_data_management_call_get_feature_status_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_tags_of_files = ai_business_data_management_call_get_tags_of_files_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_all_tags = ai_business_data_management_call_get_all_tags_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_search_similar_tags = ai_business_data_management_call_search_similar_tags_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_search_files_about_tags = ai_business_data_management_call_search_files_about_tags_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_is_file_relevant_to_tags = ai_business_data_management_call_is_file_relevant_to_tags_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_is_file_relevant_to_description = ai_business_data_management_call_is_file_relevant_to_description_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_extract_search_conditions = ai_business_data_management_call_extract_search_conditions_sync;

std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                       gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_summarys_of_files = ai_business_data_management_call_get_summarys_of_files_sync;

#endif
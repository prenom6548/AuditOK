#pragma once

#include <gmock/gmock.h>
#include <kylin-ai/business/data-management/datamanagement.h>

#include "DataManagementMockedInterface.h"

extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_similarity_search;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_similarity_search;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_add_text_files;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_add_image_files;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_delete_files;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_update_files_name;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_update_files_content_data;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_all_file_infos;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_feature_status;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_tags_of_files;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_all_tags;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_search_similar_tags;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_search_files_about_tags;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_is_file_relevant_to_tags;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_is_file_relevant_to_description;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_extract_search_conditions;
extern std::function<gboolean(AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error)>
    g_func_get_summarys_of_files;

extern DataManagementSession *data_management_create_session_no_connect();

class DataManagementMockedTest : public ::testing::Test {
protected:
    void SetUp() override;

    void TearDown() override;

public:
    ::testing::StrictMock<DataManagementMockedInterface> mock_{};
    DataManagementSession *session_ = nullptr;
};

#include "DataManagementMockedTest.h"

void DataManagementMockedTest::SetUp() {
#ifdef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    std::cout << "Start the mock and get no connect session..." << std::endl;
    session_ = data_management_create_session_no_connect();

    g_func_similarity_search = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                     GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                     GCancellable *cancellable, GError **error) {
        return mock.SimilaritySearch(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_add_text_files = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                  GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                  GCancellable *cancellable, GError **error) {
        return mock.AddTextFiles(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_add_image_files = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                   GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                   GCancellable *cancellable, GError **error) {
        return mock.AddImageFiles(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_delete_files = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                GCancellable *cancellable, GError **error) {
        return mock.DeleteFiles(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_update_files_name = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                     GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                     GCancellable *cancellable, GError **error) {
        return mock.UpdateFilesName(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_update_files_content_data = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                             GDBusCallFlags call_flags, gint timeout_msec,
                                                             gchar **out_output, GCancellable *cancellable,
                                                             GError **error) {
        return mock.UpdateFilesContentData(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_get_all_file_infos = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                      GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                      GCancellable *cancellable, GError **error) {
        return mock.GetAllFilesInfos(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_get_feature_status = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                      GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                      GCancellable *cancellable, GError **error) {
        return mock.GetFeatureStatus(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_get_tags_of_files = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                     GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                     GCancellable *cancellable, GError **error) {
        return mock.GetTagsOfFiles(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_get_all_tags = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                GCancellable *cancellable, GError **error) {
        return mock.GetAllTags(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_search_similar_tags = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                       GDBusCallFlags call_flags, gint timeout_msec, gchar **out_output,
                                                       GCancellable *cancellable, GError **error) {
        return mock.SearchSimilarTags(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_search_files_about_tags = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                           GDBusCallFlags call_flags, gint timeout_msec,
                                                           gchar **out_output, GCancellable *cancellable,
                                                           GError **error) {
        return mock.SearchFilesAboutTags(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_is_file_relevant_to_tags = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                            GDBusCallFlags call_flags, gint timeout_msec,
                                                            gchar **out_output, GCancellable *cancellable,
                                                            GError **error) {
        return mock.IsFileRelevantToTags(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_is_file_relevant_to_description =
        [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            return mock.IsFileRelevantToDescription(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable,
                                                    error);
        };
    g_func_extract_search_conditions = [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input,
                                                             GDBusCallFlags call_flags, gint timeout_msec,
                                                             gchar **out_output, GCancellable *cancellable,
                                                             GError **error) {
        return mock.ExtractSearchConditions(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
    };
    g_func_get_summarys_of_files =
        [&mock = this->mock_](AiBusinessDataManagement *proxy, const gchar *arg_input, GDBusCallFlags call_flags,
                              gint timeout_msec, gchar **out_output, GCancellable *cancellable, GError **error) {
            return mock.GetSummarysOfFiles(proxy, arg_input, call_flags, timeout_msec, out_output, cancellable, error);
        };

    return;
#endif
    std::cout << "Start get true session..." << std::endl;
    session_ = data_management_create_session();
}

void DataManagementMockedTest::TearDown() {
#ifndef DATA_MANAGEMENT_CLIENT_ENABLE_MOCK
    std::cout << "Start destroy true session..." << std::endl;
    data_management_destroy_session(session_);
#endif
}

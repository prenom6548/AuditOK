#pragma once

#include <gmock/gmock.h>

#include "datamanagementserviceglue.h"

class DataManagementMockedInterface {
public:
    MOCK_METHOD(gboolean, SimilaritySearch,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, AddTextFiles,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, AddImageFiles,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, DeleteFiles,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, UpdateFilesName,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, UpdateFilesContentData,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, GetAllFilesInfos,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, GetFeatureStatus,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, GetTagsOfFiles,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, GetAllTags,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, SearchSimilarTags,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, SearchFilesAboutTags,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, IsFileRelevantToTags,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, IsFileRelevantToDescription,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, ExtractSearchConditions,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
    MOCK_METHOD(gboolean, GetSummarysOfFiles,
                (AiBusinessDataManagement * proxy, const gchar *arg_input, GDBusCallFlags call_flags, gint timeout_msec,
                 gchar **out_output, GCancellable *cancellable, GError **error));
};

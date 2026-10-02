#ifndef UKUI_SEARCH_AUTOTEST_AI_DATAMANAGEMENT_H
#define UKUI_SEARCH_AUTOTEST_AI_DATAMANAGEMENT_H

#include <kylin-ai/business/data-management/common.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _DataManagementSession DataManagementSession;

bool data_management_is_feature_supported();
DataManagementSession *data_management_create_session();
void data_management_destroy_session(DataManagementSession *session);
void data_management_destroy_result(char *result);
DataManagementStatus data_management_get_feature_status(DataManagementSession *session, FeatureStatus *status);
DataManagementStatus data_management_check_file_format(DataManagementSession *session, const char *filepath, char **check_result);
DataManagementStatus data_management_add_text_files(DataManagementSession *session, const char *fileinfos);
DataManagementStatus data_management_add_image_files(DataManagementSession *session, const char *fileinfos);
DataManagementStatus data_management_delete_files(DataManagementSession *session, const char *fileinfos);
DataManagementStatus data_management_update_files_content(DataManagementSession *session, const char *fileinfos);
DataManagementStatus data_management_get_all_fileinfos(DataManagementSession *session, char **fileinfos_result);

#ifdef __cplusplus
}
#endif

#endif

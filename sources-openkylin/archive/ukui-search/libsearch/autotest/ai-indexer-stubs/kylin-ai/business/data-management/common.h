#ifndef UKUI_SEARCH_AUTOTEST_AI_COMMON_H
#define UKUI_SEARCH_AUTOTEST_AI_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { UNKNOWN = -1, AVAILABLE = 0, NOT_INSTALLED } FeatureStatus;

typedef enum {
    SUCCESS = 0,
    PARSE_PARAM_FAILED = 14,
    DM_ERROR_UNKNOWN = 100,
    DM_SESSION_ERROR = 101,
} ErrorCode;

typedef struct {
    bool success;
    int errorCode;
    char errorMsg[500];
} DataManagementStatus;

#ifdef __cplusplus
}
#endif

#endif

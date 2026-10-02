#include "models/result.h"
#include "utils/string_utils.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

#define MODULE "result"

judgement_result_t* judgement_result_create(void)
{
    judgement_result_t *result = (judgement_result_t*)calloc(1, sizeof(judgement_result_t));
    if (!result) {
        logger_error(MODULE, "Failed to allocate memory for judgement result");
        return NULL;
    }
    
    result->capability_id = NULL;
    result->capability_name = NULL;
    result->overall_status = CAP_STATUS_UNKNOWN;
    result->cpu_status = LAYER_STATUS_UNKNOWN;
    result->firmware_status = LAYER_STATUS_UNKNOWN;
    result->os_status = LAYER_STATUS_UNKNOWN;
    result->evidence = NULL;
    result->description = NULL;
    
    // logger_debug(MODULE, "Judgement result created");
    return result;
}

void judgement_result_free(judgement_result_t *result)
{
    if (!result) return;
    
    free(result->capability_id);
    free(result->capability_name);
    free(result->evidence);
    free(result->description);
    
    result->capability_id = NULL;
    result->capability_name = NULL;
    result->evidence = NULL;
    result->description = NULL;
    
    free(result);
    logger_debug(MODULE, "Judgement result freed");
}

psp_error_t judgement_result_set_capability_id(judgement_result_t *result, const char *cap_id)
{
    if (!result || !cap_id) {
        return ERR_INVALID_PARAM;
    }
    
    free(result->capability_id);
    result->capability_id = string_utils_strdup(cap_id);
    
    if (!result->capability_id) {
        logger_error(MODULE, "Failed to set capability ID in result");
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set capability ID in result: %s", cap_id);
    return ERR_OK;
}

psp_error_t judgement_result_set_overall_status(judgement_result_t *result, cap_status_t status)
{
    if (!result) {
        return ERR_INVALID_PARAM;
    }
    
    result->overall_status = status;
    logger_debug(MODULE, "Set overall status: %s", cap_status_to_string(status));
    return ERR_OK;
}

psp_error_t judgement_result_set_cpu_status(judgement_result_t *result, layer_status_t status)
{
    if (!result) {
        return ERR_INVALID_PARAM;
    }
    
    result->cpu_status = status;
    logger_debug(MODULE, "Set CPU status: %s", layer_status_to_string(status));
    return ERR_OK;
}

psp_error_t judgement_result_set_firmware_status(judgement_result_t *result, layer_status_t status)
{
    if (!result) {
        return ERR_INVALID_PARAM;
    }
    
    result->firmware_status = status;
    logger_debug(MODULE, "Set firmware status: %s", layer_status_to_string(status));
    return ERR_OK;
}

psp_error_t judgement_result_set_os_status(judgement_result_t *result, layer_status_t status)
{
    if (!result) {
        return ERR_INVALID_PARAM;
    }
    
    result->os_status = status;
    logger_debug(MODULE, "Set OS status: %s", layer_status_to_string(status));
    return ERR_OK;
}

psp_error_t judgement_result_set_evidence(judgement_result_t *result, const char *evidence)
{
    if (!result || !evidence) {
        return ERR_INVALID_PARAM;
    }
    
    free(result->evidence);
    result->evidence = string_utils_strdup(evidence);
    
    if (!result->evidence) {
        logger_error(MODULE, "Failed to set evidence");
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set evidence: %s", evidence);
    return ERR_OK;
}

psp_error_t judgement_result_set_capability_name(judgement_result_t *result, const char *name)
{
    if (!result || !name) {
        return ERR_INVALID_PARAM;
    }
    
    free(result->capability_name);
    result->capability_name = string_utils_strdup(name);
    
    if (!result->capability_name) {
        logger_error(MODULE, "Failed to set capability name");
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set capability name: %s", name);
    return ERR_OK;
}

psp_error_t judgement_result_set_description(judgement_result_t *result, const char *desc)
{
    if (!result) {
        return ERR_INVALID_PARAM;
    }
    
    free(result->description);
    
    if (desc) {
        result->description = string_utils_strdup(desc);
        if (!result->description) {
            logger_error(MODULE, "Failed to set description");
            return ERR_MEMORY;
        }
        logger_debug(MODULE, "Set description: %s", desc);
    } else {
        result->description = NULL;
        logger_debug(MODULE, "Cleared description");
    }
    
    return ERR_OK;
}

judgement_result_list_t* judgement_result_list_create(void)
{
    judgement_result_list_t *list = (judgement_result_list_t*)calloc(1, sizeof(judgement_result_list_t));
    if (!list) {
        logger_error(MODULE, "Failed to allocate memory for result list");
        return NULL;
    }
    
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
    
    // logger_debug(MODULE, "Judgement result list created");
    return list;
}

void judgement_result_list_free(judgement_result_list_t *list)
{
    if (!list) return;
    
    if (list->items) {
        for (size_t i = 0; i < list->count; i++) {
            if (list->items[i]) {
                judgement_result_free(list->items[i]);
            }
        }
        free(list->items);
    }
    
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
    
    free(list);
    logger_debug(MODULE, "Judgement result list freed");
}

psp_error_t judgement_result_list_add(judgement_result_list_t *list, judgement_result_t *result)
{
    if (!list || !result) {
        return ERR_INVALID_PARAM;
    }
    
    /* 检查容量 */
    if (list->count >= list->capacity) {
        size_t new_capacity = (list->capacity == 0) ? 16 : list->capacity * 2;
        judgement_result_t **new_items = (judgement_result_t**)realloc(list->items,
                                                                        new_capacity * sizeof(judgement_result_t*));
        if (!new_items) {
            logger_error(MODULE, "Failed to reallocate memory for result list");
            return ERR_MEMORY;
        }
        
        list->items = new_items;
        list->capacity = new_capacity;
    }
    
    list->items[list->count++] = result;
    logger_debug(MODULE, "Added result to list (count=%zu)", list->count);
    return ERR_OK;
}

execution_result_t* execution_result_create(void)
{
    execution_result_t *res = (execution_result_t*)calloc(1, sizeof(execution_result_t));
    if (!res) {
        logger_error(MODULE, "Failed to allocate memory for execution result");
        return NULL;
    }
    
    res->capability_id = NULL;
    res->action = NULL;
    res->operation_completed = false;
    res->message = NULL;
    res->guidance = NULL;
    res->rollback_log = NULL;
    
    logger_debug(MODULE, "Execution result created");
    return res;
}

void execution_result_free(execution_result_t *res)
{
    if (!res) return;
    
    free(res->capability_id);
    free(res->action);
    free(res->message);
    free(res->guidance);
    free(res->rollback_log);
    
    res->capability_id = NULL;
    res->action = NULL;
    res->message = NULL;
    res->guidance = NULL;
    res->rollback_log = NULL;
    
    free(res);
    logger_debug(MODULE, "Execution result freed");
}

psp_error_t execution_result_set_capability_id(execution_result_t *res, const char *cap_id)
{
    if (!res || !cap_id) {
        return ERR_INVALID_PARAM;
    }
    
    free(res->capability_id);
    res->capability_id = string_utils_strdup(cap_id);
    
    if (!res->capability_id) {
        logger_error(MODULE, "Failed to set capability ID in execution result");
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set capability ID in execution result: %s", cap_id);
    return ERR_OK;
}

psp_error_t execution_result_set_action(execution_result_t *res, const char *action)
{
    if (!res || !action) {
        return ERR_INVALID_PARAM;
    }
    
    free(res->action);
    res->action = string_utils_strdup(action);
    
    if (!res->action) {
        logger_error(MODULE, "Failed to set action in execution result");
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set action in execution result: %s", action);
    return ERR_OK;
}

psp_error_t execution_result_set_success(execution_result_t *res, bool status)
{
    if (!res) {
        return ERR_INVALID_PARAM;
    }
    
    res->operation_completed = status;
    logger_debug(MODULE, "Set operation_completed: %d", status);
    return ERR_OK;
}

psp_error_t execution_result_set_message(execution_result_t *res, const char *msg)
{
    if (!res || !msg) {
        return ERR_INVALID_PARAM;
    }
    
    free(res->message);
    res->message = string_utils_strdup(msg);
    
    if (!res->message) {
        logger_error(MODULE, "Failed to set message in execution result");
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set message in execution result: %s", msg);
    return ERR_OK;
}

psp_error_t execution_result_set_guidance(execution_result_t *res, const char *guidance)
{
    if (!res || !guidance) {
        return ERR_INVALID_PARAM;
    }
    
    free(res->guidance);
    res->guidance = string_utils_strdup(guidance);
    
    if (!res->guidance) {
        logger_error(MODULE, "Failed to set guidance in execution result");
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set guidance in execution result: %s", guidance);
    return ERR_OK;
}

psp_error_t execution_result_set_rollback_log(execution_result_t *res, const char *log)
{
    if (!res || !log) {
        return ERR_INVALID_PARAM;
    }
    
    free(res->rollback_log);
    res->rollback_log = string_utils_strdup(log);
    
    if (!res->rollback_log) {
        logger_error(MODULE, "Failed to set rollback log in execution result");
        return ERR_MEMORY;
    }
    
    logger_debug(MODULE, "Set rollback log in execution result");
    return ERR_OK;
}

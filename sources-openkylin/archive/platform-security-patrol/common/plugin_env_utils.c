/**
 * @file plugin_env_utils.c
 * @brief 插件环境信息工具函数实现（公共库）
 */

#include "plugin_env_utils.h"
#include "logger.h"
#include <stdlib.h>
#include <string.h>

#define MODULE "plugin_env_utils"

plugin_env_list_t* plugin_env_list_create(void)
{
    plugin_env_list_t *list = (plugin_env_list_t*)calloc(1, sizeof(plugin_env_list_t));
    if (!list) {
        logger_error(MODULE, "Failed to allocate memory for plugin environment list");
        return NULL;
    }
    
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;

    return list;
}

void plugin_env_list_free(plugin_env_list_t *list)
{
    if (!list) return;
    
    if (list->items) {
        for (size_t i = 0; i < list->count; i++) {
            plugin_env_item_t *item = list->items[i];
            if (item) {
                free(item->key);
                free(item->value);
                free(item->source);
                free(item);
            }
        }
        free(list->items);
    }
    
    free(list);
}

psp_error_t plugin_env_list_add(plugin_env_list_t *list, const char *key, 
                           const char *value, const char *source)
{
    if (!list || !key) {
        return ERR_INVALID_PARAM;
    }
    
    /* 容量不足时扩容 */
    if (list->count >= list->capacity) {
        size_t new_capacity = list->capacity == 0 ? 8 : list->capacity * 2;
        plugin_env_item_t **new_items = (plugin_env_item_t**)realloc(
            list->items, new_capacity * sizeof(plugin_env_item_t*));
        
        if (!new_items) {
            logger_error(MODULE, "Failed to reallocate memory for plugin environment items");
            return ERR_MEMORY;
        }
        
        list->items = new_items;
        list->capacity = new_capacity;
    }
    
    /* 创建新项 */
    plugin_env_item_t *item = (plugin_env_item_t*)calloc(1, sizeof(plugin_env_item_t));
    if (!item) {
        logger_error(MODULE, "Failed to allocate memory for plugin environment item");
        return ERR_MEMORY;
    }
    
    item->key = strdup(key);
    if (!item->key) {
        logger_error(MODULE, "Failed to duplicate key: %s", key);
        free(item);
        return ERR_MEMORY;
    }
    
    if (value) {
        item->value = strdup(value);
        if (!item->value) {
            logger_error(MODULE, "Failed to duplicate value");
            free(item->key);
            free(item);
            return ERR_MEMORY;
        }
    } else {
        item->value = NULL;
    }
    
    if (source) {
        item->source = strdup(source);
        if (!item->source) {
            logger_error(MODULE, "Failed to duplicate source");
            free(item->key);
            free(item->value);
            free(item);
            return ERR_MEMORY;
        }
    } else {
        item->source = NULL;
    }
    
    list->items[list->count++] = item;
    logger_debug(MODULE, "Added plugin env item: %s=%s (from %s)", 
                key, value ? value : "NULL", source ? source : "unknown");
    return ERR_OK;
}

bool plugin_env_list_has_key(plugin_env_list_t *list, const char *key)
{
    if (!list || !key) {
        return false;
    }
    
    for (size_t i = 0; i < list->count; i++) {
        if (list->items[i] && strcmp(list->items[i]->key, key) == 0) {
            return true;
        }
    }
    
    return false;
}

const char* plugin_env_list_get(plugin_env_list_t *list, const char *key)
{
    if (!list || !key) {
        return NULL;
    }
    
    for (size_t i = 0; i < list->count; i++) {
        if (list->items[i] && strcmp(list->items[i]->key, key) == 0) {
            return list->items[i]->value;
        }
    }
    
    return NULL;
}

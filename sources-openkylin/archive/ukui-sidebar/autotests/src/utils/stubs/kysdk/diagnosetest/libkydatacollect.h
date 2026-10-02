/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef AUTOTESTS_STUB_LIBKYDATACOLLECT_H
#define AUTOTESTS_STUB_LIBKYDATACOLLECT_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    KEVENT_CLICK = 0,
    KEVENT_VISIT = 1,
    KEVENT_SEARCH = 2,
    KEVENT_EXPOSURE = 3,
    KEVENT_STARTUP = 4,
    KEVENT_EXIT = 5,
    KEVENT_SHARE = 6,
    KEVENT_CUSTOM = 100
} KEventType;

typedef enum
{
    KEVENTSOURCE_DESKTOP = 0,
    KEVENTSOURCE_WEB = 1
} KEventSourceType;

typedef struct
{
    char *key;
    char *value;
} KCustomProperty;

typedef struct
{
    char *eventCode;
    KEventType eventType;
    KEventSourceType eventSourceType;
    KCustomProperty *customProperty;
    int customPropertyLength;
    char *pageName;
} KTrackData;

char *kdk_test_strdup(const char *src) noexcept;
#define strdup kdk_test_strdup

KTrackData *kdk_dia_data_init(KEventSourceType sourceType, KEventType evnetType);
void kdk_dia_data_free(KTrackData *node);
void kdk_dia_upload_default(KTrackData *node, char *eventCode, char *pageName);
void kdk_dia_append_custom_property(KTrackData *node, KCustomProperty *property, int property_length);

void kdk_test_reset_state(void);
int kdk_test_last_source_type(void);
int kdk_test_last_event_type(void);
const char *kdk_test_last_event_code(void);
const char *kdk_test_last_page_name(void);
int kdk_test_last_property_length(void);
const char *kdk_test_property_key(int index);
const char *kdk_test_property_value(int index);
bool kdk_test_data_freed(void);

#ifdef __cplusplus
}
#endif

#endif

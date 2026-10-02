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

#include "kysdk/diagnosetest/libkydatacollect.h"

#include <cstring>

namespace {

KEventSourceType g_lastSourceType = KEVENTSOURCE_DESKTOP;
KEventType g_lastEventType = KEVENT_CLICK;
const char *g_lastEventCode = nullptr;
const char *g_lastPageName = nullptr;
KCustomProperty *g_lastProperties = nullptr;
int g_lastPropertyLength = 0;
bool g_dataFreed = false;

void freeCopiedProperties()
{
    if (!g_lastProperties) {
        return;
    }

    for (int i = 0; i < g_lastPropertyLength; ++i) {
        ::operator delete(g_lastProperties[i].key);
        ::operator delete(g_lastProperties[i].value);
    }
    delete [] g_lastProperties;
    g_lastProperties = nullptr;
    g_lastPropertyLength = 0;
}

}

extern "C" char *kdk_test_strdup(const char *src) noexcept
{
    if (!src) {
        return nullptr;
    }

    const std::size_t len = std::strlen(src) + 1;
    char *buffer = static_cast<char *>(::operator new(len));
    std::memcpy(buffer, src, len);
    return buffer;
}

extern "C" KTrackData *kdk_dia_data_init(KEventSourceType sourceType, KEventType evnetType)
{
    KTrackData *data = new KTrackData;
    data->eventCode = nullptr;
    data->eventType = evnetType;
    data->eventSourceType = sourceType;
    data->customProperty = nullptr;
    data->customPropertyLength = 0;
    data->pageName = nullptr;
    g_lastSourceType = sourceType;
    g_lastEventType = evnetType;
    g_dataFreed = false;
    return data;
}

extern "C" void kdk_dia_data_free(KTrackData *node)
{
    g_dataFreed = true;
    delete node;
}

extern "C" void kdk_dia_upload_default(KTrackData *node, char *eventCode, char *pageName)
{
    if (node) {
        node->eventCode = eventCode;
        node->pageName = pageName;
    }
    g_lastEventCode = eventCode;
    g_lastPageName = pageName;
}

extern "C" void kdk_dia_append_custom_property(KTrackData *node, KCustomProperty *property, int property_length)
{
    freeCopiedProperties();

    if (node) {
        node->customProperty = property;
        node->customPropertyLength = property_length;
    }
    if (!property || property_length <= 0) {
        g_lastProperties = nullptr;
        g_lastPropertyLength = 0;
        return;
    }

    g_lastProperties = new KCustomProperty[property_length];
    for (int i = 0; i < property_length; ++i) {
        g_lastProperties[i].key = kdk_test_strdup(property[i].key);
        g_lastProperties[i].value = kdk_test_strdup(property[i].value);
    }
    g_lastPropertyLength = property_length;
}

extern "C" void kdk_test_reset_state(void)
{
    freeCopiedProperties();
    g_lastSourceType = KEVENTSOURCE_DESKTOP;
    g_lastEventType = KEVENT_CLICK;
    g_lastEventCode = nullptr;
    g_lastPageName = nullptr;
    g_dataFreed = false;
}

extern "C" int kdk_test_last_source_type(void)
{
    return int(g_lastSourceType);
}

extern "C" int kdk_test_last_event_type(void)
{
    return int(g_lastEventType);
}

extern "C" const char *kdk_test_last_event_code(void)
{
    return g_lastEventCode;
}

extern "C" const char *kdk_test_last_page_name(void)
{
    return g_lastPageName;
}

extern "C" int kdk_test_last_property_length(void)
{
    return g_lastPropertyLength;
}

extern "C" const char *kdk_test_property_key(int index)
{
    return (g_lastProperties && index >= 0 && index < g_lastPropertyLength) ? g_lastProperties[index].key : nullptr;
}

extern "C" const char *kdk_test_property_value(int index)
{
    return (g_lastProperties && index >= 0 && index < g_lastPropertyLength) ? g_lastProperties[index].value : nullptr;
}

extern "C" bool kdk_test_data_freed(void)
{
    return g_dataFreed;
}

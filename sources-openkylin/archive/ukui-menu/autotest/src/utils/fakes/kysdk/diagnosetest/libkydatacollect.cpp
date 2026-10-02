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

#include "libkydatacollect.h"

namespace {

FakeKdkDiaRecord g_record;

}

namespace FakeKdkDiaStore {

void reset()
{
    g_record = FakeKdkDiaRecord();
}

FakeKdkDiaRecord lastRecord()
{
    return g_record;
}

}

extern "C" {

KTrackData *kdk_dia_data_init(int source, int eventType)
{
    g_record.initSource = source;
    g_record.initEventType = eventType;

    auto *data = new KTrackData;
    data->source = source;
    data->eventType = eventType;
    return data;
}

void kdk_dia_append_custom_property(KTrackData *data, KCustomProperty *properties, int size)
{
    Q_UNUSED(data);
    g_record.appendCalled = true;
    g_record.customProperties.clear();

    for (int i = 0; i < size; ++i) {
        g_record.customProperties.append({
            QString::fromUtf8(properties[i].key ? properties[i].key : ""),
            QString::fromUtf8(properties[i].value ? properties[i].value : "")
        });
    }
}

void kdk_dia_upload_default(KTrackData *data, const char *event, const char *page)
{
    Q_UNUSED(data);
    g_record.uploadDefaultCalled = true;
    g_record.uploadEvent = QString::fromUtf8(event ? event : "");
    g_record.uploadPage = QString::fromUtf8(page ? page : "");
}

void kdk_dia_upload_search_content(KTrackData *data, const char *event, const char *page, const char *content)
{
    Q_UNUSED(data);
    g_record.uploadSearchCalled = true;
    g_record.uploadEvent = QString::fromUtf8(event ? event : "");
    g_record.uploadPage = QString::fromUtf8(page ? page : "");
    g_record.uploadContent = QString::fromUtf8(content ? content : "");
}

void kdk_dia_data_free(KTrackData *data)
{
    g_record.freeCalled = true;
    delete data;
}

}

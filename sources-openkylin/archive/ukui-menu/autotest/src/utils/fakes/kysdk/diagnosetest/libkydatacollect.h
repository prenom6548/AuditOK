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

#ifndef AUTOTEST_UTILS_FAKE_LIBKYDATACOLLECT_H
#define AUTOTEST_UTILS_FAKE_LIBKYDATACOLLECT_H

#include <QList>
#include <QPair>
#include <QString>

typedef struct {
    char *key;
    char *value;
} KCustomProperty;

typedef struct {
    int source;
    int eventType;
} KTrackData;

enum {
    KEVENTSOURCE_DESKTOP = 1,
    KEVENT_CLICK = 2,
    KEVENT_CUSTOM = 3,
    KEVENT_SEARCH = 4
};

struct FakeKdkDiaRecord
{
    int initSource = -1;
    int initEventType = -1;
    QList<QPair<QString, QString>> customProperties;
    QString uploadEvent;
    QString uploadPage;
    QString uploadContent;
    bool appendCalled = false;
    bool uploadDefaultCalled = false;
    bool uploadSearchCalled = false;
    bool freeCalled = false;
};

namespace FakeKdkDiaStore {
void reset();
FakeKdkDiaRecord lastRecord();
}

extern "C" {
KTrackData *kdk_dia_data_init(int source, int eventType);
void kdk_dia_append_custom_property(KTrackData *data, KCustomProperty *properties, int size);
void kdk_dia_upload_default(KTrackData *data, const char *event, const char *page);
void kdk_dia_upload_search_content(KTrackData *data, const char *event, const char *page, const char *content);
void kdk_dia_data_free(KTrackData *data);
}

#endif

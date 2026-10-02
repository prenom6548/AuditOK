/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#include "event-track.h"

#include <kysdk/diagnosetest/libkydatacollect.h>
#include <cstdlib>

namespace Sidebar {

KCustomProperty* appendCustomProp(KTrackData *data, const QVariantMap &map)
{
    if (!data || map.isEmpty()) {
        return nullptr;
    }

    int i = 0;
    auto properties = new KCustomProperty[map.size()];
    QMapIterator<QString, QVariant> it(map);
    while (it.hasNext()) {
        it.next();

        std::string string = it.key().toStdString();
        properties[i].key = strdup(string.c_str());

        string = it.value().toString().toStdString();
        properties[i].value = strdup(string.c_str());

        ++i;
    }

    return properties;
}

void freeKCustomProperty(KCustomProperty *customProperty, int size)
{
    if (!customProperty || size <= 0) {
        return;
    }

    for (int i = 0; i < size; ++i) {
        free(customProperty[i].key);
        free(customProperty[i].value);
    }

    delete [] customProperty;
}

EventTrack::EventTrack(QObject *parent) : QObject(parent)
{

}

void EventTrack::sendSlideEvent(const QString& code, const QString& page, const QVariantMap &map)
{
    KTrackData* data = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    KCustomProperty* properties = appendCustomProp(data, map);
    if (properties) {
        kdk_dia_append_custom_property(data, properties, map.size());
    }

    kdk_dia_upload_default(data, code.toUtf8().data(), page.toUtf8().data());
    kdk_dia_data_free(data);
    freeKCustomProperty(properties, map.size());
}

EventTrack *EventTrack::qmlAttachedProperties(QObject *object)
{
    Q_UNUSED(object)
    return EventTrack::instance();
}

EventTrack *EventTrack::instance()
{
    static EventTrack eventTrack;
    return &eventTrack;
}

} // Sidebar

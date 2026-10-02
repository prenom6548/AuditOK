/*
 * Copyright (C) 2020-2024 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "PipewireMetadata.h"

#include "PipewireManager1.h"


namespace UkuiAudioFramwork
{
#define PIPEWIRE_CAST(x) PipewireMetadata* _this = static_cast<PipewireMetadata*>(x);

//-----------------------------------------------------------------------------
static int metadata_property(void *data,
                             uint32_t id,
                             const char* key,
                             const char* type,
                             const char* value)
{
    PIPEWIRE_CAST(data);
    return _this->_metadata_property(id, key, type, value);
}

int PipewireMetadata::_metadata_property(uint32_t id,
                                          const char* key,
                                          const char* type,
                                          const char* value)
{
//    if (key == nullptr) {
//        emit onAllKeysRemoved(id);
//    } else if (value == nullptr) {
//        emit onKeyRemoved(id, key);
//    } else {
//        emit onKeyUpdated(id, key, type, value);
//    }

    return 0;
}

static const pw_metadata_events metadata_events {
    .version = PW_VERSION_METADATA_EVENTS,
    .property = metadata_property,
};

//-----------------------------------------------------------------------------

PipewireMetadata::PipewireMetadata(Pipewire *parent, uint32_t id, const spa_dict* props)
    : pipewire(parent)
{
    metadata = static_cast<pw_metadata*>(
                   pw_registry_bind(parent->registry, id, PW_TYPE_INTERFACE_Metadata, PW_VERSION_METADATA, 0));
    pw_metadata_add_listener(metadata,
                             &metadata_listener,
                             &metadata_events,
                             this);
    parent->resync();
}

PipewireMetadata::~PipewireMetadata()
{
    if (metadata != nullptr) {
        spa_hook_remove(&metadata_listener);
        pw_proxy_destroy((struct pw_proxy*) metadata);
    }
}

//-----------------------------------------------------------------------------

void PipewireMetadata::setProperty(const char *key, const char *value)
{
    pw_metadata_set_property(metadata,
                             0,
                             key,
                             nullptr,
                             value);
    pipewire->round_trip();
}

}

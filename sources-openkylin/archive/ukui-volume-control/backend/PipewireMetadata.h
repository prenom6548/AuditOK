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

#pragma once

#include <pipewire/pipewire.h>
#include <pipewire/extensions/metadata.h>

//#include <QObject>

namespace UkuiAudioFramwork
{
class Pipewire;

class PipewireMetadata
{
private:
    void onAllKeysRemoved(uint32_t id);
    void onKeyRemoved(uint32_t id, const char* key);
    void onKeyUpdated(uint32_t id, const char* key, const char* type, const char* value);

protected:
    Pipewire *pipewire = nullptr;
    struct pw_metadata *metadata = nullptr;
    struct spa_hook metadata_listener;

public:
    explicit PipewireMetadata(Pipewire *parent, uint32_t id, const spa_dict* props);
    virtual ~PipewireMetadata();

    void setProperty(const char *key, const char *value);
    void clear();

    int _metadata_property(uint32_t id,
                           const char* key,
                           const char* type,
                           const char* value);

};
}

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

#ifndef PIPEWIREDEVICE_H
#define PIPEWIREDEVICE_H

#include <spa/utils/hook.h>
#include <pipewire/device.h>

namespace UkuiAudioFramwork
{
class Pipewire;

/**
 * @todo write docs
 */
class PipewireDevice
{
    void volumeChanged(float);

public:
    /**
     * Default constructor
     */
    PipewireDevice(Pipewire *parent, uint32_t id, const struct spa_dict *props);

    /**
     * Destructor
     */
    virtual ~PipewireDevice();

    void _device_info (const struct pw_device_info *info);

    void _device_param (int seq, uint32_t id, uint32_t index, uint32_t next,
                        const struct spa_pod *param);

    // setters getters
    float volume() const { return m_volume; }
    void setVolume(float v) { m_volume = v; /*TODO*/ }

private:
    const Pipewire* pipewire;
    const uint32_t m_id;
    ::pw_device* m_device = nullptr;
    spa_hook m_listener;

    float m_volume = 0;

};
}
#endif // PIPEWIREDEVICE_H

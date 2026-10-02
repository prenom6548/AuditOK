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

#include "PipewireDevice.h"

#include <iostream>
#include <cassert>

#include "PipewireManager1.h"
#include "AudioContext.h"


namespace UkuiAudioFramwork
{
#define PIPEWIRE_CAST(x) PipewireDevice* _this = static_cast<PipewireDevice*>(x);

static void s_device_info (void* data, const struct pw_device_info* info)
{
    PIPEWIRE_CAST(data);
    _this->_device_info(info);
}

void PipewireDevice::_device_info(const struct pw_device_info* info)
{
//    spa_dict_lookup(info->props, PW_KEY_NODE_NICK);
     if (info->change_mask & PW_DEVICE_CHANGE_MASK_PROPS) {

    }
    if (info->change_mask & PW_DEVICE_CHANGE_MASK_PARAMS) {
        // TODO pw_node_enum_params(m_device, 0, SPA_PARAM_Route, 0, 0, nullptr);
    }
}


static void s_device_param (void *data, int seq,
		      uint32_t id, uint32_t index, uint32_t next,
		      const struct spa_pod *param)
{
    PIPEWIRE_CAST(data);
    _this->_device_param(seq, id, index, next, param);
}

void PipewireDevice::_device_param(int seq, uint32_t id, uint32_t index, uint32_t next, const struct spa_pod* param)
{
    // spa_debug_pod(2, nullptr, param);

    const spa_pod_prop* prop;

    prop = spa_pod_find_prop(param, nullptr, SPA_PARAM_ROUTE_props);
    if (prop == nullptr)
        return;
    const spa_pod *route_props = &prop->value;

    prop = spa_pod_find_prop(route_props, nullptr, SPA_PROP_channelVolumes);
    if (prop == nullptr)
        return;

    assert(spa_pod_is_array(&prop->value));
    const spa_pod_array *volumes = (const spa_pod_array*) &prop->value;

    uint32_t n_channels = 0;
    uint32_t type = SPA_POD_ARRAY_VALUE_TYPE(volumes);
    assert(SPA_TYPE_Float == type);
    float *data = (float*) spa_pod_get_array((const spa_pod*) volumes, &n_channels);
    m_volume = data[0];
//    emit volumeChanged(m_volume);
    AUDIO_DEBUG("New volume for device: {} volume: {}", m_id, m_volume);
}


static const pw_device_events device_events = {
    .version = PW_VERSION_CORE_EVENTS,
    .info = &s_device_info,
    .param = &s_device_param
};

PipewireDevice::PipewireDevice(Pipewire* parent, uint32_t id, const spa_dict* props)
    : pipewire(parent)
    , m_id(id)
{
    m_device = static_cast<::pw_device*>(
        pw_registry_bind(pipewire->registry, id, PW_TYPE_INTERFACE_Device,
                         PW_VERSION_CLIENT, 0));

    if (m_device == nullptr) {
        throw std::runtime_error("Error creating device proxy");
    }

    pw_device_add_listener(m_device, &m_listener, &device_events, this);
}

PipewireDevice::~PipewireDevice()
{
    if (m_device != nullptr) {
        pw_proxy_destroy((pw_proxy*) m_device);
    }
}

}

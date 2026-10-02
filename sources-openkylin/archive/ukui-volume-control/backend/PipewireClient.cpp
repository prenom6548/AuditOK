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

#include "PipewireClient.h"

#include "PipewireManager1.h"


#include <iostream>
#include <string>

namespace UkuiAudioFramwork
{
#define PIPEWIRE_CAST(x) PipewireClient* _this = static_cast<PipewireClient*>(x);

//-----------------------------------------------------------------------------

static void client_info(void *data, const pw_client_info *info)
{
    PIPEWIRE_CAST(data);
    _this->_client_info(info);
}

void PipewireClient::_client_info(const struct pw_client_info *info)
{
    if (info->props == nullptr) {
        return;
    }
    const struct spa_dict_item *item;
    spa_dict_for_each(item, info->props) {
        const std::string key = item->key;
        const std::string value = item->value;
        m_properties[key] = value;
		// std::clog << "client_" << this->m_id << ":[" << key.toStdString() << "]=" << value.toStdString() << std::endl;
//        emit propertyChanged(key, value);
    }

//    emit propertiesChanged();
}

static const pw_client_events client_events {
    .version = PW_VERSION_CLIENT_EVENTS,
    .info = client_info,
};

//-----------------------------------------------------------------------------

PipewireClient::PipewireClient(Pipewire *parent, uint32_t id, const spa_dict* props)
    : /*QObject(parent)
    , */pipewire(parent)
	, m_id(id)
{
    client = static_cast<pw_client*>(
                 pw_registry_bind(pipewire->registry, id, PW_TYPE_INTERFACE_Client, PW_VERSION_CLIENT, 0));

    if (client == nullptr) {
        throw std::runtime_error("Error creating client proxy");
    }

    pw_client_add_listener(client,
                           &client_listener,
                           &client_events,
                           this);
    pipewire->resync();
}

PipewireClient::~PipewireClient()
{
    if (client != nullptr) {
        spa_hook_remove(&client_listener);
        pw_proxy_destroy((struct pw_proxy*) client);
    }
}

}

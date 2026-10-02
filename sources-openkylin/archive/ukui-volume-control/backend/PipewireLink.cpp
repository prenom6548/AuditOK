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

#include "PipewireLink.h"
#include "PipewireManager1.h"
#include "utils.h"


namespace UkuiAudioFramwork
{
PipewireLink::PipewireLink(Pipewire* parent, uint32_t id, const spa_dict* props)
    : pipewire(parent)
    , m_id(id)
    , m_input_port(spa_dict_get_u32(props, PW_KEY_LINK_INPUT_PORT))
    , m_output_port(spa_dict_get_u32(props, PW_KEY_LINK_OUTPUT_PORT))
    , m_input_node(spa_dict_get_u32(props, PW_KEY_LINK_INPUT_NODE))
    , m_output_node(spa_dict_get_u32(props, PW_KEY_LINK_INPUT_NODE))
{
     std::cout << "Adding link (" << id << ") with props:" << std::endl;
     const struct spa_dict_item *item;
     spa_dict_for_each(item, props) {
         std::cout << '\t' << item->key << ":" << item->value << std::endl;
     }
	link = static_cast<pw_link*>(
			pw_registry_bind(pipewire->registry, id, PW_TYPE_INTERFACE_Link, PW_VERSION_CLIENT, 0));

	if (link == nullptr) {
		throw std::runtime_error("Error creating link proxy");
	}
}

PipewireLink::~PipewireLink()
{
	if (link != nullptr) {
		pw_proxy_destroy((struct pw_proxy*) link);
	}
}

}

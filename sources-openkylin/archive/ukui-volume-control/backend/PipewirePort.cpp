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

#include "PipewirePort.h"

#include <spa/utils/dict.h>
#include <stdexcept>
#include "PipewireManager1.h"


namespace UkuiAudioFramwork
{
#define PIPEWIRE_CAST(x) PipewirePort* _this = static_cast<PipewirePort*>(x);

static void static_port_info(void *data, const struct pw_port_info *info)
{
    PIPEWIRE_CAST(data);
    _this->_port_info(info);
}
static void static_port_param(void *data, int seq,
							  uint32_t id, uint32_t index, uint32_t next,
                              const struct spa_pod *param)
{
    PIPEWIRE_CAST(data);
    _this->_port_param(seq, id, index, next, param);
}

static const pw_port_events port_events {
	.version = PW_VERSION_PORT_EVENTS,
	.info = static_port_info,
	.param = static_port_param,
};

PipewirePort::PipewirePort(Pipewire* parent, uint32_t id, const spa_dict* props)
    : pipewire(parent)
	, m_id(id)
{
	port = static_cast<pw_port*>(
			pw_registry_bind(pipewire->registry, id, PW_TYPE_INTERFACE_Port, PW_VERSION_CLIENT, 0));

	if (port == nullptr) {
		throw std::runtime_error("Error creating port proxy");
	}

	pw_port_add_listener(port,
						 &port_listener,
						 &port_events,
						 this);

    std::string portName = spa_dict_lookup(props, PW_KEY_PORT_PHYSICAL) ? spa_dict_lookup(props, PW_KEY_PORT_PHYSICAL) : "";
     std::cout << "Adding port (" << id << ") with props:" << "port。name: " << portName << std::endl;
     const struct spa_dict_item *item;
     spa_dict_for_each(item, props) {
         std::cout << '\t' << item->key << ":" << item->value << std::endl;
     }
}

PipewirePort::~PipewirePort()
{
	if (port != nullptr) {
		spa_hook_remove(&port_listener);
		pw_proxy_destroy((struct pw_proxy*) port);
	}
}

uint32_t PipewirePort::id() const
{
    return m_id;
}

void PipewirePort::set_id(uint32_t id)
{
    if (m_id == id) {
        return;
    }

    m_id = id;
//    emit idChanged(m_id);
}

void PipewirePort::_port_info(const struct pw_port_info *info)
{
      std::cout << "Port info (" << this->m_id << "):" << std::endl;
      std::cout << "\t change_mask:" << info->change_mask << std::endl;
      std::cout << "\t direction:" << info->direction << std::endl;
      std::cout << "\t id:" << info->id << std::endl;
      std::cout << "\t n_params:" << info->n_params << std::endl;
      for(uint32_t i=0; i<info->n_params; i++) {
          const spa_param_info* param = info->params+i;
          std::cout << "\t params("<<i<<')' << std::endl;
          std::cout << "\t\tid:" << param->id << std::endl;
          std::cout << "\t\tflags:" << param->flags << std::endl;
          std::cout << "\t\tuser:" << param->user << std::endl;
      }
      std::cout << "\t params_id:" << info->params->id << std::endl;
      std::cout << "\t params_flags:" << info->params->flags << std::endl;
      std::cout << "\t params_user:" << info->params->user << std::endl;
      std::cout << "\t props:" << info->props << std::endl;
      const struct spa_dict_item *item;
      spa_dict_for_each(item, info->props) {
          std::cout << "\t\t" << item->key << ":" << item->value << std::endl;
      }
}

void PipewirePort::_port_param(int seq, uint32_t id, uint32_t index, uint32_t next, const struct spa_pod *param)
{
     std::cout << "Port param (" << this->m_id << "):" << std::endl;
     std::cout << "\t seq:" << seq << std::endl;
     std::cout << "\t id:" << id;
     std::cout << "\t index:" << index;
     std::cout << "\t next:" << next;
     std::cout << "\t param:" << param;
}

}

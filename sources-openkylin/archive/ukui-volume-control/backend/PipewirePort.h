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

//#include <qobject.h>
#include "PipewireManager1.h"

namespace UkuiAudioFramwork
{
/**
 * @todo write docs
 */
class PipewirePort
{

public:
    PipewirePort(Pipewire *parent, uint32_t id, const spa_dict *props);

    virtual ~PipewirePort();

    uint32_t id() const;

	void _port_info(const struct pw_port_info *info);

	void _port_param(int seq,
	                 uint32_t id, uint32_t index, uint32_t next,
	                 const struct spa_pod *param);

    void set_id(uint32_t id);

    void idChanged(uint32_t id);


private:
    Pipewire* pipewire;
    uint32_t m_id = 0;
	pw_port *port = nullptr;
	spa_hook port_listener;
};
}

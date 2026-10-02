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

#include <ostream>
#include <spa/utils/dict.h>
#include <pipewire/link.h>

namespace UkuiAudioFramwork
{
class Pipewire;

/**
 * @todo write docs
 */
class PipewireLink
{
    void idChanged();
    void inputPortChanged();
    void outputPortChanged();
    void inputNodeChanged();
    void outputNodeChanged();

protected:
    Pipewire* pipewire = nullptr;
    const uint32_t m_id;
    const uint32_t m_input_port;
    const uint32_t m_output_port;
    const uint32_t m_input_node;
    const uint32_t m_output_node;
	pw_link* link = nullptr;

public:
    /**
     * Default constructor
     */
    PipewireLink(Pipewire* parent, uint32_t id, const spa_dict* props);

    /**
     * Destructor
     */
    ~PipewireLink();

    [[nodiscard]] int id() const { return m_id; }
    [[nodiscard]] uint32_t id_u32() const { return m_id; }
    [[nodiscard]] uint32_t inputPort() const  { return m_input_port; }
    [[nodiscard]] uint32_t outputPort() const { return m_output_port; }
    [[nodiscard]] uint32_t inputNode() const  { return m_input_node; }
    [[nodiscard]] uint32_t outputNode() const { return m_output_node; }

};

std::ostream& operator<< (std::ostream& out, const PipewireLink& link);
}

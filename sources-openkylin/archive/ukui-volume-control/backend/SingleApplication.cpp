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

#include "SingleApplication.h"
#include <iostream>

SingleApplication::SingleApplication(const std::string& name)
    : m_name(name)
{
    m_lockFile = open(("/tmp/" + m_name + "-" + std::to_string(getuid())).c_str(), O_CREAT | O_RDWR, 0666);
    if (m_lockFile != -1 && flock(m_lockFile, LOCK_EX | LOCK_NB) == -1) {
        close(m_lockFile);
        m_lockFile = -1;
    }
}

SingleApplication::~SingleApplication()
{
    if (m_lockFile != -1) {
        flock(m_lockFile, LOCK_UN);
        close(m_lockFile);
    }
}

bool SingleApplication::isAlreadyRuning()
{
    return m_lockFile == -1;
}

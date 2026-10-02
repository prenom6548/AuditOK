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

#include "MediaAutoPauseModule.h"

namespace UkuiAudioFramwork
{
MediaAutoPauseModule& MediaAutoPauseModule::getInstance()
{
    static MediaAutoPauseModule instance;
    return instance;
}

void MediaAutoPauseModule::setJson(std::shared_ptr<RestoreVolumeJson> json)
{
    m_pJson = json;
}

bool MediaAutoPauseModule::getAutoPauseStatus()
{
    auto status = m_pJson->getValue("auto-pause");

    if (status)
        return *status;

    std::cerr << "getAutoPauseStatus failed!" << std::endl;

    return false;
}

void MediaAutoPauseModule::setAutoPauseStatus(bool status)
{
    m_pJson->setValue("auto-pause", status);
}
}

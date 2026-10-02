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

#include "DeviceManagerModule.h"

#include "RestoreVolumeJson.h"

namespace UkuiAudioFramwork
{
DeviceManagerModule& DeviceManagerModule::getInstance()
{
    static DeviceManagerModule instance;
    return instance;
}

void DeviceManagerModule::setManager(std::shared_ptr<AudioManager> manager)
{
    m_pAudioManager = manager;
}

void DeviceManagerModule::setJson(std::shared_ptr<RestoreVolumeJson> json)
{
    m_pJson = json;
}

/**
 * 禁用分为以下两种情况
 *  1.该声卡中除了禁用的端口外还存在其他可用的同类型的端口
 *      · 可用的端口在同一个sink/sources上 --> 需要将当前设备静音并记录之前的静音状态，并将该端口切换到其他端口上，将禁用的端口的状态同步到数据库并发出信号，通知ui程序去更新端口列表（典型事例：扬声器和模拟耳机、前后置耳机和线缆输出，内部话筒和话筒）
 *      · 可用的端口在不同的sink/source上  --> 如果另一个端口和当前端口是所在相同的配置文件下，则将当前设备静音并记录之前的静音状态，判断禁用的设备是不是默认的设备，是的话则需要切换到一个可用的优先级最高的设备上
 *                                      --> 如果另一个端口和当前端口是不在相同的配置文件下，只需要切换到另一个设备的最高优先级的配置文件即可，相当于切换设备；将禁用的端口的状态同步到数据库并发出信号，通知ui程序去更新端口列表
 *  2.该声卡中除了禁用的端口外不存在其他可用的同类型的端口
 *	    · 将当前设备静音并记录之前的静音状态，判断禁用的设备是不是默认的设备，是的话则需要切换到一个可用的优先级最高的设备上，如果不是则不需要进行切换操作；
 *		· 将禁用的端口的状态同步到数据库并发出信号，通知ui程序去更新端口列表
 */
void DeviceManagerModule::setEnabled(const std::string& card, const std::string& port, bool enable)
{
    if (!m_pAudioManager) {
        std::cerr << "Setting enable failed, please check the status of the audio server!" << std::endl;
        return;
    }

    // 1. 首先更新端口的状态
    m_pAudioManager->setEnabled(m_pJson, card, port, enable);
}

bool DeviceManagerModule::isEnabled(const std::string& card, const std::string& port) const
{
    return m_pJson->getStatus(card, port, "enable");
}
}

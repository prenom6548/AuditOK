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

#include "TrayClientManager.h"

#include <QDebug>
#include "Util.h"

TrayClientManager::TrayClientManager(std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& volumeSliderMap,
                                     std::unordered_map<int, std::shared_ptr<ITrayVolumeSliderItem>>& appvolumeSliderMap,
                                     IMainWindow* window) :
    m_volumeSliderItemMap(volumeSliderMap), m_appVolumeSliderItemMap(appvolumeSliderMap), m_pMainWid(std::move(window))
{

}

void TrayClientManager::setValue(int type, int idx, int value)
{
    if (m_pMainWid != nullptr) {
        m_pMainWid->setValue(type, idx, value);
    }
}

void TrayClientManager::setIcon(int type, int idx, const QIcon& icon)
{
    switch (type) {
    case 0:
    case 1:
        m_volumeSliderItemMap[type]->setIcon(icon);
        break;
    case 2:
    case 3:
        m_appVolumeSliderItemMap[idx]->setIcon(icon);
        break;
    default:
        break;
    }
}

void TrayClientManager::setCurrentIndex(int idx)
{
    if (m_pMainWid)
        m_pMainWid->setCurrentIndex(idx);
}

void TrayClientManager::addDevicePortItem(int type, const ClientDeviceInfo& info)
{
    QString iconStr = "";

    if (info.portName.contains("headphone", Qt::CaseInsensitive))
        iconStr = "audio-headphones-symbolic";
    else if (info.portName.contains("headset", Qt::CaseInsensitive))
        iconStr = "audio-headset-symbolic";
    else
        iconStr = "audio-volume-high-symbolic";

    if (m_pMainWid)
        m_pMainWid->addDevToWidget(iconStr, info);
}

void TrayClientManager::removeDevicePortItem(const QString& portName, const QString& devName)
{
    if (m_pMainWid)
        m_pMainWid->removeDevFromWidget(portName, devName);
}

void TrayClientManager::addAppItem(int idx, const QString& iconName, const QString& descName)
{
    if (m_pMainWid)
        m_pMainWid->addAppItem(idx, iconName, descName);
}

void TrayClientManager::removeAppItem(int idx)
{
    if (m_pMainWid)
        m_pMainWid->removeAppItem(idx);
}

void TrayClientManager::updateDescription(const QString& input, const QString& output)
{
    if (m_pMainWid)
        m_pMainWid->updateDescription(input, output);
}

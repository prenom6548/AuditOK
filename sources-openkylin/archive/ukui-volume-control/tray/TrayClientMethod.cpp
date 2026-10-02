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

#include "TrayClientMethod.h"

#include <QDebug>

TrayClientMethod& TrayClientMethod::getInstance()
{
    static TrayClientMethod instance;
    return instance;
}

void TrayClientMethod::setManager(std::shared_ptr<TrayClientManager> manager)
{
    m_pClientManager = manager;
}

void TrayClientMethod::setValue(int type, int idx, int value)
{
    if (m_pClientManager)
        m_pClientManager->setValue(type, idx, value);
}

void TrayClientMethod::setIcon(int type, const QIcon &icon)
{
    if (m_pClientManager)
        m_pClientManager->setIcon(type, 1, icon);
}

void TrayClientMethod::setChecked(int type, bool checked)
{
//    if (m_pClientManager)
//        m_pClientManager->setChecked(type, checked);
//    else {
//        qDebug() << "m_pClientManager is null, pluse init client manager!";
//    }
}

void TrayClientMethod::setCurrentIndex(int idx)
{
    if (m_pClientManager)
        m_pClientManager->setCurrentIndex(idx);
}

void TrayClientMethod::addDeviceItem(int type, const ClientDeviceInfo& info)
{
    if (m_pClientManager)
        m_pClientManager->addDevicePortItem(type, info);
}

void TrayClientMethod::removeDeviceItem(const QString& portName, const QString& devName)
{
    if (m_pClientManager)
        m_pClientManager->removeDevicePortItem(portName, devName);
}

void TrayClientMethod::addAppItem(int idx, const QString& iconName, const QString& descName)
{
    if (m_pClientManager)
        m_pClientManager->addAppItem(idx, iconName, descName);
}

void TrayClientMethod::removeAppItem(int idx)
{
    if (m_pClientManager)
        m_pClientManager->removeAppItem(idx);
}

void TrayClientMethod::updateDescription(const QString& input, const QString& output)
{
    if (m_pClientManager)
        m_pClientManager->updateDescription(input, output);
}

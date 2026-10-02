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

#include "ClientMethod.h"

#include <QDebug>

ClientMethod& ClientMethod::getInstance()
{
    static ClientMethod instance;
    return instance;
}

void ClientMethod::setManager(std::shared_ptr<ClientManager> manager)
{
    m_pClientManager = manager;
}

void ClientMethod::setValue(int type, int value)
{
    if (m_pClientManager)
        m_pClientManager->setValue(type, value);
}

void ClientMethod::setMute(int type, bool mute)
{
    if (m_pClientManager)
        m_pClientManager->setMute(type, mute);
}

void ClientMethod::setEnabled(int type, bool enable)
{
    qDebug() << __func__ << "type:" << type << "enable:" << enable;
    if (m_pClientManager)
        m_pClientManager->setEnabled(type, enable);
}

void ClientMethod::setBalanceEnabled(bool enable)
{
    qDebug() << __func__ << "enable:" << enable;
    if (m_pClientManager)
        m_pClientManager->setBalanceEnabled(enable);
}

void ClientMethod::setRange(int type, int min, int max)
{
    if (m_pClientManager)
        m_pClientManager->setRange(type, min, max);
}

void ClientMethod::setChecked(int type, bool checked)
{
    if (m_pClientManager)
        m_pClientManager->setChecked(type, checked);
    else {
        qDebug() << "m_pClientManager is null, pluse init client manager!";
    }
}

void ClientMethod::setCurrentIndex(int type, int idx)
{
    if (-1 == idx) {
        qDebug() << QString("type: %1 idx: %2 is invalid").arg(type).arg(idx);
        return;
    }

    if (m_pClientManager)
        m_pClientManager->setCurrentIndex(type, idx);

    qDebug() << "ClientMethod::setCurrentIndex, type:" << type << " idx: " << idx;
}

int ClientMethod::findData(int type, const QVariant& value, int role) const
{
    if (m_pClientManager)
        return m_pClientManager->findData(type, value, role);

    return -1;
}

int ClientMethod::findText(int type, const QString& text) const
{
    if (m_pClientManager)
        return m_pClientManager->findText(type, text);

    return -1;
}

void ClientMethod::addItem(int type, const QString& text, const QVariant& userData)
{
    if (m_pClientManager)
        m_pClientManager->addItem(type, text, userData);
}

void ClientMethod::removeItem(int type, int idx)
{
    if (m_pClientManager)
        m_pClientManager->removeItem(type, idx);
}

void ClientMethod::clear(int type)
{
    if (m_pClientManager)
        m_pClientManager->clear(type);
}

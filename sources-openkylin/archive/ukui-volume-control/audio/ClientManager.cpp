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

#include "ClientManager.h"

#include <QDebug>

#include "Util.h"

using namespace UkuiAudioFramwork;

ClientManager::ClientManager(std::unordered_map<SwitchButtonType, ISwitchButtonItem*>& switchBtnMap,
                             std::unordered_map<VolumeSliderType, IVolumeSliderItem*>& volumeSliderMap,
                             std::unordered_map<SelectComboxType, ISelectComboxItem*>& selectComboxMap)
    : m_switchBtnItemMap(switchBtnMap),
      m_volumeSliderItemMap(volumeSliderMap),
      m_selectComboxItemMap(selectComboxMap)
{

}

void ClientManager::setValue(int type, int value)
{
    // check type is vaild

    m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->setValue(value);
}

void ClientManager::setMute(int type, bool mute)
{
    m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->setMute(mute);
}

void ClientManager::setEnabled(int type, bool enable)
{
    qDebug() << __func__ << "type:" << type << "enable:" << enable;
    if (type == 0) {
        m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_INPUT]->setEnabled(enable);
        m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL]->setValue(0);
        m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_INPUTLEVEL]->setEnabled(enable);
    }
    else if (type == 1) {
        m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_OUTPUT]->setEnabled(enable);
        //start
        if (m_switchBtnItemMap[SwitchButtonType::SWITCH_BUTTON_MONO]->getSwitchButton()->isChecked()) {
            qDebug() << __func__ << "mono true , set balance false.";
            m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_BALANCE]->setEnabled(false);
        } else {
            qDebug() << __func__ << "mono false , set balance.";
            m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_BALANCE]->setEnabled(enable);
        }
        //end
    }
}

void ClientManager::setBalanceEnabled(bool enable)
{
    qDebug() << __func__ << "enable:" << enable;
    if (enable) {
        m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_BALANCE]->setEnabled(true);
    }
    else {
        m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_BALANCE]->setValue(0);
        m_volumeSliderItemMap[VolumeSliderType::VOLUME_SLIDER_BALANCE]->setEnabled(enable);
    }
}

void ClientManager::setRange(int type, int min, int max)
{
    m_volumeSliderItemMap[IntToEnum<VolumeSliderType>(type)]->setRange(min, max);
}

void ClientManager::setChecked(int type, bool checked)
{
    m_switchBtnItemMap[IntToEnum<SwitchButtonType>(type)]->setChecked(checked);
}

void ClientManager::setCurrentIndex(int type, int idx)
{
    m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->setCurrentIndex(idx);
}

int ClientManager::findData(int type, const QVariant& value, int role) const
{
    return m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->findData(value, role);
}

int ClientManager::findText(int type, const QString& text) const
{
    return m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->findText(text);
}

void ClientManager::addItem(int type, const QString& text, const QVariant& userData)
{
    qDebug() << QString("add %1 data: %2 to %3 combox").arg(text).arg(userData.toString()).arg(type);
    m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->insertItem(text, userData);
}

void ClientManager::removeItem(int type, int index)
{
    m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->removeItem(index);
}

void ClientManager::clear(int type)
{
    m_selectComboxItemMap[IntToEnum<SelectComboxType>(type)]->clear();
}

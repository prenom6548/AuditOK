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

#include "IVolumeSliderItem.h"

IVolumeSliderItem::IVolumeSliderItem(std::shared_ptr<IVolumeSlider> slider, QLabel *label)
    : m_pVolumeSlider(slider), m_pDisplayLabel(label)
{

}

//void IVolumeSliderItem::setValue(int value)
//{
//    m_pVolumeSlider->setValue(value);
//}

std::shared_ptr<IVolumeSlider> IVolumeSliderItem::getSlider() const
{
    return m_pVolumeSlider;
}

void IVolumeSliderItem::setRange(int min, int max)
{
    m_pVolumeSlider->setRange(min, max);
}

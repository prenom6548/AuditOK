/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: ShaoShuai Tian <tianshaoshuai@kylinos.cn>
 *
 */

#include "datacdrom.h"
#include "datacdromimpl.h"

using namespace KYSDKUdfBurn;

DataCDROM::DataCDROM(const char *blockName)
{
    std::string b(blockName);
    m_impl = new DataCDROMImpl(b);
}

DataCDROM::~DataCDROM()
{
    if (m_impl)
    {
        delete m_impl;
    }
}

void DataCDROM::getCDROMInfo()
{
    m_impl->calculateDiscCapacity();
}

unsigned long DataCDROM::getCDROMCapacity()
{
    return m_impl->getCDROMCapacity();
}

unsigned long DataCDROM::getCDROMUsedCapacity()
{
    return m_impl->getCDROMUsedCapacity();
}

std::string DataCDROM::getCDROMType()
{
    return m_impl->getCDROMType();
}

std::string DataCDROM::getDiscType()
{
    Device::MediaType type = m_impl->mediaType();
    return Device::mediaTypeString(type);
}

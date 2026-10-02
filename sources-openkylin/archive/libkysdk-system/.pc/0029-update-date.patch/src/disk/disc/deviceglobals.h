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

#ifndef DEVICEGLOBALS_H
#define DEVICEGLOBALS_H

#include "msf.h"
#include "devicetypes.h"

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace Device
{
    std::string deviceTypeString(int t);
    std::string writingModeString(int m);
    std::string mediaStateString(int state);
    /**
     * @param simplyfied if true the formatting state of DVD media is left out.
     */
    std::string mediaTypeString(int, bool simplyfied = false);
    void debugBitfield(unsigned char *data, long len);

    uint16_t from2Byte(const unsigned char *d);
    uint32_t from4Byte(const unsigned char *d);

    char fromBcd(const char &i);
    char toBcd(const char &i);
    bool isValidBcd(const char &i);

    /**
     * @return the maximum nuber of sectors that can be read from device @p dev starting
     * at sector @p firstSector.
     */
    // int determineMaxReadingBufferSize(Device *dev, const Msf &firstSector);

    // QDebug &operator<<(QDebug &dbg, Device::MediaType);
    // QDebug &operator<<(QDebug &dbg, MediaTypes);

    // QDebug &operator<<(QDebug &dbg, MediaState);
    // QDebug &operator<<(QDebug &dbg, MediaStates);

    // QDebug &operator<<(QDebug &dbg, WritingMode);
    // QDebug &operator<<(QDebug &dbg, WritingModes);
}
#endif // DEVICEGLOBALS_H

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

#include "deviceglobals.h"

std::string Device::deviceTypeString(int t)
{
    std::vector<std::string> s;
    if (t & Device::DEVICE_CD_R)
        s.push_back("CD-R");
    if (t & Device::DEVICE_CD_RW)
        s.push_back("CD-RW");
    if (t & Device::DEVICE_CD_ROM)
        s.push_back("CD-ROM");
    if (t & Device::DEVICE_DVD_ROM)
        s.push_back("DVD-ROM");
    if (t & Device::DEVICE_DVD_RAM)
        s.push_back("DVD-RAM");
    if (t & Device::DEVICE_DVD_R)
        s.push_back("DVD-R");
    if (t & Device::DEVICE_DVD_RW)
        s.push_back("DVD-RW");
    if (t & Device::DEVICE_DVD_R_DL)
        s.push_back("DVD-R DL");
    if (t & Device::DEVICE_HD_DVD_ROM)
        s.push_back("HD DVD-ROM");
    if (t & Device::DEVICE_HD_DVD_R)
        s.push_back("HD DVD-R");
    if (t & Device::DEVICE_HD_DVD_RAM)
        s.push_back("HD DVD-RAM");
    if (t & Device::DEVICE_BD_ROM)
        s.push_back("BD-ROM");
    if (t & Device::DEVICE_BD_R)
        s.push_back("BD-R");
    if (t & Device::DEVICE_BD_RE)
        s.push_back("BD-RE");
    if (t & Device::DEVICE_DVD_PLUS_R)
        s.push_back("DVD+R");
    if (t & Device::DEVICE_DVD_PLUS_RW)
        s.push_back("DVD+RW");
    if (t & Device::DEVICE_DVD_PLUS_R_DL)
        s.push_back("DVD+R DL");

    if (s.empty())
        return "Error";

    std::ostringstream oss;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (i != 0)
            oss << ", ";
        oss << s[i];
    }
    return oss.str();
}

std::string Device::writingModeString(int m)
{
    std::vector<std::string> s;
    if (m & Device::WRITINGMODE_SAO)
        s.push_back("SAO");
    if (m & Device::WRITINGMODE_TAO)
        s.push_back("TAO");
    if (m & Device::WRITINGMODE_RAW)
        s.push_back("RAW");
    if (m & Device::WRITINGMODE_SAO_R96P)
        s.push_back("SAO/R96P");
    if (m & Device::WRITINGMODE_SAO_R96R)
        s.push_back("SAO/R96R");
    if (m & Device::WRITINGMODE_RAW_R16)
        s.push_back("RAW/R16");
    if (m & Device::WRITINGMODE_RAW_R96P)
        s.push_back("RAW/R96P");
    if (m & Device::WRITINGMODE_RAW_R96R)
        s.push_back("RAW/R96R");
    if (m & Device::WRITINGMODE_INCR_SEQ)
        s.push_back("Incremental Sequential");
    if (m & Device::WRITINGMODE_RES_OVWR)
        s.push_back("Restricted Overwrite");
    if (m & Device::WRITINGMODE_LAYER_JUMP)
        s.push_back("Layer Jump");
    if (m & Device::WRITINGMODE_RRM)
        s.push_back("Random Recording");
    if (m & Device::WRITINGMODE_SRM)
        s.push_back("Sequential Recording");
    if (m & Device::WRITINGMODE_SRM_POW)
        s.push_back("Sequential Recording + POW");

    if (s.empty())
        return "no writing mode";

    std::ostringstream oss;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (i != 0)
            oss << ", ";
        oss << s[i];
    }
    return oss.str();
}

std::string Device::mediaTypeString(int m, bool simple)
{
    if (m == Device::MEDIA_UNKNOWN)
        return "unknown medium type";

    std::vector<std::string> s;
    if (m & Device::MEDIA_NONE)
        s.push_back("No media");
    if (m & Device::MEDIA_DVD_ROM)
        s.push_back("DVD-ROM");
    if (m & Device::MEDIA_DVD_R || (simple && (m & Device::MEDIA_DVD_R_SEQ)))
        s.push_back("DVD-R");
    if (m & Device::MEDIA_DVD_R_SEQ && !simple)
        s.push_back("DVD-R Sequential");
    if (m & Device::MEDIA_DVD_R_DL ||
        (simple && (m & (Device::MEDIA_DVD_R_DL_SEQ | Device::MEDIA_DVD_R_DL_JUMP).toInt())))
        s.push_back("DVD-R Dual Layer");
    if (m & Device::MEDIA_DVD_R_DL_SEQ && !simple)
        s.push_back("DVD-R Dual Layer Sequential");
    if (m & Device::MEDIA_DVD_R_DL_JUMP && !simple)
        s.push_back("DVD-R Dual Layer Jump");
    if (m & Device::MEDIA_DVD_RAM)
        s.push_back("DVD-RAM");
    if (m & Device::MEDIA_DVD_RW ||
        (simple && (m & (Device::MEDIA_DVD_RW_OVWR | Device::MEDIA_DVD_RW_SEQ).toInt())))
        s.push_back("DVD-RW");
    if (m & Device::MEDIA_DVD_RW_OVWR && !simple)
        s.push_back("DVD-RW Restricted Overwrite");
    if (m & Device::MEDIA_DVD_RW_SEQ && !simple)
        s.push_back("DVD-RW Sequential");
    if (m & Device::MEDIA_DVD_PLUS_RW)
        s.push_back("DVD+RW");
    if (m & Device::MEDIA_DVD_PLUS_R)
        s.push_back("DVD+R");
    if (m & Device::MEDIA_DVD_PLUS_RW_DL)
        s.push_back("DVD+RW Dual Layer");
    if (m & Device::MEDIA_DVD_PLUS_R_DL)
        s.push_back("DVD+R Dual Layer");
    if (m & Device::MEDIA_CD_ROM)
        s.push_back("CD-ROM");
    if (m & Device::MEDIA_CD_R)
        s.push_back("CD-R");
    if (m & Device::MEDIA_CD_RW)
        s.push_back("CD-RW");
    if (m & Device::MEDIA_HD_DVD_ROM)
        s.push_back("HD DVD-ROM");
    if (m & Device::MEDIA_HD_DVD_R)
        s.push_back("HD DVD-R");
    if (m & Device::MEDIA_HD_DVD_RAM)
        s.push_back("HD DVD-RAM");
    if (m & Device::MEDIA_BD_ROM)
        s.push_back("BD-ROM");
    if (m & Device::MEDIA_BD_R ||
        (simple && (m & (Device::MEDIA_BD_R_SRM | Device::MEDIA_BD_R_RRM).toInt())))
        s.push_back("BD-R");
    if (m & Device::MEDIA_BD_R_SRM && !simple)
        s.push_back("BD-R Sequential (SRM)");
    if (m & Device::MEDIA_BD_R_SRM_POW && !simple)
        s.push_back("BD-R Sequential Pseudo Overwrite (SRM+POW)");
    if (m & Device::MEDIA_BD_R_RRM && !simple)
        s.push_back("BD-R Random (RRM)");
    if (m & Device::MEDIA_BD_RE)
        s.push_back("BD-RE");

    if (s.empty())
        return "Error";

    std::ostringstream oss;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (i != 0)
            oss << ", ";
        oss << s[i];
    }
    return oss.str();
}

std::string Device::mediaStateString(int state)
{
    if (state == Device::STATE_UNKNOWN)
        return "unknown medium state";

    std::vector<std::string> s;
    if (state & Device::STATE_NO_MEDIA)
        s.push_back("no medium");
    if (state & Device::STATE_COMPLETE)
        s.push_back("complete medium");
    if (state & Device::STATE_INCOMPLETE)
        s.push_back("incomplete medium");
    if (state & Device::STATE_EMPTY)
        s.push_back("empty medium");

    if (s.empty())
        return "Error";

    std::ostringstream oss;
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (i != 0)
            oss << ", ";
        oss << s[i];
    }
    return oss.str();
}

void Device::debugBitfield(unsigned char *data, long len)
{
    for (int i = 0; i < len; ++i)
    {
        // 格式化索引
        std::cout << std::setw(4) << i << " - ";

        // 生成二进制字符串
        std::string bitString(8, '0');
        for (int bp = 7; bp >= 0; --bp)
        {
            bitString[7 - bp] = (data[i] & (1 << bp)) ? '1' : '0';
        }

        // 输出结果
        std::cout << bitString << " - " << static_cast<int>(data[i]) << std::endl;
    }
}

uint16_t Device::from2Byte(const unsigned char *d)
{
    if (d == NULL)
    {
        return 0;
    }
    return ((d[0] << 8 & 0xFF00) |
            (d[1] & 0xFF));
}

uint32_t Device::from4Byte(const unsigned char *d)
{
    if (d == NULL)
    {
        return 0;
    }
    return ((d[0] << 24 & 0xFF000000) |
            (d[1] << 16 & 0xFF0000) |
            (d[2] << 8 & 0xFF00) |
            (d[3] & 0xFF));
}

char Device::fromBcd(const char &i)
{
    return (i & 0x0f) + 10 * ((i >> 4) & 0x0f);
}

char Device::toBcd(const char &i)
{
    return (i % 10) | ((((i / 10) % 10) << 4) & 0xf0);
}

bool Device::isValidBcd(const char &i)
{
    return (i & 0x0f) <= 0x09 && (i & 0xf0) <= 0x90;
}

// QDebug &Device::operator<<(QDebug &dbg, Device::MediaType type)
// {
//     return dbg << Device::mediaTypeString(type);
// }

// QDebug &Device::operator<<(QDebug &dbg, Device::MediaTypes types)
// {
//     return dbg << Device::mediaTypeString(types);
// }

// QDebug &Device::operator<<(QDebug &dbg, Device::WritingMode mode)
// {
//     return dbg << writingModeString(mode);
// }

// QDebug &Device::operator<<(QDebug &dbg, Device::WritingModes modes)
// {
//     return dbg << writingModeString(modes);
// }

// QDebug &operator<<(QDebug &dbg, Device::MediaState state)
// {
//     return dbg << Device::mediaStateString(state);
// }

// QDebug &operator<<(QDebug &dbg, Device::MediaStates states)
// {
//     return dbg << Device::mediaStateString(states);
// }

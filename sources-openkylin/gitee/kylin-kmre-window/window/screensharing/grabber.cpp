/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "grabber.h"
#include "grabber_x11.h"

namespace kmre {

Grabber* Grabber::pGrabber = nullptr;

Grabber* Grabber::GetGrabber(uint32_t x, uint32_t y, uint32_t w, uint32_t h, bool c)
{
    DisplayPlatform platform = SessionSettings::getInstance().displayPlatform();
    if (pGrabber == nullptr) {
        if (DisplayPlatform::X11 == platform) {
            pGrabber = new X11Grabber(x, y, w, h, c);
        }
    }
    return pGrabber;
}

void Grabber::DeleteGrabber()
{
    if (pGrabber != nullptr) {
        delete pGrabber;
        pGrabber = nullptr;
    }
}

Grabber::~Grabber()
{
}

}

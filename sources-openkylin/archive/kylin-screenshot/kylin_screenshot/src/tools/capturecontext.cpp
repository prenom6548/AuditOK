/* Copyright (C) 2020, KylinSoft Co., Ltd.

*               2020 KylinSoft Co., Ltd.
* This file is part of Kylin-Screenshot.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "capturecontext.h"
#include <QScreen>
#include <QDesktopWidget>
#include <QApplication>
#include "src/utils/desktopinfo.h"

QPixmap CaptureContext::selectedScreenshotArea() const {
    if (isScrollEnabled) {
        return scrollPixmap.copy(0, 0, scrollPixmap.width(), scrollPixmap.height());
    }

    if (selection.isNull()) {
        return screenshot;
    } else {
#ifndef UNUI
        DesktopInfo info;
        if (info.waylandDectected()) {
            return screenshot.copy(selection.x()*screenshot.devicePixelRatio(),
                                   selection.y()*screenshot.devicePixelRatio(),
                                   selection.width()*screenshot.devicePixelRatio(),
                                   selection.height()*screenshot.devicePixelRatio());
        }
        else
        {
            auto screenNumber = QApplication::desktop()->screenNumber();
            QScreen *screen = QApplication::screens()[screenNumber];
            return screenshot.copy(selection.x() * screen->devicePixelRatio(),
                                   selection.y() * screen->devicePixelRatio(),
                                   selection.width() * screen->devicePixelRatio(),
                                   selection.height() * screen->devicePixelRatio());
        }
#endif
    }
}

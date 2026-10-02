/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#ifndef EMUGL_DISPLAY_UTILS_H
#define EMUGL_DISPLAY_UTILS_H

#include <stdint.h>
#include <QMap>
#include <QMutex>

class EmuGLDisplayUtils : public QObject
{
public:
    EmuGLDisplayUtils(unsigned long winId, QObject *parent = nullptr);
    ~EmuGLDisplayUtils();

    void updateWindowAttribute(int displayId, uint32_t nativeHandle, int width, int height, int orientation, bool needPost);
    void deleteWindowAttribute(int displayId);
    void setSupportDynamicSize(int displayId, uint32_t nativeHandle, int width, int height, bool support);
    int getShmFd(int displayId, uint32_t nativeHandle, bool needPost);

private:
    unsigned long mWinId;
    int mDisplayWidth;
    int mDisplayHeight;
    int mWidth;
    int mHeight;

    struct DisplayWindowAttri {
        unsigned long winId;
        int width;
        int height;
        int orientation;
    };

    QMutex mDisplayWindowsMapLock;
    QMap<int, DisplayWindowAttri> mDisplayWindowsMap;
    bool mNeedRequestCloseAndroidApp;

};

#endif // EMUGL_DISPLAY_UTILS_H

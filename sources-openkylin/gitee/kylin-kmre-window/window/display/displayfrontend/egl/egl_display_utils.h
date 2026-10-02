/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  MaChao    machao@kylinos.cn
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

#ifndef EGL_DISPLAY_UTILS_H
#define EGL_DISPLAY_UTILS_H

#include <stdint.h>
#include <QObject>
#include <QSize>
#include <memory>
#include <vector>

#include "displaymanager/android_display/displaywidget.h"
#include "typedef.h"
#include "gles/texture.h"

#include <GLES2/gl2.h>
#include <EGL/egl.h>

class EglDisplayUtils : public QObject
{
    Q_OBJECT
public:
    EglDisplayUtils(DisplayWidget* widget, QObject* parent = nullptr);
    ~EglDisplayUtils();

    bool initializeUtils();

    void updateImage(int name, int width, int height, int stride, int bpp);
    void drawDisplay();

    GLuint framebufferTextureId();
    QSize framebufferSize();

    struct Data;
private:
    void destroyEglContext();
    void importImage(int name, int width, int height, int stride, int bpp);
    sp<Data> mData;
};


#endif // EGL_DISPLAY_UTILS_H

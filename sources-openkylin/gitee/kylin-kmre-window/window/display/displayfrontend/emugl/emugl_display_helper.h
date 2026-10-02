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

#ifndef EMUGL_DISPLAY_HELPER_H
#define EMUGL_DISPLAY_HELPER_H

#include <QObject>
#include <memory>
#include "emugl_display_utils.h"
#include "displayfrontend/displayhelper.h"

class EmuGLDisplayHelper : public DisplayHelper
{
    Q_OBJECT
public:
    explicit EmuGLDisplayHelper(DisplayWidget* widget, QObject* parent = nullptr);
    ~EmuGLDisplayHelper();

protected:
    int importBuffer(int nativeHandle, int width, int height, int stride, int bpp, int orientation) override;

public slots:
    void initDisplay() override;
    void destroyDisplay() override;
    void update(int name, int width, int height, int stride, int bpp, int orientation) override;
    void redraw() override;

private:
    int mCurrentOrientation = 0;
    int mCurrentAppStatus = 0;
    int mWidth = 0;
    int mHeight = 0;
    std::shared_ptr<EmuGLDisplayUtils> mDisplayUtils;
};

#endif // EMUGL_DISPLAY_HELPER_H

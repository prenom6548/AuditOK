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

#ifndef GLES_DISPLAY_HELPER_H
#define GLES_DISPLAY_HELPER_H

#include <QObject>
#include <memory>
#include "displayfrontend/displayhelper.h"

class GLESDisplayUtils;

class GLESDisplayHelper : public DisplayHelper
{
    Q_OBJECT

public:
    explicit GLESDisplayHelper(DisplayWidget* widget, QObject* parent = nullptr);
    ~GLESDisplayHelper();

public slots:
    virtual void destroyDisplay() override;
    virtual void update(int name, int width, int height, int stride, int bpp, int orientation) override;
    virtual void redraw() override;

protected:
    virtual void initDisplay() override;
    virtual int importBuffer(int name, int width, int height, int stride, int bpp, int orientation) override;

private:
    void drawBuffer();

private:
    int mTryInitCounter;
    int mCurrentOrientation;
    int mCurrentAppStatus = 0;
    std::shared_ptr<GLESDisplayUtils> mDisplayUtils;
    int mImageWidth;
    int mImageHeight;
    const unsigned long mWid;
};

#endif // GLES_DISPLAY_HELPER_H

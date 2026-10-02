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

#ifndef EGL_DISPLAY_HELPER_H
#define EGL_DISPLAY_HELPER_H

#include <QObject>
#include <QMutex>
#include <QMutexLocker>

#include <memory>

#include "displayfrontend/displayhelper.h"
#include "egl_display_utils.h"
#include "typedef.h"

using namespace kmre;

class EglDisplayWidget;

class EglDisplayHelper : public DisplayHelper
{
    Q_OBJECT

public:
    explicit EglDisplayHelper(DisplayWidget* widget, QObject* parent = nullptr);
    ~EglDisplayHelper();

public:
    void lock();
    void unlock();
    GLuint framebufferTextureId();
    QSize framebufferSize();

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
    int mCurrentOrientation;
    bool mIsReadyForRender;
    sp<EglDisplayUtils> mDisplayUtils;
    QMutex mUtilsMutex;
    EglDisplayWidget* mDisplayWidget;
};

#endif // EGL_DISPLAY_HELPER_H
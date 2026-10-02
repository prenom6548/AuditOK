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

#ifndef EGL_DISPLAY_WORK_MANAGER_H
#define EGL_DISPLAY_WORK_MANAGER_H

#include <QObject>
#include <QMutex>

#include "displayfrontend/displayworkmanager.h"

class EglDisplayHelper;

class EglDisplayWorkManager : public DisplayWorkManager
{
public:
    friend class EglDisplayWidget;
    explicit EglDisplayWorkManager(DisplayWidget* widget, QObject *parent = nullptr);
    ~EglDisplayWorkManager();

    virtual void initialize() override;
    virtual void enableDisplayUpdate(bool enable) override;
    virtual void blurUpdate(int msecond) override;

private:
    EglDisplayHelper* getDisplayHelper();
};

#endif // EGL_DISPLAY_WORK_MANAGER_H

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

#ifndef GLES_DISPLAY_WORK_MANAGER_H
#define GLES_DISPLAY_WORK_MANAGER_H

#include <QObject>

#include "displayfrontend/displayworkmanager.h"

class GLESDisplayWorkManager : public DisplayWorkManager
{
public:
    explicit GLESDisplayWorkManager(DisplayWidget* widget, QObject *parent = nullptr);
    ~GLESDisplayWorkManager();

    virtual void initialize() override;
    virtual void enableDisplayUpdate(bool enable) override;
    virtual void blurUpdate(int msecond) override;

};

#endif // GLES_DISPLAY_WORK_MANAGER_H

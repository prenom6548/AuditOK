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

#ifndef NORMALDISPLAYWIDGET_H
#define NORMALDISPLAYWIDGET_H

#include "displaywidget.h"

class NormalDisplayWidget : public DisplayWidget
{
    Q_OBJECT
public:
    NormalDisplayWidget(KmreWindow* window, int id, int width, int height, int orientation, QString displayName);
    ~NormalDisplayWidget();

    void setAutoUpdate(bool enable) override;

protected:
    void showEvent(QShowEvent *event) override;
//    QPaintEngine* paintEngine() const;
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;
};

#endif // NORMALDISPLAYWIDGET_H

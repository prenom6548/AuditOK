/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef NETWORKBUTTON_H
#define NETWORKBUTTON_H

#include <QWidget>
#include <kylin-nm/kynetworkicon.h>
#include "statusbutton.h"

class NetWorkButton : public KyNetworkIcon
{
    Q_OBJECT
public:
    explicit NetWorkButton(QWidget *parent = 0);
    /**
     * @brief 设置按钮当前状态
     * @param selected 是否选中
     * @return
     */
    void setClickedStatus(int status);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);

private:
    int curStatus = NORMAL;
};

#endif // NETWORKBUTTON_H

/*
 * Qt5-UKUI's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: xibowen <lanyue@kylinos.cn>
 *
 */


#ifndef ControlParameters_H
#define ControlParameters_H

#include <QObject>

namespace UKUIConfigStyleSpace {

class ControlParameters : public QObject
{
    Q_OBJECT
public:
    explicit ControlParameters(QObject *parent = nullptr);

    ~ControlParameters();

protected:
    bool elemFocusEnable;
public:
    bool getFocusEnable() const;
    void setFocusEnable(bool enable);

//    virtual void getParameters(){}
};
}


namespace UKUIElementStatus {

    enum ProgressBarOrientation { //记录进度条当前形状
        HorizontalForward = 0, //正常状态
        HorizontalInverted,//水平 反向
        VerticalForward,//垂直 从下往上
        VertivalInverted //垂直，从上往下
    };

}


#endif // ControlParameters_H

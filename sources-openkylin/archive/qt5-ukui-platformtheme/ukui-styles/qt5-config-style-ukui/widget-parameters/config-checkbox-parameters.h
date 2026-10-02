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


#ifndef ConfigCheckBoxParameters_H
#define ConfigCheckBoxParameters_H

#include <QBrush>
#include <QPen>
#include "control-parameters.h"

namespace UKUIConfigStyleSpace {

class ConfigCheckBoxParameters : public ControlParameters
{
    Q_OBJECT
public:
    ConfigCheckBoxParameters();

//private:
    int radius;

    QBrush checkBoxDefaultBrush;
    QBrush checkBoxHoverBrush;
    QBrush checkBoxClickBrush;
    QBrush checkBoxDisableBrush;
    QBrush checkBoxDisableOnNoChangeBrush;
    QBrush checkBoxOnDefaultBrush;
    QBrush checkBoxOnHoverBrush;
    QBrush checkBoxOnClickBrush;
    QBrush checkBoxPathBrush;
    QBrush checkBoxPathDisableBrush;

    QBrush checkboxHookLineBrush;
    QBrush checkboxHookLineDisableBrush;

    QPen checkBoxDefaultPen;
    QPen checkBoxFocusPen;
    QPen checkBoxHoverPen;
    QPen checkBoxClickPen;
    QPen checkBoxDisablePen;
    QPen checkBoxOnDefaultPen;
    QPen checkBoxOnHoverPen;
    QPen checkBoxOnClickPen;
    QPen checkBoxContentPen;

    QBrush checkBoxTextBrush;
    QBrush checkBoxTextDisableBrush;

//    bool animation = true;
    bool onHoverBrushIslinearGradient = false;


    /*
     * 需要配置文件(token)加几个用于CheckBox动画的参数：

      1、checked状态下，对钩的内部颜色（已经暂时直接设置Qt::white）
      2、具有半选态的按钮，半选态下，水平线的颜色（已经暂时直接设置Qt::white）
      3、水平线的厚度（处于所在按钮矩形的高度的百分比）
      4、复选框默认情况下边框颜色，及厚度（有defaultPen为黑色，但动画需求是灰色）

      QBrush checkboxHookBrush;
      QBrush checkboxHorizontalLineBrush;
      float lineHeight;

      ...
    */

};
}
#endif // ConfigCheckBoxParameters_H

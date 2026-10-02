/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "customSiderBarScrollArea.h"
#include "UI/base/widgetstyle.h"

CustomSiderBarScrollArea::CustomSiderBarScrollArea(QWidget *parent) : QScrollArea(parent)
{
    // QScrollArea本身主题有背景，避免干扰需要设置全透明，不使用主题的背景颜色,只有1种恰当方式, rgba(0,0,0,0)
    this->setStyleSheet("background-color: rgba(0, 0, 0, 0);");

    updateScrollBarStyle(WidgetStyle::themeColor);
}

void CustomSiderBarScrollArea::updateScrollBarStyle(int themeColor)
{
    /** setStyleBar组件设置样式技巧：通过整体设置背景、大小等参数，之后可以通过::来指定控件中其他小部件， ：可以设置点击、垂直方向等用户手动操作的状态参数;
     * width 越小，border-radius也应该越小，否则设置不生效;
     * 通过margin-left和margin-right来控制滑块大小。
     */
    // 滚动条宽度从16px缩到6px，防止遮挡侧边栏内容（侧边栏200px，内容区200px，原16px滚动条会导致右侧16px被遮挡）
    if (themeColor == 1) {
        // 深色主题：使用浅色滑块，提高对比度
        this->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{width:6px; border-radius:3px;}"
                                                 "QScrollBar::handle:vertical{background:rgba(255,255,255, 25%); width: 4px;border-radius: 2px; margin-left:1px;margin-right:1px;}"
                                                 "QScrollBar::handle:vertical:hover{background:rgba(255,255,255, 40%); width: 4px; border-radius:2px;margin-left:1px;margin-right:1px;}"
                                                 "QScrollBar::sub-line:vertical{border: none; background: none; color:none;}"
                                                 "QScrollBar::add-line:vertical{border: none; background: none; color:none;}"
                                                 "QScrollBar::sub-page:vertical{background:none;}"
                                                 "QScrollBar::add-page:vertical{background:none;}"
                                                 "QScrollBar::up-arrow:vertical{background:none;}"
                                                 "QScrollBar::down-arrow:vertical{background:none;}"
                                                 );
    } else {
        // 浅色主题：使用深色滑块
        this->verticalScrollBar()->setStyleSheet("QScrollBar:vertical{width:6px; border-radius:3px;}"
                                                 "QScrollBar::handle:vertical{background:rgba(0,0,0, 10%); width: 4px;border-radius: 2px; margin-left:1px;margin-right:1px;}"
                                                 "QScrollBar::handle:vertical:hover{background:rgba(0,0,0, 15%); width: 4px; border-radius:2px;margin-left:1px;margin-right:1px;}"
                                                 "QScrollBar::sub-line:vertical{border: none; background: none; color:none;}"
                                                 "QScrollBar::add-line:vertical{border: none; background: none; color:none;}"
                                                 "QScrollBar::sub-page:vertical{background:none;}"
                                                 "QScrollBar::add-page:vertical{background:none;}"
                                                 "QScrollBar::up-arrow:vertical{background:none;}"
                                                 "QScrollBar::down-arrow:vertical{background:none;}"
                                                 );
    }
}

CustomSiderBarScrollArea::~CustomSiderBarScrollArea()
{
}

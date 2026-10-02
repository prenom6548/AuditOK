/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef WIDGETSTYLE_H
#define WIDGETSTYLE_H


#define PC_HIS_NUM 5                  // PC模式历史记录最大条数
#define MAX_HIS_NUM 50                // 最大化（平板模式）历史记录最大条数
#define STANDARD_WINDOWW 432          // 标准汇率窗口宽度
#define STANDARD_WINDOWH 628          // 标准汇率窗口高度
#define INTEL_STANDARD_WINDOWW 400    // intel标准窗口宽度
#define INTEL_STANDARD_WINDOWH 510    // intel标准窗口高度
#define SCIENTIFIC_WINDOWW 864        // 科学窗口宽度
#define SCIENTIFIC_WINDOWH 628        // 科学窗口高度
#define INTEL_SCIENTIFIC_WINDOWW 1200 // intel科学窗口宽度
#define INTEL_SCIENTIFIC_WINDOWH 625  // intel科学窗口高度
#define ORDINARY_BUTTON_W 80          // 普通按钮宽度
#define SPECIAL_BUTTON_W 162          // 特殊按钮宽度
#define BUTTON_H 56                   // 按钮高度
#define TITLEH 38                     //标题栏高度
#define TITLE_BTN_W 30                //标题栏按钮大小
#define TITLE_BTN_H 30                //标题栏按钮大小
#define STANDARD_ICON_W 16            // 标准按钮icon宽度
#define STANDARD_ICON_H 16            // 标准按钮icon高度
#define SPECIAL_ICON_W 24             // 特殊icon宽度
#define SPECIAL_ICON_H 24             // 特殊icon高度
#define OUTPUTWIDH 270                //数据输出界面高度
#define BUTTONWIDH 320                //数据按钮界面高度

#define SHADOW 6         //阴影宽度
#define WIDGETRADIUS 3   //窗口圆角
#define BUTTONRADIUS 4   //按钮圆角
#define SHADOWALPHA 0.16 //阴影透明度

// 程序员模式
#define PROGRAM_WIN_WIDTH SCIENTIFIC_WINDOWW       // 窗口宽度
#define PROGRAM_WIN_HEIGHT SCIENTIFIC_WINDOWH      // 674 // 窗口高度
#define PROGRAM_DISPLAY_HEIGHT 160  // 235 // 显示屏高度
#define PROGRAM_BIN_HEIGHT 95       // 二进制显示屏高度
#define PROGRAM_TOOL_HEIGHT 36      // 工具栏显示屏高度
#define PROGRAM_KEYBOARY_HEIGHT 350 // 键盘高度
#define PROGRAM_CODE_WIDTH 50       // 编码显示屏宽度
#define PROGRAM_HIS_HEIGHT 60       // 历史记录显示屏高度
#define PROGRAM_BUDGET_HEIGHT 30    // 预算显示屏高度
#define PROGRAM_CUR_HEIGHT 60       // 表达式显示屏高度

#include <QDebug>
//窗口显示在屏幕中心
#include <QApplication>
#include <QScreen>
//控件
#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
//布局
#include <QBoxLayout>
//读取本地字体
#include <QFontDatabase>
//窗体阴影
#include <QPainter>


class WidgetStyle : public QWidget
{
    Q_OBJECT

public:
    WidgetStyle(QWidget *parent = nullptr) {}

    enum ThemeColor {
        LIGHT = 0, // 浅色
        DARK = 1,  // 深色
    };

    static int themeColor; // 主题颜色适配--默认浅色

private:
    QWidget *title = nullptr;           //标题栏
    QLabel *text = nullptr;             //标题
    QWidget *body = nullptr;            //窗体
    QLabel *icon = nullptr;             //图标
    QPushButton *widgetClose = nullptr; //关闭窗口
    QPushButton *widgetMin = nullptr;   //最小化窗口
};

#endif

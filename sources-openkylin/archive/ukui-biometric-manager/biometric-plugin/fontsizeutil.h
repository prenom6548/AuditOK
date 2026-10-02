/*
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef FONTSIZEUTIL_H
#define FONTSIZEUTIL_H

#include <QLabel>
#include <QApplication>

class FontSizeUtil
{
public:
    /**
     * @brief 根据系统字体设置调整标签字体大小
     * @param label 要设置字体大小的QLabel
     * @param baseFontSize 基础字体大小
     */
    static void setFontSize(QLabel *label, double baseFontSize);

private:
    /**
     * @brief 获取点转像素的转换系数
     * @return 转换系数
     */
    static double getPtToPx();
};

#endif // FONTSIZEUTIL_H


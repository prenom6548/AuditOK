/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
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

#ifndef KTABBAR_H
#define KTABBAR_H

#include <QTabBar>

namespace kdk
{

/** @defgroup bar模块
  * @{
  */

/**
 * @brief 支持三种样式
 */
enum KTabBarStyle
{
    SegmentDark,
    SegmentLight,
    Sliding
};

class KTabBarPrivate;

/**
 * @brief KTabBar,支持三种样式
 */
class KTabBar: public QTabBar
{
    Q_OBJECT

public:
    KTabBar(KTabBarStyle barStyle = SegmentLight,QWidget* parent = nullptr);
    ~KTabBar();

    void readyKTabBar();
    
    /**
     * @brief 设置TabBar样式
     * @param barStyle
     */
    void setTabBarStyle(KTabBarStyle barStyle);

    /**
     * @brief 返回TabBar样式
     * @return
     */
    KTabBarStyle barStyle();

    /**
     * @brief 设置圆角半径，只对SegmentDark，SegmentLight样式生效
     * @param radius
     */
    void setBorderRadius(int radius);

    /**
     * @brief 获取圆角半径
     * @return
     */
    int borderRadius();

protected:
    QSize sizeHint() const;
    QSize minimumTabSizeHint(int index) const;

private:
    Q_DECLARE_PRIVATE(KTabBar)
    KTabBarPrivate*const d_ptr;
};
}
/**
  * @example testtabbar/widget.h
  * @example testtabbar/widget.cpp
  * @}
  */
#endif //KTABBAR_H

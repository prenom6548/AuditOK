/*
 * libkysdk-qtwidgets's Library
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
 * Authors: Zhenyu Wang <wangzhenyu@kylinos.cn>
 *
 */

#ifndef KARROWTABBAR_H
#define KARROWTABBAR_H

#include "knativetabbar.h"
#include <QWidget>

namespace kdk
{

/** @defgroup BarModule
 * @{
 */

class KArrowTabBarPrivate;

/**
 * @brief 可显示箭头的tabbar
 * @since 2.5.2.0
 */
class KArrowTabBar : public QWidget
{
    Q_OBJECT
public:
    KArrowTabBar(QWidget *parent = nullptr);

    /**
     * @brief 获取knativetabbar
     * @return
     */
    KNativeTabbar *tabbar();

Q_SIGNALS:
    /**
     * @brief 点击会发出信号
     */
    void sizeChange();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Q_DECLARE_PRIVATE(KArrowTabBar)
    KArrowTabBarPrivate *const d_ptr;
};

}

#endif // KArrowTabBar_H

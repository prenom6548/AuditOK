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

#ifndef KNAVIGATIONTRAIL_H
#define KNAVIGATIONTRAIL_H

#include "kcrumb.h"
#include <QWidget>

namespace kdk
{

enum ButtonPosition {
    LeftPosition,
    LeftOrRight,
    RightPosition
};

/**
 * @defgroup BarModule
 * @since 2.5.2.0
 */
class KNavigationTrailPrivate;

class KNavigationTrail : public QWidget
{
    Q_OBJECT
public:
    KNavigationTrail(QWidget *parent = nullptr);

    /**
     * @brief 获取kcrumb
     * @return
     */
    KCrumb *crumb();

    /**
     * @brief 设置箭头位置
     * @param type
     */
    void setPosition(ButtonPosition type);

Q_SIGNALS:
    /**
     * @brief 点击会发出信号
     */
    void sizeChange();

protected:
    void resizeEvent(QResizeEvent *event);

private:
    Q_DECLARE_PRIVATE(KNavigationTrail)
    KNavigationTrailPrivate *const d_ptr;
};

}

#endif // KNAVIGATIONTRAIL_H

/*
 *
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
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */
#ifndef ADDBUTTON_H
#define ADDBUTTON_H

#include <QObject>
#include <QPainterPath>
#include <QPushButton>
#include <QWidget>

#include "kbaseframe.h"

/**
 * @defgroup PushButtonModule
 * @brief 添加按钮
 * @since 3.0.1.0
 * {
 */
namespace kdk
{

class KAddButtonPrivate;

class KAddButton : public QPushButton
{
    Q_OBJECT
public:
    explicit KAddButton(QWidget *parent = nullptr, KBaseFrame::BorderRadiusStyle style = KBaseFrame::BorderRadiusStyle::Bottom, bool heightAdaptive = true);
    ~KAddButton();

    /**
     * @brief 设置圆角模式
     *
     * @param style
     */
    void setRadiusType(KBaseFrame::BorderRadiusStyle style);
    /**
     * @brief 获取平板模式
     *
     * @return bool 是否是平板模式
     */
    bool getTabletMode();

    /**
     * @brief 设置高度是否自适应
     * @param flag
     */
    void setHeightAdaptive(bool flag);

protected:
    void paintEvent(QPaintEvent *event);

Q_SIGNALS:
    void tabletModeChanged(bool b);

private:
    Q_DECLARE_PRIVATE(KAddButton)
    KAddButtonPrivate *const d_ptr;
};

}

#endif // ADDBUTTON_H

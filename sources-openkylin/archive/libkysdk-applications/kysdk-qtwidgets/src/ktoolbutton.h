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
 * Authors: Zhen Sun <sunzhen1@kylinos.cn>
 *
 */

#ifndef KTOOLBUTTON_H
#define KTOOLBUTTON_H

#include "gui_g.h"
#include <QEvent>
#include <QToolButton>
#include <QWidget>

namespace kdk
{

/** @defgroup PushbuttonModule
 * @{
 */

/**
 * @brief 支持三种样式,暂不支持文字显示
 * @since 1.0.0
 */
enum KToolButtonType {
    Flat,
    SemiFlat,
    Background,
    Frameless
};

class KToolButtonPrivate;

/**
 * @brief 提供了一个快速访问按钮，暂不支持显示文本
 */
class GUI_EXPORT KToolButton : public QToolButton
{
    Q_OBJECT

public:
    KToolButton(QWidget *parent);

    /**
     * @brief 返回类型
     * @return
     */
    KToolButtonType type();

    /**
     * @brief 设置类型
     * @param type
     */
    void setType(KToolButtonType type);

    /**
     * @brief 设置Icon
     * @param icon
     */
    void setIcon(const QIcon &icon);

    /**
     * @brief 设置正在加载状态,仅不带箭头的toolbuttuon支持该状态
     * @param flag
     */
    void setLoading(bool flag);

    /**
     * @brief 返回是否正在加载
     * @return
     */
    bool isLoading();

    /**
     * @brief 获取Icon
     * @return
     */
    QIcon icon();

    /**
     * @brief 设置是否显示向下箭头，默认不显示
     * @param flag
     */
    void setArrow(bool flag);

    /**
     * @brief 返回是否显示箭头
     * @return
     */
    bool hasArrow() const;

    /**
     * @brief 设置图标颜色
     * @param flag
     * @param color
     * @since 2.4.1.0
     */
    void setIconColor(bool flag, QColor color);

    /**
     * @brief setBorderRadius
     * @param bottomLeft
     * @param topLeft
     * @param topRight
     * @param bottomRight
     * @since 2.5.2.0
     */
    void setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight);

    /**
     * @brief setBackgroundColorEnabled
     * @param flag
     * @since 2.5.2.0
     */
    void setBackgroundColorEnabled(bool flag);

    /**
     * @brief backgroundColorEnabled
     * @return
     * @since 2.5.2.0
     */
    bool backgroundColorEnabled();

Q_SIGNALS:
    void checkedChanged(bool checked);

protected:
    bool eventFilter(QObject *watched, QEvent *event);
    QSize sizeHint() const;
    void paintEvent(QPaintEvent *event);

private:
    Q_DECLARE_PRIVATE(KToolButton)
    KToolButtonPrivate *const d_ptr;
};
}
/**
 * @example testtoolbutton/widget.h
 * @example testtoolbutton/widget.cpp
 * @}
 */
#endif // KTOOLBUTTON_H

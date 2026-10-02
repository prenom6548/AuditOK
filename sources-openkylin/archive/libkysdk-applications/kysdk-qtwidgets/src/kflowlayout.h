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
#ifndef KFLOWLAYOUT_H
#define KFLOWLAYOUT_H

#include <QLayout>
#include <QRect>
#include <QWidget>

namespace kdk
{

/**
 * @defgroup LayoutModule
 * @brief 存在水平间距、垂直间距的布局
 * @since 3.0.1.0
 * {
 */
class KFlowLayoutPrivate;

class KFlowLayout : public QLayout
{
    Q_OBJECT
public:
    /**
     * @brief 创建一个父对象为parent，边距为margin，水平间距为hSpacing，垂直间距为vSpacing的布局。
     *
     * @param home 是否首页布局
     * @param margin 边距
     * @param hSpacing 水平间距
     * @param vSpacing 垂直间距
     * @param parent
     */
    explicit KFlowLayout(QWidget *parent, bool home = false, int margin = -1, int hSpacing = -1, int vSpacing = -1);

    /**
     * @brief 创建一个边距为margin，水平间距为hSpacing，垂直间距为vSpacing的布局。
     *
     * @param margin 边距
     * @param hSpacing 水平间距
     * @param vSpacing 垂直间距
     */
    explicit KFlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);

    ~KFlowLayout();

public:
    /**
     * @brief 水平间距
     *
     * @param 无
     */
    int horizontalSpacing() const;

    /**
     * @brief 垂直间距
     *
     * @param 无
     */
    int verticalSpacing() const;

protected:
    void addItem(QLayoutItem *item) Q_DECL_OVERRIDE;

    Qt::Orientations expandingDirections() const Q_DECL_OVERRIDE;

    bool hasHeightForWidth() const Q_DECL_OVERRIDE;

    int heightForWidth(int) const Q_DECL_OVERRIDE;

    int count() const Q_DECL_OVERRIDE;

    QLayoutItem *itemAt(int index) const Q_DECL_OVERRIDE;

    QSize minimumSize() const Q_DECL_OVERRIDE;

    void setGeometry(const QRect &rect) Q_DECL_OVERRIDE;

    QSize sizeHint() const Q_DECL_OVERRIDE;

    QLayoutItem *takeAt(int index) Q_DECL_OVERRIDE;

private:
    Q_DECLARE_PRIVATE(KFlowLayout)
    KFlowLayoutPrivate *const d_ptr;
};
}

#endif // KFLOWLAYOUT_H

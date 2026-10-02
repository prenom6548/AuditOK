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

#ifndef KCRUMB_H
#define KCRUMB_H

#include "kcustomtabbar.h"
#include <QTabBar>

namespace kdk
{
/**
 *  @defgroup KCrumb
 *  @since 2.5.2.0
 */
class KCrumbPrivate;

/**
 * @brief KCrumbType，包括两种类型。
 */
enum KCrumbType {
    FlatBreadCrumb,
    CubeBreadCrumb
};

/**
 * @brief 面包屑控件
 */
class KCrumb : public KCustomTabBar
{
    Q_OBJECT

public:
    explicit KCrumb(QWidget *parent = nullptr);

    /**
     * @brief 添加tab
     * @param text
     * @return
     */
    int addTab(const QString &text);

    /**
     * @brief 添加tab
     * @param icon
     * @param text
     * @return
     */
    int addTab(const QIcon &icon, const QString &text);

    /**
     * @brief 在index位置插入tab
     * @param index
     * @param text
     * @return
     */
    int insertTab(int index, const QString &text);

    /**
     * @brief 在index位置插入tab
     * @param index
     * @param icon
     * @param text
     * @return
     */
    int insertTab(int index, const QIcon &icon, const QString &text);

    /**
     * @brief 删除index位置的tab
     * @param index
     */
    void removeTab(int index);

    /**
     * @brief 将from位置tab移至to位置
     * @param from
     * @param to
     */
    void moveTab(int from, int to);

    /**
     * @brief 设置图标
     * @param icon
     */
    void setIcon(const QIcon &icon);

    /**
     * @brief 获取图标
     * @return
     */
    QIcon icon() const;

    /**
     * @brief 返回按钮边框是否凸起
     * @return
     */
    bool isFlat() const;

    /**
     * @brief 设置按钮边框是否凸起
     * @param flat
     */
    void setFlat(bool flat);

public slots:
    void leftScroll();
    void rightScroll();

protected:
    QSize tabSizeHint(int index) const;
    void paintEvent(QPaintEvent *event);

private:
    Q_DECLARE_PRIVATE(KCrumb)
    KCrumbPrivate *const d_ptr;
};
}

#endif // KCRUMB_H

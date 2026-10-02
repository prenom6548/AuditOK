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

#ifndef KTABLEVIEW_H
#define KTABLEVIEW_H

#include "ktableheaderview.h"
#include <QTableView>

namespace kdk
{

/**
 * @defgroup ContainerModules
 * @brief 继承qtbaleview
 * @since 2.5.2.0
 * {
 */
class KTableViewPrivate;

class KTableView : public QTableView
{
    Q_OBJECT
public:
    KTableView(QWidget *parent = nullptr);

    /**
     * @brief 设置水平标题
     * @param list
     */
    void setHorizontalTitle(QStringList list);

    /**
     * @brief 返回水平标题栏
     * @return
     */
    KTableHeaderView *headerView();

    /**
     * @brief 返回所有选中item的行索引
     * @return
     */
    QList<int> selectList();

Q_SIGNALS:
    void hoverIndexChanged(QModelIndex index);

protected:
    void mouseMoveEvent(QMouseEvent *event);
    bool eventFilter(QObject *object, QEvent *event);

private:
    Q_DECLARE_PRIVATE(KTableView)
    KTableViewPrivate *const d_ptr;
};
}
#endif // KTABLEVIEW_H

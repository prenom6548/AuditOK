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

#ifndef KTABLEHEADVIEW_H
#define KTABLEHEADVIEW_H

#include <QCheckBox>
#include <QHeaderView>
#include <QMap>
#include <QObject>

namespace kdk
{

/**
 * @brief 继承自QHeaderView，设置view视图标题
 * @since 2.5.2.0
 */
class KTableHeaderViewPrivate;

class KTableHeaderView : public QHeaderView
{
    Q_OBJECT
public:
    /**
     * @brief KTableHeaderView 构造函数
     * @param orientation 方向
     * @param parent 父类对象
     */
    KTableHeaderView(Qt::Orientation orientation, QWidget *parent = nullptr);

    /**
     * @brief 设置水平标题
     * @param list
     */
    void setTitle(QStringList list);

    /**
     * @brief 设置menu
     * @param menu
     * @param column
     */
    void setMenu(QMenu *menu, int column);

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
    void mousePressEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    bool event(QEvent *e);

Q_SIGNALS:
    void checkStateChange(int state);
    void clickedHeader(int column);

public slots:
    void checkStateChangeSlot(int iState);

private:
    Q_DECLARE_PRIVATE(KTableHeaderView)
    KTableHeaderViewPrivate *const d_ptr;
};

}
#endif // KTABLEHEADVIEW_H

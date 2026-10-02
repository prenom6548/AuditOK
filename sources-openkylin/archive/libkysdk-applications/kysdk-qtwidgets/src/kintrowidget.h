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

#ifndef KINTROWIDGET_H
#define KINTROWIDGET_H

#include "gui_g.h"
#include <QWidget>

namespace kdk
{

enum ViewType {
    IconView,
    ListView
};

/**
 * @defgroup FrameModule
 * @since 2.5.2.0
 */
class KIntroWidgetPrivate;

class GUI_EXPORT KIntroWidget : public QWidget
{
    Q_OBJECT
public:
    KIntroWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置视图样式
     * @param type
     */
    void setViewType(ViewType type);

    /**
     * @brief 设置主文本
     * @param str
     */
    void setMainLabel(QString str);

    /**
     * @brief 设置次级文本
     * @param str
     */
    void setSubLabel(QString str);

    /**
     * @brief 设置圆角
     * @param radius
     */
    void setBorderRadius(int radius);

    /**
     * @brief 设置圆角
     * @param bottomLeft
     * @param topLeft
     * @param topRight
     * @param bottomRight
     */
    void setBorderRadius(int bottomLeft, int topLeft, int topRight, int bottomRight);

    /**
     * @brief 设置标题颜色
     * @param color
     */
    void setTitleColor(QColor color);

Q_SIGNALS:
    void rightClick(QPoint point);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    QSize sizeHint() const override;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    Q_DECLARE_PRIVATE(KIntroWidget)
    KIntroWidgetPrivate *const d_ptr;
};

}

#endif // KINTROWIDGET_H

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

#ifndef KFILEWIDGET_H
#define KFILEWIDGET_H

#include "gui_g.h"
#include <QWidget>

namespace kdk
{

/** @defgroup FrameModule
 * {
 */

class KFileWidgetPrivate;

/**
 * @brief The KFileWidget class
 * @since 2.4.1.0
 */
class GUI_EXPORT KFileWidget : public QWidget
{
    Q_OBJECT
public:
    KFileWidget(QWidget *parent = nullptr);

public:
    /** @brief 设置主要文本
     *  @param str
     */
    void setMainText(QString str);

    /**
     * @brief 设置次级文本
     * @param str
     */
    void setSubText(QString str);

    /**
     * @brief 设置图片
     * @param icon
     */
    void setIcon(QIcon icon);

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

Q_SIGNALS:
    void clicked();

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Q_DECLARE_PRIVATE(KFileWidget)
    KFileWidgetPrivate *const d_ptr;
};

}
#endif // KFILEWIDGET_H

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

#ifndef KTITLEWINDOW_H
#define KTITLEWINDOW_H

#include "gui_g.h"
#include "kpushbutton.h"
#include <QPushButton>
#include <QStyleOptionButton>
#include <QTextStream>
#include <QWidget>
#include <QStyleOptionButton>

namespace kdk
{

/**
 * @defgroup FrameModule
 * @since 2.5.2.0
 * {
 */

class KTitleWindowPrivate;

class GUI_EXPORT KTitleWindow : public QWidget
{
    Q_OBJECT

public:
    KTitleWindow(QWidget *parent = nullptr);

    /**
     * @brief 获取添加按钮
     * @return
     */
    KPushButton *addButton();

    /**
     * @brief 获取颜色按钮
     * @return
     */
    QPushButton *colorButton();

    /**
     * @brief 获取menu按钮
     * @return
     */
    QPushButton *menuButton();

    /**
     * @brief 获取widget
     * @return
     */
    QWidget *baseWidget();

    /**
     * @brief 设置标题颜色
     * @param color
     */
    void setTitleColor(QColor color);

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Q_DECLARE_PRIVATE(KTitleWindow)
    KTitleWindowPrivate *const d_ptr;
};

}

#endif // KTITLEWINDOW_H

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

#ifndef KLOOPPLAYPAGE_H
#define KLOOPPLAYPAGE_H

#include "gui_g.h"
#include <QWidget>

namespace kdk
{

class KLoopPlayPagePrivate;

/**
 * @defgroup FrameModule
 * @brief 轮播图片控件
 * @since 2.5.2.0
 * {
 */

class GUI_EXPORT KLoopPlayPage : public QWidget
{
    Q_OBJECT
public:
    KLoopPlayPage(QWidget *parent = nullptr);

    /**
     * @brief 添加widget
     * @param widget
     */
    void addWidget(QWidget *widget);

    /**
     * @brief 删除widget
     * @param widget
     */
    void removeWidget(QWidget *widget);

    /**
     * @brief 设置当前中心界面
     * @param i
     */
    void setCurrentWidget(int i);

    /**
     * @brief 获取当前中心界面index
     * @return
     */
    int currentWidget();

    /**
     * @brief 设置窗体间距
     * @param spacing
     */
    void setWidgetSpacing(int spacing);

    /**
     * @brief 设置轮播时长
     * @param time
     */
    void setLoopTime(int time);

Q_SIGNALS:
    void leftClick();
    void centerClick();
    void rightClick();

protected:
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent *event) override;

private:
    KLoopPlayPagePrivate *const d_ptr;
    Q_DECLARE_PRIVATE(KLoopPlayPage)
};

}

#endif // KLOOPPLAYPAGE_H

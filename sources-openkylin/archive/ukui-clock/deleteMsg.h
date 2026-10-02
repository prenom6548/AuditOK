/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef DELETE_MSG_H
#define DELETE_MSG_H

#include <QDialog>
#include <QStyleOption>
#include <QPainter>
#include <QMouseEvent>
#include <QPainterPath>
#include "constant_class.h"
namespace Ui {
class delete_msg;
}
/**
 * @brief 删除消息弹框
 */
class delete_msg : public QDialog
{
    Q_OBJECT

public:
    explicit delete_msg(QWidget *parent = nullptr);
    ~delete_msg();
    friend class TestClass;  // 核心修改：声明测试类为友元

    int close_sure = 0;
    //绘制底部阴影
    // Draw bottom shadow
    void paintEvent(QPaintEvent *event);
    Ui::delete_msg *ui;
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

private slots:
    void on_closebtn_clicked();

    void on_surebtn_clicked();

    void on_cancelbtn_clicked();

private:
    QPoint dragPosition;                                            //拖动坐标
    bool mousePressed = false;                                              //鼠标是否按下
};

#endif // DELETE_MSG_H

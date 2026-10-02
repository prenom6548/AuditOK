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
#ifndef VERTICALSCROLL_99_H
#define VERTICALSCROLL_99_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QPainter>
#include <QTimer>
#include <QStyleOption>
#include <QLineEdit>
#include "baseverticalscroll.h"

namespace Ui {

class VerticalScroll_99;

}

class VerticalScroll_99 : public BaseVerticalScroll

{

    Q_OBJECT

    Q_PROPERTY(int deviation READ readDeviation WRITE setDeviation )

public:

    explicit VerticalScroll_99(QWidget *parent = 0);

    ~VerticalScroll_99();
    //设置范围
    // set range
    void setRange(int min, int max);
    //获取当前值
    // Get current value
    int readValue();

    QLineEdit *timeEdit_99;
    void timeEditStyle_99();

    void setupUi(QWidget *VerticalScroll_99);

    void retranslateUi(QWidget *VerticalScroll_99);

private slots:
    void  listClickslot();


protected:
    void mousePressEvent(QMouseEvent *) override;


    void mouseReleaseEvent(QMouseEvent *) override;


    void paintEvent(QPaintEvent *) override;
    void enterEvent ( QEvent * event ) override;
    void leaveEvent ( QEvent * event ) override;
    //描绘数字
    //使选中的数字回到屏幕中间
    //Bring the selected number back to the middle of the screen
    //鼠标移动偏移量，默认为0
    // Mouse movement offset, default is 0
    int readDeviation();
    //设置偏移量
    // Set offset
    void setDeviation(int n);


signals:

    void currentValueChanged(int value);

    void deviationChange(int deviation);

private:

    Ui::VerticalScroll_99 *ui;
    QTimer *timer_21111;

private:
};

#endif // VERTICALSCROLL_99_H

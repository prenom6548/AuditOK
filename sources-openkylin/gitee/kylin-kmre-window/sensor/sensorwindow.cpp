/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sensorwindow.h"
#include <QDebug>

void SensorWindow::pullUpWindow(const QString &msg)
{
    Q_UNUSED(msg)
    this->hide();
    this->show();
    showNormal();
}

SensorWindow::SensorWindow(QWidget *parent) : QMainWindow(parent)
{
    setFixedSize(400,400);
    setWindowTitle(tr("重力传感器"));
    upBtn = new DirectBtn(this);
    upBtn->num->setText("1");
    upBtn ->move(170,0);
    upBtn->setText("上");

    lowBtn = new DirectBtn(this);
    lowBtn->num->setText("2");
    lowBtn ->move(170,300);
    lowBtn->setText("下");

    leftBtn = new DirectBtn(this);
    leftBtn->num->setText("3");
    leftBtn ->move(10,150);
    leftBtn->setText("左");

    rightBtn = new DirectBtn(this);
    rightBtn->num->setText("4");
    rightBtn ->move(300,150);
    rightBtn->setText("右");

    leftupBtn = new DirectBtn(this);
    leftupBtn->num->setText("5");
    leftupBtn ->move(10,0);
    leftupBtn->setText("左上");

    leftlowBtn = new DirectBtn(this);
    leftlowBtn->num->setText("6");
    leftlowBtn ->move(10,300);
    leftlowBtn->setText("左下");

    rightupBtn = new DirectBtn(this);
    rightupBtn->num->setText("7");
    rightupBtn ->move(300,0);
    rightupBtn->setText("右上");

    rightlowBtn = new DirectBtn(this);
    rightlowBtn->num->setText("8");
    rightlowBtn ->move(300,300);
    rightlowBtn->setText("右下");

    pauseBtn = new DirectBtn(this);
    pauseBtn->num->setText("9");
    pauseBtn ->move(170,150);
    pauseBtn->setText("暂停");



}

SensorWindow::~SensorWindow()
{

}



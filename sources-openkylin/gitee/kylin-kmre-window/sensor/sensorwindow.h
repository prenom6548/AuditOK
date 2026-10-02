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

#ifndef SENSORWINDOW_H
#define SENSORWINDOW_H
#include <QWidget>
#include <QMainWindow>
#include <directbtn.h>

class SensorWindow : public QMainWindow
{
    Q_OBJECT

public:
    ~SensorWindow();
    SensorWindow(QWidget *parent = nullptr);
    void pullUpWindow(const QString &msg);
protected:

private:    
    DirectBtn *upBtn = nullptr;
    DirectBtn *lowBtn = nullptr;
    DirectBtn *leftBtn = nullptr;
    DirectBtn *rightBtn = nullptr;
    DirectBtn *leftupBtn = nullptr;
    DirectBtn *leftlowBtn = nullptr;
    DirectBtn *rightupBtn = nullptr;
    DirectBtn *rightlowBtn = nullptr;
    DirectBtn *pauseBtn = nullptr;

public Q_SLOTS:

};

#endif // SENSORWINDOW_H

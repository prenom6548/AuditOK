/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 *
 * Authors: wangyan <wangyan@kylinos.cn>
 *
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include "common.h"
#include "ui_calibration.h"

namespace Ui {
class TouchScreen;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget() override;

    void resetTouchList(const QList<Device>& devices);
    void resetOutputList(const QList<Device>& devices);
    Device currentTouchDevice() const;
    Device currentOutputDevice() const;

signals:
    void currentOutputChanged(Device device);
    void currentTouchChanged(Device device);
    void mapButtonClicked();
    void calibratButtonClicked();

private:
    void initConnection();
    void initUi();

    Ui::Calibration* ui;
    QList<Device> m_touchDevices;
    QList<Device> m_outputDevices;
};

#endif // WIDGET_H

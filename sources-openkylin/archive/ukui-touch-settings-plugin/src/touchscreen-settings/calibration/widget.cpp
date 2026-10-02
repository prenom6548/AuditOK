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

#include "widget.h"

#include <QDebug>
#include <QPushButton>
#include <QComboBox>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calibration())
{
    ui->setupUi(this);

    initConnection();
    initUi();
}

void Widget::initUi()
{
    ui->screenFrame->show();

    if (env::isTabletMode()){
        qDebug()<<"working in tablet mode";
        ui->screenFrame->setFixedHeight(64);
        ui->monitorCombo->setFixedHeight(48);
        ui->touchscreenFrame->setFixedHeight(64);
        ui->touchscreenCombo->setFixedHeight(48);
        ui->deviceinfoFrame->setFixedHeight(64);
    }

    ui->touchscreenFrame->show();
    ui->deviceinfoFrame->show();
    ui->mapButton->show();
    ui->CalibrationButton->hide();
}

Widget::~Widget() {
    delete ui;
    ui = nullptr;
}

void Widget::initConnection() {
    connect(ui->monitorCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this]() {
        emit currentOutputChanged(currentOutputDevice());
    });

    connect(ui->touchscreenCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [this]() {
        Device device = currentTouchDevice();
        ui->touchnameContent->setText(device.name);
        emit currentTouchChanged(device);
    });

    connect(ui->mapButton, &QPushButton::clicked, this, &Widget::mapButtonClicked);
    connect(ui->CalibrationButton, &QPushButton::clicked, this, &Widget::calibratButtonClicked);
}

Device Widget::currentTouchDevice() const
{
    QString currentTouchScreen = ui->touchscreenCombo->currentText();
    if (currentTouchScreen.isEmpty()) {
        qDebug() << "Current touchscreenCombo text is empty.";
        return Device();
    }

    int deviceId = currentTouchScreen.toInt();
    for (const Device& device : qAsConst(m_touchDevices)) {
        if (deviceId == device.deviceId)
            return device;
    }
    qWarning() << "Can't find touch device:" << deviceId << "from device list";
    return Device();
}

Device Widget::currentOutputDevice() const
{
    QString deviceName = ui->monitorCombo->currentText();
    for (const Device& device : qAsConst(m_outputDevices)) {
        if (deviceName == device.name)
            return device;
    }
    qWarning() << "Can't find output device:" << deviceName << "from device list";
    return Device();
}

void Widget::resetTouchList(const QList<Device>& devices)
{
    qDebug() << "TouchCalibrate widget reset touch list";
    m_touchDevices = devices;
    ui->touchscreenCombo->clear();

    for (const Device& device : qAsConst(devices)) {
        ui->touchscreenCombo->addItem(QString::number(device.deviceId));
    }
}

void Widget::resetOutputList(const QList<Device>& devices)
{
    qDebug() << "TouchCalibrate widget reset output list";
    m_outputDevices = devices;
    ui->monitorCombo->clear();

    for (const Device& device : qAsConst(devices)) {
        ui->monitorCombo->addItem(device.name);
    }
}

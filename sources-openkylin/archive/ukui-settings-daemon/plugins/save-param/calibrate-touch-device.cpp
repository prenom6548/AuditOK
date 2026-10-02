/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2023 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <QProcess>
#include <QSettings>
#include <QDir>

#include "calibrate-touch-device.h"
#include "touch-calibrate.h"
#include "clib-syslog.h"
#include <QTimer>
#define MAP_CONFIG "touchcfg.ini"

CalibrateTouchDevice::CalibrateTouchDevice(QObject *parent)
{

}

CalibrateTouchDevice::~CalibrateTouchDevice()
{

}

void CalibrateTouchDevice::setUserName(QString userName)
{
    m_userName = userName;
}

void CalibrateTouchDevice::calibration()
{

    TouchCalibrate  calibrate;
    calibrate.calibrate();
    Q_EMIT calibrateFinished();

}

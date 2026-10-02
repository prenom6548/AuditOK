/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DEVPOWERCONTROL_H
#define DEVPOWERCONTROL_H

#include <QObject>
#include <map>
#include "devfreqpolicyrpm.h"
#include <memory>
#include "pcirpm.h"
#include "i2crpm.h"
#include "usbrpm.h"
#include "satalinkrpm.h"
#include "audiorpm.h"
#include "pcieaspm.h"
#include "keyboardbacklight.h"
#include "touchscreen.h"
#include "netwolrpm.h"
#include "wakesourcerpm.h"
#include "devrpminterface.h"

#include "../upmcustomhardwareconfig.h"

class DevPowerControl : public QObject
{
    Q_OBJECT
public:
    explicit DevPowerControl(QObject *parent = nullptr);
    ~DevPowerControl();

    void setAllDevicePerformance();
    void setAllDeviceBalance();
    void setAllDeviceSave();

    void SetDevicePowerCtl(int mode, QString scene);

private:
    void setAllDevice(QString mode);
    void InitRpmDeviceMap();

    int m_powerPolicy;
    std::map<QString, std::unique_ptr<DevRpmInterface>> m_rpmDeviceMap;
};

#endif // DEVPOWERCONTROL_H

/*
 * Copyright 2021 KylinSoft Co., Ltd.
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

#ifndef UPOWERDBUS_H
#define UPOWERDBUS_H
#include "upowerinfo/upowerinfo.h"

#include <dbus.h>

class UpowerDBus : public DBus
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface","org.ukui.upower")
public:
    UpowerDBus();

    ~UpowerDBus();

public slots:
    /**
     * @brief UPowerVersion
     * ukui-upower版本
     * @return
     */
    QString UPowerVersion(void);

    /**
     * @brief CanHibernate
     * @return
     * 是否可以休眠的接口
     * 或可直接调用login1 的
     * 重新封装的意义在于可以针对部分机型做特殊处理
     * 并且可以
     */
    QString CanHibernate(void);

    /**
     * @brief MachineType
     * @return
     * 机器类型判断
     */
    QString MachineType();

    /**
     * @brief OnBattery
     * AC状态还是DC状态
     * @return
     */
    bool OnBattery();

    /**
     * @brief LidIsClosed
     * 当前合盖状态
     * @return
     */
    bool LidIsClosed();

    /**
     * @brief DaemonVersion
     * 判断upower版本
     * @return
     */
    QString DaemonVersion();

    /**
     * @brief CanAdjustBacklight
     * 能否调节背光
     * @return
     */
    bool CanAdjustBacklight();

signals:
    /**
     * @brief PowerState
     */
    void PowerState(bool);

    /**
     * @brief LidState
     */
    void LidState(bool);

private:
    UPowerInfo *upowerinfo;

    void dealPowerState(bool);

    void dealLidState(bool);

};

#endif // UPOWERDBUS_H

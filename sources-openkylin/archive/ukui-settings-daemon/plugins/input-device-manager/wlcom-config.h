/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: sundagao <sundagao@kylinos.cn>
 */

#ifndef WLCOMDEVICE_H
#define WLCOMDEVICE_H

#include <QObject>
#include <QDBusArgument>

/**wl type**/
#define WLR_INPUT_DEVICE_KEYBOARD 0
#define WLR_INPUT_DEVICE_POINTER 1
#define WLR_INPUT_DEVICE_TOUCH 2
#define WLR_INPUT_DEVICE_TABLET_TOOL 3
#define WLR_INPUT_DEVICE_TABLET_PAD 4
#define WLR_INPUT_DEVICE_SWITCH 5

struct WlcomDevice {
    QString name;
    union {
        struct {
          uint32_t tap_finger_count : 3;
          uint32_t click_methods : 3;
          uint32_t scroll_methods : 3;
          uint32_t accel_profiles : 3;
          uint32_t send_events_modes : 3;
          uint32_t has_rotation : 1;
          uint32_t has_dwtp : 1;
          uint32_t has_dwt : 1;
          uint32_t has_middle_emulation : 1;
          uint32_t has_left_hand : 1;
          uint32_t has_natual_scroll : 1;
          uint32_t has_calibration_matrix : 1;
          uint32_t has_pointer_accel : 1;
          uint32_t support_mapped_to_output : 1;
          uint32_t : 8;
        };
        uint32_t prop;
    };

    friend QDBusArgument &operator<<(QDBusArgument &argument, const WlcomDevice &device);
    friend const QDBusArgument &operator>>(const QDBusArgument& argument, WlcomDevice& device);
};

Q_DECLARE_METATYPE(WlcomDevice)

QDBusArgument &operator<<(QDBusArgument &argument, const WlcomDevice &device)
{
    argument.beginStructure();
    argument << device.name << device.prop;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument& argument, WlcomDevice& device)
{
    argument.beginStructure();
    argument >> device.name >> device.prop;
    argument.endStructure();
    return argument;
}

struct WlcomSeat {
    QString name;
    QString config;
    friend QDBusArgument &operator<<(QDBusArgument &argument, const WlcomSeat &wlSeat);
    friend const QDBusArgument &operator>>(const QDBusArgument& argument, WlcomSeat& wlSeat);
};
Q_DECLARE_METATYPE(WlcomSeat)

QDBusArgument &operator<<(QDBusArgument &argument, const WlcomSeat &wlSeat)
{
    argument.beginStructure();
    argument << wlSeat.name << wlSeat.config;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument& argument, WlcomSeat& wlSeat)
{
    argument.beginStructure();
    argument >> wlSeat.name >> wlSeat.config;
    argument.endStructure();
    return argument;
}


#endif // WLCOMDEVICE_H

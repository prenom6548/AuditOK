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
 */

#ifndef INPUTEVENT_H
#define INPUTEVENT_H

#include <libinput.h>

#define bit(x_) (1UL << (x_))

typedef libinput_event_type InputEventType;
typedef libinput_switch SwitchEventType;
typedef libinput_switch_state SwitchEventState;

/*!
 * \brief libinput event
 */
struct SwipeGestureEvent{
    bool        cancelled;
    int         time;
    int         fingerCount;
    double      deltaX;
    double      deltaY;
    double      unacceleratedDeltaX;
    double      unacceleratedDeltaY;
};

struct PinchGestureEvent{
    bool        cancelled;
    int         time;
    int         fingerCount;
    double      deltaX;
    double      deltaY;
    double      unacceleratedDeltaX;
    double      unacceleratedDeltaY;
    double      scale;
    double      angleDelta;
};

struct TapGestureEvent{
    int         time;
    int         fingerCount;
    double      deltaX;
    double      deltaY;
    bool        cancelled;
};

struct SwitchToggleEvent
{
//    SwitchToggleEvent() {}
    libinput_switch switchType;
    libinput_switch_state switchState;
};

enum DeviceCapability {
	DEVICE_POINTER		= bit(0),
	DEVICE_KEYBOARD		= bit(1),
	DEVICE_TOUCH		= bit(2),
	DEVICE_TABLET		= bit(3),
	DEVICE_TABLET_PAD	= bit(4),
	DEVICE_GESTURE		= bit(5),
	DEVICE_SWITCH		= bit(6),
};

/*!
 *  \brief input device
 */
struct Device {
    char        name[256];
};

/*!
 *  \brief EventData
 */
union EventData{
    SwipeGestureEvent       swipeEventData;
    PinchGestureEvent       pinchEventData;
    TapGestureEvent         tapEventData;
    SwitchToggleEvent       switchEventDate;
};

/*!
 * \brief Event
 */
struct Event{
    InputEventType          type;
    Device                  device;
    EventData               event;
};
#endif // INPUTEVENT_H

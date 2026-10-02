/*
 * Copyright (C) 2016 Simon Fels <morphis@gravedo.de>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wswitch-default"
#include "kmreplatform.h"
#include "mousekey/input/device.h"
#include "mousekey/input/manager.h"
#include "eventdata.h"
#include <signal.h>
#include <sys/types.h>
//#pragma GCC diagnostic pop
#include <iostream>
#include <syslog.h>
#include <QDebug>
#include <QThread>

using namespace std;

KmrePlatform::KmrePlatform(
    const std::shared_ptr<kmre::input::Manager> &input_manager,
    const uint32_t width, const uint32_t height, QObject *parent)
    : QObject(parent),
     input_manager_(input_manager),
     mutex(QMutex::Recursive) {
    screen_ = input_manager->create_device();
    screen_->set_name("kmre_touchscreen");
    screen_->set_driver_version(1);
    screen_->set_input_id({BUS_VIRTUAL, 1, 1, 1});
    screen_->set_physical_location("none");
    screen_->set_prop_bit(INPUT_PROP_DIRECT);
    screen_->set_abs_bit(ABS_MT_SLOT);
    screen_->set_abs_bit(ABS_MT_TOUCH_MAJOR);
    screen_->set_abs_bit(ABS_MT_POSITION_X);
    screen_->set_abs_bit(ABS_MT_POSITION_Y);
    screen_->set_abs_bit(ABS_MT_TRACKING_ID);
    screen_->set_abs_bit(ABS_MT_PRESSURE);
    screen_->set_abs_min(ABS_MT_POSITION_X, 0);
    screen_->set_abs_max(ABS_MT_POSITION_X, width - 1);
    screen_->set_abs_min(ABS_MT_POSITION_Y, 0);
    screen_->set_abs_max(ABS_MT_POSITION_Y, height - 1);
    screen_->set_abs_min(ABS_MT_PRESSURE, 0);
    screen_->set_abs_max(ABS_MT_PRESSURE, 100);
    screen_->set_abs_max(ABS_MT_SLOT, 9);
    screen_->set_abs_max(ABS_MT_TOUCH_MAJOR, 255);
    screen_->set_abs_max(ABS_MT_TRACKING_ID, 65535);

    mouse_ = input_manager->create_device();
    mouse_->set_name("kmre_mouse");
    mouse_->set_driver_version(1);
    mouse_->set_input_id({BUS_VIRTUAL, 1, 2, 1});
    mouse_->set_physical_location("none");
    mouse_->set_key_bit(BTN_MOUSE);
    mouse_->set_key_bit(BTN_LEFT);
    mouse_->set_key_bit(BTN_RIGHT);
    mouse_->set_key_bit(BTN_MIDDLE);
    mouse_->set_rel_bit(REL_WHEEL);
    mouse_->set_rel_bit(REL_X);
    mouse_->set_rel_bit(REL_Y);
    mouse_->set_abs_bit(ABS_X);
    mouse_->set_abs_bit(ABS_Y);

    keyboard_ = input_manager->create_device();
    keyboard_->set_name("kmre_keyboard");
    keyboard_->set_driver_version(1);
    keyboard_->set_input_id({BUS_VIRTUAL, 1, 3, 1});
    keyboard_->set_physical_location("none");
    keyboard_->set_key_bit(BTN_MISC);
    keyboard_->set_key_bit(KEY_Q);
    keyboard_->set_key_bit(KEY_OK);

}

KmrePlatform::~KmrePlatform()
{

}

void KmrePlatform::onSendEventData(QList<EventData> events, bool isTouch, int winId, int eventType)
{
    //syslog(LOG_DEBUG, "[%s] eventType = %d, isTouch = %d, event size = %d", 
    //    __func__, eventType, isTouch, events.size());
    Q_UNUSED(winId);
    mutex.lock();

    if (eventType == CONTROL_MOUSE_EVENT) {
        if (isTouch && events.size() > 0 ) {
            screen_->send_events(events);
        } else if (!isTouch && events.size() > 0 ) {
            mouse_->send_events(events);
        }
    } else if (eventType == CONTROL_KEYBOARD_EVENT) {
        if (events.size() > 0) {
            keyboard_->send_events(events);
        }
    }

    mutex.unlock();
}


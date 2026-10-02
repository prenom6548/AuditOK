/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "devicemonitoreventfilter.h"
#include "geeventmemmover.h"

#include <memory>

#include <QByteArray>
#include <QTimer>
#include <QDebug>

// some lib of x11 need include after qt, because of some definitions
#include <X11/extensions/XInput2.h>
#include <xcb/xcb.h>
#include <xcb/xinput.h>

struct FreeDeleter {
    void operator()(void* p) const Q_DECL_NOTHROW { return std::free(p); }
};
#define XCB_REPLY(call, ...) \
    std::unique_ptr<call##_reply_t, FreeDeleter>( \
        call##_reply(XCB_REPLY_CONNECTION_ARG(__VA_ARGS__), call(__VA_ARGS__), nullptr) \
    )
#define XCB_REPLY_CONNECTION_ARG(connection, ...) connection

namespace {
bool existTouchDevice(xcb_input_touch_mode_t type);
bool hasTouchDeviceInQueryReply(xcb_input_xi_query_device_reply_t* queryReply , xcb_input_touch_mode_t type);
bool findTouchModeFromDevice(xcb_input_xi_device_info_t* deviceInfo, xcb_input_touch_mode_t type);
}

DeviceMonitorEventFilter::DeviceMonitorEventFilter(QObject* parent)
    : QObject(parent), QAbstractNativeEventFilter(), m_timer(new QTimer(this))
{
    m_timer->setSingleShot(true);
}

bool DeviceMonitorEventFilter::nativeEventFilter(const QByteArray &eventType,
                                                 void* message, long* result)
{
    //! \todo support wayland
    Q_UNUSED(result)
    if (eventType != "xcb_generic_event_t") {
        qWarning() << __FILE__ << __FUNCTION__ << __LINE__
                   << "Event type is:" << eventType
                   << "But currently only supported xcb_generic_event_t";
        return false;
    }
    return handleXcbGeEvent(message);
}

bool DeviceMonitorEventFilter::handleXcbGeEvent(void* message)
{
    xcb_generic_event_t* event = static_cast<xcb_generic_event_t *>(message);
    if (!event) {
        qWarning() << __FILE__ << __FUNCTION__ << __LINE__ << "Message is empty";
        return false;
    }

    const uint8_t type = event->response_type & ~0x80;
    if (type != XCB_GE_GENERIC) {
        return false;
    }

    GeEventMemMover ge(event);
    switch (ge->event_type) {
    case XI_HierarchyChanged: {
        auto* he = reinterpret_cast<xcb_input_hierarchy_event_t *>(event);
        if ((he->flags & (XCB_INPUT_HIERARCHY_MASK_SLAVE_REMOVED
                          | XCB_INPUT_HIERARCHY_MASK_SLAVE_ADDED
                          | XCB_INPUT_HIERARCHY_MASK_DEVICE_ENABLED
                          | XCB_INPUT_HIERARCHY_MASK_DEVICE_DISABLED))) {
            // XI_HierarchyChanged events are sent continuously as the device changes
            if (m_timer->isActive())
                break;
            m_timer->start(20);

            setupTouchDevices();
        }
        break;
    }
    default:
        break;
    }

    return false;
}

void DeviceMonitorEventFilter::setupTouchDevices()
{
    bool existTouchScreen = existTouchDevice(XCB_INPUT_TOUCH_MODE_DIRECT);
    qDebug() << __FILE__ << __FUNCTION__ << __LINE__
             << "Touch Screen exist state change to:" << existTouchScreen;
    Q_EMIT touchScreenChanged(existTouchScreen);
}

namespace {
bool findTouchModeFromDevice(xcb_input_xi_device_info_t* deviceInfo, xcb_input_touch_mode_t type)
{
    // iterate over all classes of device
    auto classes_it = xcb_input_xi_device_info_classes_iterator(deviceInfo);
    for (; classes_it.rem > 0; xcb_input_device_class_next(&classes_it)) {
        xcb_input_device_class_t* classinfo = classes_it.data;
        if (XCB_INPUT_DEVICE_CLASS_TYPE_TOUCH != classinfo->type)
            continue;

        auto* touchClassInfo = reinterpret_cast<xcb_input_touch_class_t *>(classinfo);
        if (type == touchClassInfo->mode)
            return true;
    }
    return false;
}

bool hasTouchDeviceInQueryReply(xcb_input_xi_query_device_reply_t* queryReply , xcb_input_touch_mode_t type)
{
    // iterate over all devices
    auto it = xcb_input_xi_query_device_infos_iterator(queryReply);
    for (; it.rem > 0; xcb_input_xi_device_info_next(&it)) {
        xcb_input_xi_device_info_t* deviceInfo = it.data;

        if (deviceInfo->enabled == 0)
            continue;

        // touch device is slave pointer device
        if (deviceInfo->type != XCB_INPUT_DEVICE_TYPE_SLAVE_POINTER)
            continue;

        if (findTouchModeFromDevice(deviceInfo, type))
            return true;
    }
    return false;
}

bool existTouchDevice(xcb_input_touch_mode_t type)
{
    // connect to xserver
    int primaryScreen = 0;
    xcb_connection_t* conn = xcb_connect(nullptr, &primaryScreen);
    if (!conn) {
        qWarning() << __FILE__ << __FUNCTION__ << __LINE__
                   << "The X11 connection broke. Did the X11 server die?";
        return false;
    }
    int error = xcb_connection_has_error(conn);
    if (error) {
        qWarning() << __FILE__ << __FUNCTION__ << __LINE__
                   << "The X11 connection broke. Did the X11 server die?"
                   << "Error:" << error;
        xcb_disconnect(conn);
        return false;
    }

    // query all device
    auto reply = XCB_REPLY(xcb_input_xi_query_device, conn, XCB_INPUT_DEVICE_ALL);
    if (!reply) {
        qWarning() << __FILE__ << __FUNCTION__ << __LINE__
                   << "Failed to query device";
        xcb_disconnect(conn);
        return false;
    }
    xcb_disconnect(conn);

    return hasTouchDeviceInQueryReply(reply.get(), type);
}
} // namespace

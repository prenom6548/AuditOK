/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
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

#include "windowmanager.h"
#include "eventmanager.h"
#include "inputmethod/inputmethodchannel.h"
#include "mousekey/network/published_socket_connector.h"
#include "mousekey/runtime.h"
#include "mousekey/input/manager.h"
#include "eventdata.h"
#include "sessionsettings.h"
#include <linux/input.h>
#include <unistd.h>
#include <syslog.h>

#define PROPERTY_INPUT_DEVICE_CONNECTIONS "kmre.input_devices.connection"

EventManager::EventManager() 
    : QObject(nullptr)
    , mFocusedDisplayId(0)
    , rt(kmre::Runtime::create())
    , appmgr_start_timer(rt->service())
{
    qRegisterMetaType<QVector<int>>();
    qRegisterMetaType<EventData>();
    qRegisterMetaType<QList<EventData>>();
    qRegisterMetaType<EventRecordData>();

    syslog(LOG_DEBUG, "Reset kmre input devices connections");
    kmre::DbusClient::getInstance()->SetPropOfContainer(
            kmre::utils::getUserName(), getuid(), PROPERTY_INPUT_DEVICE_CONNECTIONS, "0");

    mInputmethodChannel = new InputmethodChannel();
    connect(this, SIGNAL(sendInputmethodData(QString)), mInputmethodChannel, SLOT(onSend(QString)));
    mInputmethodThread = new QThread();
    mInputmethodChannel->moveToThread(mInputmethodThread);
    //connect(mInputmethodThread, &QThread::started, mInputmethodChannel, &InputmethodChannel::onInitSocket);
    connect(mInputmethodThread, &QThread::finished, mInputmethodChannel, &InputmethodChannel::deleteLater);
    connect(mInputmethodThread, &QThread::finished, mInputmethodThread, &QThread::deleteLater);
    mInputmethodThread->start();

    global_track_id = 1;
    mInputManager = std::make_shared<kmre::input::Manager>(rt);
    QSize displaySize = SessionSettings::getInstance().getVirtualDisplaySize();
    mPlatform = std::make_shared<KmrePlatform>(mInputManager, displaySize.width(), displaySize.height());
    connect(this, SIGNAL(sendMouseKeyData(QList<EventData>,bool,int,int)), mPlatform.get(), SLOT(onSendEventData(QList<EventData>,bool,int,int)));
    rt->start();

    syslog(LOG_DEBUG, "Set kmre input devices connections");
    kmre::DbusClient::getInstance()->SetPropOfContainer(
            kmre::utils::getUserName(), getuid(), PROPERTY_INPUT_DEVICE_CONNECTIONS, "1");
}

EventManager::~EventManager() 
{
    mInputmethodThread->quit();
    mInputmethodThread->wait();
}

void EventManager::sendMouseEvent(const int displayId, MouseEventInfo mouseEvent)
{
    //syslog(LOG_DEBUG, "[%s] mFocusedDisplayId = %d, displayId = %d", 
    //        __func__, mFocusedDisplayId.load(), displayId);
    if (mFocusedDisplayId == displayId) {
        mouseEventHandler(mouseEvent.type, mouseEvent.slot, mouseEvent.x, mouseEvent.y);
    }
}

void EventManager::sendKeyBoardEvent(const int displayId, KeycodeBufferData* keyCodeBuf)
{
    if (mFocusedDisplayId == displayId) {
        QVector<int> keyCodes;
        for (int i = 0; i < keyCodeBuf->keycode_count; i++) {
            keyCodes.append(keyCodeBuf->keycodes[i]);
        }
        keyboardEventHandler(CONTROL_KEYBOARD_EVENT, std::move(keyCodes));
    }
}

void EventManager::sendInputMethodData(const int displayId, const QString &text)
{
    Q_UNUSED(displayId)
    emit sendInputmethodData(text);
}

void EventManager::mouseEventHandler(int type, int slot, int x, int y)
{
    struct timespec  mono;
    QList<EventData> mouse_events;
    std::uint64_t sec = 0;
    std::uint64_t usec = 0;

    clock_gettime(CLOCK_MONOTONIC, &mono);

    sec = mono.tv_sec;
    usec = mono.tv_nsec / 1000;

    QList<EventData> touch_events;

    switch (type) {
    case Button1_Press://m_type = type; m_code = code; m_value = value
        touch_events.append(EventData(EV_ABS, ABS_MT_SLOT, slot, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_TRACKING_ID, global_track_id++, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_TOUCH_MAJOR, 180, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_PRESSURE, 80, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_X, x, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_Y, y, sec, usec));
        touch_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));//send_event(touch_fd, EV_SYN, 0, 0);
        emit sendMouseKeyData(touch_events, true, 0/*m_id*/, CONTROL_MOUSE_EVENT);
        break;
    case Button1_Release:
        touch_events.append(EventData(EV_ABS, ABS_MT_SLOT, slot, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_PRESSURE, 0, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_X, x, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_Y, y, sec, usec));
        touch_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));//send_event(touch_fd, EV_SYN, 0, 0);
        touch_events.append(EventData(EV_ABS, ABS_MT_TRACKING_ID, -1, sec, usec));
        touch_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));//send_event(touch_fd, EV_SYN, 0, 0);
        emit sendMouseKeyData(touch_events, true, 0/*m_id*/, CONTROL_MOUSE_EVENT);
        break;
    case Button3_Press:
        mouse_events.append(EventData(EV_ABS, ABS_X, x, sec, usec));
        mouse_events.append(EventData(EV_ABS, ABS_Y, y, sec, usec));
        mouse_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));
        mouse_events.append(EventData(EV_KEY, BTN_RIGHT, 1, sec, usec));
        mouse_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));
        emit sendMouseKeyData(mouse_events, false, 0/*m_id*/, CONTROL_MOUSE_EVENT);
        break;
    case Button3_Release:
        mouse_events.append(EventData(EV_ABS, ABS_X, x, sec, usec));
        mouse_events.append(EventData(EV_ABS, ABS_Y, y, sec, usec));
        mouse_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));
        mouse_events.append(EventData(EV_KEY, BTN_RIGHT, 0, sec, usec));
        mouse_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));
        emit sendMouseKeyData(mouse_events, false, 0/*m_id*/, CONTROL_MOUSE_EVENT);
        break;
    case Button4_Up:
        ///touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_X, x, sec, usec));
        ///touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_Y, y, sec, usec));
        ///touch_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));//send_event(touch_fd, EV_SYN, 0, 0);

        mouse_events.append(EventData(EV_ABS, ABS_X, x, sec, usec));
        mouse_events.append(EventData(EV_ABS, ABS_Y, y, sec, usec));
        //mouse_events.append(EventData(EV_REL, REL_X, -1, sec, usec));
        //mouse_events.append(EventData(EV_REL, REL_Y, -1, sec, usec));
        mouse_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));
        mouse_events.append(EventData(EV_REL, REL_WHEEL, SCROLL_UP, sec, usec));//send_event(mouse_fd, EV_REL, REL_WHEEL, 1);
        mouse_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));//send_event(mouse_fd, EV_SYN, 0, 0);

        //emit sendMouseKeyData(touch_events, true, 0/*m_id*/);
        emit sendMouseKeyData(mouse_events, false, 0/*m_id*/, CONTROL_MOUSE_EVENT);
        break;
    case Button5_Down:
        //touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_X, x, sec, usec));
        //touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_Y, y, sec, usec));
        //touch_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));//send_event(touch_fd, EV_SYN, 0, 0);

        mouse_events.append(EventData(EV_ABS, ABS_X, x, sec, usec));
        mouse_events.append(EventData(EV_ABS, ABS_Y, y, sec, usec));
        //mouse_events.append(EventData(EV_REL, REL_X, -1, sec, usec));
        //mouse_events.append(EventData(EV_REL, REL_Y, -1, sec, usec));
        mouse_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));
        mouse_events.append(EventData(EV_REL, REL_WHEEL, SCROLL_DOWN, sec, usec));//send_event(mouse_fd, EV_REL, REL_WHEEL, -1);
        mouse_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));//send_event(mouse_fd, EV_SYN, 0, 0);

        //emit sendMouseKeyData(touch_events, true, 0);
        emit sendMouseKeyData(mouse_events, false, 0, CONTROL_MOUSE_EVENT);
        break;
    case Mouse_Motion:
        touch_events.append(EventData(EV_ABS, ABS_MT_SLOT, slot, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_PRESSURE, 80, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_X, x, sec, usec));
        touch_events.append(EventData(EV_ABS, ABS_MT_POSITION_Y, y, sec, usec));
        touch_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));//send_event(touch_fd, EV_SYN, 0, 0);
        emit sendMouseKeyData(touch_events, true, 0/*m_id*/, CONTROL_MOUSE_EVENT);
        break;
    default:
        break;
    }

    if(global_track_id == 65536)
        global_track_id = 1;

}

void EventManager::keyboardEventHandler(int eventType, QVector<int> keyCodes)
{
    uint64_t sec;
    uint64_t usec;
    struct timespec ts;

    QList<EventData> keyboard_events;
    int i;
    for (i = 0; i < keyCodes.size(); i++) {
        int code_down = keyCodes[i];
        bool down = code_down & 0x400;
        int code = code_down & 0x3ff;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        sec = ts.tv_sec;
        usec = ts.tv_nsec / 1000;
        keyboard_events.append(EventData(EV_KEY, code, down, sec, usec));

        clock_gettime(CLOCK_MONOTONIC, &ts);
        sec = ts.tv_sec;
        usec = ts.tv_nsec / 1000;
        keyboard_events.append(EventData(EV_SYN, SYN_REPORT, 0, sec, usec));
    }
    emit sendMouseKeyData(keyboard_events, false, 0, eventType);
}
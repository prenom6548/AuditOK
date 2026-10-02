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

#ifndef TESTCOMMAND_H
#define TESTCOMMAND_H
#include <QObject>
#include <QString>
#include <QDebug>
#include <QtCore/qcommandlineoption.h>
#include <QtCore/qcommandlineparser.h>

#include "ukuiinputgatherclient.h"
#include "inputgatherclient.h"
class testCommand
{
public:
public:
    testCommand();
    ~testCommand();

    void helpTool();
    void connectServerTool();
    void testCommandHelpStr();
    void testCommandConnectServer();
    void testCommanDebugEvents();
    void getEvent2(Event *event){
        switch (event->type) {
        case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:{
            qDebug()<<"LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN"<<" "<<event->event.swipeEventData.fingerCount<<" "<<event->event.swipeEventData.deltaX
                   <<"|"<<event->event.swipeEventData.deltaY;
            break;
        }
        case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:{
            qDebug()<<"LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE"<<" "<<event->event.swipeEventData.fingerCount<<" "<<event->event.swipeEventData.deltaX
                   <<"|"<<event->event.swipeEventData.deltaY;
            break;
        }
        case LIBINPUT_EVENT_GESTURE_SWIPE_END:{
            qDebug()<<"LIBINPUT_EVENT_GESTURE_SWIPE_END"<<" "<<event->event.swipeEventData.fingerCount<<" "<<event->event.swipeEventData.deltaX
                   <<"|"<<event->event.swipeEventData.deltaY <<"Cancel:"<<event->event.swipeEventData.cancelled;
            break;
        }
        case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:{
            qDebug()<<"LIBINPUT_EVENT_GESTURE_PINCH_BEGIN"<<" "<<event->event.pinchEventData.fingerCount<<" "<<event->event.pinchEventData.deltaX
                   <<"|"<<event->event.pinchEventData.deltaY;
            break;
        }
        case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:{
            qDebug()<<"LIBINPUT_EVENT_GESTURE_PINCH_UPDATE"<<" "<<event->event.pinchEventData.fingerCount<<" "<<event->event.pinchEventData.deltaX
                   <<"|"<<event->event.pinchEventData.deltaY;
            break;
        }
        case LIBINPUT_EVENT_GESTURE_PINCH_END:{
            qDebug()<<"LIBINPUT_EVENT_GESTURE_PINCH_END"<<" "<<event->event.pinchEventData.fingerCount<<" "<<event->event.pinchEventData.deltaX
                   <<"|"<<event->event.pinchEventData.deltaY<<"Cancel:"<<event->event.pinchEventData.cancelled;
            break;
        }
        case LIBINPUT_EVENT_SWITCH_TOGGLE: {
            qDebug() << "Event: LIBINPUT_EVENT_SWITCH_TOGGLE";
            if (event->event.switchEventDate.switchType == LIBINPUT_SWITCH_TABLET_MODE) {
                if (event->event.switchEventDate.switchState == LIBINPUT_SWITCH_STATE_ON) {
                     qDebug() << "Switch Type: LIBINPUT_SWITCH_TABLET_MODE\n"
                              << "Switch State: LIBINPUT_SWITCH_STATE_ON";
                }
                else if (event->event.switchEventDate.switchState == LIBINPUT_SWITCH_STATE_OFF) {
                    qDebug() << "Switch Type: LIBINPUT_SWITCH_TABLET_MODE\n"
                             << "Switch State: LIBINPUT_SWITCH_STATE_OFF";
                }
                else {
                    qDebug() << "Switch Type: LIBINPUT_SWITCH_TABLET_MODE\n"
                             << "Switch State: UNKNOW";
                }
            }
            else if (event->event.switchEventDate.switchType == LIBINPUT_SWITCH_LID) {
                if (event->event.switchEventDate.switchState == LIBINPUT_SWITCH_STATE_ON) {
                     qDebug() << "Switch Type: LIBINPUT_SWITCH_LID\n"
                              << "Switch State: LIBINPUT_SWITCH_STATE_ON";
                }
                else if (event->event.switchEventDate.switchState == LIBINPUT_SWITCH_STATE_OFF) {
                    qDebug() << "Switch Type: LIBINPUT_SWITCH_LID\n"
                             << "Switch State: LIBINPUT_SWITCH_STATE_OFF";
                }
                else {
                    qDebug() << "Switch Type: LIBINPUT_SWITCH_LID\n"
                             << "Switch State: UNKNOW";
                }
            }
            else {
                qDebug() << "Switch Type: UNKNOW";
            }
            break;
        }
        case LIBINPUT_EVENT_RAW_TAP: {
            qDebug() << "LIBINPUT_EVENT_RAW_TAP with fingercount of" << event->event.tapEventData.fingerCount;
            break;
        }
        default:
            break;
        }
    }

};


#endif // TESTCOMMAND_H

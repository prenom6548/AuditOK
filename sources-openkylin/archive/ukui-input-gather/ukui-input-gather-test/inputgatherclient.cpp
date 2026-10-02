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

#include "inputgatherclient.h"

#include "event.h"

#include <QTime>
#include <QTimer>
#include <QDataStream>

InputGatherClient::InputGatherClient(QObject *parent)
    :QObject(parent)
    ,m_clientSocket(nullptr)
{
    init();
    initConnect();
}

InputGatherClient::~InputGatherClient()
{

}

void InputGatherClient::init()
{
    m_clientSocket = new QLocalSocket;
}

void InputGatherClient::initConnect()
{
    connect(m_clientSocket, &QLocalSocket::readyRead, this, &InputGatherClient::onReceiveMessgae);
}

void InputGatherClient::connectServer(const QString &serverName)
{
    m_clientSocket->connectToServer(serverName);
    if(m_clientSocket->waitForConnected())
    {
        qDebug() << "服务端链接成功!";
    }else{
        qDebug() << "服务端链接失败!  2s后尝试重连!"
                 << "error msg =" << m_clientSocket->errorString();
        QTimer::singleShot(2000, this, [=](){ connectServer(serverName); });
    }
}

void InputGatherClient::onReceiveMessgae()
{
    bool isSupported = true;
    QByteArray byteArray = m_clientSocket->readAll();
    // qDebug() << m_clientSocket->readAll();
    Event *event = reinterpret_cast<Event *>(byteArray.data());

    switch (event->type) {
    case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
    case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
    case LIBINPUT_EVENT_GESTURE_SWIPE_END:{
        printfSwipeEvent(event->event.swipeEventData);
        break;
    }
    case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
    case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
    case LIBINPUT_EVENT_GESTURE_PINCH_END:{
        printfPinchEvent(event->event.pinchEventData);
        break;
    }
    case LIBINPUT_EVENT_RAW_TAP: {
        printfRawTapEvent(event->event.tapEventData);
        break;
    }
    default:
        isSupported = false;
        break;
    }

    if(isSupported)
    {
        // TODO
    }
}

void InputGatherClient::printfSwipeEvent(const SwipeGestureEvent &e)
{
    qDebug() << "swipe.fingercount: " << e.fingerCount << endl
             << "swipe.cancelled: " << e.cancelled << endl
             << "swipe.deltaX: " << e.deltaX << endl
             << "swipe.deltaY: " << e.deltaY << endl
             << "swipe.time: " << e.time;
}

void InputGatherClient::printfPinchEvent(const PinchGestureEvent &e)
{
    qDebug() << "pinch.fingercount: " << e.fingerCount << endl
             << "pinch.cancelled: " << e.cancelled << endl
             << "pinch.deltaX: " << e.deltaX << endl
             << "pinch.deltaY: " << e.deltaY << endl
             << "pinch.angleDalta: " << e.angleDelta << endl
             << "pinch.scale: " << e.scale << endl
             << "pinch.time: " << e.time;
}

void InputGatherClient::printfRawTapEvent(const TapGestureEvent &e)
{
    qDebug() << "rawtap.fingercount: " << e.fingerCount << Qt::endl
             << "rawtap.time: " << e.time;
}

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

#include "socketclient.h"

#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>

#include <QTime>
#include <QDebug>
#include <QThread>

/**
 * Unix domain socket (abstract namespace).
 */
static const std::string SOCKET_NAME{"\0/InputGatherServer", 10}; // NOLINT


SocketClient::SocketClient()
{

}

SocketClient::~SocketClient()
{

}

bool SocketClient::connectToServer()
{
    bool connected = false;

    m_socket = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (m_socket == -1) {
        return false;
    }

    struct sockaddr_un addr = {};
    addr.sun_family         = AF_UNIX;
    memset(&addr.sun_path[ 0 ], 0, sizeof(addr.sun_path));
    SOCKET_NAME.copy(&addr.sun_path[ 0 ], SOCKET_NAME.size(), 0);

    auto socketAddr = reinterpret_cast<sockaddr *>(&addr); // NOLINT
    if (connect(m_socket, socketAddr, sizeof(addr)) == -1) {
        printf("服务端链接失败!\n");
        connected = false;
    } else {
        printf("服务端链接成功!\n");
        connected = true;
    }

    return connected;
}

bool SocketClient::isConnected()
{

}

void SocketClient::receiveEvent()
{
    std::thread receiveThread{[ this ]() {
        printf("开始接收事件!\n");
        bool connected = false;
        Event event;
        while(true)
        {
            if(!connected){
                if(connectToServer()){
                    connected = true;
                }else{
                    connected = false;
                }
            }
            if (connected) {
                int total = read(m_socket, &event, sizeof(event));
                if (total <= 0) {
                    printf("读取数据失败!\n");
                    connected = false;
                } else {
                    bool isSupported = true;
                    switch (event.type) {
                    case LIBINPUT_EVENT_DEVICE_ADDED:
                    case LIBINPUT_EVENT_DEVICE_REMOVED:
                        break;
                    case LIBINPUT_EVENT_GESTURE_SWIPE_BEGIN:
                    case LIBINPUT_EVENT_GESTURE_SWIPE_UPDATE:
                    case LIBINPUT_EVENT_GESTURE_SWIPE_END:{
                        // printfSwipeEvent(event.event.swipeEventData);
                        break;
                    }
                    case LIBINPUT_EVENT_GESTURE_PINCH_BEGIN:
                    case LIBINPUT_EVENT_GESTURE_PINCH_UPDATE:
                    case LIBINPUT_EVENT_GESTURE_PINCH_END:{
                        // printfPinchEvent(event.event.pinchEventData);
                        break;
                    }
                    case LIBINPUT_EVENT_RAW_TAP: {
                        break;
                    }
                    default:
                        isSupported = false;
                        break;
                    }

                    if(isSupported)
                    {
                        // TODO
                        if(sendEvent){
                            sendEvent(event);
                        }
                    }
                }
            }
            if (!connected) {
                close(m_socket);
                int retryTime = 2;
                printf("%ds后开始重新尝试链接！\n", retryTime);
                QThread::sleep(retryTime);
            }
        }
    }};
    receiveThread.detach();
}


void SocketClient::printfSwipeEvent(const SwipeGestureEvent &e)
{
    qDebug() << "swipe.fingercount: " << e.fingerCount << endl
             << "swipe.cancelled: " << e.cancelled << endl
             << "swipe.deltaX: " << e.deltaX << endl
             << "swipe.deltaY: " << e.deltaY << endl
             << "swipe.time: " << e.time;
}

void SocketClient::printfPinchEvent(const PinchGestureEvent &e)
{
    qDebug() << "pinch.fingercount: " << e.fingerCount << endl
             << "pinch.cancelled: " << e.cancelled << endl
             << "pinch.deltaX: " << e.deltaX << endl
             << "pinch.deltaY: " << e.deltaY << endl
             << "pinch.angleDalta: " << e.angleDelta << endl
             << "pinch.scale: " << e.scale << endl
             << "pinch.time: " << e.time;
}

void SocketClient::printfRawTapEvent(const TapGestureEvent &e)
{
    qDebug() << "rawtap.fingercount: " << e.fingerCount << Qt::endl
             << "rawtap.time: " << e.time;
}
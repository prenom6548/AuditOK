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

#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include "event.h"
#include <string>
#include <functional>


class SocketClient
{
public:
    SocketClient();
    ~SocketClient();

    void setEventCallback(std::function<void(Event)> fun){ sendEvent = fun; }
    void receiveEvent();
    bool isConnected();

private:
    std::function<void(Event)> sendEvent = nullptr;

    bool connectToServer();

    void printfPinchEvent(const PinchGestureEvent &e);
    void printfSwipeEvent(const SwipeGestureEvent &e);
    void printfRawTapEvent(const TapGestureEvent &e);

    int m_socket;
};

#endif // SOCKETCLIENT_H

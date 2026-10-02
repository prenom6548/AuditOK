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

#ifndef INPUTGATHERCLIENT_H
#define INPUTGATHERCLIENT_H

#include <QObject>

#include <QLocalSocket>
#include "event.h"

#define INPUT_GATHER_SERVER_NAME "InputGatherServer"

class InputGatherClient : public QObject
{
    Q_OBJECT
public:
    InputGatherClient(QObject *parent = nullptr);
    ~InputGatherClient();

    void connectServer(const QString &serverName);

private slots:
    void onReceiveMessgae();
    void printfSwipeEvent(const SwipeGestureEvent &e);
    void printfPinchEvent(const PinchGestureEvent &e);
    void printfRawTapEvent(const TapGestureEvent &e);

private:
    void init();
    void initConnect();

    QLocalSocket *m_clientSocket;
};

#endif // INPUTGATHERCLIENT_H

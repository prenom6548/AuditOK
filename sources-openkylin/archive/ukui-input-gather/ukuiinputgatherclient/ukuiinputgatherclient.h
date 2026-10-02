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

#ifndef UKUIINPUTGATHERCLIENT_H
#define UKUIINPUTGATHERCLIENT_H
#include "ukuiinputgatherclient_global.h"

#include "event.h"
#include <functional>

class UKUIINPUTGATHERCLIENT_EXPORT UKUIInputGatherClient
{
public:
    UKUIInputGatherClient();
    ~UKUIInputGatherClient();

    void startToReceiveEvent();
    void setEventCallBack(std::function<void(Event*)> sendEvent);
    /*! \param libinputTabletSwitchState()
     *  \return return 1 if has device with capability of LIBINPUT_SWITCH_TABLET_MODE
     *          and state is LIBINPUT_SWITCH_STATE_ON
     *          return 0 if has device with capability of LIBINPUT_SWITCH_TABLET_MODE
     *          amd state is LIBINPUT_SWITCH_STATE_OFF
     *          return -1 if dose not have device with capability of
     *          LIBINPUT_SWITCH_TABLET_MODE
    */
    int libinputTabletSwitchState();
    QJsonArray libinputDeviceList();

private:
    bool connectToServer();

    Event* m_event;
    std::function<void(Event*)> m_sendEventCallBack;

    int m_socket;
};
#endif // UKUIINPUTGATHERCLIENT_H

/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/

#include "libinputswitchevent.h"

typedef std::function<void(Event *)> sendEvent;
LibinputSwitchEvent::LibinputSwitchEvent(QObject *parent)
    : QObject(parent), m_machineModel(MachineModel::getMachineModelInstance())
{
    m_machineType = m_machineModel->getTheMachineType();
    sendEvent se = std::bind(&LibinputSwitchEvent::dealEvent, this, std::placeholders::_1);
    m_inputGatherClient = new UKUIInputGatherClient();
    m_inputGatherClient->setEventCallBack(se);
    m_inputGatherClient->startToReceiveEvent();
}

LibinputSwitchEvent::~LibinputSwitchEvent()
{
    //    delete m_inputGatherClient;
}

bool LibinputSwitchEvent::geInitDevicesStatus()
{
    //其他处理
    if (m_machineType == QStringLiteral("SLATE")) {
        return true;
    } else if (m_machineType == "LAPTOP") { //永久附加键盘笔记本
        // return true;
    } else if (m_machineType == "ALLINONE") { //台式
        return false;
    } else {
        // 0 非平板
        // 1 平板
        //-1 不支持
        //        int status = m_inputGatherClient->libinputTabletSwitchState();
        //        qInfo() << __FILE__ << __LINE__<< "当前设备的状态:" << status;
        //        if(status == 1)
        //            return true;
        //        else
        return false;
    }
}

void LibinputSwitchEvent::dealEvent(Event *e)
{
    switch (e->type) {
        case LIBINPUT_EVENT_SWITCH_TOGGLE:
            qInfo() << __FILE__ << __LINE__ << "=LIBINPUT_EVENT_SWITCH_TOGGLE=";
            if (e->event.switchEventDate.switchType == LIBINPUT_SWITCH_TABLET_MODE) {
                qInfo() << __FILE__ << __LINE__ << "switch type" << e->event.switchEventDate.switchType;
                qInfo() << __FILE__ << __LINE__ << "switch status" << e->event.switchEventDate.switchState;
                if (e->event.switchEventDate.switchState == 1) {
                    Q_EMIT tabletModeStatusChanged(1);
                } else {
                    Q_EMIT tabletModeStatusChanged(0);
                }
            }
            break;
        default:
            break;
    }
}

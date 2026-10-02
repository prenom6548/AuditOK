/*
 * Copyright (C) 2024 KylinSoftCo., Ltd.
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
#ifndef LIBINPUTSWITCHEVENT_H
#define LIBINPUTSWITCHEVENT_H

#include <QObject>
#include <QDebug>
#include <ukui/event.h>
#include <ukui/ukuiinputgatherclient.h>
#include "machinemodel.h"
//头文件以及顺序不可改，不可删
class LibinputSwitchEvent : public QObject
{
    Q_OBJECT
public:
    explicit LibinputSwitchEvent(QObject *parent = nullptr);

    ~LibinputSwitchEvent();
    UKUIInputGatherClient * m_inputGatherClient = nullptr;
    bool geInitDevicesStatus();
signals:
    void tabletModeStatusChanged(int tabletmode);
private:
    void dealEvent(Event* e);

private:
    QString m_machineType;
    std::shared_ptr<MachineModel> m_machineModel = nullptr;
};

#endif // LIBINPUTSWITCHEVENT_H

/*
* Copyright (C) 2024, KylinSoft Co., Ltd.
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
* Authors: zhouyong <zhouyong01@kylinos.cn>
*
*/
#ifndef COMMON_H
#define COMMON_H

#include <QObject>
#include <QVariant>

class Common : public QObject
{
    Q_OBJECT
public:
    static Common& getInstance();

    enum TaskID {
        TaskID_Init = 0,
        TaskID_GetSinkVolume,
        TaskID_SetSinkVolume,
        TaskID_GetSinkMute,
        TaskID_SetSinkMute,
        TaskID_SetSinkInputVolume,
        TaskID_GetSourceVolume,
        TaskID_SetSourceVolume,
        TaskID_SetSourceOutputVolume,
        TaskID_GetSinkAvailablePortList,
        TaskID_GetSourceAvailablePortList,
        TaskID_GetSinkInputList,
        TaskID_GetSourceInputList,
        TaskID_GetSinkDefaultDevice,
        TaskID_SetSinkDefaultDevice,
        TaskID_GetSourceDefaultDevice,
        TaskID_SetSourceDefaultDevice,
        TaskID_JumpControlPanelAudio,
        TaskID_JumpAppControlPanelAudio,
        TaskID_GetSinkVolumeBoostStatus,
        TaskID_GetSinkVolumeBoostValue,
        TaskID_SendSinkInputVolumeChanged,

        TaskID_End = 999
    };
    Q_ENUM(TaskID)

public:
    Q_INVOKABLE QVariant getThemePropertyByName(QString);

private:
    Common() = default;
    Common(const Common&) = delete;
    Common(Common&&) = delete;
    Common operator=(const Common&) = delete;
    Common operator=(Common&&) = delete;
    ~Common() = default;


private:


};


#endif // COMMON_H

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
#include "ukui-audio-control.h"
#include <QCoreApplication>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QProcess>
#include "common.h"
#include "ukui-audio-sink-control.h"
#include "ukui-audio-source-control.h"
#include "ukui-audio-sink-model.h"
#include "ukui-audio-sink-app-volume-model.h"

UKUIAudioControl& UKUIAudioControl::getInstance()
{
    static UKUIAudioControl instance;
    return instance;
}

bool UKUIAudioControl::initData()
{
    bool ret = true;

    return ret;
}

bool UKUIAudioControl::initSlots()
{
    bool ret = true;

    return ret;
}

bool UKUIAudioControl::initDbusConnect()
{
    bool ret = true;
    DBusClient::getInstance().initDbusConnect();
    return ret;
}

bool UKUIAudioControl::jumpControlPanelAudio()
{
    qDebug() << __func__ << "enter";
    bool ret = QProcess::startDetached("ukui-control-center", {"-m", "Audio"});
    qDebug() << __func__ << "leave ret" << ret;

    return ret;
}

bool UKUIAudioControl::jumpAppControlPanelAudio()
{
    qDebug() << __func__ << "enter";
    bool ret = QProcess::startDetached("ukui-control-center", QStringList() << "-m" << "Audio" << "appcontrol");
    qDebug() << __func__ << "leave ret" << ret;

    return ret;
}

void UKUIAudioControl::initMainThreadEvent()
{
    QThread* mainThread = QCoreApplication::instance()->thread();
    this->moveToThread(mainThread);
    UKUIAudioSinkControl::getInstance().moveToThread(mainThread);
    UKUIAudioSourceControl::getInstance().moveToThread(mainThread);
    UKUIAudioSinkModel::getInstance().moveToThread(mainThread);
    UKUIAudioSinkAppVolumeModel::getInstance().moveToThread(mainThread);

    this->initSlots();
    this->initDbusConnect();
    UKUIAudioSinkControl::getInstance().initSlots();
    UKUIAudioSourceControl::getInstance().initSlots();

    UKUIAudioSinkControl::getInstance().initView();
}

void UKUIAudioControl::startTask(int taskID, QVariant var1, QVariant var2, QVariant var3)
{
    qDebug() << __func__ << "enter taskID:" << taskID << "var1:" << var1 << "var2" << var2;
    QMutex& taskMutex = m_taskMutex;
    QFuture<void> future = QtConcurrent::run([taskID,var1,var2,var3,&taskMutex,this]() {
        qDebug() << "UKUIAudioControl::startTask run taskID:" << taskID;
        QMetaEnum metaEnum = QMetaEnum::fromType<Common::TaskID>();
        if (metaEnum.isValid()) {
            const char* enumName = metaEnum.valueToKey(taskID);
            qDebug() << "UKUIAudioControl::startTask run taskID:" << taskID << "enumName:" << (enumName ? enumName : "unvalid");
        }
        bool ret = true;
        if (taskMutex.tryLock(3000)) {
            QVariant retValue;
            switch (taskID) {
            case Common::TaskID_Init: {
                initData();
                UKUIAudioSinkControl::getInstance().initData();
                UKUIAudioSourceControl::getInstance().initData();
            }
            break;
            case Common::TaskID_GetSinkVolume: {
                ret = UKUIAudioSinkControl::getInstance().getSinkVolume(var1.toInt(), retValue);
            }
            break;
            case Common::TaskID_SetSinkVolume: {
                ret = UKUIAudioSinkControl::getInstance().setSinkVolume(var1.toInt(), var2.toInt());
            }
            break;
            case Common::TaskID_GetSinkMute: {
                ret = UKUIAudioSinkControl::getInstance().getSinkMute(var1.toInt(), retValue);
            }
            break;
            case Common::TaskID_SetSinkMute: {
                ret = UKUIAudioSinkControl::getInstance().setSinkMute(var1.toInt(), var2.toBool());
            }
            break;
            case Common::TaskID_SetSinkInputVolume: {
                ret = UKUIAudioSinkControl::getInstance().setSinkInputVolume(var1.toInt(), var2.toInt());
            }
            break;
            case Common::TaskID_GetSourceVolume: {
                ret = UKUIAudioSourceControl::getInstance().getSourceVolume(var1.toInt(), retValue);
            }
            break;
            case Common::TaskID_SetSourceVolume: {
                ret = UKUIAudioSourceControl::getInstance().setSourceVolume(var1.toInt(), var2.toInt());
            }
            break;
            case Common::TaskID_GetSinkAvailablePortList: {
                ret = UKUIAudioSinkControl::getInstance().getSinkAvailablePortList(retValue);
            }
            break;
            case Common::TaskID_GetSourceAvailablePortList: {
                ret = UKUIAudioSourceControl::getInstance().getSourceAvailablePortList(retValue);
            }
            break;
            case Common::TaskID_GetSinkInputList: {
                ret = UKUIAudioSinkControl::getInstance().getSinkInputList(retValue);
            }
            break;
            case Common::TaskID_GetSourceInputList: {
                ret = UKUIAudioSourceControl::getInstance().getSourceOutputList(retValue);
            }
            break;
            case Common::TaskID_GetSinkDefaultDevice: {
                ret = UKUIAudioSinkControl::getInstance().getSinkDefaultDevice(var1.toInt(), retValue);
            }
            break;
            case Common::TaskID_SetSinkDefaultDevice: {
                ret = UKUIAudioSinkControl::getInstance().setSinkDefaultDevice(var1.toInt(), var2.toString(), var3.toString());
            }
            break;
            case Common::TaskID_GetSourceDefaultDevice: {

            }
            break;
            case Common::TaskID_SetSourceDefaultDevice: {

            }
            break;
            case Common::TaskID_JumpControlPanelAudio: {
                jumpControlPanelAudio();
            }
            break;
            case Common::TaskID_JumpAppControlPanelAudio: {
                jumpAppControlPanelAudio();
            }
            break;
            case Common::TaskID_GetSinkVolumeBoostStatus: {
                ret = UKUIAudioSinkControl::getInstance().getSinkVolumeBoostStatus(retValue);
            }
            break;
            case Common::TaskID_GetSinkVolumeBoostValue: {
                ret = UKUIAudioSinkControl::getInstance().getSinkVolumeBoostValue(retValue);
            }
            break;
            case Common::TaskID_SendSinkInputVolumeChanged: {
                ret = UKUIAudioSinkControl::getInstance().sendSinkInputVolumeChanged(var1.toString(), var2.toInt(), var3.toBool());
            }
            break;
            default: {
            }
            break;
            }

            if (ret) {
                emit taskCompleted(taskID, retValue);
            } else {
                emit taskError(taskID);
            }
            taskMutex.unlock();
        } else {
            qDebug() << "UKUIAudioControl::startTask tryLock timeout give up taskID:" << taskID;
            ret = false;
            emit taskError(taskID);
        }
    });
}

void UKUIAudioControl::releaseData()
{
    qDebug() << __func__ << "enter";
    UKUIAudioSinkControl::getInstance().releaseData();
    UKUIAudioSourceControl::getInstance().releaseData();
    qDebug() << __func__ << "leave";
}

void UKUIAudioControl::releaseSlots()
{
    qDebug() << __func__ << "enter";
    UKUIAudioSinkControl::getInstance().releaseSlots();
    UKUIAudioSourceControl::getInstance().releaseSlots();
    qDebug() << __func__ << "leave";
}

UKUIAudioControl::~UKUIAudioControl()
{
    qDebug() << __func__ << "enter";
    releaseSlots();
    releaseData();
}

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
#include "ukui-audio-source-control.h"

#include "Util.h"
#include "../model/ukui-audio-source-model.h"

using namespace UkuiAudioFramwork;

UKUIAudioSourceControl& UKUIAudioSourceControl::getInstance()
{
    static UKUIAudioSourceControl instance;
    return instance;
}

bool UKUIAudioSourceControl::initData()
{
    bool ret = true;

    return ret;
}

bool UKUIAudioSourceControl::initSlots()
{
    bool ret = true;
    ret = connect(&DBusClient::getInstance(), SIGNAL(volumeChangedSignal(int, int, const QDBusVariant&)), this, SLOT(volumeChangedSlots(int, int, const QDBusVariant&)));

    return ret;
}

void UKUIAudioSourceControl::volumeChangedSlots(int type, int idx, const QDBusVariant& value)
{
    qDebug() << __func__ << "type:" << type << "idx:" << idx << "value:" << value.variant();
    if (type == EnumToInt(NodeType::INPUT)) {
        setSourceVolumeValue(value.variant().toInt());
    }
}

void UKUIAudioSourceControl::deviceChangedSlots(int type, const QString& portName, const QString& cardName)
{
    qDebug() << __func__ << "type:" << type << "portName:" << portName << "cardName:" << cardName;
    switch (IntToEnum<NodeType>(type)) {
    case NodeType::INPUT: {
        emit sourceDeviceChanged(portName, cardName);
    }
        break;
    case NodeType::OUTPUT:
    case NodeType::SINK_INPUT:
    case NodeType::SOURCE_OUTPUT:
    default:
        break;
    }
}

void UKUIAudioSourceControl::deviceAdjustSlots(int type)
{
    qDebug() << __func__ << "type:" << type;
    switch (IntToEnum<NodeType>(type)) {
    case NodeType::INPUT: {
        emit sourceDeviceAdjust();
    }
        break;
    case NodeType::OUTPUT:
    case NodeType::SINK_INPUT:
    case NodeType::SOURCE_OUTPUT:
    default:
        break;
    }
}

bool UKUIAudioSourceControl::getSourceVolume(int index, QVariant& retValue)
{
    qDebug() << __func__ << "index" << index;
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                     AUDIO_OBJECT_PATH,
                                                                     AUDIO_INTERFACE,
                                                                     "getVolume",
                                                                     EnumToInt(NodeType::INPUT), index);
    if (reply.arguments().isEmpty())
    {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    qDebug() << __func__ << "reply" << reply.arguments().value(0);
    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSourceControl::updateSourceVolume(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    setSourceVolumeValue(retValue.toInt());
}

bool UKUIAudioSourceControl::setSourceVolume(int index, int value)
{
    qDebug() << __func__ << "index" << index << "value" << value;
    setSourceVolumeValue(value);

    bool ret = true;
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setVolume",
                                             EnumToInt(NodeType::INPUT), index, value);

    return ret;
}

bool UKUIAudioSourceControl::getSourceAvailablePortList(QVariant& retValue)
{
    qDebug() << __func__ << "enter";
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                     AUDIO_OBJECT_PATH,
                                                                     AUDIO_INTERFACE,
                                                                     "getAvailablePortList",
                                                                     EnumToInt(NodeType::INPUT));
    if (reply.arguments().isEmpty())
    {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSourceControl::updateSourceAvailablePortList(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    UKUIAudioSourceModel::getInstance().clearDevice();
    UKUIAudioSourceModel::getInstance().addDeviceHeader();
    const QDBusArgument& dbusArgs = retValue.value<QList<QVariant>>().at(0).value<QDBusArgument>();
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientDeviceInfo info;
        dbusArgs >> info;
        UKUIAudioSourceModel::getInstance().addDevice(info);
    }
    dbusArgs.endArray();
}

bool UKUIAudioSourceControl::getSourceOutputList(QVariant& retValue)
{
    qDebug() << __func__ << "enter";
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                     AUDIO_OBJECT_PATH,
                                                                     AUDIO_INTERFACE,
                                                                     "getSourceOutputList");
    if (reply.arguments().isEmpty())
    {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSourceControl::updateSourceInputList(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    UKUIAudioSourceModel::getInstance().clearStream();
    UKUIAudioSourceModel::getInstance().addStreamHeader();
    const QDBusArgument& dbusArgs = retValue.value<QList<QVariant>>().at(0).value<QDBusArgument>();
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        StreamInfo info;
        dbusArgs >> info;
        UKUIAudioSourceModel::getInstance().addStream(info);
    }
    dbusArgs.endArray();
}

bool UKUIAudioSourceControl::getSourceDefaultDevice(int index, QVariant& retValue)
{
    qDebug() << __func__ << "enter";
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getDefaultDevice",
                                                                      EnumToInt(NodeType::INPUT), index);
    if (reply.arguments().isEmpty()) {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSourceControl::updateSourceDefaultDevice(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    const QDBusArgument& dbusArgs = retValue.value<QList<QVariant>>().at(0).value<QDBusArgument>();
    ClientDefaultDeviceInfo info;
    dbusArgs >> info;
    UKUIAudioSourceModel::getInstance().setDeviceActivePort(info.activePortName, info.cardName);
}

void UKUIAudioSourceControl::updateSourceDefaultDevice(const QString& portName, const QString& cardName)
{
    qDebug() << __func__ << "enter" << "portName:" << portName << "cardName:" << cardName;
    UKUIAudioSourceModel::getInstance().setDeviceActivePort(portName, cardName);
}

bool UKUIAudioSourceControl::setSourceDefaultDevice(int index, const QString& portName, const QString& cardName)
{
    qDebug() << __func__ << "enter";
    bool ret = true;
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setDefaultDevice",
                                             EnumToInt(NodeType::INPUT), index, portName, cardName);

    return ret;
}

void UKUIAudioSourceControl::releaseData()
{
    qDebug() << "UKUIAudioSourceControl::releaseData" << "enter";
    m_sourceVolumeValue = -1;
    UKUIAudioSourceModel::getInstance().releaseModel();
    qDebug() << "UKUIAudioSourceControl::releaseData" << "leave";
}

void UKUIAudioSourceControl::releaseSlots()
{
    qDebug() << "UKUIAudioSourceControl::releaseSlots" << "enter";
    disconnect(&DBusClient::getInstance(), SIGNAL(volumeChangedSignal(int, int, const QDBusVariant&)), this, SLOT(volumeChangedSlots(int, int, const QDBusVariant&)));
    qDebug() << "UKUIAudioSourceControl::releaseSlots" << "leave";
}

UKUIAudioSourceControl::~UKUIAudioSourceControl()
{
    qDebug() << __func__ << "enter";
    releaseSlots();
    releaseData();
}

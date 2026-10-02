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
#include "ukui-audio-sink-control.h"

#include "Util.h"
#include "../model/ukui-audio-sink-model.h"
#include "../model/ukui-audio-sink-app-volume-model.h"
#include "./ukui-audio-control.h"
#include "../common/common.h"
#include <QSet>

using namespace UkuiAudioFramwork;

Q_DECLARE_METATYPE(QList<ClientStreamMediaInfo>)

namespace {
const QString kKylinRecorderIconName = QStringLiteral("kylin-recorder");
}

UKUIAudioSinkControl& UKUIAudioSinkControl::getInstance()
{
    static UKUIAudioSinkControl instance;
    return instance;
}

void UKUIAudioSinkControl::collectKylinRecorderInputIndices(const ClientStreamMediaInfo& info)
{
    if (info.iconName.compare(kKylinRecorderIconName) != 0) {
        return;
    }

    for (const auto& media : info.mediaInfoList) {
        // direction=0: source-output（录音流）
        if (media.direction == UKUIAudioSinkAppVolumeModel::kStreamDirectionInput) {
            m_kylinRecorderInputIndices.insert(static_cast<int>(media.index));
        }
    }
}

void UKUIAudioSinkControl::applyKylinRecorderVolumeEnabled()
{
    const bool enabled = m_kylinRecorderInputIndices.isEmpty();
    qDebug() << __func__ << "kylin-recorder streamVolumeEnabled:" << enabled
             << "recordingStreamCount:" << m_kylinRecorderInputIndices.size();
    UKUIAudioSinkAppVolumeModel::getInstance().setStreamVolumeEnabled(kKylinRecorderIconName, enabled);
}

bool UKUIAudioSinkControl::initData()
{
    bool ret = true;

    return ret;
}

bool UKUIAudioSinkControl::initSlots()
{
    bool ret = true;
    ret = connect(&DBusClient::getInstance(), SIGNAL(volumeChangedSignal(int, int, const QDBusVariant&)), this, SLOT(volumeChangedSlots(int, int, const QDBusVariant&)));
    ret = connect(&DBusClient::getInstance(), SIGNAL(muteChangedSignal(int, int, bool)), this, SLOT(muteChangedSlots(int, int, bool)));
    ret = connect(&DBusClient::getInstance(), SIGNAL(deviceChangedSignal(int, const QString&, const QString&)), this, SLOT(deviceChangedSlots(int, const QString&, const QString&)));
    ret = connect(&DBusClient::getInstance(), SIGNAL(deviceAdjustSignal(int)), this, SLOT(deviceAdjustSlots(int)));
    ret = connect(&DBusClient::getInstance(), SIGNAL(addStreamSignal(int, int, int, bool, const QString&, const QString&)),
                  this, SLOT(addStreamSlots(int, int, int, bool, const QString&, const QString&)));
    ret = connect(&DBusClient::getInstance(), SIGNAL(removeStreamSignal(int, int)), this, SLOT(removeStreamSlots(int, int)));
    ret = connect(&DBusClient::getInstance(), SIGNAL(settingsChangedSignal(QString,QDBusVariant)), this, SLOT(settingsChangedSlots(QString,QDBusVariant)));
    //start
    ret = QDBusConnection::sessionBus().connect(QString(), QString("/"), QString("org.kylin.music"), QString("sinkInputVolumeChanged"),
                                                this, SLOT(streamVolumeChangedSlots(QString,int,bool)));
    ret = QDBusConnection::sessionBus().connect(QString(), QString("/"), QString("org.kylin.video"), QString("sinkInputVolumeChanged"),
                                                this, SLOT(streamVolumeChangedSlots(QString,int,bool)));
    //end
    return ret;
}

bool UKUIAudioSinkControl::initView()
{
    bool ret = true;
    // 标题由 QML 独立绘制；设备 / 应用音量为两个独立 Model
    return ret;
}

void UKUIAudioSinkControl::volumeChangedSlots(int type, int idx, const QDBusVariant& value)
{
    qDebug() << __func__ << "type:" << type << "idx:" << idx << "value:" << value.variant();
    if (type == EnumToInt(NodeType::OUTPUT)) {
        setSinkVolumeValue(value.variant().toInt());
    }
}

void UKUIAudioSinkControl::muteChangedSlots(int type, int index, bool mute)
{
    qDebug() << __func__ << "type:" << type << "index:" << index << "mute:" << mute;
    if (type == EnumToInt(NodeType::OUTPUT)) {
        setSinkMuteStatus(mute);
    }
}

void UKUIAudioSinkControl::deviceChangedSlots(int type, const QString& portName, const QString& cardName)
{
    qDebug() << __func__ << "type:" << type << "portName:" << portName << "cardName:" << cardName;
    switch (IntToEnum<NodeType>(type)) {
    case NodeType::OUTPUT: {
        emit sinkDeviceChanged(portName, cardName);
    }
        break;
    case NodeType::INPUT:
    case NodeType::SINK_INPUT:
    case NodeType::SOURCE_OUTPUT:
    default:
        break;
    }
}

void UKUIAudioSinkControl::deviceAdjustSlots(int type)
{
    qDebug() << __func__ << "type:" << type;
    switch (IntToEnum<NodeType>(type)) {
    case NodeType::OUTPUT: {
        emit sinkDeviceAdjust();
    }
        break;
    case NodeType::INPUT:
    case NodeType::SINK_INPUT:
    case NodeType::SOURCE_OUTPUT:
    default:
        break;
    }
}

void UKUIAudioSinkControl::addStreamSlots(int dire, int idx, int value, bool mute, const QString& iconName, const QString& descName)
{
    qDebug() << __func__ << "dire:" << dire << "idx:" << idx << "value:" << value << "mute:" << mute
             << "iconName:" << iconName << "descName:" << descName;

    if (UKUIAudioSinkAppVolumeModel::shouldIgnoreStreamItem(iconName, descName)) {
        return;
    }

    ClientStreamMediaInfo info;
    info.iconName = iconName;

    const auto oldStreamList = UKUIAudioSinkAppVolumeModel::getInstance().getStreamMediaInfoList();
    for (const auto& oldInfo : oldStreamList) {
        if (oldInfo.iconName == iconName) {
            info.mediaRole = oldInfo.mediaRole;
            info.binary = oldInfo.binary;
            break;
        }
    }

    ClientMediaInfo media;
    media.direction = static_cast<uint32_t>(dire);
    media.index = static_cast<uint32_t>(idx);
    media.volume = static_cast<uint32_t>(value);
    media.mute = mute;
    media.name = descName;
    info.mediaInfoList.push_back(media);

    UKUIAudioSinkAppVolumeModel::getInstance().addStream(info);

    // 与控制中心对齐：录音过程中录音应用输出音量不可调
    if (iconName.compare(kKylinRecorderIconName) == 0
        && dire == static_cast<int>(UKUIAudioSinkAppVolumeModel::kStreamDirectionInput)) {
        m_kylinRecorderInputIndices.insert(idx);
        applyKylinRecorderVolumeEnabled();
    }

    if (UKUIAudioSinkAppVolumeModel::getInstance().getStreamCount() > 0
        && UKUIAudioSinkAppVolumeModel::getInstance().hasEmptyStream()) {
        UKUIAudioSinkAppVolumeModel::getInstance().removeEmptyStream();
    }
}

void UKUIAudioSinkControl::removeStreamSlots(int dire, int idx)
{
    qDebug() << __func__ << "dire:" << dire << "idx:" << idx;

    UKUIAudioSinkAppVolumeModel::getInstance().removeStreamByIndex(static_cast<quint32>(dire),
                                                                   static_cast<quint32>(idx));

    if (dire == static_cast<int>(UKUIAudioSinkAppVolumeModel::kStreamDirectionInput)
        && m_kylinRecorderInputIndices.remove(idx)) {
        applyKylinRecorderVolumeEnabled();
    }

    if (UKUIAudioSinkAppVolumeModel::getInstance().getStreamCount() <= 0
        && !UKUIAudioSinkAppVolumeModel::getInstance().hasEmptyStream()) {
        UKUIAudioSinkAppVolumeModel::getInstance().addEmptyStream();
    }
}

void UKUIAudioSinkControl::settingsChangedSlots(const QString& key, const QDBusVariant& value)
{
    qDebug() << __func__ << "key:" << key << "value:" << value.variant();
    if (key.compare(VOLUME_BOOST_KEY) == 0)
    {
        setSinkVolumeBoostStatus(value.variant().toBool());
    }
}

void UKUIAudioSinkControl::streamVolumeChangedSlots(QString name, int value, bool muteStatus)
{
    qDebug() << __func__ << "name" << name << "value" << value << "muteStatus" << muteStatus;
    Q_UNUSED(muteStatus);

    int streamIndex = UKUIAudioSinkAppVolumeModel::getInstance().getStreamIndexByStreamIconName(name);
    if (streamIndex == -1) {
        qDebug() << __func__ << "streamIndex invaild .";
        return;
    }

    UKUIAudioControl::getInstance().startTask(Common::TaskID_SetSinkInputVolume, streamIndex, value);
}

bool UKUIAudioSinkControl::getSinkVolume(int index, QVariant& retValue)
{
    qDebug() << __func__ << "index" << index;
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                     AUDIO_OBJECT_PATH,
                                                                     AUDIO_INTERFACE,
                                                                     "getVolume",
                                                                     EnumToInt(NodeType::OUTPUT), index);
    if (reply.arguments().isEmpty())
    {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSinkControl::updateSinkVolume(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    setSinkVolumeValue(retValue.value<QList<QVariant>>().at(0).toInt());
}

bool UKUIAudioSinkControl::setSinkVolume(int index, int value)
{
    qDebug() << __func__ << "index" << index << "value" << value;
    setSinkVolumeValue(value);

    bool ret = true;
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setVolume",
                                             EnumToInt(NodeType::OUTPUT), index, value);

    return ret;
}

bool UKUIAudioSinkControl::getSinkMute(int index, QVariant& retValue)
{
    qDebug() << __func__ << "index" << index;
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getMute",
                                                                      EnumToInt(NodeType::OUTPUT), index);
    if (reply.arguments().isEmpty())
    {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSinkControl::updateSinkMute(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    setSinkMuteStatus(retValue.value<QList<QVariant>>().at(0).toBool());
}

bool UKUIAudioSinkControl::setSinkMute(int index, bool muted)
{
    qDebug() << __func__ << "index" << index << "muted" << muted;
    setSinkMuteStatus(muted);

    bool ret = true;
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setMute",
                                             EnumToInt(NodeType::OUTPUT), index, muted);

    return ret;
}

bool UKUIAudioSinkControl::getSinkAvailablePortList(QVariant& retValue)
{
    qDebug() << __func__ << "enter";
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                     AUDIO_OBJECT_PATH,
                                                                     AUDIO_INTERFACE,
                                                                     "getAvailablePortList",
                                                                     EnumToInt(NodeType::OUTPUT));
    if (reply.arguments().isEmpty())
    {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    isExitMultiAudioDevice();

    return ret;
}

void UKUIAudioSinkControl::updateSinkAvailablePortList(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    QList<ClientDeviceInfo> newDeviceList;
    const QDBusArgument& dbusArgs = retValue.value<QList<QVariant>>().at(0).value<QDBusArgument>();
    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientDeviceInfo info;
        dbusArgs >> info;
        newDeviceList.push_back(info);
    }
    dbusArgs.endArray();

    if (m_isExitMultiAudioDevice) {
        ClientDeviceInfo info;
        info.cardName = m_SinkMutiAudioDeviceInfo.cardName;
        info.cardDesc = m_SinkMutiAudioDeviceInfo.activePortDesc;
        info.portName = m_SinkMutiAudioDeviceInfo.activePortName;
        info.portLabel = QObject::tr("Multi Bluetooth Output");
        info.priority = 0;
        info.direction = 0;
        info.available = 0;
        info.enabled = true;
        newDeviceList.push_back(info);
    }

    auto makeDeviceKey = [](const ClientDeviceInfo& info) {
        return QString("%1|%2").arg(info.cardName, info.portName);
    };

    QList<ClientDeviceInfo> oldDeviceList = UKUIAudioSinkModel::getInstance().getDeviceInfoList();
    QSet<QString> newDeviceKeySet;
    QSet<QString> oldDeviceKeySet;

    for (const auto& info : newDeviceList) {
        newDeviceKeySet.insert(makeDeviceKey(info));
        qDebug() << __func__ << "newDeviceKeySet:" << makeDeviceKey(info);
    }
    for (const auto& info : oldDeviceList) {
        oldDeviceKeySet.insert(makeDeviceKey(info));
        qDebug() << __func__ << "oldDeviceKeySet:" << makeDeviceKey(info);
    }

    for (const auto& oldInfo : oldDeviceList) {
        if (!newDeviceKeySet.contains(makeDeviceKey(oldInfo))) {
            UKUIAudioSinkModel::getInstance().removeDevice(oldInfo.cardName, oldInfo.portName);
        }
    }

    for (const auto& newInfo : newDeviceList) {
        if (!oldDeviceKeySet.contains(makeDeviceKey(newInfo))) {
            UKUIAudioSinkModel::getInstance().addDevice(newInfo);
        }
    }

    for (const auto& newInfo : newDeviceList) {
        if (oldDeviceKeySet.contains(makeDeviceKey(newInfo))) {
            UKUIAudioSinkModel::getInstance().updateDevice(newInfo);
        }
    }

    if (UKUIAudioSinkModel::getInstance().getDeviceCount() <= 0
        || UKUIAudioSinkModel::getInstance().getEnabledDeviceCount() <= 0) {
        if (!UKUIAudioSinkModel::getInstance().hasEmptyDevice()) {
            UKUIAudioSinkModel::getInstance().addEmptyDevice();
        }
    } else if (UKUIAudioSinkModel::getInstance().hasEmptyDevice()) {
        UKUIAudioSinkModel::getInstance().removeEmptyDevice();
    }
}

bool UKUIAudioSinkControl::getSinkInputList(QVariant& retValue)
{
    qDebug() << __func__ << "enter";
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  AUDIO_INTERFACE,
                                                                  "getStreamMediaList");
    if (reply.arguments().isEmpty())
    {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSinkControl::updateSinkInputList(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    qRegisterMetaType<ClientMediaInfo>();
    qRegisterMetaType<ClientStreamMediaInfo>();
    qRegisterMetaType<QList<ClientStreamMediaInfo>>();
    qDBusRegisterMetaType<ClientMediaInfo>();
    qDBusRegisterMetaType<ClientStreamMediaInfo>();
    QList<ClientStreamMediaInfo> newStreamList;
    m_kylinRecorderInputIndices.clear();

    const QList<QVariant> args = retValue.value<QList<QVariant>>();
    if (args.isEmpty()) {
        qDebug() << __func__ << "arguments is empty...";
        return;
    }

    const QVariant& first = args.at(0);
    auto appendIfValid = [&newStreamList](const ClientStreamMediaInfo& info) {
        if (UKUIAudioSinkAppVolumeModel::shouldIgnoreStream(info) || info.mediaInfoList.empty()) {
            return;
        }
        newStreamList.push_back(info);
    };

    // Session-bus reply: first arg is a readable QDBusArgument array.
    // In-process / whitebox callers may pass QList<ClientStreamMediaInfo> directly
    // (local QDBusArgument is write-only and cannot be demarshalled).
    if (first.userType() == qMetaTypeId<QDBusArgument>()) {
        const QDBusArgument& dbusArgs = first.value<QDBusArgument>();
        dbusArgs.beginArray();
        while (!dbusArgs.atEnd()) {
            ClientStreamMediaInfo info;
            dbusArgs >> info;
            appendIfValid(info);
        }
        dbusArgs.endArray();
    } else if (first.canConvert<QList<ClientStreamMediaInfo>>()) {
        const auto list = first.value<QList<ClientStreamMediaInfo>>();
        for (const auto& info : list) {
            appendIfValid(info);
        }
    } else {
        qDebug() << __func__ << "unsupported payload type" << first.typeName();
        return;
    }

    const QList<ClientStreamMediaInfo> oldStreamList =
        UKUIAudioSinkAppVolumeModel::getInstance().getStreamMediaInfoList();
    // 与 audio UI 一致：按 iconName 判定同一应用
    QSet<QString> newIconSet;
    QSet<QString> oldIconSet;

    for (const auto& s : newStreamList) {
        newIconSet.insert(s.iconName);
    }
    for (const auto& s : oldStreamList) {
        oldIconSet.insert(s.iconName);
    }

    for (const auto& oldInfo : oldStreamList) {
        if (!newIconSet.contains(oldInfo.iconName)) {
            for (const auto& media : oldInfo.mediaInfoList) {
                UKUIAudioSinkAppVolumeModel::getInstance().removeStreamByIndex(media.direction, media.index);
            }
        }
    }

    for (const auto& newInfo : newStreamList) {
        collectKylinRecorderInputIndices(newInfo);
        if (!oldIconSet.contains(newInfo.iconName)) {
            UKUIAudioSinkAppVolumeModel::getInstance().addStream(newInfo);
        } else {
            // 全量刷新：直接替换 mediaInfoList，避免残留输入流索引
            UKUIAudioSinkAppVolumeModel::getInstance().updateStream(newInfo, true);
        }
    }

    // 组件非常驻：先录音再打开时，需在初始化列表中根据录音流置灰
    applyKylinRecorderVolumeEnabled();

    if (UKUIAudioSinkAppVolumeModel::getInstance().getStreamCount() <= 0) {
        if (!UKUIAudioSinkAppVolumeModel::getInstance().hasEmptyStream()) {
            UKUIAudioSinkAppVolumeModel::getInstance().addEmptyStream();
        }
    } else if (UKUIAudioSinkAppVolumeModel::getInstance().hasEmptyStream()) {
        UKUIAudioSinkAppVolumeModel::getInstance().removeEmptyStream();
    }
}

void UKUIAudioSinkControl::updateSinkInputVolume(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }
}

bool UKUIAudioSinkControl::setSinkInputVolume(int index, int value)
{
    qDebug() << __func__ << "index" << index << "value" << value;

    bool ret = true;
    // 仅同步输出方向(sink-input)索引，避免把同应用 source-output 索引误当 SINK_INPUT
    const auto streamIndices = UKUIAudioSinkAppVolumeModel::getInstance().getStreamIndicesForIndex(index);
    qDebug() << __func__ << "outputStreamIndices" << streamIndices;
    for (const auto streamIndex : streamIndices) {
        DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                 AUDIO_OBJECT_PATH,
                                                 AUDIO_INTERFACE,
                                                 "setVolume",
                                                 EnumToInt(NodeType::SINK_INPUT), static_cast<int>(streamIndex), value);
    }

    return ret;
}

bool UKUIAudioSinkControl::sendSinkInputVolumeChanged(const QString& iconName, int value, bool muteStatus)
{
    qDebug() << __func__ << "iconName" << iconName << "value" << value << "muteStatus" << muteStatus;

    bool ret = true;
    if ((iconName.compare(QString("kylin-music")) == 0)
        || (iconName.compare(QString("kylin-video")) == 0)) {
        QDBusMessage msg = QDBusMessage::createSignal(QString("/"), QString("org.ukui.media"), QString("sinkVolumeChanged"));
        msg << iconName << value << muteStatus;
        QDBusConnection::sessionBus().send(msg);
        qDebug() << __func__ << "iconName:" << iconName << "value:" << value;
    }

    return ret;
}

bool UKUIAudioSinkControl::getSinkDefaultDevice(int index, QVariant& retValue)
{
    qDebug() << __func__ << "enter" << index;
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getDefaultDevice",
                                                                      EnumToInt(NodeType::OUTPUT), index);
    if (reply.arguments().isEmpty()) {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSinkControl::updateSinkDefaultDevice(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    const QDBusArgument& dbusArgs = retValue.value<QList<QVariant>>().at(0).value<QDBusArgument>();
    ClientDefaultDeviceInfo info;
    dbusArgs >> info;
    UKUIAudioSinkModel::getInstance().setDeviceActivePort(info.activePortName, info.cardName);
}

void UKUIAudioSinkControl::updateSinkDefaultDevice(const QString& portName, const QString& cardName)
{
    qDebug() << __func__ << "enter" << "portName:" << portName << "cardName:" << cardName;
    UKUIAudioSinkModel::getInstance().setDeviceActivePort(portName, cardName);
}

bool UKUIAudioSinkControl::setSinkDefaultDevice(int index, const QString &portName, const QString&cardName)
{
    qDebug() << __func__ << "enter" << index << "portName:" << portName << "cardName:" << cardName;
    bool ret = true;
    DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                             AUDIO_OBJECT_PATH,
                                             AUDIO_INTERFACE,
                                             "setDefaultDevice",
                                             EnumToInt(NodeType::OUTPUT), index, portName, cardName);

    return ret;
}

bool UKUIAudioSinkControl::getSinkVolumeBoostStatus(QVariant& retValue)
{
    qDebug() << __func__ << "enter";
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  SETTINGS_INTERFACE,
                                                                  "getVolumeBoostStatus");
    if (reply.arguments().isEmpty()) {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSinkControl::updateSinkVolumeBoostStatus(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    setSinkVolumeBoostStatus(retValue.value<QList<QVariant>>().at(0).toBool());
}

bool UKUIAudioSinkControl::getSinkVolumeBoostValue(QVariant& retValue)
{
    qDebug() << __func__ << "enter";
    bool ret = true;
    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  SETTINGS_INTERFACE,
                                                                  "getVolumeBoostValue");
    if (reply.arguments().isEmpty()) {
        qDebug() << __func__ << "arguments is nullptr...";
        ret = false;
        return ret;
    }

    retValue.setValue(reply.arguments());

    return ret;
}

void UKUIAudioSinkControl::updateSinkVolumeBoostValue(QVariant retValue)
{
    qDebug() << __func__ << "enter";
    if (!retValue.isValid()) {
        qDebug() << __func__ << "retValue is invalid...";
        return;
    }

    setSinkVolumeBoostValue(retValue.value<QList<QVariant>>().at(0).toInt());
}

bool UKUIAudioSinkControl::isExitMultiAudioDevice()
{
    m_isExitMultiAudioDevice = false;
    auto status = getSinkMultiAudioCombineStatus();
    if (status) {
        auto sinkList = getSinkList();
        auto it = std::find_if(sinkList.begin(), sinkList.end(), [](const ClientDefaultDeviceInfo& info) {
            return info.name.contains("combine");
        });

        if (it != sinkList.end()) {
            m_isExitMultiAudioDevice = true;
            m_SinkMutiAudioDeviceInfo = *it;
            qDebug() << __func__ << "Add Multiple Bluetooth outputs, (*it).activePortName " << (*it).activePortName <<
                "(*it).cardName" << (*it).cardName;
        }
    }

    return m_isExitMultiAudioDevice;
}

bool UKUIAudioSinkControl::getSinkMultiAudioCombineStatus() const
{
    QDBusReply<bool> reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                      AUDIO_OBJECT_PATH,
                                                                      AUDIO_INTERFACE,
                                                                      "getMultiAudioCombineStatus",
                                                                      EnumToInt(NodeType::OUTPUT));

    return reply;
}

QList<ClientDefaultDeviceInfo> UKUIAudioSinkControl::getSinkList() const
{
    QList<ClientDefaultDeviceInfo> list;

    QDBusMessage reply = DBusClient::getInstance().dbusMethodCall(AUDIO_OBJECT_SERVICE,
                                                                  AUDIO_OBJECT_PATH,
                                                                  AUDIO_INTERFACE,
                                                                  "getSinkList");

    if (reply.arguments().isEmpty()) {
        qDebug() << __func__ << "empty sinklist";
        return {};
    }

    const QDBusArgument& dbusArgs = reply.arguments().at(0).value<QDBusArgument>();

    dbusArgs.beginArray();
    while (!dbusArgs.atEnd()) {
        ClientDefaultDeviceInfo info;
        dbusArgs >> info;
        list.push_back(info);
    }
    dbusArgs.endArray();

    return list;
}

int UKUIAudioSinkControl::getSinkDeviceCount()
{
    return UKUIAudioSinkModel::getInstance().getDeviceCount();
}

int UKUIAudioSinkControl::getEnabledSinkDeviceCount()
{
    return UKUIAudioSinkModel::getInstance().getEnabledDeviceCount();
}

void UKUIAudioSinkControl::releaseData()
{
    qDebug() << "UKUIAudioSinkControl::releaseData" << "enter";
    m_sinkVolumeValue = -1;
    m_sinkMuteStatus = false;
    m_sinkVolumeStatusIcon = "";
    m_sinkVolumeBoostStatus = false;
    m_sinkVolumeBoostValue = -1;
    m_isExitMultiAudioDevice = false;
    m_kylinRecorderInputIndices.clear();
    UKUIAudioSinkModel::getInstance().releaseModel();
    UKUIAudioSinkAppVolumeModel::getInstance().releaseModel();
    qDebug() << "UKUIAudioSinkControl::releaseData" << "leave";
}

void UKUIAudioSinkControl::releaseSlots()
{
    qDebug() << "UKUIAudioSinkControl::releaseSlots" << "enter";
    disconnect(&DBusClient::getInstance(), SIGNAL(volumeChangedSignal(int, int, const QDBusVariant&)), this, SLOT(volumeChangedSlots(int, int, const QDBusVariant&)));
    disconnect(&DBusClient::getInstance(), SIGNAL(muteChangedSignal(int, int, bool)), this, SLOT(muteChangedSlots(int, int, bool)));
    disconnect(&DBusClient::getInstance(), SIGNAL(deviceChangedSignal(int, const QString&, const QString&)), this, SLOT(deviceChangedSlots(int, const QString&, const QString&)));
    disconnect(&DBusClient::getInstance(), SIGNAL(deviceAdjustSignal(int)), this, SLOT(deviceAdjustSlots(int)));
    disconnect(&DBusClient::getInstance(), SIGNAL(addStreamSignal(int, int, int, bool, const QString&, const QString&)), this, SLOT(addStreamSlots(int, int, int, bool, const QString&, const QString&)));
    disconnect(&DBusClient::getInstance(), SIGNAL(removeStreamSignal(int, int)), this, SLOT(removeStreamSlots(int, int)));
    qDebug() << "UKUIAudioSinkControl::releaseSlots" << "leave";
}

UKUIAudioSinkControl::~UKUIAudioSinkControl()
{
    qDebug() << __func__ << "enter";
    releaseSlots();
    releaseData();
}

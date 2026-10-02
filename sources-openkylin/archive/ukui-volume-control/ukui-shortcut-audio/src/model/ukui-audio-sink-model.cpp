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
#include "ukui-audio-sink-model.h"
#include <QDebug>

namespace {
QString buildDevicePortIcon(const QString& portName)
{
    if (portName.contains(QString("headphone"), Qt::CaseInsensitive)) {
        return "audio-headphones-symbolic";
    }
    if (portName.contains(QString("headset"), Qt::CaseInsensitive)) {
        return "audio-headset-symbolic";
    }
    if (portName.contains(QString("combine"), Qt::CaseInsensitive)) {
        return "audio-speakers-bluetooth-symbolic";
    }
    return "audio-speakers-symbolic";
}
}

UKUIAudioSinkModel& UKUIAudioSinkModel::getInstance()
{
    static UKUIAudioSinkModel instance;
    return instance;
}

void UKUIAudioSinkModel::initData()
{
}

void UKUIAudioSinkModel::initSlots()
{
}

int UKUIAudioSinkModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_sinkInfos.size();
}

QVariant UKUIAudioSinkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_sinkInfos.size()) {
        return QVariant();
    }

    const SinkInfo& sinkInfo = m_sinkInfos.at(index.row());
    switch (role) {
    case ItemTypeRole: return sinkInfo.itemType;
    case TitleRole: return sinkInfo.title;
    case CardNameRole: return sinkInfo.cardName;
    case CardDescRole: return sinkInfo.cardDesc;
    case PortNameRole: return sinkInfo.portName;
    case PortLabelRole: return sinkInfo.portLabel;
    case PriorityRole: return sinkInfo.priority;
    case DirectionRole: return sinkInfo.direction;
    case AvailableRole: return sinkInfo.available;
    case PortEnabledRole: return sinkInfo.enabled;
    case PortTypeRole: return sinkInfo.type;
    case PortIconRole: return sinkInfo.portIcon;
    case IsActivePortRole: return sinkInfo.isActivePort;
    default: return QVariant();
    }
}

QHash<int,QByteArray> UKUIAudioSinkModel::roleNames() const
{
    return {
        {ItemTypeRole, "itemType"},
        {TitleRole, "title"},
        {CardNameRole, "cardName"},
        {CardDescRole, "cardDesc"},
        {PortNameRole, "portName"},
        {PortLabelRole, "portLabel"},
        {PriorityRole, "priority"},
        {DirectionRole, "direction"},
        {AvailableRole, "available"},
        {PortEnabledRole, "portEnabled"},
        {PortTypeRole, "portType"},
        {PortIconRole, "portIcon"},
        {IsActivePortRole, "isActivePort"}
    };
}

void UKUIAudioSinkModel::addEmptyDevice()
{
    beginInsertRows(QModelIndex(), m_sinkInfos.size(), m_sinkInfos.size());
    SinkInfo info;
    info.itemType = EmptyDeviceItem;
    info.title = QObject::tr("No output sound card detected");
    m_sinkInfos.append(info);
    endInsertRows();
}

void UKUIAudioSinkModel::addDevice(ClientDeviceInfo deviceInfo)
{
    qDebug() << __func__ << QString("Add card: %1 port: %2 to device list").arg(deviceInfo.cardName).arg(deviceInfo.portName);

    SinkInfo sinkInfo;
    deviceInfo >> sinkInfo;
    sinkInfo.portIcon = buildDevicePortIcon(sinkInfo.portName);

    beginInsertRows(QModelIndex(), m_sinkInfos.size(), m_sinkInfos.size());
    m_sinkInfos.append(sinkInfo);
    endInsertRows();
}

void UKUIAudioSinkModel::addDevice(ClientDefaultDeviceInfo deviceInfo)
{
    qDebug() << __func__ << QString("Add card: %1 port: %2 to device list").arg(deviceInfo.cardName).arg(deviceInfo.activePortName);

    SinkInfo sinkInfo;
    deviceInfo >> sinkInfo;
    sinkInfo.portIcon = buildDevicePortIcon(sinkInfo.portName);

    beginInsertRows(QModelIndex(), m_sinkInfos.size(), m_sinkInfos.size());
    m_sinkInfos.append(sinkInfo);
    endInsertRows();
}

QList<ClientDeviceInfo> UKUIAudioSinkModel::getDeviceInfoList() const
{
    QList<ClientDeviceInfo> list;
    for (const auto& sinkInfo : m_sinkInfos) {
        if (sinkInfo.itemType != DeviceItem) {
            continue;
        }
        ClientDeviceInfo info;
        info.cardName = sinkInfo.cardName;
        info.cardDesc = sinkInfo.cardDesc;
        info.portName = sinkInfo.portName;
        info.portLabel = sinkInfo.portLabel;
        info.priority = sinkInfo.priority;
        info.direction = sinkInfo.direction;
        info.available = sinkInfo.available;
        info.enabled = sinkInfo.enabled;
        list.push_back(info);
    }
    return list;
}

bool UKUIAudioSinkModel::updateDevice(const ClientDeviceInfo& deviceInfo)
{
    for (int index = 0; index < m_sinkInfos.size(); ++index) {
        auto& sinkInfo = m_sinkInfos[index];
        if (sinkInfo.itemType != DeviceItem) {
            continue;
        }
        if (sinkInfo.cardName.compare(deviceInfo.cardName) != 0
            || sinkInfo.portName.compare(deviceInfo.portName) != 0) {
            continue;
        }

        const QString newPortIcon = buildDevicePortIcon(deviceInfo.portName);
        const bool changed = sinkInfo.cardDesc.compare(deviceInfo.cardDesc) != 0
            || sinkInfo.portLabel.compare(deviceInfo.portLabel) != 0
            || sinkInfo.priority != deviceInfo.priority
            || sinkInfo.direction != deviceInfo.direction
            || sinkInfo.available != deviceInfo.available
            || sinkInfo.enabled != deviceInfo.enabled
            || sinkInfo.portIcon.compare(newPortIcon) != 0;

        if (!changed) {
            return false;
        }

        sinkInfo.cardDesc = deviceInfo.cardDesc;
        sinkInfo.portLabel = deviceInfo.portLabel;
        sinkInfo.priority = deviceInfo.priority;
        sinkInfo.direction = deviceInfo.direction;
        sinkInfo.available = deviceInfo.available;
        sinkInfo.enabled = deviceInfo.enabled;
        sinkInfo.portIcon = newPortIcon;

        const QModelIndex idx = createIndex(index, 0, nullptr);
        emit dataChanged(idx, idx);
        return true;
    }
    return false;
}

bool UKUIAudioSinkModel::removeDevice(const QString& cardName, const QString& portName)
{
    for (int index = m_sinkInfos.size() - 1; index >= 0; --index) {
        const auto& info = m_sinkInfos.at(index);
        if (info.itemType != DeviceItem) {
            continue;
        }
        if (info.cardName.compare(cardName) == 0 && info.portName.compare(portName) == 0) {
            removeDevice(index);
            return true;
        }
    }
    return false;
}

bool UKUIAudioSinkModel::hasEmptyDevice() const
{
    for (const auto& sinkInfo : m_sinkInfos) {
        if (sinkInfo.itemType == EmptyDeviceItem) {
            return true;
        }
    }
    return false;
}

bool UKUIAudioSinkModel::removeEmptyDevice()
{
    for (int index = m_sinkInfos.size() - 1; index >= 0; --index) {
        if (m_sinkInfos.at(index).itemType != EmptyDeviceItem) {
            continue;
        }
        beginRemoveRows(QModelIndex(), index, index);
        m_sinkInfos.removeAt(index);
        endRemoveRows();
        return true;
    }
    return false;
}

bool UKUIAudioSinkModel::setDeviceActivePort(QString portName, QString cardName)
{
    qDebug() << __func__ << "enter" << "portName:" << portName << "cardName:" << cardName;
    bool ret = false;
    int firstChanged = -1;
    int lastChanged = -1;

    for (int index = 0; index < m_sinkInfos.size(); ++index) {
        if (m_sinkInfos.at(index).itemType != DeviceItem) {
            continue;
        }

        const bool shouldActive = (m_sinkInfos.at(index).cardName.compare(cardName) == 0)
                && (m_sinkInfos.at(index).portName.compare(portName) == 0);
        if (m_sinkInfos.at(index).isActivePort == shouldActive) {
            continue;
        }

        m_sinkInfos[index].isActivePort = shouldActive;
        if (firstChanged < 0) {
            firstChanged = index;
        }
        lastChanged = index;
        if (shouldActive) {
            ret = true;
        }
    }

    // 只用 dataChanged，避免 beginResetModel 销毁 ListView delegate 导致键盘焦点丢失
    if (firstChanged >= 0) {
        const QModelIndex topLeft = createIndex(firstChanged, 0, nullptr);
        const QModelIndex bottomRight = createIndex(lastChanged, 0, nullptr);
        emit dataChanged(topLeft, bottomRight, {IsActivePortRole});
    }

    qDebug() << __func__ << "leave" << "ret:" << ret;
    return ret;
}

void UKUIAudioSinkModel::removeDevice(int index)
{
    if (index < 0 || index >= m_sinkInfos.size()) {
        return;
    }
    if (m_sinkInfos.at(index).itemType != DeviceItem) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    m_sinkInfos.removeAt(index);
    endRemoveRows();
}

void UKUIAudioSinkModel::clearDevice()
{
    for (int index = m_sinkInfos.size() - 1; index >= 0; --index) {
        if (m_sinkInfos.at(index).itemType == DeviceItem
                || m_sinkInfos.at(index).itemType == EmptyDeviceItem) {
            beginRemoveRows(QModelIndex(), index, index);
            m_sinkInfos.removeAt(index);
            endRemoveRows();
        }
    }
}

int UKUIAudioSinkModel::getDeviceCount()
{
    int count = 0;
    for (const auto& sinkInfo : m_sinkInfos) {
        if (sinkInfo.itemType == DeviceItem) {
            ++count;
        }
    }
    return count;
}

int UKUIAudioSinkModel::getEnabledDeviceCount()
{
    int count = 0;
    for (const auto& sinkInfo : m_sinkInfos) {
        if (sinkInfo.itemType == DeviceItem && sinkInfo.enabled) {
            ++count;
        }
    }
    return count;
}

void UKUIAudioSinkModel::releaseModel()
{
    beginResetModel();
    m_sinkInfos.clear();
    endResetModel();
}

UKUIAudioSinkModel::~UKUIAudioSinkModel()
{
    releaseModel();
}

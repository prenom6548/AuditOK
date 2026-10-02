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
#include "ukui-audio-source-model.h"
#include <QDebug>

UKUIAudioSourceModel& UKUIAudioSourceModel::getInstance()
{
    static UKUIAudioSourceModel instance;
    return instance;
}

void UKUIAudioSourceModel::initData()
{

}

void UKUIAudioSourceModel::initSlots()
{

}

int UKUIAudioSourceModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_sourceInfos.size();
}

QVariant UKUIAudioSourceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_sourceInfos.size())
    {
        return QVariant();
    }

    const SourceInfo& sourceInfo = m_sourceInfos.at(index.row());
    switch (role) {
    case ItemTypeRole: return sourceInfo.itemType;
    case TitleRole: return sourceInfo.title;
    case CardNameRole: return sourceInfo.cardName;
    case CardDescRole: return sourceInfo.cardDesc;
    case PortNameRole: return sourceInfo.portName;
    case PortLabelRole:return sourceInfo.portLabel;
    case PriorityRole:return sourceInfo.priority;
    case DirectionRole: return sourceInfo.direction;
    case AvailableRole: return sourceInfo.available;
    case PortEnabledRole: return sourceInfo.enabled;
    case PortTypeRole: return sourceInfo.type;
    case PortIconRole: return sourceInfo.portIcon;
    case IsActivePortRole: return sourceInfo.isActivePort;
    case StreamNameRole: return sourceInfo.streamName;
    case StreamIndexRole: return sourceInfo.streamIndex;
    case StreamVolumeRole: return sourceInfo.streamVolume;
    case StreamRole: return sourceInfo.streamRole;
    case StreamIconNameRole: return sourceInfo.streamIconName;
    default: return QVariant();
    }
}

QHash<int,QByteArray> UKUIAudioSourceModel::roleNames() const
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
        {IsActivePortRole, "isActivePort"},
        {StreamNameRole, "streamName"},
        {StreamIndexRole, "streamIndex"},
        {StreamVolumeRole, "streamVolume"},
        {StreamRole, "streamRole"},
        {StreamIconNameRole, "streamIconName"}
    };
}

void UKUIAudioSourceModel::addDeviceHeader()
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_sourceInfos.append({HeaderOutput, QObject::tr("Input"), "", "", "", "", 0, 0, 0, true, -1, "", false,
                        "", 0, 0, "", ""});
    endInsertRows();
}

void UKUIAudioSourceModel::addDevice(ClientDeviceInfo deviceInfo)
{
    SourceInfo SourceInfo;
    deviceInfo >> SourceInfo;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_sourceInfos.append(SourceInfo);
    endInsertRows();
}

bool UKUIAudioSourceModel::setDeviceActivePort(QString portName, QString cardName)
{
    qDebug() << __func__ << "enter" << "portName:" << portName << "cardName:" << cardName;
    bool ret = false;
    int index = 0;
    for (; index < m_sourceInfos.size(); ++index) {
        if (m_sourceInfos.at(index).itemType != DeviceItem)
        {
            continue;
        }

        qDebug() << __func__ << "index:" << index << "cardName:" << m_sourceInfos.at(index).cardName
                 << "portName:" << m_sourceInfos.at(index).portName;
        if ((m_sourceInfos.at(index).cardName.compare(cardName) == 0)
                && (m_sourceInfos.at(index).portName.compare(portName) == 0))
        {
            m_sourceInfos[index].isActivePort = true;
            QModelIndex topLeft = createIndex(index, 0, nullptr);
            QModelIndex bottomRight = createIndex(index, 0, nullptr);
            emit dataChanged(topLeft, bottomRight);
            ret = true;
        }
        else
        {
            m_sourceInfos[index].isActivePort = false;
            QModelIndex topLeft = createIndex(index, 0, nullptr);
            QModelIndex bottomRight = createIndex(index, 0, nullptr);
            emit dataChanged(topLeft, bottomRight);
        }
    }

    return ret;
}

void UKUIAudioSourceModel::removeDevice(int index)
{
    if (index < 0 || index >= m_sourceInfos.size())
    {
        qDebug() << __func__ << "index is invaild" << index;
        return;
    }

    if (m_sourceInfos.at(index).itemType != DeviceItem)
    {
        qDebug() << __func__ << "this index no DeviceItem" << index;
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_sourceInfos.removeAt(index);
    endRemoveRows();
}

void UKUIAudioSourceModel::clearDevice()
{
    int index;
    for (index = 0; index < m_sourceInfos.size(); ++index) {
        if (m_sourceInfos.at(index).itemType == DeviceItem)
        {
            beginRemoveRows(QModelIndex(), index, index);
            m_sourceInfos.removeAt(index);
            endRemoveRows();
        }
    }
}

void UKUIAudioSourceModel::addStreamHeader()
{
    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_sourceInfos.append({HeaderAppVolume, QObject::tr("Application volume"), "", "", "", "", 0, 0, 0, true, -1, "", false,
                        "", 0, 0, "", ""});
    endInsertRows();
}

void UKUIAudioSourceModel::addStream(StreamInfo streamInfo)
{
    SourceInfo SourceInfo;
    streamInfo >> SourceInfo;

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    m_sourceInfos.append(SourceInfo);
    endInsertRows();
}

void UKUIAudioSourceModel::removeStream(int index)
{
    if (index < 0 || index >= m_sourceInfos.size())
    {
        qDebug() << __func__ << "index is invaild" << index;
        return;
    }

    if (m_sourceInfos.at(index).itemType != StreamItem)
    {
        qDebug() << __func__ << "this index no StreamItem" << index;
        return;
    }

    beginRemoveRows(QModelIndex(), index, index);
    m_sourceInfos.removeAt(index);
    endRemoveRows();
}

void UKUIAudioSourceModel::clearStream()
{
    int index;
    for (index = 0; index < m_sourceInfos.size(); ++index) {
        if (m_sourceInfos.at(index).itemType == StreamItem)
        {
            beginRemoveRows(QModelIndex(), index, index);
            m_sourceInfos.removeAt(index);
            endRemoveRows();
        }
    }
}

int UKUIAudioSourceModel::getStreamCount()
{
    int index = 0;
    int count = 0;
    for (; index < m_sourceInfos.size(); ++index) {
        if (m_sourceInfos.at(index).itemType == StreamItem)
        {
            count++;
        }
    }
    qDebug() << __func__ << "count:" << count;

    return count;
}

void UKUIAudioSourceModel::releaseModel()
{
    qDebug() << "UKUIAudioSourceModel::releaseModel" << "enter";
//    beginResetModel();
    m_sourceInfos.clear();
//    endResetModel();
    qDebug() << "UKUIAudioSourceModel::releaseModel" << "leave";
}

UKUIAudioSourceModel::~UKUIAudioSourceModel()
{
    qDebug() << __func__ << "enter";
    releaseModel();
}

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
#ifndef UKUIAUDIOSOURCEMODEL_H
#define UKUIAUDIOSOURCEMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "DBusClient.h"

class UKUIAudioSourceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // 定义角色（暴露给 QML 的属性）
    enum SourceInfoRoles {
        ItemTypeRole = Qt::UserRole + 1,
        TitleRole,      // 标题内容（仅 Header 使用）
        CardNameRole,
        CardDescRole,
        PortNameRole,
        PortLabelRole,
        PriorityRole,
        DirectionRole,
        AvailableRole,
        PortEnabledRole,
        PortTypeRole,
        PortIconRole,
        IsActivePortRole,

        StreamNameRole,
        StreamIndexRole,
        StreamVolumeRole,
        StreamRole,
        StreamIconNameRole
    };
    Q_ENUM(SourceInfoRoles)

    // 定义数据项类型
    enum ItemType {
        Invaild = -1,
        HeaderOutput,    // 输出设备标题
        HeaderAppVolume, // 应用音量标题
        DeviceItem,      // 设备项
        StreamItem,      // 应用音量项
        EmptyDeviceItem, // 空设备项
        EmptyStreamItem  // 空应用音量项
    };
    Q_ENUM(ItemType)

    // 内部数据结构
    struct SourceInfo
    {
        ItemType itemType = Invaild; //列表item类型 ItemType
        QString title = ""; // 标题文案
        QString cardName;  // 声卡名称
        QString cardDesc;  // 声卡描述
        QString portName;  // 设备端口名称
        QString portLabel; // 设备端口描述
        uint32_t priority; // 端口优先级
        quint32 direction; // 端口方向 input or output
        quint32 available; // 端口可用状态
        bool enabled = true;  // 端口是否启用
        int32_t type;  // 端口的类型
        QString portIcon = ""; //  设备图标
        bool isActivePort = false;
        friend const ClientDeviceInfo& operator>>(const ClientDeviceInfo& argument, SourceInfo& info) {
            info.itemType = DeviceItem;
            info.cardName = argument.cardName;
            info.cardDesc = argument.cardDesc;
            info.portName = argument.portName;
            info.portLabel = argument.portLabel;
            info.priority = argument.priority;
            info.direction = argument.direction;
            info.available = argument.available;
//            info.enabled = argument.enabled;
//            info.type = argument.type;
            return argument;
        }

        QString streamName;     // 流媒体名称
        quint32 streamIndex;    // 流媒体索引
        uint32_t streamVolume;        // 流媒体音量
        QString streamRole;     // 流媒体角色
        QString streamIconName; // 流媒体图标名称
        friend const StreamInfo& operator>>(const StreamInfo& argument, SourceInfo& info) {
            info.itemType = StreamItem;
            info.streamName = argument.name;
            info.streamIndex = argument.index;
            info.streamVolume = argument.volume;
            info.streamRole = argument.role;
            info.streamIconName = argument.iconName;
            return argument;
        }
    };

    static UKUIAudioSourceModel& getInstance();

    void initData();
    void initSlots();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int,QByteArray> roleNames() const override;

    void addDeviceHeader();
    void addDevice(ClientDeviceInfo deviceInfo);
    bool setDeviceActivePort(QString portName, QString cardName);
    void removeDevice(int index);
    void clearDevice();

    void addStreamHeader();
    void addStream(StreamInfo streamInfo);
    void removeStream(int index);
    void clearStream();
    int getStreamCount();

    void releaseModel();

private Q_SLOTS:

private:
    UKUIAudioSourceModel() = default;
    UKUIAudioSourceModel(const UKUIAudioSourceModel&) = delete;
    UKUIAudioSourceModel(UKUIAudioSourceModel&&) = delete;
    ~UKUIAudioSourceModel();

public:

private:
    QList<SourceInfo> m_sourceInfos;

};

#endif // UKUIAUDIOSOURCEMODEL_H

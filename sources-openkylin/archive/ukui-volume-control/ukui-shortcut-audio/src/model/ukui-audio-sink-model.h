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
#ifndef UKUIAUDIOSINKMODEL_H
#define UKUIAUDIOSINKMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include "DBusClient.h"

/**
 * 输出设备列表 Model（与应用音量 Model 分离）。
 */
class UKUIAudioSinkModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum SinkInfoRoles {
        ItemTypeRole = Qt::UserRole + 1,
        TitleRole,
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
        IsActivePortRole
    };
    Q_ENUM(SinkInfoRoles)

    enum ItemType {
        Invaild = -1,
        DeviceItem = 0,
        EmptyDeviceItem
    };
    Q_ENUM(ItemType)

    struct SinkInfo
    {
        ItemType itemType = Invaild;
        QString title;
        QString cardName;
        QString cardDesc;
        QString portName;
        QString portLabel;
        uint32_t priority = 0;
        quint32 direction = 0;
        quint32 available = 0;
        bool enabled = true;
        int32_t type = 0;
        QString portIcon;
        bool isActivePort = false;

        friend const ClientDeviceInfo& operator>>(const ClientDeviceInfo& argument, SinkInfo& info) {
            info.itemType = DeviceItem;
            info.cardName = argument.cardName;
            info.cardDesc = argument.cardDesc;
            info.portName = argument.portName;
            info.portLabel = argument.portLabel;
            info.priority = argument.priority;
            info.direction = argument.direction;
            info.available = argument.available;
            info.enabled = argument.enabled;
            info.type = 0;
            return argument;
        }

        friend const ClientDefaultDeviceInfo& operator>>(const ClientDefaultDeviceInfo& argument, SinkInfo& info) {
            info.itemType = DeviceItem;
            info.cardName = argument.cardName;
            info.cardDesc = argument.activePortDesc;
            info.portName = argument.activePortName;
            info.portLabel = tr("Multi Bluetooth Output");
            info.priority = -1;
            info.direction = 0;
            info.available = 0;
            info.enabled = 1;
            info.type = 0;
            return argument;
        }
    };

    static UKUIAudioSinkModel& getInstance();

    void initData();
    void initSlots();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int,QByteArray> roleNames() const override;

    void addEmptyDevice();
    void addDevice(ClientDeviceInfo deviceInfo);
    void addDevice(ClientDefaultDeviceInfo deviceInfo);
    QList<ClientDeviceInfo> getDeviceInfoList() const;
    bool updateDevice(const ClientDeviceInfo& deviceInfo);
    bool removeDevice(const QString& cardName, const QString& portName);
    bool hasEmptyDevice() const;
    bool removeEmptyDevice();
    bool setDeviceActivePort(QString portName, QString cardName);
    void removeDevice(int index);
    void clearDevice();
    int getDeviceCount();
    int getEnabledDeviceCount();
    void releaseModel();

private:
    UKUIAudioSinkModel() = default;
    UKUIAudioSinkModel(const UKUIAudioSinkModel&) = delete;
    UKUIAudioSinkModel(UKUIAudioSinkModel&&) = delete;
    ~UKUIAudioSinkModel();

    QList<SinkInfo> m_sinkInfos;
};

#endif // UKUIAUDIOSINKMODEL_H

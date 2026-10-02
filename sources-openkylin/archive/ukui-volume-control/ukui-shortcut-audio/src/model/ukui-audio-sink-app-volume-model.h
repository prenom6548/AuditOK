/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef UKUIAUDIOSINKAPPVOLUMEMODEL_H
#define UKUIAUDIOSINKAPPVOLUMEMODEL_H

#include <QAbstractListModel>
#include <QList>
#include <QString>
#include "DBusClient.h"

/**
 * 应用音量列表 Model（与设备 Model 分离）。
 * 一行对应一个应用：ItemInfo.streamMedia 为 ClientStreamMediaInfo，
 * mediaInfoList 内按 direction 区分输入/输出多路流。
 */
class UKUIAudioSinkAppVolumeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum StreamInfoRoles {
        ItemTypeRole = Qt::UserRole + 1,
        TitleRole,
        StreamNameRole,
        StreamIndexRole,
        StreamVolumeRole,
        StreamRole,
        StreamIconNameRole,
        StreamVolumeEnabledRole
    };
    Q_ENUM(StreamInfoRoles)

    enum ItemType {
        Invaild = -1,
        StreamItem = 0,
        EmptyStreamItem
    };
    Q_ENUM(ItemType)

    // 与 backend / addStream 信号一致：0=输入流(source-output)，1=输出流(sink-input)
    static constexpr uint32_t kStreamDirectionInput = 0;
    static constexpr uint32_t kStreamDirectionOutput = 1;

    struct ItemInfo
    {
        ItemType itemType = Invaild;
        QString title;
        // 一个 ClientStreamMediaInfo = 一个应用声音
        ClientStreamMediaInfo streamMedia;
        bool streamVolumeEnabled = true; // 应用音量滑条是否可调（录音中置灰）
    };

    static UKUIAudioSinkAppVolumeModel& getInstance();
    static bool shouldIgnoreStreamItem(const QString& iconName,
                                       const QString& descName,
                                       const QString& mediaRole = QString(),
                                       const QString& binary = QString());
    static bool shouldIgnoreStream(const ClientStreamMediaInfo& streamInfo);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void addEmptyStream();
    bool hasEmptyStream() const;
    bool removeEmptyStream();
    void addStream(const ClientStreamMediaInfo& streamInfo);
    QList<ClientStreamMediaInfo> getStreamMediaInfoList() const;
    bool updateStream(const ClientStreamMediaInfo& streamInfo, bool replaceMediaList = false);
    bool removeStreamByIndex(quint32 direction, quint32 streamIndex);
    void removeStream(int index);
    void clearStream();
    int getStreamCount();
    int getStreamIndexByStreamIconName(QString iconName);
    // 仅返回输出方向(sink-input)索引，供 setSinkInputVolume 使用
    QList<quint32> getStreamIndicesForIndex(quint32 streamIndex) const;
    int findStreamRowByIconName(const QString& iconName) const;
    bool setStreamVolumeEnabled(const QString& iconName, bool enabled);
    void releaseModel();

private:
    UKUIAudioSinkAppVolumeModel() = default;
    UKUIAudioSinkAppVolumeModel(const UKUIAudioSinkAppVolumeModel&) = delete;
    UKUIAudioSinkAppVolumeModel(UKUIAudioSinkAppVolumeModel&&) = delete;
    ~UKUIAudioSinkAppVolumeModel();

    QList<ItemInfo> m_items;
};

#endif // UKUIAUDIOSINKAPPVOLUMEMODEL_H

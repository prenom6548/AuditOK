/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "ukui-audio-sink-app-volume-model.h"
#include <QDebug>
#include <algorithm>

namespace {

const QStringList& streamBlackList()
{
    static const QStringList list = {
        QStringLiteral("sound-theme-player"),
        QStringLiteral("Remapped Stream"),
        QStringLiteral("ukui-control-center"),
        QStringLiteral("Echo-Cancel Sink Stream"),
        QStringLiteral("Echo-Cancel Source Stream"),
        QStringLiteral("kylin-device-daemon"),
        QStringLiteral("pacat"),
        QStringLiteral("ukui-powermanagement"),
        QStringLiteral("kylin-kmre-audio"),
        QStringLiteral("ukui-audio-service")
    };
    return list;
}

// UI/调音优先使用输出流；仅有输入流时回退到首条
const ClientMediaInfo* primaryDisplayMedia(const ClientStreamMediaInfo& streamMedia)
{
    for (const auto& media : streamMedia.mediaInfoList) {
        if (media.direction == UKUIAudioSinkAppVolumeModel::kStreamDirectionOutput) {
            return &media;
        }
    }
    return streamMedia.mediaInfoList.empty() ? nullptr : &streamMedia.mediaInfoList.front();
}

void upsertMedia(std::list<ClientMediaInfo>& list, const ClientMediaInfo& media)
{
    for (auto& existing : list) {
        if (existing.direction == media.direction && existing.index == media.index) {
            existing.volume = media.volume;
            existing.mute = media.mute;
            if (!media.name.isEmpty()) {
                existing.name = media.name;
            }
            return;
        }
    }
    list.push_back(media);
}

} // namespace

bool UKUIAudioSinkAppVolumeModel::shouldIgnoreStreamItem(const QString& iconName,
                                                         const QString& descName,
                                                         const QString& mediaRole,
                                                         const QString& binary)
{
    if (descName.contains("Loopback from") || descName.contains("Loopback to")
            || descName.contains("Simultaneous output on")) {
        return true;
    }

    if (mediaRole == QStringLiteral("filter")
        || mediaRole == QStringLiteral("abstract")
        || mediaRole == QStringLiteral("event")) {
        return true;
    }

    const auto& blacklist = streamBlackList();
    return blacklist.contains(iconName)
        || blacklist.contains(descName)
        || blacklist.contains(binary);
}

bool UKUIAudioSinkAppVolumeModel::shouldIgnoreStream(const ClientStreamMediaInfo& streamInfo)
{
    if (shouldIgnoreStreamItem(streamInfo.iconName, QString(), streamInfo.mediaRole, streamInfo.binary)) {
        return true;
    }
    for (const auto& media : streamInfo.mediaInfoList) {
        if (shouldIgnoreStreamItem(QString(), media.name)) {
            return true;
        }
    }
    return false;
}

UKUIAudioSinkAppVolumeModel& UKUIAudioSinkAppVolumeModel::getInstance()
{
    static UKUIAudioSinkAppVolumeModel instance;
    return instance;
}

int UKUIAudioSinkAppVolumeModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_items.size();
}

QVariant UKUIAudioSinkAppVolumeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size()) {
        return QVariant();
    }

    const ItemInfo& item = m_items.at(index.row());
    switch (role) {
    case ItemTypeRole: return item.itemType;
    case TitleRole: return item.title;
    case StreamNameRole: {
        const auto* media = primaryDisplayMedia(item.streamMedia);
        return media ? media->name : QString();
    }
    case StreamIndexRole: {
        const auto* media = primaryDisplayMedia(item.streamMedia);
        return media ? media->index : 0;
    }
    case StreamVolumeRole: {
        const auto* media = primaryDisplayMedia(item.streamMedia);
        return media ? media->volume : 0;
    }
    case StreamRole: return item.streamMedia.mediaRole;
    case StreamIconNameRole: return item.streamMedia.iconName;
    case StreamVolumeEnabledRole: return item.streamVolumeEnabled;
    default: return QVariant();
    }
}

QHash<int, QByteArray> UKUIAudioSinkAppVolumeModel::roleNames() const
{
    return {
        {ItemTypeRole, "itemType"},
        {TitleRole, "title"},
        {StreamNameRole, "streamName"},
        {StreamIndexRole, "streamIndex"},
        {StreamVolumeRole, "streamVolume"},
        {StreamRole, "streamRole"},
        {StreamIconNameRole, "streamIconName"},
        {StreamVolumeEnabledRole, "streamVolumeEnabled"}
    };
}

void UKUIAudioSinkAppVolumeModel::addEmptyStream()
{
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    ItemInfo item;
    item.itemType = EmptyStreamItem;
    item.title = QObject::tr("There are currently no applications that play sound");
    m_items.append(item);
    endInsertRows();
}

bool UKUIAudioSinkAppVolumeModel::hasEmptyStream() const
{
    for (const auto& item : m_items) {
        if (item.itemType == EmptyStreamItem) {
            return true;
        }
    }
    return false;
}

bool UKUIAudioSinkAppVolumeModel::removeEmptyStream()
{
    for (int index = m_items.size() - 1; index >= 0; --index) {
        if (m_items.at(index).itemType != EmptyStreamItem) {
            continue;
        }
        beginRemoveRows(QModelIndex(), index, index);
        m_items.removeAt(index);
        endRemoveRows();
        return true;
    }
    return false;
}

void UKUIAudioSinkAppVolumeModel::addStream(const ClientStreamMediaInfo& streamInfo)
{
    qDebug() << __func__ << "iconName:" << streamInfo.iconName << "binary:" << streamInfo.binary;
    if (shouldIgnoreStream(streamInfo) || streamInfo.mediaInfoList.empty()) {
        return;
    }

    // 与 audio UI 一致：按 iconName 判定同一应用
    if (findStreamRowByIconName(streamInfo.iconName) >= 0) {
        updateStream(streamInfo, false);
        return;
    }

    ItemInfo item;
    item.itemType = StreamItem;
    item.streamMedia = streamInfo;
    beginInsertRows(QModelIndex(), m_items.size(), m_items.size());
    m_items.append(item);
    endInsertRows();
}

QList<ClientStreamMediaInfo> UKUIAudioSinkAppVolumeModel::getStreamMediaInfoList() const
{
    QList<ClientStreamMediaInfo> list;
    for (const auto& item : m_items) {
        if (item.itemType == StreamItem) {
            list.push_back(item.streamMedia);
        }
    }
    return list;
}

bool UKUIAudioSinkAppVolumeModel::updateStream(const ClientStreamMediaInfo& streamInfo, bool replaceMediaList)
{
    for (int index = 0; index < m_items.size(); ++index) {
        auto& item = m_items[index];
        if (item.itemType != StreamItem) {
            continue;
        }
        if (item.streamMedia.iconName != streamInfo.iconName) {
            continue;
        }

        ClientStreamMediaInfo updated = item.streamMedia;
        if (!streamInfo.mediaRole.isEmpty()) {
            updated.mediaRole = streamInfo.mediaRole;
        }
        if (!streamInfo.iconName.isEmpty()) {
            updated.iconName = streamInfo.iconName;
        }
        if (!streamInfo.binary.isEmpty()) {
            updated.binary = streamInfo.binary;
        }

        if (replaceMediaList) {
            updated.mediaInfoList = streamInfo.mediaInfoList;
        } else {
            for (const auto& media : streamInfo.mediaInfoList) {
                upsertMedia(updated.mediaInfoList, media);
            }
        }

        item.streamMedia = updated;
        const QModelIndex idx = createIndex(index, 0, nullptr);
        emit dataChanged(idx, idx);
        return true;
    }
    return false;
}

bool UKUIAudioSinkAppVolumeModel::removeStreamByIndex(quint32 direction, quint32 streamIndex)
{
    for (int index = m_items.size() - 1; index >= 0; --index) {
        auto& item = m_items[index];
        if (item.itemType != StreamItem) {
            continue;
        }

        const auto before = item.streamMedia.mediaInfoList.size();
        item.streamMedia.mediaInfoList.remove_if([direction, streamIndex](const ClientMediaInfo& media) {
            return media.direction == direction && media.index == streamIndex;
        });
        if (item.streamMedia.mediaInfoList.size() == before) {
            continue;
        }
        if (item.streamMedia.mediaInfoList.empty()) {
            removeStream(index);
        } else {
            const QModelIndex idx = createIndex(index, 0, nullptr);
            emit dataChanged(idx, idx);
        }
        return true;
    }
    return false;
}

void UKUIAudioSinkAppVolumeModel::removeStream(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return;
    }
    if (m_items.at(index).itemType != StreamItem) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    m_items.removeAt(index);
    endRemoveRows();
}

void UKUIAudioSinkAppVolumeModel::clearStream()
{
    for (int index = m_items.size() - 1; index >= 0; --index) {
        if (m_items.at(index).itemType == StreamItem
                || m_items.at(index).itemType == EmptyStreamItem) {
            beginRemoveRows(QModelIndex(), index, index);
            m_items.removeAt(index);
            endRemoveRows();
        }
    }
}

int UKUIAudioSinkAppVolumeModel::getStreamCount()
{
    int count = 0;
    for (const auto& item : m_items) {
        if (item.itemType == StreamItem) {
            ++count;
        }
    }
    return count;
}

int UKUIAudioSinkAppVolumeModel::getStreamIndexByStreamIconName(QString iconName)
{
    const int row = findStreamRowByIconName(iconName);
    if (row < 0) {
        return -1;
    }
    const auto* media = primaryDisplayMedia(m_items.at(row).streamMedia);
    return media ? static_cast<int>(media->index) : -1;
}

QList<quint32> UKUIAudioSinkAppVolumeModel::getStreamIndicesForIndex(quint32 streamIndex) const
{
    for (const auto& item : m_items) {
        if (item.itemType != StreamItem) {
            continue;
        }

        const bool contains = std::any_of(item.streamMedia.mediaInfoList.begin(),
                                          item.streamMedia.mediaInfoList.end(),
                                          [streamIndex](const ClientMediaInfo& media) {
            return media.index == streamIndex;
        });
        if (!contains) {
            continue;
        }

        QList<quint32> outputIndices;
        for (const auto& media : item.streamMedia.mediaInfoList) {
            if (media.direction == kStreamDirectionOutput) {
                outputIndices.append(media.index);
            }
        }
        // 无输出流时返回空，避免把 source-output 索引误当 SINK_INPUT
        return outputIndices;
    }
    return {streamIndex};
}

int UKUIAudioSinkAppVolumeModel::findStreamRowByIconName(const QString& iconName) const
{
    for (int index = 0; index < m_items.size(); ++index) {
        if (m_items.at(index).itemType == StreamItem
                && m_items.at(index).streamMedia.iconName == iconName) {
            return index;
        }
    }
    return -1;
}

bool UKUIAudioSinkAppVolumeModel::setStreamVolumeEnabled(const QString& iconName, bool enabled)
{
    const int row = findStreamRowByIconName(iconName);
    if (row < 0) {
        return false;
    }

    auto& item = m_items[row];
    if (item.streamVolumeEnabled == enabled) {
        return true;
    }

    item.streamVolumeEnabled = enabled;
    const QModelIndex modelIndex = createIndex(row, 0, nullptr);
    emit dataChanged(modelIndex, modelIndex, {StreamVolumeEnabledRole});
    return true;
}

void UKUIAudioSinkAppVolumeModel::releaseModel()
{
    beginResetModel();
    m_items.clear();
    endResetModel();
}

UKUIAudioSinkAppVolumeModel::~UKUIAudioSinkAppVolumeModel()
{
    releaseModel();
}

/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#ifndef AUTOTESTS_WINDOWS_STUB_NOTIFICATION_MODEL_H
#define AUTOTESTS_WINDOWS_STUB_NOTIFICATION_MODEL_H

#include <QAbstractListModel>
#include <QDateTime>

namespace UkuiNotification {

struct TestNotificationData
{
    uint id = 0;
    QString appName;
    bool noFold = false;
    bool isStored = false;
    bool showOnLockScreen = true;
    QString summary;
    QString body;
    int urgency = 0;
    QDateTime createTime;
};

class NotificationItem
{
    Q_GADGET
public:
    enum Property {
        Id = 0,
        Display,
        AppName,
        AppIconName,
        Icon,
        Summary,
        Body,
        Category,
        Image,
        CreateTime,
        Actions,
        ActionState,
        HasDefaultAction,
        EnableActionIcons,
        SoundFile,
        SuppressSound,
        Resident,
        Transient,
        Urgency,
        Timout,
        NoFold,
        PopupTimeout,
        ToBeStored,
        IsStored,
        IsExpired,
        GroupIndex,
        GroupName,
        GroupCount,
        GroupIsExpand,
        ShowOnLockScreen,
        ShowContentOnLockScreen,
        ProcessedBody,
        DelayRemove
    };
    Q_ENUM(Property)
};

class NotificationModel : public QAbstractListModel
{
    Q_OBJECT
public:
    static NotificationModel *instance();

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void removeNotification(uint id);
    void setItems(const QList<TestNotificationData> &items);
    void reset();
    QList<uint> removedIds() const;
    int getUnreadMessageCount() const;
    void storePopupNotification(bool store);

private:
    explicit NotificationModel(QObject *parent = nullptr);

    QList<TestNotificationData> m_items;
    QList<uint> m_removedIds;

Q_SIGNALS:
    void unreadMessageCountChanged();
};

} // namespace UkuiNotification

#endif

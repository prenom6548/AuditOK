/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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

#ifndef UKUI_SIDEBAR_NOTIFICATION_MODEL_H
#define UKUI_SIDEBAR_NOTIFICATION_MODEL_H

#include <QAbstractListModel>

#include <popup-notification.h>
#include <notification-close-reason.h>
#include <optional>

#include "notification-utils.h"

namespace UkuiNotification {

class NotificationModel;
class NotificationModelPrivate;
class SingleApplicationSettings;

class NotificationItem
{
    Q_GADGET
    friend class NotificationModel;
    friend class NotificationModelPrivate;
public:
    enum Flag {
        TransientPopup        = 0x01,     // 默认flag,自动消失弹窗
        ResidentPopup         = 0x02,     // 常驻弹窗
        Stored                = 0x04,     // 已被存储
        Expired               = 0x08      // 已过期
    };
    Q_ENUM(Flag)
//    Q_DECLARE_FLAGS(Flags, Flag)

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
        /* 新属性 */
        ToBeStored,
        IsStored,
        IsExpired,
        GroupIndex, // 23
        GroupName,
        GroupCount,
        GroupIsExpand,
        ShowOnLockScreen,
        ShowContentOnLockScreen,
        ProcessedBody,
        /* 删除相关属性 */
        DelayRemove
    };
    Q_ENUM(Property)
//    Q_DECLARE_FLAGS(Propertys, Property)

    static QHash<int, QByteArray> roles();
    NotificationItem() = default;
    explicit NotificationItem(const PopupNotification& notification);

    uint id() const;

private:
    void updateActions();
    void setData(const PopupNotification &notification);

    NotificationCloseReason::CloseReason closeReason {NotificationCloseReason::Expired};
    qint32 flag {TransientPopup};
    quint64 generation {0};
    QList<NotificationAction> actions;
    PopupNotification data;

    bool isShowOnLockScreen {false};
    bool isShowContentOnLockScreen {false};
    bool isToBeStored {false};
    bool isDelayRemove {false};
};

class NotificationModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(QString linkActiveColor READ linkActiveColor WRITE setLinkActiveColor NOTIFY linkActiveColorChanged)
public:
    static NotificationModel *instance();

    Q_DISABLE_COPY_MOVE(NotificationModel)
    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    Q_INVOKABLE void closeNotification(uint id);
    Q_INVOKABLE void execAction(uint id, QString action = "");
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void removeNotification(uint id);
    Q_INVOKABLE bool setNotificationData(uint id, const QVariant& value, int role);

    QString linkActiveColor();
    void setLinkActiveColor(QString linkActiveColor);

    int getUnreadMessageCount();

Q_SIGNALS:
    void linkActiveColorChanged();
public Q_SLOTS:
    void storePopupNotification(bool isShow);

private Q_SLOTS:
    void onNotificationReceived(const UkuiNotification::PopupNotification &notification);
    void onNotificationClosed(uint id, UkuiNotification::NotificationCloseReason::CloseReason closeReason);
    void removeNotifications();

private:

    explicit NotificationModel(QObject *parent = nullptr);
    void init();

    // 查找消息在数组中的索引
    int findNotificationIndex(uint id) const;
    void initNewNotification(NotificationItem &item, SingleApplicationSettings *appSetting);
    void playSound(const QString& soundName);
    void playSoundFile(const QString& soundFile);

    // 通知数据更新
    void updateNotification(int row, const PopupNotification &notification, const SingleApplicationSettings *appSetting);
    void scheduleNotificationStateChange(NotificationItem &item);
    void scheduleNotificationStateChange(uint id, std::optional<QPair<int, int32_t>> priorityOperation, std::optional<QPair<int, int32_t>> delayOperation);
    void markAsStoredNotification(uint id);
    void markAsExpiredNotification(uint id);

    void checkNotificationItemChanged(NotificationItem &item, const PopupNotification &notification, QVector<int> &changeProperties);

    void addUnreadNotification(uint id);
    void removeUnreadNotification(uint id);
    QString updateNotificationBody(QString text) const;

private:
    NotificationModelPrivate *d { nullptr };
    bool m_sidebarVisible {false};
    QVector<uint> m_unreadNotifications;
    QString m_linkActiveColor;

Q_SIGNALS:
    void unreadMessageCountChanged();
};

} // Notification

//Q_DECLARE_METATYPE(UkuiNotification::NotificationItem::Property)

#endif //UKUI_SIDEBAR_NOTIFICATION_MODEL_H

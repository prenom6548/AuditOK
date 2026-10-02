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

#include <notification-client.h>
#include <applications-settings.h>
#include <notification-global-settings.h>

#include <QHash>
#include <QVector>
#include <QTimer>
#include <QDebug>
#include <QDBusMessage>
#include <QDBusConnection>
#include <QDBusPendingReply>

#include <mutex>

#include "notification-model.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QTextFragment>
#include "screen-monitor.h"
//声音播放音效dbus
#define UKUI_SOUNDTHEME_NAME          "org.ukui.sound.theme.player"
#define UKUI_SOUNDTHEME_PATH          "/org/ukui/sound/theme/player"
#define UKUI_SOUNDTHEME_INTERFACE     "org.ukui.sound.theme.player"
#define PLAY_SOUND_METHOD             "playAlertSound"


std::once_flag once_flag;

namespace UkuiNotification {

QHash<int, QByteArray> NotificationItem::roles()
{
    static QHash<int, QByteArray> roles;

    std::call_once(once_flag, [] {
        roles.insert(Id, "id");
        roles.insert(Display, "display");
        roles.insert(AppName, "appName");
        roles.insert(AppIconName, "appIconName");
        roles.insert(Icon, "icon");
        roles.insert(Summary, "summary");
        roles.insert(Body, "body");
        roles.insert(Category, "category");
        roles.insert(Image, "image");
        roles.insert(CreateTime, "createTime");
        roles.insert(Actions, "actions");
        roles.insert(ActionState, "actionState");
        roles.insert(HasDefaultAction, "hasDefaultAction");
        roles.insert(EnableActionIcons, "enableActionIcons");
        roles.insert(SoundFile, "soundFile");
        roles.insert(SuppressSound, "suppressSound");
        roles.insert(Resident, "resident");
        roles.insert(Transient, "transient");
        roles.insert(Urgency, "urgency");
        roles.insert(Timout, "timout");
        roles.insert(NoFold, "noFold");
        roles.insert(PopupTimeout, "popupTimeout");
        roles.insert(IsStored, "isStored");
        roles.insert(ToBeStored, "toBeStored");
        roles.insert(IsExpired, "isExpired");
        roles.insert(GroupName, "groupName");
        roles.insert(GroupCount, "groupCount");
        roles.insert(GroupIndex, "groupIndex");
        roles.insert(GroupIsExpand, "groupIsExpand");
        roles.insert(ShowOnLockScreen, "showOnLockScreen");
        roles.insert(ShowContentOnLockScreen, "showContentOnLockScreen");
        roles.insert(ProcessedBody, "processedBody");
        roles.insert(DelayRemove, "delayRemove");
    });

    return roles;
}

NotificationItem::NotificationItem(const PopupNotification& notification)
{
    setData(notification);
}

uint NotificationItem::id() const
{
    return data.id();
}

void NotificationItem::setData(const PopupNotification &notification)
{
    data = notification;
    updateActions();
}

void NotificationItem::updateActions()
{
    actions.clear();

    ActionList actionList(data.actions());
    QStringList actionState(data.actionState());

    for (int i = 0; i < actionList.size(); ++i) {
        NotificationAction action;

        action.setEnable(true);
        action.setDefault(data.hasDefaultAction() && actionList[i].first == "default");
        action.setIndex(i);
        action.setIcon(data.enableActionIcons() ? actionList[i].second : "");
        action.setName(actionList[i].second);
        action.setAction(actionList[i].first);
        action.setState(i < actionState.size() ? actionState[i] : "");

        actions.append(action);
    }
}

// ===== Private ======//
class NotificationModelPrivate
{
public:
    virtual ~NotificationModelPrivate();
    NotificationGlobalSettings *globalSettings {nullptr};
    // 消息客户端，收取消息
    NotificationClient *client {nullptr};

    // 全部消息记录
//    QVector<PopupNotification> notifications;
    QVector<NotificationItem> notifications;
    // 计时器，控制每条弹窗消息定时收起
    QHash<uint, QTimer*> notificationTimers;

    // 即将删除的消息Id列表
    QSet<uint> pendingRemovalIds;
    // 特殊处理：当一条通知进入等待移除的列表中，记录其 <id， generation>。
    // 通知没有真正删除前，后续如果同样id的一条通知又进来（比如应用重新发送），
    // 可以通过比较 generation 来判断是否应该忽略它
    QMap<uint, quint64> m_ignoredCloseGenerations;
    // 计时器，在计时器结束后，删除消息
    QTimer *pendingRemovalTimer {nullptr};

public:
    static quint64 nextGeneration();
    void deleteTimer(const uint &id);
    void prepareToDeleteNotification(const uint &id);
};

NotificationModelPrivate::~NotificationModelPrivate()
{
    qDeleteAll(notificationTimers);
    notificationTimers.clear();
}

quint64 NotificationModelPrivate::nextGeneration()
{
    static quint64 generation {1};
    return generation++;
}

void NotificationModelPrivate::deleteTimer(const uint &id)
{
    delete notificationTimers.take(id);
}

void NotificationModelPrivate::prepareToDeleteNotification(const uint &id)
{
    // 删除定时器
    deleteTimer(id);
    // 重启计时器，删除通知
    pendingRemovalTimer->stop();
    pendingRemovalIds.insert(id);
    pendingRemovalTimer->start();
}

// ===== M ====== //
NotificationModel *NotificationModel::instance()
{
    static NotificationModel instance;
    return &instance;
}

NotificationModel::NotificationModel(QObject *parent) : QAbstractListModel(parent), d(new NotificationModelPrivate)
{
    qRegisterMetaType<NotificationAction>("NotificationAction");
    init();
}

void NotificationModel::init()
{
    // 初始化timer
    d->pendingRemovalTimer = new QTimer(this);
    d->pendingRemovalTimer->setSingleShot(true);
    d->pendingRemovalTimer->setInterval(50);
    connect(d->pendingRemovalTimer, &QTimer::timeout, this, &NotificationModel::removeNotifications);

    // 通知全局设置
    d->globalSettings = new NotificationGlobalSettings(this);

    d->client = new NotificationClient(this);

    connect(d->client, &NotificationClient::newNotification, this, &NotificationModel::onNotificationReceived);
    connect(d->client, &NotificationClient::notificationClosed, this, &NotificationModel::onNotificationClosed);

    if (!d->client->registerClient()) {
        qWarning() << "register client failed.";
    }
}

int NotificationModel::findNotificationIndex(uint id) const
{
    auto it = std::find_if(d->notifications.constBegin(), d->notifications.constEnd(), [&id] (const NotificationItem &n) {
        return n.id() == id;
    });

    if (it == d->notifications.constEnd()) {
        return -1;
    }

    return std::distance(d->notifications.constBegin(), it);
}

void NotificationModel::initNewNotification(NotificationItem &item, SingleApplicationSettings *appSetting)
{
//        switch (appSetting->popupStyle()) {
//            case SettingsProperty::TransientPopup: {
//                if (item.data.popupTimeout() < 0) {
//                    item.flag = NotificationItem::ResidentPopup;
//                    break;
//                }
//
//                if (item.data.popupTimeout() > 0) {
//                    item.flag = NotificationItem::TransientPopup;
//                    break;
//                }
//
//                item.flag = NotificationItem::Stored;
//                break;
//            }
//            case SettingsProperty::ResidentPopup:
//                item.flag = NotificationItem::ResidentPopup;
//                break;
//            case SettingsProperty::NoPopup:
//            default:
//                item.flag = NotificationItem::Stored;
//        }

    if (item.data.popupTimeout() < 0) {
        item.flag = NotificationItem::ResidentPopup;

    } else if (item.data.popupTimeout() > 0) {
        item.flag = m_sidebarVisible ? NotificationItem::Stored : NotificationItem::TransientPopup;

    } else {
        item.flag = NotificationItem::Stored;
    }

    item.isShowOnLockScreen = appSetting->showNotificationOnLockScreen();
    item.isShowContentOnLockScreen = appSetting->showContentOnLockScreen();

    if (appSetting->allowSound() && !item.data.suppressSound()) {
        // TODO: 一段时间内只响一次
        // 响铃 ding ~
        if (item.data.soundFile().isEmpty()) {
            playSound(item.data.soundName().isEmpty() ? "notification-general" : item.data.soundName());

        } else {
            playSound("notification-general");
            // FIXME: 实现声音文件播放
//                playSoundFile(item.data.soundFile());
        }
    }
}

void NotificationModel::onNotificationReceived(const PopupNotification &notification)
{
    if (!d->globalSettings->receiveNotificationsFromApps() && !ApplicationsSettings::self()->isInAllowList(notification.desktopEntry())) {
        return;
    }

    SingleApplicationSettings *appSetting = ApplicationsSettings::self()->creatSettings(notification);
    if (!appSetting->allowNotify()) {
        return;
    }

    if (d->pendingRemovalIds.contains(notification.id())) {
        int index = findNotificationIndex(notification.id());
        if (index >= 0) {
            beginRemoveRows(QModelIndex(), index, index);
            d->deleteTimer(notification.id());
            removeUnreadNotification(notification.id());
            auto item = d->notifications.takeAt(index);
            d->m_ignoredCloseGenerations.insert(item.id(), item.generation);
            endRemoveRows();
            d->client->closeNotification(item.id(), item.closeReason);
        }
        d->pendingRemovalIds.remove(notification.id());
    }

    int index = findNotificationIndex(notification.id());
    if (index >= 0) {
        // 已经存在消息，更新消息
        updateNotification(index, notification, appSetting);
        return;
    }

    if (d->notifications.size() >= 500) {
        beginRemoveRows(QModelIndex(), 0, 0);
        d->deleteTimer(d->notifications.at(0).id());
        d->notifications.removeAt(0);
        endRemoveRows();

//        int count = d->notifications.size() - 500;
//        for (int i = count - 1; i >= 0; --i) {
//            beginRemoveRows(QModelIndex(), i, i);
//            d->notifications.removeAt(i);
//            endRemoveRows();
//        }
    }

    // TODO 查询应用设置，检查系统设置，响铃

    /*
     *   bool allowNotify() const;  y
     *   bool allowSound() const ;  y
     *   bool showContentOnLockScreen() const ;  n
     *   bool showNotificationOnLockScreen() const;  n
     *   SettingsProperty::Property popupStyle() const;  y
     */

    NotificationItem item(notification);
    item.generation = NotificationModelPrivate::nextGeneration();

    // 勿扰模式
    if (d->globalSettings->isDND()) {
        if (d->globalSettings->notifyAlarmWhileDND() && item.data.desktopEntry().contains("ukui-clock")) {
            initNewNotification(item, appSetting);
        } else {
            item.flag = NotificationItem::Stored;
            addUnreadNotification(item.id());
        }

    } else {
        initNewNotification(item, appSetting);
    }

    scheduleNotificationStateChange(item);

    // 插入数据
    beginInsertRows(QModelIndex(), d->notifications.size(), d->notifications.size());
    d->notifications.append(std::move(item));
    endInsertRows();
}

void NotificationModel::playSound(const QString& soundName)
{
    QDBusMessage message = QDBusMessage::createMethodCall(UKUI_SOUNDTHEME_NAME, UKUI_SOUNDTHEME_PATH, UKUI_SOUNDTHEME_INTERFACE, PLAY_SOUND_METHOD);
    message << soundName;

    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [] (QDBusPendingCallWatcher *self) {
        if (self) {
            QDBusPendingReply<QString, QByteArray> reply = *self;
            if (reply.isError()) {
                qWarning() << "playSound error:" << reply.error().message();
            }

            self->deleteLater();
        }
    });
}

void NotificationModel::playSoundFile(const QString &soundFile)
{
    // TODO: 播放自定义声音文件
}

void
NotificationModel::onNotificationClosed(uint id, UkuiNotification::NotificationCloseReason::CloseReason closeReason)
{
    int row = findNotificationIndex(id);
    if (row < 0) {
        d->m_ignoredCloseGenerations.remove(id);
        return;
    }

    if (d->m_ignoredCloseGenerations.contains(id) && d->m_ignoredCloseGenerations.value(id) != d->notifications[row].generation) {
        d->m_ignoredCloseGenerations.remove(id);
        return;
    }

    if (!m_sidebarVisible) {
        d->prepareToDeleteNotification(id);
    } else {
        NotificationItem &item = d->notifications[row];
        item.isDelayRemove = true;
        Q_EMIT dataChanged(index(row), index(row), {NotificationItem::DelayRemove});
    }
    // 插入数据
    // beginRemoveRows(QModelIndex(), index, index);
    // d->notifications.removeAt(index);
    // endRemoveRows();
}

void NotificationModel::updateNotification(int row, const PopupNotification &notification, const SingleApplicationSettings *appSetting)
{
    QVector<int> roles;
    NotificationItem &item = d->notifications[row];

    item.isShowOnLockScreen = appSetting->showNotificationOnLockScreen();
    item.isShowContentOnLockScreen = appSetting->showContentOnLockScreen();
    item.generation = NotificationModelPrivate::nextGeneration();

    checkNotificationItemChanged(item, notification, roles);
    item.setData(notification);
    Q_EMIT dataChanged(index(row), index(row), roles);
}

void NotificationModel::scheduleNotificationStateChange(NotificationItem& item)
{
    int expiredTimeout = item.data.timeout();
    int popupTimeout = item.data.popupTimeout();

    //定义通知定时执行的动作, {time: 定时时间， falg：定时结束执行的动作标志}
    using NotificationScheduledAction = std::optional<QPair<int, int32_t>>;
    //优先执行的定时动作
    NotificationScheduledAction priorityAction{std::nullopt};
    //延迟执行的定时动作
    NotificationScheduledAction delayAction{std::nullopt};

    //消息过期时间和消息被收藏时间，需要先定时短的
    if (item.flag & NotificationItem::TransientPopup) {
        const int delayTime = qAbs(expiredTimeout - popupTimeout);
        if (expiredTimeout > popupTimeout) {
            priorityAction = QPair<int, int32_t>{popupTimeout, NotificationItem::Stored};
            delayAction = QPair<int, int32_t>{delayTime, NotificationItem::Expired};
        } else if (expiredTimeout < popupTimeout) {
            if (expiredTimeout > 0) {
                priorityAction = QPair<int, int32_t>{expiredTimeout, NotificationItem::Expired};
                delayAction = QPair<int, int32_t>{delayTime, NotificationItem::Stored};
            } else {
                priorityAction = QPair<int, int32_t>{popupTimeout, NotificationItem::Stored};
            }
        } else {
            priorityAction = QPair<int, int32_t>{popupTimeout, NotificationItem::Stored | NotificationItem::Expired};
        }
    } else {
        //不弹窗，只标记过期
        if (expiredTimeout > 0) {
            priorityAction = QPair<int, int32_t>{expiredTimeout, NotificationItem::Expired};
        }
    }
    scheduleNotificationStateChange(item.id(), priorityAction, delayAction);
}

/**
 * 初始化某条消息的timer,
 * 如果已经存在timer，那就重新启动timer，将消息收起的时间延长
 * timer计时结束后，将允许收起的消息收起
 * @param id 消息id
 * @param timeout 超时时间，单位：毫秒
 * @param isExpired 在定时结束后，消息变为过期状态
 */

void NotificationModel::scheduleNotificationStateChange(uint id, std::optional<QPair<int, int32_t>> priorityAction, std::optional<QPair<int, int32_t>> delayAction)
{
    if (!priorityAction) {
        d->deleteTimer(id);
        return;
    }
    QTimer* timer = d->notificationTimers.value(id, nullptr);
    if (!timer) {
        timer = new QTimer();
        timer->setSingleShot(true);
        d->notificationTimers.insert(id, timer);
    }
    // timer的两种状态:
    // 1.作为弹窗计时器 (计时结束后收起弹窗)
    // 2.作为消息生命周期计时器 (计时结束后删除消息)
    auto timeoutHandler = [this, delayAction]()
    {
        const auto* timer = qobject_cast<QTimer*>(sender());
        uint id = timer->property("id").value<uint>();
        int32_t flag = timer->property("flag").value<int32_t>();
        if (flag & NotificationItem::Expired) {
            markAsExpiredNotification(id);
        }

        if (flag & NotificationItem::Stored) {
            markAsStoredNotification(id);
        }

        if (!delayAction) {
            d->deleteTimer(id);
            return;
        }
        scheduleNotificationStateChange(id, delayAction, std::nullopt);
    };
    timer->stop();
    timer->disconnect();
    connect(timer, &QTimer::timeout, this, timeoutHandler);

    timer->setInterval(priorityAction->first);
    timer->setProperty("id", id);
    timer->setProperty("flag", priorityAction->second);
    timer->start();
}

void NotificationModel::markAsStoredNotification(uint id)
{
    int idx = findNotificationIndex(id);
    if (idx < 0) {
        d->deleteTimer(id);
        return;
    }
    QVector<int> roles;
    NotificationItem& item = d->notifications[idx];
    if (item.flag & NotificationItem::TransientPopup) {
        if (Sidebar::ScreenMonitor::getInstance()->getScreenLockState() && !item.isShowOnLockScreen) {
            item.flag |= NotificationItem::Stored;
            roles.append(NotificationItem::IsStored);
        } else {
            item.isToBeStored = true;
            roles.append(NotificationItem::ToBeStored);
        }
        addUnreadNotification(item.id());
    }

    if (index(idx).isValid() && !roles.isEmpty()) {
        Q_EMIT dataChanged(index(idx), index(idx), roles);
    }
};


void NotificationModel::markAsExpiredNotification(uint id)
{
    int idx = findNotificationIndex(id);
    if (idx < 0) {
        d->deleteTimer(id);
        return;
    }

    QVector<int> roles;
    NotificationItem& item = d->notifications[idx];
    for (auto& action : item.actions) {
        action.setEnable(false);
    }
    roles.append(NotificationItem::Actions);

    item.flag |= NotificationItem::Expired;
    roles.append(NotificationItem::Expired);

    if (index(idx).isValid() && !roles.isEmpty()) {
        Q_EMIT dataChanged(index(idx), index(idx), roles);
    }
}

void NotificationModel::addUnreadNotification(uint id)
{
    if (!m_unreadNotifications.contains(id)) {
        m_unreadNotifications.append(id);
    }
    Q_EMIT unreadMessageCountChanged();
}

void NotificationModel::removeUnreadNotification(uint id)
{
    if (m_unreadNotifications.contains(id)) {
        m_unreadNotifications.removeAll(id);
    }
    Q_EMIT unreadMessageCountChanged();
}

void NotificationModel::checkNotificationItemChanged(NotificationItem &item, const PopupNotification &notification, QVector<int> &roles)
{
    if (item.data.display() != notification.display()){
        roles.append(NotificationItem::Display);
    }

    if (item.data.applicationName() != notification.applicationName()){
        roles.append(NotificationItem::AppName);
    }

    if (item.data.applicationIconName() != notification.applicationIconName()){
        roles.append(NotificationItem::AppIconName);
    }

    if (item.data.icon() != notification.icon()){
        roles.append(NotificationItem::Icon);
    }

    if (item.data.summary() != notification.summary()){
        roles.append(NotificationItem::Summary);
    }

    if (item.data.body() != notification.body()){
        roles.append(NotificationItem::Body);
        roles.append(NotificationItem::ProcessedBody);
    }

    if (item.data.category() != notification.category()){
        roles.append(NotificationItem::Category);
    }

    if (item.data.image() != notification.image()){
        roles.append(NotificationItem::Image);
    }

    if (item.data.createdTime() != notification.createdTime()){
        roles.append(NotificationItem::CreateTime);
    }

    if (item.data.actions() != notification.actions()){
        roles.append(NotificationItem::Actions);
    }

    if (item.data.actionState() != notification.actionState()){
        roles.append(NotificationItem::ActionState);
    }

    if (item.data.hasDefaultAction() != notification.hasDefaultAction()){
        roles.append(NotificationItem::HasDefaultAction);
    }

    if (item.data.enableActionIcons() != notification.enableActionIcons()){
        roles.append(NotificationItem::EnableActionIcons);
    }

    if (item.data.soundFile() != notification.soundFile()){
        roles.append(NotificationItem::SoundFile);
    }

    if (item.data.suppressSound() != notification.suppressSound()){
        roles.append(NotificationItem::SuppressSound);
    }

    if (item.data.resident() != notification.resident()){
        roles.append(NotificationItem::Resident);
    }

    if (item.data.transient() != notification.transient()){
        roles.append(NotificationItem::Transient);
    }

    if (item.data.urgency() != notification.urgency()){
        roles.append(NotificationItem::Urgency);
    }

    if (item.data.timeout() != notification.timeout()){
        roles.append(NotificationItem::Timout);
    }

    if (item.data.noFold() != notification.noFold()){
        roles.append(NotificationItem::NoFold);
    }

    if (item.data.popupTimeout() != notification.popupTimeout()){
        roles.append(NotificationItem::PopupTimeout);
    }
}

void NotificationModel::removeNotifications()
{
    if (d->pendingRemovalIds.isEmpty()) {
        return;
    }

    QVector<int> rows;
    rows.reserve(d->pendingRemovalIds.size());

    for (uint id : d->pendingRemovalIds) {
        int row = findNotificationIndex(id);
        if (row < 0) {
            continue;
        }
        d->notifications[row].isDelayRemove = false;
        rows.append(row);
    }

    if (rows.isEmpty()) {
        d->pendingRemovalIds.clear();
        return;
    }

    std::sort(rows.begin(), rows.end());

    // 准备删除row数据
    QVector<QPair<int, int>> rangeList;
    QPair<int, int> range {rows[0], rows[0]};

    for (int row : rows) {
        if (row > (range.second + 1)) {
            rangeList.append(range);
            range.first = row;
        }

        range.second = row;
    }

    if (rangeList.isEmpty() || (rangeList.last() != range)) {
        rangeList.append(range);
    }

    int removedRows = 0;
    for (int i = rangeList.count() - 1; i >= 0; --i) {
        QVector<QPair<uint, NotificationCloseReason::CloseReason>> pendingCloseNotifications;
        range = rangeList[i];
        beginRemoveRows(QModelIndex(), range.first, range.second);
        for (int j = range.second; j >= range.first; --j) {
            // close notification
            NotificationItem item = d->notifications.takeAt(j);
            pendingCloseNotifications.append({item.id(), item.closeReason});
            d->m_ignoredCloseGenerations.insert(item.id(), item.generation);
            removeUnreadNotification(item.id());
            ++removedRows;
        }
        endRemoveRows();
        for (const auto& notification : pendingCloseNotifications) {
            d->client->closeNotification(notification.first, notification.second);
        }
    }

    d->pendingRemovalIds.clear();
}


// ====== !!! Model oh ~ oh ~ oh ~ oh ~ oh ~ ======
int NotificationModel::rowCount(const QModelIndex &parent) const
{
    return d->notifications.count();
}

QVariant NotificationModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid)) {
        return {};
    }

    const NotificationItem &item = d->notifications[index.row()];
    switch (role) {
        case NotificationItem::Id:
            return item.id();
        case NotificationItem::Display:
            return item.data.display();
        case NotificationItem::AppName:
            return item.data.applicationName();
        case NotificationItem::AppIconName: {
            if (item.data.icon().isEmpty()) {
                if (item.data.applicationIconName().isEmpty()) {
                    return "application-x-desktop";
                }
                return item.data.applicationIconName();
            }
            return item.data.icon();
        }
        case NotificationItem::Icon:
            return item.data.icon();
        case NotificationItem::Summary:
            return item.data.summary();
        case NotificationItem::Body: {
            return item.data.body();
        }
        case NotificationItem::Category:
            return item.data.category();
        case NotificationItem::Image: {
            if (!item.data.image().isNull()) {
                return item.data.image();
            }

            return {};
        }
        case NotificationItem::CreateTime:
            return item.data.createdTime();
        case NotificationItem::Actions: {
            // for qt 5.12
            QVariantList list;
            for (const auto &ac : item.actions) {
                list.append(QVariant::fromValue(ac));
            }
            return list;
//            return QVariant::fromValue(item.actions);
        }
        case NotificationItem::EnableActionIcons:
            return item.data.enableActionIcons();
        case NotificationItem::SoundFile:
            return item.data.soundFile();
        case NotificationItem::SuppressSound:
            return item.data.suppressSound();
        case NotificationItem::Resident:
            return item.data.resident();
        case NotificationItem::Transient:
            return item.data.transient();
        case NotificationItem::Urgency:
            return item.data.urgency();
        case NotificationItem::Timout:
            return item.data.timeout();
        case NotificationItem::NoFold:
            return item.data.noFold();
        case NotificationItem::PopupTimeout:
            return item.data.popupTimeout();
        case NotificationItem::ToBeStored:
            return item.isToBeStored;
        case NotificationItem::IsStored:
            return item.flag & NotificationItem::Stored;
        case NotificationItem::IsExpired:
            return item.flag & NotificationItem::Expired;
        case NotificationItem::HasDefaultAction:
            return item.data.hasDefaultAction();
        case NotificationItem::ShowOnLockScreen:
            return item.isShowOnLockScreen;
        case NotificationItem::ShowContentOnLockScreen:
            return item.isShowContentOnLockScreen;
        case NotificationItem::ProcessedBody:
            return updateNotificationBody(item.data.body());
        case NotificationItem::DelayRemove:
            return item.isDelayRemove;
        default:
            break;
    }

    return {};
}

QHash<int, QByteArray> NotificationModel::roleNames() const
{
    return NotificationItem::roles();
}

bool NotificationModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() < 0 || index.row() >= d->notifications.size()) {
        return false;
    }
    NotificationItem &item = d->notifications[index.row()];
    if (role == NotificationItem::IsStored && value.toBool()) {
        if (!(item.flag & NotificationItem::Stored)) {
            item.flag = NotificationItem::Stored;
            item.isToBeStored = false;
            Q_EMIT dataChanged(index, index, {NotificationItem::IsStored, NotificationItem::ToBeStored});
            return true;
        }
    } else if (role == NotificationItem::DelayRemove) {
        item.isDelayRemove = value.toBool();
        if (!item.isDelayRemove) {
            closeNotification(index.data(NotificationItem::Id).toUInt());
        }
        Q_EMIT dataChanged(index, index, {NotificationItem::DelayRemove});
        return true;
    }
    return QAbstractListModel::setData(index, value, role);
}

void NotificationModel::closeNotification(uint id)
{
    int index = findNotificationIndex(id);
    if (index < 0) {
        return;
    }

    d->notifications[index].closeReason = NotificationCloseReason::DismissedByUser;
    d->prepareToDeleteNotification(id);
}

void NotificationModel::execAction(uint id, QString action)
{
    int index = findNotificationIndex(id);
    if (index < 0) {
        return;
    }

    NotificationItem &item = d->notifications[index];
    if (item.flag & NotificationItem::Expired) {
        item.closeReason = NotificationCloseReason::DismissedByUser;
        d->prepareToDeleteNotification(id);
        return;
    }

    d->client->invokeAction(id, action);
    if (!item.data.resident()) {
        item.closeReason = NotificationCloseReason::DismissedByUser;
        d->prepareToDeleteNotification(id);
    }
}

void NotificationModel::clearAll()
{
    auto count = d->notifications.size();
    if (count == 0) {
        return;
    }
    QVector<QPair<uint, NotificationCloseReason::CloseReason>> pendingCloseNotifications;
    beginRemoveRows(QModelIndex(), 0, count - 1);
    int i = count - 1;
    for (; i >= 0; --i) {
        d->notifications[i].closeReason = NotificationCloseReason::DismissedByUser;
        // prepareToDeleteNotification
        d->deleteTimer(d->notifications.at(i).id());
        NotificationItem item = d->notifications.takeAt(i);
        removeUnreadNotification(item.id());
        pendingCloseNotifications.append({item.id(), item.closeReason});
    }
    endRemoveRows();

    for (const auto& notification : pendingCloseNotifications) {
        d->client->closeNotification(notification.first, notification.second);
    }
}

void NotificationModel::removeNotification(uint id)
{
    int index = findNotificationIndex(id);
    if (index < 0) {
        return;
    }

    d->prepareToDeleteNotification(id);
}

bool NotificationModel::setNotificationData(uint id, const QVariant& value, int role)
{
    // GetModelIndex
    int row = findNotificationIndex(id);
    QModelIndex modelIndex;
    if (row < 0) {
        return false;
    }
    modelIndex = index(row, 0);
    if (!modelIndex.isValid() || modelIndex.parent().isValid()) {
        return false;
    }

    // SetData
    return setData(modelIndex, value, role);
}

QString NotificationModel::linkActiveColor()
{
    return m_linkActiveColor;
}

void NotificationModel::setLinkActiveColor(QString linkActiveColor)
{
    if (linkActiveColor == m_linkActiveColor) return;
    m_linkActiveColor = linkActiveColor;
    for (int i = 0; i < d->notifications.count(); i++) {
        Q_EMIT dataChanged(index(i), index(i), {NotificationItem::ProcessedBody});
    }
}

QString NotificationModel::updateNotificationBody(QString text) const
{
    if (Qt::mightBeRichText(text)) {
        QTextDocument doc;
        doc.setHtml(text);
        for (QTextBlock block = doc.begin(); block != doc.end(); block = block.next()) {
            for (QTextBlock::iterator item = block.begin(); item != block.end(); item++) {
                QTextFragment fragment = item.fragment();
                if (fragment.isValid()) {
                    QTextCharFormat format = fragment.charFormat();
                    if (!format.anchorHref().isEmpty()) {
                        int index = text.lastIndexOf(fragment.text());
                        if (index != -1) {
                            QString leftText = text.mid(0,index);
                            QString rightText = text.mid(index + fragment.text().length());
                            QString newText = leftText + (QString("<font color='%1'>").arg(m_linkActiveColor)) + fragment.text()
                                    + QString("</font>") + rightText;
                            return newText;
                        }
                    }
                }
            }
        }
    }
    return text;
}

int NotificationModel::getUnreadMessageCount()
{
    return m_unreadNotifications.length();
}

void NotificationModel::storePopupNotification(bool isShow)
{
    if (m_sidebarVisible == isShow) return;
    if (isShow) {
        for (int i = 0; i < d->notifications.length(); i++) {
            NotificationItem &item = d->notifications[i];
            if (item.flag == NotificationItem::TransientPopup) {
                item.flag = NotificationItem::Stored;
                Q_EMIT dataChanged(index(i), index(i), {NotificationItem::Stored});
            }
        }
        m_unreadNotifications.clear();
        Q_EMIT unreadMessageCountChanged();
    }
    m_sidebarVisible = isShow;
}


} // Notification

/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef UKUI_NOTIFICATION_NOTIFICATION_CLIENT_H
#define UKUI_NOTIFICATION_NOTIFICATION_CLIENT_H
#include "ukui-notification_global.h"
#include <QObject>
#include "popup-notification.h"
#include "notification-close-reason.h"
namespace UkuiNotification {
class NotificationClientPrivate;
/**
 * @short 通知中心客户端接口.
 * @author iaom
 */
class UKUINOTIFICATION_EXPORT NotificationClient : public  QObject
{
    Q_OBJECT
public:
    explicit NotificationClient(QObject *parent = nullptr);
    ~NotificationClient();
   /**
    * @brief 客户端注册，注册后可监听通知发送&关闭信号或调用通知动作执行以及主动关闭通知的接口。
    * @return 是否成功操作
    * @retval 成功：true；失败：false
    */
    bool registerClient();
    /**
     * @brief 注销当前客户端，注销后将不能再接收到新通知或通知关闭信号。
     * @return 是否成功操作
     * @retval 成功：true；失败：false
     */
    void unregisterClient();
    /**
     * @brief 关闭一条通知
     * @param id：通知id
     * @param reason：通知关闭原因
     * @return 是否成功操作
     * @retval 成功：true；失败：false
     */
    bool closeNotification(uint id, NotificationCloseReason::CloseReason reason);
    /**
     * @brief 执行动作
     * @param id： 通知id
     * @param action_key： action的唯一标识
     * @return 是否成功操作
     * @retval 成功：true；失败：false
     */
    bool invokeAction(uint id, const QString &action_key);
Q_SIGNALS:
    /**
     * 通知服务收到新通知时发送.
     */
    void newNotification(const UkuiNotification::PopupNotification &notification);
    /**
     * 通知被关闭时发送.
     */
    void notificationClosed(uint id, UkuiNotification::NotificationCloseReason::CloseReason closeReason);
private:
    NotificationClientPrivate *d = nullptr;
};

} // UkuiNotification

#endif //UKUI_NOTIFICATION_NOTIFICATION_CLIENT_H

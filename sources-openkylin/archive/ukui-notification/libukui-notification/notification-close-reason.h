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

#ifndef UKUI_NOTIFICATION_NOTIFICATIONCLOSEREASON_H
#define UKUI_NOTIFICATION_NOTIFICATIONCLOSEREASON_H


#include <QObject>
#include "ukui-notification_global.h"
namespace UkuiNotification {
/**
 * @short 通知被关闭的原因.
 * @author iaom
 */
class UKUINOTIFICATION_EXPORT NotificationCloseReason
{
    Q_GADGET
public:
    /**
     * The reason a notification was closed
     */
    enum CloseReason {
        /**
         * 通知过期.
         */
        Expired = 1,
        /**
         * 通知被用户关闭.
         */
        DismissedByUser = 2,
        /**
         * 通知被发送者通过调用CloseNotification接口关闭
         */
        Revoked = 3,
        /**
         * 未定义/保留原因
         */
        Undefined = 4,
    };
    Q_ENUM(CloseReason)
};
}
#endif //UKUI_NOTIFICATION_NOTIFICATIONCLOSEREASON_H
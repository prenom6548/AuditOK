/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "systemnotifiessender.h"
#include "loghelper.h"
#include <QDebug>

namespace
{
const char *enable_reousrce_limit_key = "0";
}

SystemNotifiesSender::SystemNotifiesSender(std::shared_ptr<ConfigManager> configManager)
    : m_configManager(configManager)
    , m_notifyDBusInterface(new NotifyDBusInterface)
    , m_notifyTimer(new QTimer)
    , m_resourceWarningNotifyId(0)
    , m_hasSendNotification(false)
{
    m_notifyTimer->setSingleShot(true);
    initConnections();
}

void SystemNotifiesSender::initConnections()
{
    m_notifyDBusInterface->setActionInvokedCallback([this](uint notifyId, const QString &actionKey) {
        handleResourceWarningActionInvoked(notifyId, actionKey.toStdString());
    });
    m_notifyDBusInterface->setNotifyCloseCallback([this](uint notifyId, uint reason) {
        handleResourceWarningNotifyClose(notifyId, reason);
    });
}

void SystemNotifiesSender::handleResourceWarningNotifyClose(uint notifyId, uint reason)
{
    if (notifyId != m_resourceWarningNotifyId)
        return;
    PRINT_NOTICE("Receive NotifyClose signal from notify: %d,  %d\n", notifyId, reason);

    m_resourceWarningNotifyId = 0;
    m_hasSendNotification = false;
    m_notifyTimer->start(60000); // after close notify, no more repeated notify within 60s.
}

void SystemNotifiesSender::handleResourceWarningActionInvoked(uint notifyId, const std::string &actionKey)
{
    if (notifyId != m_resourceWarningNotifyId)
        return;
    PRINT_NOTICE("Receive ActionInvoked signal from notify: %d, %s\n", notifyId, actionKey.c_str());

    if (actionKey == enable_reousrce_limit_key) {
        m_configManager->setResourceLimitEnabled(true);
        m_resourceWarningNotifyId = 0;
        m_hasSendNotification = false;
    }
}

void SystemNotifiesSender::sendResourceWarningNotify()
{
    // 发送请求后不一定会立刻被处理，避免重复发送请求
    if (m_hasSendNotification)
        return;
    if (m_notifyTimer->isActive()) {
        PRINT_NOTICE("Just close notify: %d after repeate.", m_notifyTimer->remainingTime());
        return;
    }

    const QString summary;
    const QString body =
        QObject::tr("The current system is detected to be stuck, we recommend that you enable the hierarchical freezing function");
    const QStringList actions = {enable_reousrce_limit_key, QObject::tr("Open")};
    const QVariantMap hints = {{QString("urgency"), QVariant::fromValue(QString("2"))},
                               {QString("x-ukui-popup-timeout"), QVariant::fromValue(-1)}};

    auto sendNotifyCallback = [this](uint notifyId, bool successful) {
        if (!successful) {
            PRINT_NOTICE("Send notification error.");
            m_hasSendNotification = false;
            return;
        }
        PRINT_NOTICE("Send notification: %d\n", notifyId);
        m_resourceWarningNotifyId = notifyId;
    };
    m_hasSendNotification = true;
    m_notifyDBusInterface->sendNotificationAsync(summary, body, actions, hints,
                                                 0, sendNotifyCallback);
}

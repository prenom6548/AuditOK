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

#ifndef UKUI_SIDEBAR_NOTIFICATION_WINDOW_H
#define UKUI_SIDEBAR_NOTIFICATION_WINDOW_H

#include <QQuickView>

#include "notification-group-model.h"
#include <shared-engine-view.h>
#include <window-helper.h>

class QScreen;

namespace UkuiNotification {

class NotificationCenterWindow : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
    Q_PROPERTY(bool contentVisible READ contentVisible WRITE setContentVisible NOTIFY contentVisibleChanged)
public:
    static NotificationGroupModel *globalGroupModel();
    explicit NotificationCenterWindow(QWindow *parent = nullptr);

    void init();

    bool contentVisible() const;
    void setContentVisible(bool contentVisible);

Q_SIGNALS:
    void contentVisibleChanged();

public Q_SLOTS:
    void activeNotificationCenter(bool active);

private Q_SLOTS:
    void onPrimaryScreenChanged(QScreen *newScreen);
    void updateGeometry();
    void callNotificationCenter(int posY);
    void callNotificationCenterEnd(int posX, int posY);

private:
    bool event(QEvent *event) override;

private:
    bool m_isTabletMode = false;
    bool m_contentVisible = false;
    UkuiQuick::WindowProxy *m_windowProxy {nullptr};
    QPointer<QScreen> m_screen {nullptr};
};

} // Notification

#endif //UKUI_SIDEBAR_NOTIFICATION_WINDOW_H

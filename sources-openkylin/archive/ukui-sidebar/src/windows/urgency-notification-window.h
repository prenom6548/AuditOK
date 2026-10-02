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
#ifndef URGENCYNOTIFICATIONWINDOW_H
#define URGENCYNOTIFICATIONWINDOW_H

#include <QObject>
#include <QQuickView>
#include <shared-engine-view.h>

#include "urgency-notification-model.h"

namespace UkuiNotification {
class UrgencyNotificationWindow : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
public:
    explicit UrgencyNotificationWindow(QWindow *parent = nullptr);

private:
    UrgencyNotificationModel *m_urgencyModel;
    void setWindowProperties();
    void setWindowPosition();

protected:
    bool event(QEvent *event) override;

private Q_SLOTS:
    void showUrgencyNotificationWindow();
    void closeUrgencyNotificationWindow();

};

} // Notification

#endif // URGENCYNOTIFICATIONWINDOW_H

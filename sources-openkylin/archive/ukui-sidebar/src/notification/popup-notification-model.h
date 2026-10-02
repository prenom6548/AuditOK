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

#ifndef UKUI_SIDEBAR_POPUP_NOTIFICATION_MODEL_H
#define UKUI_SIDEBAR_POPUP_NOTIFICATION_MODEL_H

#include <QSortFilterProxyModel>

namespace UkuiNotification {

class PopupNotificationModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit PopupNotificationModel(QObject *parent = nullptr);
    ~PopupNotificationModel() override = default;

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    bool m_isTabletMode {false};
    bool m_screenLockState {false};
    bool m_sessionToolsState {false};
};
}

#endif //UKUI_SIDEBAR_POPUP_NOTIFICATION_MODEL_H

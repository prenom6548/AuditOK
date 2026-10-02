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

#ifndef UKUI_SIDEBAR_POPUP_NOTIFICATION_WINDOW_H
#define UKUI_SIDEBAR_POPUP_NOTIFICATION_WINDOW_H

#include <QQuickView>
#include <QStringList>
#include <QVariant>
#include <shared-engine-view.h>

namespace UkuiQuick {
class WindowProxy;
}

namespace UkuiNotification {

class PopupNotificationWindow : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
    Q_PROPERTY(int itemWidth READ itemWidth NOTIFY itemWidthChanged)
    Q_PROPERTY(int viewWidth READ viewWidth NOTIFY viewWidthChanged)
    Q_PROPERTY(bool screenLockState READ screenLockState NOTIFY screenLockStateChanged)
public:
    explicit PopupNotificationWindow(QWindow *parent = nullptr);
    ~PopupNotificationWindow() = default;

    Q_INVOKABLE void updataWindowRegion(QVariantMap windowRect, int contentY);
    Q_INVOKABLE void updataGroupsPosition(int relativeY);
    Q_INVOKABLE void enableWindowBlur(bool enable);

    void updatePopupWindowGeometry(bool needDisplace, int sidebarWidth);
    void loadQML();

    int itemWidth() const;
    int viewWidth() const;
    bool screenLockState() const;

Q_SIGNALS:
    void itemWidthChanged();
    void viewWidthChanged();
    void screenLockStateChanged();

protected:
    bool event(QEvent *event) override;

private Q_SLOTS:
    void onPrimaryScreenChanged(QScreen *newScreen);
    void onRootWidthChanged();

private:
    void initWindow();
    void updateGeometry();
    void initNotificationModel();

private:
    bool m_needDisplace = false;
    bool m_enable = false;
    bool m_screenLockState = false;
    QPoint m_notificationPoint;
    QRegion m_windowRegion;

    int m_sidebarWidth = 400;
    int m_itemWidth = 374;
    int m_viewWidth = 374 + 8; // margin = 8

    UkuiQuick::WindowProxy *m_windowProxy {nullptr};
    QPointer<QScreen> m_screen {nullptr};
};
}
#endif //UKUI_SIDEBAR_POPUP_NOTIFICATION_WINDOW_H

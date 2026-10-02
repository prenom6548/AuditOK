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

#ifndef UKUI_SIDEBAR_TABLET_POPUP_VIEW_H
#define UKUI_SIDEBAR_TABLET_POPUP_VIEW_H

#include <QPoint>
#include <QQuickView>
#include <shared-engine-view.h>

#include "window-helper.h"

class QScreen;

namespace UkuiNotification {

class TabletPopupView : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
    Q_PROPERTY(int windowWidth READ windowWidth NOTIFY windowWidthChanged)
    Q_PROPERTY(int windowMaxHeight READ windowMaxHeight NOTIFY windowMaxHeightChanged)
    Q_PROPERTY(bool enableAnimation READ enableAnimation NOTIFY enableAnimationChanged)
public:
    explicit TabletPopupView(QWindow *parent = nullptr);
    void init();

    int windowWidth() const;
    int windowMaxHeight() const;
    bool enableAnimation() const;
    Q_INVOKABLE void enableWindowBlur(int radius, bool enable = true);

    Q_INVOKABLE void updateHeight(int height);

Q_SIGNALS:
    void windowWidthChanged();
    void windowMaxHeightChanged();
    void enableAnimationChanged();

private Q_SLOTS:
    void onPrimaryScreenChanged(QScreen *newScreen);
    void onScreenGeometryChanged();
    void updateGeometry();

private:
    bool event(QEvent *event) override;

private:
    bool m_enableAnimation = {true};
    int m_windowMargin = 12;
    int m_windowWidth = 0;

    QPoint m_topLeft;
    UkuiQuick::WindowProxy* m_windowProxy {nullptr};
    QPointer<QScreen> m_screen {nullptr};
};

} // Notification

#endif //UKUI_SIDEBAR_TABLET_POPUP_VIEW_H

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

#ifndef UKUI_SIDEBAR_SIDEBAR_VIEW_H
#define UKUI_SIDEBAR_SIDEBAR_VIEW_H

#include "window-type.h"
#include "window-helper.h"
#include <shared-engine-view.h>
#include <QPointer>

namespace Sidebar {

class WindowBlurHelper;

class SidebarView : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
    Q_PROPERTY(bool contentVisible READ contentVisible WRITE setContentVisible NOTIFY contentVisibleChanged)
    Q_PROPERTY(bool isNotificationEmpty READ isNotificationEmpty WRITE setIsNotificationEmpty NOTIFY isNotificationEmptyChanged)
    Q_PROPERTY(bool isWindowFold READ isWindowFold WRITE setIsWindowFold NOTIFY isWindowFoldChanged)
    Q_PROPERTY(bool isTabletMode READ isTabletMode NOTIFY isTabletModeChanged)
    Q_PROPERTY(qint32 radius READ radius WRITE setRadius NOTIFY radiusChanged)
    Q_PROPERTY(qint32 windowPadding READ windowPadding NOTIFY windowPaddingChanged)
    Q_PROPERTY(qint32 windowExpandPadding READ windowExpandPadding WRITE setWindowExpandPadding NOTIFY windowExpandPaddingChanged)
    Q_PROPERTY(qint32 minimumThreshold READ minimumThreshold NOTIFY minimumThresholdChanged)
    Q_PROPERTY(qint32 primaryScreenRight READ primaryScreenRight NOTIFY primaryScreenRightChanged)
    Q_PROPERTY(qint32 minWindowSize READ minWindowSize NOTIFY minWindowSizeChanged)
    Q_PROPERTY(qint32 contentWidth READ contentWidth)

public:
    explicit SidebarView(QWindow *parent = nullptr);
    void init();

    bool isTabletMode();
    bool contentVisible() const;
    void setContentVisible(bool visible);
    bool isNotificationEmpty() const;
    void setIsNotificationEmpty(bool isEmpty);
    bool isWindowFold() const;
    void setIsWindowFold(bool isFold);
    int windowExpandPadding() const;
    void setWindowExpandPadding(int padding);

    qint32 radius() const;
    void setRadius(int radius);
    qint32 windowPadding() const;
    qint32 minimumThreshold() const;
    qint32 primaryScreenRight() const;
    qint32 minWindowSize() const;

    Q_INVOKABLE void setBlurHeight(int height);
    qint32 contentWidth() const;

Q_SIGNALS:
    void isTabletModeChanged();
    void contentVisibleChanged();
    void isNotificationEmptyChanged();
    void isWindowFoldChanged();
    void radiusChanged();
    void windowPaddingChanged();
    void windowExpandPaddingChanged();
    void minimumThresholdChanged();
    void primaryScreenRightChanged();
    void minWindowSizeChanged();
    void requestShowContent();
    void requestHideContent();

public Q_SLOTS:
    void activeWindow(bool active);
    void setWindowVisible(bool visible);

protected:
    bool event(QEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void exposeEvent(QExposeEvent *event) override;

private Q_SLOTS:
    void updateWindowGeometry();
    void onSystemModeChanged();

private:
    void updateGeometryOfWindow();
    void updateGeometryOfMask();

private:
    bool m_isTabletMode {false};
    bool m_contentVisible {false};
    bool m_isNotificationEmpty {true};
    bool m_isWindowFold {true};
    qint32 m_radius = 12;
    qint32 m_blurHeight = 144;
    qint32 m_padding = 8;
    qint32 m_expandPadding = 0;
    qint32 m_minimumThreshold = 100;
    qint32 m_primaryScreenRight = 0;
    qint32 m_minWindowSize = 160;
    qint32 m_contentWidth = 0;
    QRect m_windowGeometry = QRect(0,0,0,0);
    QRect m_maskGeometry = QRect(0,0,0,0);
    WindowBlurHelper *m_windowBlurHelper {nullptr};
    UkuiQuick::WindowProxy* m_windowProxy {nullptr};
    QPointer<QScreen> m_screen {nullptr};
};

} // Sidebar

#endif //UKUI_SIDEBAR_SIDEBAR_VIEW_H

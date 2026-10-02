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

#ifndef UKUI_SIDEBAR_SHORTCUTS_WINDOW_H
#define UKUI_SIDEBAR_SHORTCUTS_WINDOW_H

#include <shared-engine-view.h>
#include <window-helper.h>

class QScreen;

namespace Sidebar {
class WindowBlurHelper;
}

class ShortcutsWindow : public UkuiQuick::SharedEngineView
{
    Q_OBJECT
    Q_PROPERTY(bool isTabletMode READ isTabletMode NOTIFY tabletModeChanged)
    Q_PROPERTY(int windowMargin READ windowMargin NOTIFY windowMarginChanged)
    Q_PROPERTY(int panelPos READ panelPos NOTIFY panelPosChanged)
    Q_PROPERTY(QPoint position MEMBER m_position NOTIFY positionChanged)
    Q_PROPERTY(QString currentId READ currentId WRITE setCurrentId NOTIFY currentIdChanged)
public:
    explicit ShortcutsWindow(QWindow *parent = nullptr);
    bool isTabletMode() const;
    int windowMargin() const;
    int panelPos() const;

    QString currentId() const;
    void setCurrentId(QString currentId);

    void activeShortcutsWindow(bool active = true);

    bool requestMenuWidget(QString widgetId = "", bool showReturnButton = false);
    void backToShortcuts();

    Q_INVOKABLE void requestPanelFocusRestore();

    Q_INVOKABLE static bool inTargetCircle(int x, int y, int cx, int cy, int r);
    Q_INVOKABLE static bool inTargetRect(int x, int y, QRectF rect);

public Q_SLOTS:
    void setBlurStrength();

Q_SIGNALS:
    void tabletModeChanged();
    void windowMarginChanged();
    void panelPosChanged();
    void currentIdChanged();

    void showMenuWidget(QString widgetId, bool showReturnButton, QString returnButtonName);
    void hideMenuWidget();
    void positionChanged();

private Q_SLOTS:
    void updateGeometry();
    void onTabletModeChanged();
    void moveShortcutPanel(int distance);
    void onRight2LeftReleased(int posX, int posY);

protected:
    bool event(QEvent *event) override;

private:
    void initUI();
    void initSettings();
    void initConnections();
    void updateEffects();
    void restorePanelFocus();
    void clearPanelFocusReturn();
    void callPanelMethod(const char *method);

    bool m_isTabletMode {false};
    int m_panelPos{4};
    int m_panelSize{48};
    int m_panelType{0};
    int m_settingsIslandPosition{0};
    int m_topbarSize{0};
    QPoint m_position{0,0};
    QString m_currentId{""};
    Qt::LayoutDirection m_layoutDirection{Qt::LayoutDirection::LeftToRight};
    UkuiQuick::WindowProxy *m_windowProxy {nullptr};
    QPointer<QScreen> m_screen {nullptr};
    bool m_restorePanelFocusOnHide {false};
};


#endif //UKUI_SIDEBAR_SHORTCUTS_WINDOW_H

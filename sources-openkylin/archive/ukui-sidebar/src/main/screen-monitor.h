/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

//
// Created by hxf on 22-8-4.
//

#ifndef UKUI_SIDEBAR_SCREEN_MONITOR_H
#define UKUI_SIDEBAR_SCREEN_MONITOR_H

#define UKUI_PANEL_SETTING          "org.ukui.panel.settings"
#define UKUI_PANEL_POSITION_KEY     "panelposition"
#define UKUI_PANEL_SIZE_KEY         "panelsize"

#include <QScreen>
#include <QSize>
#include <QGSettings>
#include <QPointer>

namespace Sidebar {

/**
 * 用于监听主屏幕的变化
 * 获取主屏幕坐标，尺寸等
 */
class ScreenMonitor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QScreen* primaryScreen READ getPrimaryScreen NOTIFY primaryScreenChanged)
    Q_PROPERTY(QRect geometry READ getGeometry NOTIFY geometryChanged)
public:
    static ScreenMonitor* getInstance();
    Q_INVOKABLE int getPanelPosition();
    Q_INVOKABLE int getPanelSize();

    bool getScreenLockState();
    bool getSessionToolsState();

public Q_SLOTS:
    qreal screenRatio();
    QScreen *getPrimaryScreen();
    QRect getGeometry();
    Qt::LayoutDirection getLayoutDirection();
    void primaryScreenChangedSlot(QScreen *screen);
    void screenRemovedSlot(QScreen *screen);
    void screenAddedSlot(QScreen *screen);
    void panelPropertyChangedSlot(const QString& key);
    void layoutDirectionChangedSlot(Qt::LayoutDirection direction);
    void screenLockSlot();
    void screenUnLockSlot();
    void sessionToolsStateChangedSlot(bool visible);

Q_SIGNALS:
    void primaryScreenChanged();
    void geometryChanged(const QRect &geometry);
    // 任务栏的位置和大小变化
    void panelPropertyChanged();
    void layoutDirectionChanged();
    void screenLockStateChanged(bool isLocked);
    void sessionToolsStateChanged(bool visible);
    void screenRemoved(QScreen *screen);

private:
    explicit ScreenMonitor(QObject *parent = nullptr);
    void initPanelMonitor();
    void initScreenSaver();

    /**
     * 任务栏位置与屏幕：上: 1, 下: 0, 左: 2, 右: 3
     * 如果为其他值，则说明任务栏不存在
     */
    typedef struct {
        int position;
        int size;
    } PanelProperty;

private:
    PanelProperty m_panelProperty {4, 0};
    QPointer<QScreen> m_primaryScreen = nullptr;
    QGSettings *m_panelSetting = nullptr;
    Qt::LayoutDirection m_direction = Qt::LayoutDirection::LeftToRight;
    bool m_screenLockState = false;
    bool m_sessionToolsState = false;
};
}

#endif //UKUI_SIDEBAR_SCREEN_MONITOR_H

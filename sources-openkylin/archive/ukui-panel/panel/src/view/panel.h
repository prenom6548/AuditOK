/*
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: hxf <hewenfei@kylinos.cn>
 *          iaom <zhangpengfei@kylinos.cn>
 */

#ifndef UKUI_PANEL_PANEL_H
#define UKUI_PANEL_PANEL_H

#include <QQuickView>
#include <QScreen>
#include <QRect>
#include <QTimer>
#include <types.h>
#include <QGSettings>
#include <window-helper.h>
#include <island-view.h>
#include <QLoggingCategory>
#include <QJsonObject>

#include "screens-manager.h"
#include "general-config-define.h"
#include "common.h"

class QAction;

namespace UkuiPanel {
Q_DECLARE_LOGGING_CATEGORY(PANEL_PANEL)
class Panel : public UkuiQuick::IslandView
{
    Q_OBJECT
    Q_PROPERTY(bool lockPanel READ lockPanel NOTIFY lockPanelChanged)
    Q_PROPERTY(bool enableCustomSize READ enableCustomSize NOTIFY enableCustomSizeChanged)
    Q_PROPERTY(bool disableMouseLeftButton READ disableMouseLeftButton NOTIFY disableMouseLeftButtonChanged)
    Q_PROPERTY(bool disableMouseRightButton READ disableMouseRightButton NOTIFY disableMouseRightButtonChanged)
    Q_PROPERTY(bool disableMouseHover READ disableMouseHover NOTIFY disableMouseHoverChanged)
    Q_PROPERTY(bool disableContextMenu READ disableContextMenu NOTIFY disableContextMenuChanged)
public:
    Panel(Screen *screen, const QString &id, QWindow *parent = nullptr);
    ~Panel() override;
    void setPanelScreen(Screen *screen);

    bool lockPanel() const;
    bool enableCustomSize() const;
    Q_INVOKABLE void changeCursor(const QString &type);
    Q_INVOKABLE void changePanelPos(UkuiQuick::Types::Pos pos);
    Q_INVOKABLE void startResizeCustomPanelSize(const Qt::Edges &edge);
    Q_INVOKABLE void endResizeCustomPanelSize();
    bool disableMouseLeftButton() const;
    bool disableMouseRightButton() const;
    bool disableMouseHover() const;
    bool disableContextMenu() const;

Q_SIGNALS:
    void lockPanelChanged();
    void enableCustomSizeChanged();
    void disableMouseLeftButtonChanged();
    void disableMouseRightButtonChanged();
    void disableMouseHoverChanged();
    void disableContextMenuChanged();

protected:
    bool event(QEvent *e) override;

public Q_SLOTS:
    /**
     * @brief primaryScreenChanged 主屏改变时修改托盘的显示状态
     * @param screen 改变之后的主屏
     * @return
     */
    void primaryScreenChanged(Screen *screen);
    //锁屏时隐藏任务栏，解锁时恢复（独立于自动隐藏状态机）
    void setScreenLockHidden(bool hidden);
    void setShellHidden(bool hidden);
private Q_SLOTS:
    void onScreenGeometryChanged(const QRect &geometry);
    void onConfigChanged(const QString &key);
    void setPosition(UkuiQuick::Types::Pos position, bool updateConfig = true);
    void setPanelSize(int size, bool updateConfig = true);
    void setPanelPolicy(GeneralConfigDefine::PanelSizePolicy sizePolicy, bool updateConfig = true);
    void resetWorkArea(bool cut);

    void setLockPanel(bool locked, bool updateConfig = true);
    // widget opt
    void widgetActionTriggered(const QAction *);
    void disableWidget(const QString &id, bool disable);

    void updateHideState();
    void setAutoHide(bool autoHide, bool updateConfig = true);
    void setHidden(bool hidden);
    void activeHideTimer(bool active = true);
    void onFunctionControlChanged(const QString& name, bool exit = false);

private:
    void initConfig();
    void initPanelConfig();
    void loadWidgetsConfig();
    void updateGeometry();
    /**
     * 根据panel的大小，计算窗口大小、窗口中panel的对应区域
     * @param size
     */
    void calculatePanelRectFromSize(int size);
    /**
     * 更新panel窗口尺寸限制（最大、最小宽高）
     */
    void updateLimitSize();
    /**
     * 调用m_windowProxy接口，设置窗口属性（geometry）
     */
    void setGeometry();
    void initGsettings();

    void loadActions();
    void initSizeAction();
    void updateSizeAction();
    void initPositionAction();
    void updatePositionAction();
    void initLockPanelAction();
    void updateLockPanelAction();
    void updateMask();
    void applyWindowVisibility();
    void initIsland();
    bool windowSuppressed() const;
    //初始化系统托盘是否显示
    void setShowSystemTrayStatus();
    void setGSettings(const QString &key, const QVariant &value);
    void setShowSystemTrayOnAllPanels(bool isShow);
    void setControlHidden(bool hidden);
private:
    int m_panelSize {48};
    int m_panelPosition {0};// 任务栏位置与屏幕：上: 1, 下: 0, 左: 2, 右: 3, 如果为其他值，则说明任务栏不存在
    QRect m_windowRegion;
    GeneralConfigDefine::PanelSizePolicy m_sizePolicy {GeneralConfigDefine::Small};
    QString m_id;
    Screen *m_screen {nullptr};

    QAction *m_sizeAction {nullptr};
    QAction *m_positionAction {nullptr};
    QAction *m_lockPanelAction {nullptr};
    QAction *m_autoHideAction {nullptr};
    QList<QAction*> m_widgetActions;
    QStringList m_disabledWidgets;

    bool m_startUp = true;//任务栏是否正在启动
    bool m_isHidden = false;//当前是否已经隐藏
    bool m_autoHide = false;//是否自动隐藏(鼠标离开任务栏区域即隐藏)
    bool m_isResizingPanel = false;// 任务栏是否正在被拖拽改变大小
    bool m_lockPanel = false;
    WindowSuppressionState m_windowSuppression;
    QTimer *m_hideTimer = nullptr;
    UnavailableArea m_unavailableArea;
    UkuiQuick::WindowProxy *m_windowProxy = nullptr;
    QGSettings *m_settings = nullptr;
    bool m_containsMouse = false;
    int m_panelMaxSize = 92;
    int m_panelMinSize = 48;
    int m_systemTrayInstance = -1;
    QString m_lastConfigVersion;
    QString m_lastPanelConfigVersion;
    //管控配置
    bool m_disableMouseLeftButton = false;
    bool m_disableMouseRightButton = false;
    bool m_disableMouseHover = false;
    bool m_disableContextMenu = false;
    QStringList m_controlDisableWidgets;
    QStringList m_controlEnableWidgets;
};

} // UkuiPanel

Q_DECLARE_METATYPE(QList<int>)

#endif //UKUI_PANEL_PANEL_H

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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef UKUI_PANEL_PANEL_NEXT_H
#define UKUI_PANEL_PANEL_NEXT_H
#include <QTimer>
#include <QMenu>
#include <types.h>
#include <QGSettings>
#include <island-view.h>
#include <QLoggingCategory>
#include <window-helper.h>
#include <widget-container.h>

#include "common.h"
#include "screens-manager.h"
#include "blur-region-helper.h"
#include "general-config-define.h"

namespace UkuiPanel {

Q_DECLARE_LOGGING_CATEGORY(PANEL_PANELNEXT)

class AutoHideHelper : public QObject
{
    Q_OBJECT

public:
    explicit AutoHideHelper(Screen *screen, QObject* parent = nullptr);
    void setScreen(Screen *screen);
    bool needAutoHide();

private:
    void update();
    Screen *m_screen = nullptr;
    bool m_needAutoHide = false;
    QString m_activeWindow;

Q_SIGNALS:
    void autoHideStateChanged(bool);
};
class PanelNext : public UkuiQuick::IslandView
{
    Q_OBJECT
    Q_PROPERTY(bool lockPanel READ lockPanel NOTIFY lockPanelChanged)
    Q_PROPERTY(bool enableCustomSize READ enableCustomSize NOTIFY enableCustomSizeChanged)

public:
    PanelNext(Screen* screen, const QString&id, QWindow* parent = nullptr);
    ~PanelNext();

    void setPanelScreen(Screen* screen);
    bool lockPanel() const;
    bool enableCustomSize() const;
    void addIsland(const QString &id);
    void removeIsland(const QString &id);
    void dragRemove(const QString &id);
    void dragAdding(const QString &id, bool addWhenDrop);
    bool addCustomWidget(const QString &id, int instanceId = -1);
    bool removeCustomWidget(const QString &id, int instanceId = -1);
    //锁屏时隐藏任务栏，解锁时恢复（独立于自动隐藏状态机）
    void setScreenLockHidden(bool hidden);
    void setShellHidden(bool hidden);

    Q_INVOKABLE void setPanelLength(int length);
    Q_INVOKABLE void endResizeCustomPanelSize();
    Q_INVOKABLE void changeCursor(const QString &type);
    Q_INVOKABLE void startResizeCustomPanelSize(const Qt::Edges &edge);

Q_SIGNALS:
    void lockPanelChanged();
    void enableCustomSizeChanged();
    void islandRemoved(const QString &id);
    void customWidgetAdded(const QString &id, int instanceId);
    void customWidgetRemoved(const QString &id, int instanceId);

protected:
    bool event(QEvent* e) override;

private Q_SLOTS:
    void onConfigChanged(const QString &key);

    void onScreenGeometryChanged(const QRect&geometry);

    void setPanelPolicy(GeneralConfigDefine::PanelSizePolicy sizePolicy, bool updateConfig = true);

    void setPosition(UkuiQuick::Types::Pos position, bool updateConfig = true);

    void setPanelSize(int size, bool updateConfig = true);

    void setLockPanel(bool locked, bool updateConfig = true);

    void setAutoHide(bool autoHide, bool updateConfig = true);

    void setHidden(bool hidden);


    void activeHideTimer(bool active = true);

    void onBlurRegionChanged(QRegion region);

    void changeForm(const QString &id, GeneralConfigDefine::IslandsForm form);

    void updateHideState();

private:
    void initGSettings();

    void initMainContainer();

    void initIslandsConfig();

    void updateGeometry();

    void initConfig();

    /**
     * 根据panel的大小，计算窗口大小、窗口中panel的对应区域
     * @param size
     */
    void calculatePanelRectFromSize(int panelSize);

    /**
     * 更新panel窗口尺寸限制（最大、最小宽高）
     */
    void updateLimitSize();

    /**
     * 调用m_windowProxy接口，设置窗口属性（geometry）
     */
    void setGeometry();

    void updateMask();
    void applyWindowVisibility();
    bool windowSuppressed() const;

    void initPanelConfig();

    void loadActions();

    void initSizeAction();

    void updateSizeAction();

    void initPositionAction();

    void updatePositionAction();

    void initIslandFormAction();

    void updateIslandFormAction();

    void disableWidget(const QString &id, bool disable);

    void updateDisableWidget();

    void setGSettings(const QString &key, const QVariant &value);

private:
    QString m_id;
    QRect m_windowRegion;
    UkuiQuick::WindowProxy* m_windowProxy = nullptr;
    Screen* m_screen{nullptr};
    int m_panelSize{56};
    int m_panelPosition{0};
    GeneralConfigDefine::PanelSizePolicy m_sizePolicy{GeneralConfigDefine::Small};

    QAction* m_sizeAction{nullptr};
    QAction* m_lockPanelAction{nullptr};
    QAction* m_autoHideAction{nullptr};
    QAction* m_dataIslandFormAction{nullptr};
    QAction* m_settingsIslandFormAction{nullptr};

    bool m_isHidden = false;//当前是否已经隐藏
    bool m_autoHide = false;//是否自动隐藏(鼠标离开任务栏区域即隐藏)
    bool m_needHide = false;//当前是否有激活的最大化窗口需要任务栏隐藏
    bool m_lockPanel = false;
    WindowSuppressionState m_windowSuppression;
    bool m_startUp = true;
    bool m_isResizingPanel = false;//当前任务栏是否在拖拽改变大小
    QTimer* m_hideTimer = nullptr;
    QGSettings *m_settings = nullptr;
    QMap<QString, UkuiQuick::WidgetContainer *> m_islands;
    BlurRegionHelper* m_blurHelper = nullptr;
    QList<QAction *> m_dataIslandActions;
    QList<QAction *> m_appIslandActions;
    QList<QAction *> m_settingsIslandActions;
    QList<QMenu *> m_menus; //用于QMenu的统一销毁
    AutoHideHelper *m_autoHideHelper = nullptr;
    bool m_containsMouse = false;

    QStringList m_disabledWidgets;
};
} // UkuiPanel

#endif //UKUI_PANEL_PANEL_NEXT_H

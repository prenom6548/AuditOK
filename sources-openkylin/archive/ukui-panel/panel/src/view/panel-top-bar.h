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

#ifndef PANEL_TOP_BAR_H
#define PANEL_TOP_BAR_H

#include <island-view.h>
#include <window-helper.h>
#include "screens-manager.h"
#include "general-config-define.h"
#include "common.h"

namespace UkuiPanel {
class PanelTopBar : public UkuiQuick::IslandView
{
    Q_OBJECT
public:
    PanelTopBar(Screen* screen, const QString&id, QWindow* parent = nullptr);
    ~PanelTopBar();

    void setTopBarScreen(Screen* screen);
    void addIsland(const QString &id);
    void removeIsland(const QString &id);
    void dragRemove(const QString &id);
    void dragAdding(const QString &id, bool addWhenDrop);

    //锁屏时隐藏顶栏，解锁时恢复
    void setScreenLockHidden(bool hidden);

Q_SIGNALS:
    void islandRemoved(const QString &id);

private Q_SLOTS:
    void onScreenGeometryChanged(const QRect&geometry);
    void resetWorkArea(bool cut);
    void setPosition(UkuiQuick::Types::Pos position);
    void updateVisbility();
    void setAutoHide(bool autoHide);
    void setHidden(bool hidden);
    void activeHideTimer(bool active = true);
    void onWidgetAdded(UkuiQuick::Widget *widget);
    void onWidgetRemoved(UkuiQuick::Widget *widget);

private:
    void initMainContainer();
    void updateGeometry();
    void refreshRect();
    void updateMask();
    void applyWindowVisibility();
    bool windowSuppressed() const;
    void initConfig();
    void initIslandsConfig();
    void addContainer(const QString &id);
    void loadActions();
    void changeForm(const QString &id, GeneralConfigDefine::IslandsForm form);
    void initIslandFormAction();
    void updateIslandFormAction();
    void updateDisableWidget();
    void disableWidget(const QString &id, bool disable);
    void updateIslandState();
    void setTopBarSize(int size);

    QString m_id;
    UkuiQuick::WindowProxy *m_windowProxy = nullptr;
    Screen *m_screen {nullptr};
    int m_panelTopBarSize {32};
    UnavailableArea m_unavailableArea;
    GeneralConfigDefine::PanelSizePolicy m_sizePolicy {GeneralConfigDefine::Small};

    bool m_isHidden = true;
    bool m_autoHide = false;
    bool m_lockPanel = false;
    WindowSuppressionState m_windowSuppression;
    QTimer *m_hideTimer = nullptr;
    QMap<QString, UkuiQuick::WidgetContainer *> m_islands;
    QList<QAction *> m_dataIslandActions;
    QList<QAction *> m_settingsIslandActions;
    QList<QMenu *> m_menus; //用于QMenu的统一销毁
    QAction* m_dataIslandFormAction{nullptr};
    QAction* m_settingsIslandFormAction{nullptr};
    bool m_dragAdding = false;
    QStringList m_disabledWidgets;

};
}


#endif //PANEL_TOP_BAR_H

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

#ifndef UKUI_PANEL_SHELL_H
#define UKUI_PANEL_SHELL_H

#include <QObject>
#include <QMap>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <remote-config.h>
#include "screens-manager.h"
#include <keep-alive-client.h>

class QScreen;

namespace UkuiQuick {
class DbusConnector;
}

namespace UkuiPanel {

class Panel;
class PanelNext;
class PanelTopBar;

class Shell : public QObject
{
    Q_OBJECT
public:
    enum PanelType {
        Version4,
        Version5
    };
    static Shell* self();
    void start();
    void switchTo(PanelType type);
    PanelType currentVersion();

public Q_SLOTS:
    bool addCustomWidget(const QString &id);
    bool removeCustomWidget(const QString &id);

Q_SIGNALS:
    void islandRemovedFromPanel(const QString &id);
    void islandRemovedFromTopBar(const QString &id);
    void dragRemovingFromPanel(const QString &id, bool );
    void customWidgetAdded(const QString &id, int instanceId = -1);
    void customWidgetRemoved(const QString &id, int instanceId = -1);

private Q_SLOTS:
    void onTabletModeChanged(bool);
    void onFunctionControlChanged(const QString& name, bool exit = false);
    void onScreenLocked();
    void onScreenUnlocked();

protected:
    bool event(QEvent* e);

private:
    explicit Shell(QObject *parent=nullptr);
    ~Shell() override;

    void initRemoteConfig();
    void initDbus();
    void startVersion4panel();
    Panel *createPanel(Screen *screen, const QString &id);
    void startVersion5Panel();
    PanelNext *createPanelNext(Screen *screen, const QString &id);
    PanelTopBar *createPanelTopBar(Screen *screen, const QString &id);
    void keepAlive();
private:
    bool m_isTabletMode {false};
    bool m_screenLocked {false};//当前是否处于锁屏隐藏状态
    UkuiQuick::DbusConnector *m_screenSaverConnector {nullptr};//锁屏服务连接器，等待服务就绪后同步初始状态
    QMap<Screen*, Panel*> m_panelsVersion4;
    QMap<Screen*, PanelNext*> m_panelsVersion5;
    QMap<Screen*, PanelTopBar*> m_panelTopBars;
    RemoteConfig *m_remoteConfig = nullptr;
    QRemoteObjectHost m_qroHost;
    ScreensManager *m_screenManager = nullptr;
    bool m_started = false;
    PanelType m_currentVersion;
    UkuiQuick::Config *m_config = nullptr;
    //是否在所有屏幕上显示任务栏
    bool m_showPanelOnAllScreens = true;
    bool m_configDataChange = false;
    UkuiQuick::HeartBeat::KeepAliveClient *m_keepAliveClient = nullptr;
};

} // UkuiPanel

#endif //UKUI_PANEL_SHELL_H

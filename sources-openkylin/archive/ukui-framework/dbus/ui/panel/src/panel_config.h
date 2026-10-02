/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef __PANEL_CONFIG_H__
#define __PANEL_CONFIG_H__

#include <QObject>
#include <QDBusContext>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include "general_config_define.h"
#include "rep_remote_config_replica.h"

class RemoteConfigReplica;

#define PEEK_SERVER_STATE_RET_INT() \
if (!isConnected()) {    \
        if (calledFromDBus())   \
        sendErrorReply(QDBusError::NoServer, "can't connect to panel"); \
        return 999; \
}

#define PEEK_SERVER_STATE_RET_BOOL() \
if (!isConnected()) {    \
     if (calledFromDBus())   \
        sendErrorReply(QDBusError::NoServer, "can't connect to panel"); \
        return false; \
}

class ConfigUkPanel: public QObject, QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.UI.Panel")

    Q_PROPERTY(int MergeIcons READ GetMergeIcons WRITE SetMergeIcon NOTIFY MergeIconsChanged)
    Q_PROPERTY(int PanelLocation READ GetPanelLocation WRITE SetPanelLocation NOTIFY PanelLocationChanged)
    Q_PROPERTY(int PanelSizePolicy READ GetPanelSizePolicy WRITE SetPanelSizePolicy NOTIFY PanelSizePolicyChanged)
    Q_PROPERTY(bool PanelAutoHide READ GetPanelAutoHide WRITE SetPanelAutoHide NOTIFY PanelAutoHideChanged)
    Q_PROPERTY(bool PanelLockState READ GetPanelLockState WRITE SetPanelLockState NOTIFY PanelLockStateChanged)
    Q_PROPERTY(bool ShowPanelOnAllScreens READ GetShowPanelOnAllScreens WRITE SetShowPanelOnAllScreens NOTIFY ShowPanelOnAllScreensChanged)
    Q_PROPERTY(bool ShowSystemTrayOnAllPanels READ GetShowSystemTrayOnAllPanels WRITE SetShowSystemTrayOnAllPanels NOTIFY ShowSystemTrayOnAllPanelsChanged)
    Q_PROPERTY(int TaskBarIconsShowedOn READ GetTaskBarIconsShowedOn WRITE SetTaskBarIconsShowedOn NOTIFY TaskBarIconsShowOnChanged)
    Q_PROPERTY(bool ShowTaskView READ GetShowTaskView WRITE SetShowTaskView NOTIFY ShowTaskViewChanged)
    Q_PROPERTY(bool ShowSearch READ GetShowSearch WRITE SetShowSearch NOTIFY ShowSearchChanged)
    Q_PROPERTY(QStringList QuickLaunchers READ GetQuickLaunchers WRITE SetQuickLaunchers)
public:

    explicit ConfigUkPanel(QObject *parent = nullptr);
    ~ConfigUkPanel();
public Q_SLOTS:
    /**
     * @brief GetMergeIcons
     * @return
     */
    int GetMergeIcons() const;

    /**
     * @brief SetMergeIcons
     * @return
     */
    bool SetMergeIcon(int type);

    /**
     * @brief GetPanelLocation
     * @return
     */
    int GetPanelLocation() const;

    /**
     * @brief SetPanelLocation
     */
    bool SetPanelLocation(int location);

    /**
     * @brief GetPanelSizePolicy
     * @return
     */
    int GetPanelSizePolicy() const;

    /**
     * @brief SetPanelSizePolicy
     * @param size
     */
    bool SetPanelSizePolicy(int size);

    /**
     * @brief GetPanelAutoHide
     * @return
     */
    bool GetPanelAutoHide() const;

    /**
     * @brief SetPanelAutoHide
     */
    bool SetPanelAutoHide(bool isHide);


    /**
     * @brief GetPanelLockState
     * @return
     */
    bool GetPanelLockState() const;

    /**
     * @brief SetPanelLockState
     * @param lockState
     */
    bool SetPanelLockState(bool lockState);

    /**
     * @brief GetShowPanelOnAllScreens
     * @return
     */
    bool GetShowPanelOnAllScreens() const;

    /**
     * @brief SetShowPanelOnAllScreens
     * @param isShow
     */
    bool SetShowPanelOnAllScreens(bool isShow);

    /**
     * @brief GetShowSystemTrayOnAllPanels
     */
    bool GetShowSystemTrayOnAllPanels() const;

    /**
     * @brief SetShowSystemTrayOnAllPanels
     * @param isShow
     */
    bool SetShowSystemTrayOnAllPanels(bool isShow);

    /**
     * @brief GetTaskBarIconsShowedOn
     * @return
     */
    int GetTaskBarIconsShowedOn() const;

    /**
     * @brief SetTaskBarIconsShowedOn
     */
    bool SetTaskBarIconsShowedOn(int showOn);

    /**
     * @brief GetShowTaskView
     * @return
     */
    bool GetShowTaskView() const;
    /**
     * @brief SetShowTaskView
     */
    bool SetShowTaskView(bool isShow);

    /**
     * @brief GetShowSearch
     * @return
     */
    bool GetShowSearch() const;
    /**
     * @brief SetShowSearch
     */
    bool SetShowSearch(bool isShow);

    /**
     * @brief GetQuickLaunchers
     * @return QuickLaunchers currently in task manager
     */
    QStringList GetQuickLaunchers() const;

    /**
     * @brief SetQuickLaunchers
     * @param applications QuickLaunchers to set in task manager
     */
    void SetQuickLaunchers(const QStringList &applications);

Q_SIGNALS:
    void connectStateChanged(bool isSuccess);
    void MergeIconsChanged(int);
    void PanelLocationChanged(int);
    void PanelSizePolicyChanged(int);
    void PanelAutoHideChanged(bool);
    void PanelLockStateChanged(bool);
    void ShowSystemTrayOnAllPanelsChanged(bool);
    void ShowPanelOnAllScreensChanged(bool);
    void TaskBarIconsShowOnChanged(int);
    void ShowTaskViewChanged(bool);
    void ShowSearchChanged(bool);

private:
    void connectToSource();
    /**
     * @brief connectSignal
     *
     */
    void connectSignal() const;
    void initEnum();
    void sourceStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState);
    bool isConnected() const;
    /**
     * @brief island
     * 三岛不支持：合并图标、调整任务栏位置、在所有任务栏显示托盘,现在没有接口获取三岛状态、先记录不实现.
     * @return
     */
    bool island() const;

    template<typename T>
    bool convertEnumValue(const QString &str, const QMetaEnum &meta, T &result) const;

    template<typename T>
    bool convertEnumValue(const int &value, const QMetaEnum &meta, T &result) const;

private Q_SLOTS:
    void initConnection();
    void onMergeIconsChanged(GeneralConfigDefine::MergeIcons mergeIcons);
    void onPanelSizePolicyChanged(GeneralConfigDefine::PanelSizePolicy sizePolicy);
    void onPanelAutoHideChanged(bool isAutoHide);
    void onPanelLockStateChanged(bool lockState);
    void onShowSystemTrayOnAllPanelsChanged(bool isAll);
    void onPanelLocationChanged(GeneralConfigDefine::PanelLocation location);
    void onShowPanelOnAllScreensChanged(bool isAll);
    void onTaskBarIconsShowedOnChanged(GeneralConfigDefine::TaskBarIconsShowedOn showPolicy);
    void onDisableWidgetsChanged(QStringList list);
private:
    QMetaEnum   m_mergeEnum;
    QMetaEnum   m_locationEnum;
    QMetaEnum   m_sizeEnum;
    QMetaEnum   m_taskBarShowOnEnum;
    const QString C_TASK_VIEW = "org.ukui.panel.taskView";
    const QString C_SEARCH_V = "org.ukui.panel.search";
    QRemoteObjectNode m_qroNode;
    QRemoteObjectDynamicReplica *m_pIrep;
    bool m_showSearch = false;
    bool m_showTaskView = false;

    // 基础配置项
    const char* c_mergeIcons = "mergeIcons";
    const char* c_panelLocation = "panelLocation";
    const char* c_panelSizePolicy = "panelSizePolicy";
    const char* c_allowCustomSize = "allowCustomSize";
    const char* c_panelAutoHide = "panelAutoHide";
    const char* c_panelLock = "panelLock";
    const char* c_panelWidgets = "panelWidgets";
    const char* c_disabledWidgets = "disabledWidgets";
    const char* c_trayIconsInhibited = "trayIconsInhibited";
    const char* c_taskBarIconsShowedOn = "taskBarIconsShowedOn";
    const char* c_panelType = "panelType";
    const char* c_showPanelOnAllScreens = "showPanelOnAllScreens";
    const char* c_showSystemTrayOnAllPanels = "showSystemTrayOnAllPanels";

    // 备用配置项
    const char* c_panelLocationNext = "panelLocationNext";
    const char* c_panelSizePolicyNext = "panelSizePolicyNext";
    const char* c_panelLockNext = "panelLockNext";
    const char* c_panelWidgetsNext = "panelWidgetsNext";
    const char* c_disabledWidgetsNext = "disabledWidgetsNext";

};


#endif //__PANEL_CONFIG_H__

/*
 *
 *  * Copyright (C) 2023, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef UKUI_PANEL_REMOTE_CONFIG_H
#define UKUI_PANEL_REMOTE_CONFIG_H

#include "rep_remote-config_source.h"
#include <QJsonObject>

namespace UkuiQuick {
    class Config;
}

class RemoteConfig : public RemoteConfigSource
{
    Q_OBJECT
public:
    explicit RemoteConfig(QObject *parent = nullptr);
    ~RemoteConfig() override;
    GeneralConfigDefine::MergeIcons mergeIcons() const override;
    GeneralConfigDefine::PanelLocation panelLocation() const override;
    GeneralConfigDefine::PanelSizePolicy panelSizePolicy() const override;
    bool allowCustomSize() const override;
    bool panelAutoHide() const override;
    bool panelLock() const override;
    QStringList panelWidgets() const override { return m_panelWidgets; }
    QStringList disabledWidgets() const override;
    QStringList trayIconsInhibited() const override {return m_trayIconsInhibited; }
    QStringList trayIconsCanInhibited() const override {return m_trayIconsCanInhibited; }
    GeneralConfigDefine::TaskBarIconsShowedOn taskBarIconsShowedOn() const override;
    GeneralConfigDefine::PanelType panelType() const override {return m_panelType;}
    bool showPanelOnAllScreens() const override;
    bool showSystemTrayOnAllPanels() const override;
    GeneralConfigDefine::PanelLocation panelLocationNext() const override { return m_panelLocationNext; }
    GeneralConfigDefine::PanelSizePolicy panelSizePolicyNext() const override { return m_panelSizePolicyNext; }
    bool panelAutoHideNext() const override { return m_panelAutoHideNext; }
    bool panelLockNext() const override { return m_panelLockNext; }
    QStringList panelWidgetsNext() const override { return m_panelWidgetsNext; }
    QStringList disabledWidgetsNext() const override {return m_disabledWidgetsNext; };

    void setMergeIcons(GeneralConfigDefine::MergeIcons mergeIcons) override;
    void setPanelLocation(GeneralConfigDefine::PanelLocation panelLocation) override;
    void setPanelSizePolicy(GeneralConfigDefine::PanelSizePolicy panelSizePolicy) override;
    void setPanelAutoHide(bool panelAutoHide) override;
    void setPanelLock(bool panelLock) override;
    void setPanelWidgets(QStringList panelWidgets) override;
    void setTrayIconsInhibited(QStringList trayIconsInhibited) override;
    void setTaskBarIconsShowedOn(GeneralConfigDefine::TaskBarIconsShowedOn taskBarIconsShowedOn) override;
    void setPanelType(GeneralConfigDefine::PanelType panelType) override;
    void setShowPanelOnAllScreens(bool showPanelOnAllScreens) override;
    void setShowSystemTrayOnAllPanels(bool showSystemTrayOnAllPanels) override;
    void setPanelLocationNext(GeneralConfigDefine::PanelLocation panelLocation) override;
    void setPanelSizePolicyNext(GeneralConfigDefine::PanelSizePolicy panelSizePolicy) override;
    void setPanelAutoHideNext(bool panelAutoHide) override;
    void setPanelLockNext(bool panelLock) override;
    void setPanelWidgetsNext(QStringList panelWidgets) override;

    void updateMainViewConfig();
public Q_SLOTS:
    void disableWidget(const QString &id, bool disable) override;
    void disableWidgetNext(const QString &id, bool disable) override;
    void onFunctionControlChanged(const QString& name, bool exit = false);
private:
    void onPanelConfigChanged(const QString &key);
    void onPanelViewConfigChanged(const QString &key);
    void onTaskManagerConfigChanged(const QString &key);
    void onSystemTrayConfigChanged(const QString &key);
    void updatePanelLocation();
    void onPanelViewConfigNextChanged(const QString &key);
    void updatePanelLocationNext();
    QStringList disableWidgetsByControl() const;
    QStringList enableWidgetsByControl() const;

    GeneralConfigDefine::MergeIcons m_mergeIcons;
    GeneralConfigDefine::PanelLocation m_panelLocation;
    GeneralConfigDefine::PanelSizePolicy m_panelSizePolicy;
    bool m_allowCustomSize;
    bool m_panelAutoHide;
    bool m_panelLock;
    QStringList m_panelWidgets;
    QStringList m_disabledWidgets;
    QStringList m_trayIconsInhibited;
    QStringList m_trayIconsCanInhibited;
    GeneralConfigDefine::TaskBarIconsShowedOn m_taskBarIconsShowedOn;
    GeneralConfigDefine::PanelType m_panelType;
    bool m_showPanelOnAllScreens;
    bool m_showSystemTrayOnAllPanels;
    GeneralConfigDefine::PanelLocation m_panelLocationNext;
    GeneralConfigDefine::PanelSizePolicy m_panelSizePolicyNext;
    bool m_panelAutoHideNext;
    bool m_panelLockNext;
    QStringList m_panelWidgetsNext;
    QStringList m_disabledWidgetsNext;

    UkuiQuick::Config *m_panelConfig = nullptr;
    UkuiQuick::Config *m_mainViewConfig = nullptr;
    UkuiQuick::Config *m_mainViewNextConfig = nullptr;
    UkuiQuick::Config *m_mainViewTopBarConfig = nullptr;
    UkuiQuick::Config *m_taskManagerConfig = nullptr;
    UkuiQuick::Config *m_systemTrayConfig = nullptr;

    //管控配置
    QJsonObject m_controlConfig;
};


#endif //UKUI_PANEL_REMOTE_CONFIG_H

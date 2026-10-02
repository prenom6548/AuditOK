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

#ifndef UKUI_PANEL_PANEL_UKCC_PLUGIN_H
#define UKUI_PANEL_PANEL_UKCC_PLUGIN_H

#include <QObject>
#include <QVBoxLayout>
#include <QtRemoteObjects/qremoteobjectnode.h>
#include <QComboBox>
#include <QJsonObject>
#include <QPair>
#include <ukcc/interface/interface.h>
#include <ukcc/interface/interface.h>
#include <kysdk/applications/kswitchbutton.h>
#include <kysdk/applications/klabel.h>
#include <config-loader.h>
#include <config.h>
#include "rep_remote-config_replica.h"

class PanelUkccPlugin : public QObject, public CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    explicit PanelUkccPlugin(QObject *parent = nullptr);
    ~PanelUkccPlugin();

    QString plugini18nName()   Q_DECL_OVERRIDE;
    int pluginTypes()       Q_DECL_OVERRIDE;
    QWidget * pluginUi()   Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    QString translationPath() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

    //Virtual Keyboard
    enum VirtualKeyboardPolicy {
        NeverShow = 0,
        AlwaysSHow,
        ShowWhenKeyboardIsConnected
    };
    struct TrayIconInfo
    {
        QString id;
        QString name;
        QString iconName;

        TrayIconInfo(const QString& id_, const QString& name_, const QString& iconName_)
            : id(id_), name(name_), iconName(iconName_) {}
    };


public Q_SLOTS:
    void onFunctionControlChanged(const QString& name, bool exit = false);
    void onTrayIconsCanInhibitedChanged(const QStringList &list);

private:
    void initUI();
    QFrame * setLine(QFrame *parent = nullptr);
    void connectToSource();
    void initData();
    void sourceStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState);
    void updateData();
    int virtualKeyboardCurrentIndex();

    void initFunctionControl();

    QPair<QStringList, QStringList> compareStringLists(const QStringList& oldList, const QStringList& newList);
    int getTrayIconIndex(TrayIconInfo info);
    void removeSystemTrayIcon(const QString &id);

    QRemoteObjectNode m_qroNode;
    RemoteConfigReplica *m_configReplica = nullptr;

    //main page
    QWidget *m_widget = nullptr;

    //"panel"
    QVBoxLayout *m_mainLayout = nullptr;
    kdk::KLabel *m_titleLabel1 = nullptr;
    QFrame *m_frame1 = nullptr;
    QVBoxLayout *m_vLayout1 = nullptr;

    //panelmode
    QFrame *m_panelTypeFrame = nullptr;
    QHBoxLayout *m_panelTypeLayout = nullptr;
    QLabel *m_panelTypeLabel = nullptr;
    QButtonGroup *m_panelTypeGroup = nullptr;
    QPushButton *m_classicType = nullptr;
    QPushButton *m_threeIslandType = nullptr;
    QLabel* m_classicTypeLabel = nullptr;
    QLabel* m_threeIslandTypeLabel = nullptr;
    QHBoxLayout *m_panelTypeButtonLayout = nullptr;
    QVBoxLayout *m_classicTypeLayout = nullptr;
    QVBoxLayout *m_threeIslandTypeLayout = nullptr;

    //merge icons
    QFrame *m_mergeIconsFrame = nullptr;
    QHBoxLayout *m_mergeIconsLayout = nullptr;
    QLabel *m_mergeIconsLabel = nullptr;
    QComboBox *m_mergeIcons = nullptr;
    //panel location
    QFrame *m_panelLocationFrame = nullptr;
    QHBoxLayout *m_panelLocationLayout = nullptr;
    QLabel *m_panelLocationLabel = nullptr;
    QComboBox *m_panelLocation= nullptr;
    //panel size
    QFrame *m_panelSizePolicyFrame = nullptr;
    QHBoxLayout *m_panelSizePolicyLayout = nullptr;
    QLabel *m_panelSizePolicyLabel = nullptr;
    QComboBox *m_panelSizePolicy = nullptr;
    //panel auto hide
    QFrame *m_panelAutoHideFrame = nullptr;
    QHBoxLayout *m_panelAutoHideLayout = nullptr;
    QLabel *m_panelAutoHideLabel = nullptr;
    kdk::KSwitchButton *m_panelAutoHide = nullptr;
    //panel lock
    QFrame *m_panelLockFrame = nullptr;
    QHBoxLayout *m_panelLockLayout = nullptr;
    QLabel *m_panelLockLabel = nullptr;
    kdk::KSwitchButton *m_panelLock = nullptr;

    //按设计稿要求，空出间隙
    QFrame *m_frame1_2{};
    QVBoxLayout *m_vLayout1_2{};

    //show panel on all screens
    QFrame *m_showPanelOnAllScreensFrame = nullptr;
    QHBoxLayout *m_showPanelOnAllScreensLayout = nullptr;
    QLabel *m_showPanelOnAllScreensLabel = nullptr;
    kdk::KSwitchButton *m_showPanelOnAllScreens = nullptr;

    //show system tray on all panel
    QFrame *m_showSystemTrayOnAllPanelsFrame = nullptr;
    QHBoxLayout *m_showSystemTrayOnAllPanelsLayout = nullptr;
    QLabel *m_showSystemTrayOnAllPanelsLabel = nullptr;
    kdk::KSwitchButton *m_showSystemTrayOnAllPanels = nullptr;

    //icons showed on witch panel
    QFrame *m_iconsShowedOnFrame = nullptr;
    QHBoxLayout *m_iconsShowedOnLayout = nullptr;
    QLabel *m_iconsShowedOnLabel = nullptr;
    QComboBox *m_iconsShowedOn = nullptr;

    //"widgets showed on panel"
    kdk::KLabel *m_titleLabel2{};
    QFrame *m_frame2{};
    QVBoxLayout *m_vLayout2{};

    //task view
    QFrame *m_showTaskViewFrame = nullptr;
    QHBoxLayout *m_showTaskViewLayout = nullptr;
    QLabel *m_showTaskViewIconLabel = nullptr;
    QLabel *m_showTaskViewLabel = nullptr;
    kdk::KSwitchButton *m_showTaskView = nullptr;

    //show search
    QFrame *m_showSearchFrame = nullptr;
    QHBoxLayout *m_showSearchLayout = nullptr;
    QLabel *m_showSearchIconLabel = nullptr;
    QLabel *m_showSearchLabel = nullptr;
    kdk::KSwitchButton *m_showSearch = nullptr;

    //"widgets showed on panel"
    kdk::KLabel *m_titleLabel3{};
    QFrame *m_frame3{};
    QVBoxLayout *m_vLayout3{};

    QStringList m_systemTrayIconId = {
            "ukui-bluetooth",
            "Fcitx"
    };

    QList<TrayIconInfo> m_systemTrayIconInfo;
    UkuiQuick::Config *m_systemTrayConfig = nullptr;

    //Virtual Keyboard
    QFrame *m_virtualKeyboardFrame = nullptr;
    QHBoxLayout *m_virtualKeyboardLayout = nullptr;
    QLabel *m_virtualKeyboardIconLabel = nullptr;
    QLabel *m_virtualKeyboardLabel = nullptr;
    QComboBox *m_virtualKeyboardPolicy = nullptr;
    QGSettings *m_gsetting = nullptr;
    GeneralConfigDefine::PanelType m_panelType = GeneralConfigDefine::PanelType::Version4;
    //管控配置
    QJsonObject m_controlConfig;
    bool m_initialized = false;
};


#endif //UKUI_PANEL_PANEL_UKCC_PLUGIN_H

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

#include "panel-ukcc-plugin.h"
#include <QTranslator>
#include <QApplication>
#include <QSet>
#include "general-config-define.h"
#include "switch-button.h"
#include <QButtonGroup>
#include <function-control.h>
#include <QJsonArray>
#include <QStandardItemModel>
#include <unistd.h>

#define TRAY_ICON_SHOW_POLICY      "trayIconShowPolicy"

PanelUkccPlugin::PanelUkccPlugin(QObject *parent): QObject(parent)
{
    auto translator = new QTranslator(this);
    if(!translator->load(TRANSLATIONS_INSTALL_PATH + QLocale::system().name())) {
        qWarning() << TRANSLATIONS_INSTALL_PATH + QLocale::system().name() << "load failed";
    }
    QApplication::installTranslator(translator);
}

PanelUkccPlugin::~PanelUkccPlugin()
{
    if (m_gsetting) {
        delete m_gsetting;
        m_gsetting = nullptr;
    }
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-v11"));
    UkuiQuick::FunctionControl::instance()->removeWatch(QStringLiteral("ukui-panel-taskManager-v11"));
}

QString PanelUkccPlugin::plugini18nName()
{
    return tr("Panel");
}

int PanelUkccPlugin::pluginTypes()
{
    return PERSONALIZED;
}

QWidget *PanelUkccPlugin::pluginUi()
{
    if (!m_widget) {
        initFunctionControl();
        initUI();
        connectToSource();
    }
    return m_widget;
}

const QString PanelUkccPlugin::name() const
{
    // 不需要翻译，作为key，用于在控制面板激活并显示插件
    return "Panel";
}

QString PanelUkccPlugin::translationPath() const
{
    return QStringLiteral("/usr/share/ukui-panel/panel-ukcc-plugin/translations/%1.ts");;
}

bool PanelUkccPlugin::isShowOnHomePage() const
{
    return false;
}

QIcon PanelUkccPlugin::icon() const
{
    return QIcon::fromTheme("ukui-panel-symbolic");
}

bool PanelUkccPlugin::isEnable() const
{
    return true;
}

void PanelUkccPlugin::connectToSource()
{
    QString displayEnv = (qgetenv("XDG_SESSION_TYPE") == "wayland") ? QLatin1String("WAYLAND_DISPLAY") : QLatin1String("DISPLAY");
    QString display(qgetenv(displayEnv.toUtf8().data()));
    QUrl address(QStringLiteral("local:ukui-panel-config-")  + QString(qgetenv("USER")) + QStringLiteral("-") + QString::number(getuid()) + QStringLiteral("-") + display);

    qDebug() << "panel ukcc plugin connect to source:" << m_qroNode.connectToNode(address);
    m_configReplica = m_qroNode.acquire<RemoteConfigReplica>();

    if (m_configReplica->waitForSource(2000)) {
        initData();
    }
    connect(m_configReplica, &QRemoteObjectReplica::initialized, this, &PanelUkccPlugin::initData);
    connect(m_configReplica, &QRemoteObjectReplica::stateChanged, this, &PanelUkccPlugin::sourceStateChanged);
}

void PanelUkccPlugin::initUI()
{
    m_widget = new QWidget;
    // main page
    m_mainLayout = new QVBoxLayout(m_widget);
    m_mainLayout->setSpacing(8);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);

    // panel
    m_titleLabel1 = new kdk::KLabel(m_widget);
    m_titleLabel1->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Panel/Panel
    m_titleLabel1->setText(tr("Panel"));
    m_mainLayout->addWidget(m_titleLabel1);

    m_frame1 = new QFrame(m_widget);
    m_frame1->setFrameShape(QFrame::Shape::Box);
    m_vLayout1 = new QVBoxLayout(m_frame1);
    m_vLayout1->setContentsMargins(0, 0, 0, 0);
    m_vLayout1->setSpacing(0);

    //panel mode
    m_panelTypeFrame = new QFrame(m_frame1);
    m_panelTypeFrame->setFixedHeight(147);
    m_panelTypeLayout = new QHBoxLayout(m_panelTypeFrame);
    m_panelTypeLayout->setContentsMargins(16, 0, 16, 0);
    m_panelTypeLabel = new QLabel(m_panelTypeFrame);
    m_panelTypeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_panelTypeLabel->setText(tr("Panel Appearance"));
    m_panelTypeGroup = new QButtonGroup(m_panelTypeFrame);

    m_classicType = new QPushButton(m_panelTypeFrame);
    m_threeIslandType = new QPushButton(m_panelTypeFrame);
    m_classicType->setIconSize(QSize(140,90));
    m_classicType->setFixedSize(QSize(142,92));
    m_classicType->setCheckable(true);
    m_threeIslandType->setCheckable(true);
    m_threeIslandType->setIconSize(QSize(140,90));
    m_threeIslandType->setFixedSize(QSize(142,92));
    m_classicType->setIcon(QIcon(":/classic.png"));
    m_threeIslandType->setIcon(QIcon(":/island.png"));
    m_panelTypeGroup->addButton(m_classicType, 0);
    m_panelTypeGroup->addButton(m_threeIslandType, 1);
    m_panelTypeGroup->setExclusive(true);
    QString buttonStyle(
        "QPushButton{background: palette(window);"
        "border-color: palette(highlight);"
        "border-width: 0px;border-style: solid;border-radius: 6px}"
        "QPushButton:hover{background: palette(window);"
        "border-color: palette(highlight);"
        "border-width: 1px;border-style: solid;border-radius: 6px}"
        "QPushButton:checked{background: palette(window);"
        "border-color: palette(highlight);"
        "border-width: 2px;border-style: solid;border-radius: 6px}"
        "QPushButton:!enabled{background: palette(window);"
        "border-width: 0px}");
    m_classicType->setStyleSheet(buttonStyle);
    m_classicType->setEnabled(false);
    m_threeIslandType->setEnabled(false);
    m_threeIslandType->setStyleSheet(buttonStyle);
    m_classicTypeLabel = new QLabel(m_panelTypeFrame);
    m_classicTypeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_classicTypeLabel->setText(tr("classic"));
    m_classicTypeLabel->setMaximumWidth(140);
    m_classicTypeLabel->setAlignment(Qt::AlignCenter);
    m_threeIslandTypeLabel = new QLabel(m_panelTypeFrame);
    m_threeIslandTypeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_threeIslandTypeLabel->setText(tr("threeIsland"));
    m_threeIslandTypeLabel->setMaximumWidth(140);
    m_threeIslandTypeLabel->setAlignment(Qt::AlignCenter);

    m_classicTypeLayout = new QVBoxLayout(m_panelTypeFrame);
    m_classicTypeLayout->setContentsMargins(0, 16, 0, 16);
    m_classicTypeLayout->addWidget(m_classicType);
    m_classicTypeLayout->addWidget(m_classicTypeLabel);

    m_threeIslandTypeLayout = new QVBoxLayout(m_panelTypeFrame);
    m_threeIslandTypeLayout->setContentsMargins(0, 16, 0, 16);
    m_threeIslandTypeLayout->addWidget(m_threeIslandType);
    m_threeIslandTypeLayout->addWidget(m_threeIslandTypeLabel);

    m_panelTypeButtonLayout = new QHBoxLayout(m_panelTypeFrame);
    m_panelTypeButtonLayout->setContentsMargins(0, 0, 0, 0);
    m_panelTypeButtonLayout->addLayout(m_classicTypeLayout);
    m_panelTypeButtonLayout->addLayout(m_threeIslandTypeLayout);

    m_panelTypeLayout->addWidget(m_panelTypeLabel);
    m_panelTypeLayout->addLayout(m_panelTypeButtonLayout);

    m_vLayout1->addWidget(m_panelTypeFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //merge icons
    m_mergeIconsFrame = new QFrame(m_frame1);
    m_mergeIconsFrame->setFixedHeight(56);
    m_mergeIconsLayout = new QHBoxLayout(m_mergeIconsFrame);
    m_mergeIconsLayout->setContentsMargins(16, 0, 16, 0);

    m_mergeIconsLabel = new QLabel(m_mergeIconsFrame);
    m_mergeIconsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Merge icons on the taskbar
    m_mergeIconsLabel->setText(tr("Merge icons on the taskbar"));
    m_mergeIcons = new QComboBox(m_mergeIconsFrame);
    m_mergeIcons->setAccessibleName("panel_ukcc_plugin_mergeIcons");
    m_mergeIcons->setEnabled(false);
    m_mergeIcons->setMinimumWidth(320);
    m_mergeIcons->addItem(tr("Always"), GeneralConfigDefine::MergeIcons::Always);
    m_mergeIcons->addItem(tr("Never"), GeneralConfigDefine::MergeIcons::Never);
    m_mergeIconsLayout->addWidget(m_mergeIconsLabel);
    m_mergeIconsLayout->addWidget(m_mergeIcons);
    m_vLayout1->addWidget(m_mergeIconsFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //panel location
    m_panelLocationFrame = new QFrame(m_frame1);
    m_panelLocationFrame->setFixedHeight(56);
    m_panelLocationLayout = new QHBoxLayout(m_panelLocationFrame);
    m_panelLocationLayout->setContentsMargins(16, 0, 16, 0);

    m_panelLocationLabel = new QLabel(m_panelLocationFrame);
    m_panelLocationLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Panel location
    m_panelLocationLabel->setText(tr("Panel location"));
    m_panelLocation = new QComboBox(m_panelLocationFrame);
    m_panelLocation->setAccessibleName("panel_ukcc_plugin_panelLocation");
    m_panelLocation->setEnabled(false);
    m_panelLocation->setMinimumWidth(320);
    m_panelLocation->addItem(tr("Bottom"), GeneralConfigDefine::PanelLocation::Bottom);
    m_panelLocation->addItem(tr("Left"), GeneralConfigDefine::PanelLocation::Left);
    m_panelLocation->addItem(tr("Top"), GeneralConfigDefine::PanelLocation::Top);
    m_panelLocation->addItem(tr("Right"), GeneralConfigDefine::PanelLocation::Right);
    m_panelLocationLayout->addWidget(m_panelLocationLabel);
    m_panelLocationLayout->addWidget(m_panelLocation);
    m_vLayout1->addWidget(m_panelLocationFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //panel size
    m_panelSizePolicyFrame = new QFrame(m_frame1);
    m_panelSizePolicyFrame->setFixedHeight(56);
    m_panelSizePolicyLayout = new QHBoxLayout(m_panelSizePolicyFrame);
    m_panelSizePolicyLayout->setContentsMargins(16, 0, 16, 0);

    m_panelSizePolicyLabel = new QLabel(m_panelSizePolicyFrame);
    m_panelSizePolicyLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Panel size
    m_panelSizePolicyLabel->setText(tr("Panel size"));
    m_panelSizePolicy = new QComboBox(m_panelSizePolicyFrame);
    m_panelSizePolicy->setAccessibleName("panel_ukcc_plugin_panelSizePolicy");
    m_panelSizePolicy->setEnabled(false);
    m_panelSizePolicy->setMinimumWidth(320);
    m_panelSizePolicy->addItem(tr("Small"), GeneralConfigDefine::PanelSizePolicy::Small);
    m_panelSizePolicy->addItem(tr("Medium"), GeneralConfigDefine::PanelSizePolicy::Medium);
    m_panelSizePolicy->addItem(tr("Large"), GeneralConfigDefine::PanelSizePolicy::Large);
    m_panelSizePolicy->addItem(tr("Custom"), GeneralConfigDefine::PanelSizePolicy::Custom);
    m_panelSizePolicyLayout->addWidget(m_panelSizePolicyLabel);
    m_panelSizePolicyLayout->addWidget(m_panelSizePolicy);
    m_vLayout1->addWidget(m_panelSizePolicyFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //panel auto hide
    m_panelAutoHideFrame = new QFrame(m_frame1);
    m_panelAutoHideFrame->setFixedHeight(56);
    m_panelAutoHideLayout = new QHBoxLayout(m_panelAutoHideFrame);
    m_panelAutoHideLayout->setContentsMargins(16, 0, 16, 0);
    m_panelAutoHideLabel = new QLabel(m_panelAutoHideFrame);
    m_panelAutoHideLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Panel auto hide
    m_panelAutoHideLabel->setText(tr("Panel auto hide"));
    m_panelAutoHide = new kdk::KSwitchButton(m_panelAutoHideFrame);
    m_panelAutoHide->setAccessibleName("panel_ukcc_plugin_autohide");
    m_panelAutoHide->setFixedSize(48,24);
//    m_panelAutoHide->setCheckable(false);
    m_panelAutoHide->setEnabled(false);
    m_panelAutoHideLayout->addWidget(m_panelAutoHideLabel);
    m_panelAutoHideLayout->addWidget(m_panelAutoHide);
    m_vLayout1->addWidget(m_panelAutoHideFrame);
    m_vLayout1->addWidget(setLine(m_frame1));

    //panel lock
    m_panelLockFrame = new QFrame(m_frame1);
    m_panelLockFrame->setFixedHeight(56);
    m_panelLockLayout = new QHBoxLayout(m_panelLockFrame);
    m_panelLockLayout->setContentsMargins(16, 0, 16, 0);
    m_panelLockLabel = new QLabel(m_panelLockFrame);
    m_panelLockLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Panel lock
    m_panelLockLabel->setText(tr("Panel lock"));
    m_panelLock = new kdk::KSwitchButton(m_panelLockFrame);
    m_panelLock->setAccessibleName("panel_ukcc_plugin_lockpanel");
    m_panelLock->setFixedSize(48,24);
    m_panelLock->setEnabled(false);
    m_panelLockLayout->addWidget(m_panelLockLabel);
    m_panelLockLayout->addWidget(m_panelLock);
    m_vLayout1->addWidget(m_panelLockFrame);

    m_mainLayout->addWidget(m_frame1);

    //按设计稿要求，空出间隙
    m_frame1_2 = new QFrame(m_widget);
    m_frame1_2->setFrameShape(QFrame::Box);
    m_vLayout1_2 = new QVBoxLayout(m_frame1_2);
    m_vLayout1_2->setContentsMargins(0, 0, 0, 0);
    m_vLayout1_2->setSpacing(0);


    //show panel on all screens
    m_showPanelOnAllScreensFrame = new QFrame(m_frame1_2);
    m_showPanelOnAllScreensFrame->setFixedHeight(56);
    m_showPanelOnAllScreensLayout = new QHBoxLayout(m_showPanelOnAllScreensFrame);
    m_showPanelOnAllScreensLayout->setContentsMargins(16, 0, 16, 0);
    m_showPanelOnAllScreensLabel = new QLabel(m_showPanelOnAllScreensFrame);
    m_showPanelOnAllScreensLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    //~ contents_path /Panel/Show panel on all screens
    m_showPanelOnAllScreensLabel->setText(tr("Show panel on all screens"));
    m_showPanelOnAllScreens = new kdk::KSwitchButton(m_showPanelOnAllScreensFrame);
    m_showPanelOnAllScreens->setAccessibleName("panel_ukcc_plugin_showpanelonallscreens");
    m_showPanelOnAllScreens->setFixedSize(48,24);
    m_showPanelOnAllScreens->setEnabled(false);
    m_showPanelOnAllScreensLayout->addWidget(m_showPanelOnAllScreensLabel);
    m_showPanelOnAllScreensLayout->addWidget(m_showPanelOnAllScreens);
    m_vLayout1_2->addWidget(m_showPanelOnAllScreensFrame);
    m_vLayout1_2->addWidget(setLine(m_frame1_2));

    //show system tray on all panel
    m_showSystemTrayOnAllPanelsFrame = new QFrame(m_frame1_2);
    m_showSystemTrayOnAllPanelsFrame->setFixedHeight(56);
    m_showSystemTrayOnAllPanelsLayout = new QHBoxLayout(m_showSystemTrayOnAllPanelsFrame);
    m_showSystemTrayOnAllPanelsLayout->setContentsMargins(16, 0, 16, 0);
    m_showSystemTrayOnAllPanelsLabel = new QLabel(m_showSystemTrayOnAllPanelsFrame);
    m_showSystemTrayOnAllPanelsLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_showSystemTrayOnAllPanelsLabel->setText(tr("Show system tray area on all panels "));
    m_showSystemTrayOnAllPanels = new kdk::KSwitchButton(m_showSystemTrayOnAllPanelsFrame);
    m_showSystemTrayOnAllPanels->setAccessibleName("panel_ukcc_plugin_showsystemtrayonallpanels");
    m_showSystemTrayOnAllPanels->setFixedSize(48,24);
    m_showSystemTrayOnAllPanels->setEnabled(false);
    m_showSystemTrayOnAllPanelsLayout->addWidget(m_showSystemTrayOnAllPanelsLabel);
    m_showSystemTrayOnAllPanelsLayout->addWidget(m_showSystemTrayOnAllPanels);
    m_vLayout1_2->addWidget(m_showSystemTrayOnAllPanelsFrame);
    m_vLayout1_2->addWidget(setLine(m_frame1_2));

    //taskmanager icons showed on
    m_iconsShowedOnFrame = new QFrame(m_frame1_2);
    m_iconsShowedOnFrame->setFixedHeight(56);
    m_iconsShowedOnLayout = new QHBoxLayout(m_iconsShowedOnFrame);
    m_iconsShowedOnLayout->setContentsMargins(16, 0, 16, 0);
    m_iconsShowedOnLabel = new kdk::KLabel(m_iconsShowedOnFrame);
    m_iconsShowedOnLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_iconsShowedOnLabel->setMinimumWidth(200);
    m_iconsShowedOnLabel->setText(tr("When existing multiple panels, show window icons on"));
    m_iconsShowedOn = new QComboBox(m_iconsShowedOnFrame);
    m_iconsShowedOn->setAccessibleName("panel_ukcc_plugin_iconsShowedOn");
    m_iconsShowedOn->setEnabled(false);
    m_iconsShowedOn->setMinimumWidth(320);
    m_iconsShowedOn->addItem(tr("All panels"), GeneralConfigDefine::TaskBarIconsShowedOn::AllPanels);
    m_iconsShowedOn->addItem(tr("Primary screen panel and panel where window is open"), GeneralConfigDefine::TaskBarIconsShowedOn::PrimaryScreenPanelAndPanelWhereWindowIsOpen);
    m_iconsShowedOn->addItem(tr("Panel where window is open"), GeneralConfigDefine::TaskBarIconsShowedOn::PanelWhereWindowIsOpen);
    m_iconsShowedOnLayout->addWidget(m_iconsShowedOnLabel);
    m_iconsShowedOnLayout->addWidget(m_iconsShowedOn);
    m_vLayout1_2->addWidget(m_iconsShowedOnFrame);

    m_mainLayout->addWidget(m_frame1_2);
    m_mainLayout->addSpacerItem(new QSpacerItem(40, 40, QSizePolicy::Fixed));

    //widgets showed on panel
    m_titleLabel2 = new kdk::KLabel(m_widget);
    m_titleLabel2->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Panel/Widgets always showed on panel
    m_titleLabel2->setText(tr("Widgets always showed on panel"));
    m_mainLayout->addWidget(m_titleLabel2);

    m_frame2 = new QFrame(m_widget);
    m_frame2->setFrameShape(QFrame::Box);
    m_vLayout2 = new QVBoxLayout(m_frame2);
    m_vLayout2->setContentsMargins(0, 0, 0, 0);
    m_vLayout2->setSpacing(0);

    //task view
    m_showTaskViewFrame = new QFrame(m_frame2);
    m_showTaskViewFrame->setFixedHeight(56);
    m_showTaskViewLayout = new QHBoxLayout(m_showTaskViewFrame);
    m_showTaskViewLayout->setContentsMargins(16, 0, 16, 0);
    m_showTaskViewLayout->setSpacing(16);
    m_showTaskViewIconLabel = new QLabel(m_showTaskViewFrame);
    m_showTaskViewIconLabel->setPixmap(QIcon::fromTheme("ukui-taskview-black-symbolic").pixmap(32, 32));
    m_showTaskViewIconLabel->setProperty("useIconHighlightEffect", 0x2);
    m_showTaskViewLabel = new QLabel(m_showTaskViewFrame);
    m_showTaskViewLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_showTaskViewLabel->setText(tr("Task view"));
    m_showTaskView = new kdk::KSwitchButton(m_showTaskViewFrame);
    m_showTaskView->setAccessibleName("panel_ukcc_plugin_showtaskview");
    m_showTaskView->setFixedSize(48,24);
//    m_showTaskView->setCheckable(false);
    m_showTaskView->setEnabled(false);
    m_showTaskViewLayout->addWidget(m_showTaskViewIconLabel);
    m_showTaskViewLayout->addWidget(m_showTaskViewLabel);
    m_showTaskViewLayout->addWidget(m_showTaskView);
    m_vLayout2->addWidget(m_showTaskViewFrame);
    m_vLayout2->addWidget(setLine(m_frame2));

    //show search
    m_showSearchFrame = new QFrame(m_frame2);
    m_showSearchFrame->setFixedHeight(56);
    m_showSearchLayout = new QHBoxLayout(m_showSearchFrame);
    m_showSearchLayout->setContentsMargins(16, 0, 16, 0);
    m_showSearchLayout->setSpacing(16);
    m_showSearchIconLabel = new QLabel(m_showTaskViewFrame);
    m_showSearchIconLabel->setPixmap(QIcon::fromTheme("kylin-search").pixmap(32, 32));
    m_showSearchIconLabel->setProperty("useIconHighlightEffect", 0x2);
    m_showSearchLabel = new QLabel(m_showSearchFrame);
    m_showSearchLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_showSearchLabel->setText(tr("Search"));
    m_showSearch = new kdk::KSwitchButton(m_showSearchFrame);
    m_showSearch->setAccessibleName("panel_ukcc_plugin_showsearch");
    m_showSearch->setFixedSize(48,24);
    m_showSearch->setEnabled(false);
    m_showSearchLayout->addWidget(m_showSearchIconLabel);
    m_showSearchLayout->addWidget(m_showSearchLabel);
    m_showSearchLayout->addWidget(m_showSearch);
    m_vLayout2->addWidget(m_showSearchFrame);

    m_mainLayout->addWidget(m_frame2);
    m_mainLayout->addSpacerItem(new QSpacerItem(40, 40, QSizePolicy::Fixed));

    //Icons showed on system tray
    m_titleLabel3 = new kdk::KLabel(m_widget);
    m_titleLabel3->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /Panel/Icons showed on system tray
    m_titleLabel3->setText(tr("Icons showed on system tray"));
    m_mainLayout->addWidget(m_titleLabel3);

    m_frame3 = new QFrame(m_widget);
//    m_frame3->setFixedHeight(56);
    m_frame3->setFrameShape(QFrame::Box);
    m_vLayout3 = new QVBoxLayout(m_frame3);
    m_vLayout3->setContentsMargins(0, 0, 0, 0);
    m_vLayout3->setSpacing(0);

    m_systemTrayIconInfo.append(TrayIconInfo("ukui-power-manager-tray", tr("Power"), "battery-symbolic"));
    m_systemTrayIconInfo.append(TrayIconInfo("Fcitx", tr("Fcitx"), "input-keyboard-symbolic"));

    //Virtual Keyboard
    m_virtualKeyboardFrame = new QFrame(m_frame3);
    m_virtualKeyboardFrame->setFixedHeight(56);
    m_virtualKeyboardLayout = new QHBoxLayout(m_virtualKeyboardFrame);
    m_virtualKeyboardLayout->setContentsMargins(16, 0, 16, 0);
    m_virtualKeyboardLayout->setSpacing(16);

    m_virtualKeyboardIconLabel = new QLabel(m_virtualKeyboardFrame);
    m_virtualKeyboardIconLabel->setPixmap(QIcon::fromTheme("preferences-desktop-keyboard-symbolic").pixmap(16, 16));
    m_virtualKeyboardIconLabel->setProperty("useIconHighlightEffect", 0x2);
    m_virtualKeyboardLabel = new QLabel(m_virtualKeyboardFrame);
    m_virtualKeyboardLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_virtualKeyboardLabel->setText(tr("Virtual Keyboard"));
    m_virtualKeyboardPolicy = new QComboBox(m_virtualKeyboardFrame);
    m_virtualKeyboardPolicy->setAccessibleName("panel_ukcc_plugin_virtualKeyboardPolicy");
    m_virtualKeyboardPolicy->setEnabled(false);
    m_virtualKeyboardPolicy->setMinimumWidth(320);
    m_virtualKeyboardPolicy->addItem(tr("Never Show"));
    m_virtualKeyboardPolicy->addItem(tr("Always Show"));
    m_virtualKeyboardPolicy->addItem(tr("Show When Keyboard Is Connected"));
    m_virtualKeyboardLayout->addWidget(m_virtualKeyboardIconLabel);
    m_virtualKeyboardLayout->addWidget(m_virtualKeyboardLabel);
    m_virtualKeyboardLayout->addWidget(m_virtualKeyboardPolicy);
    m_vLayout3->addWidget(m_virtualKeyboardFrame);

    m_mainLayout->addWidget(m_frame3);
    m_mainLayout->addStretch();
}

QFrame *PanelUkccPlugin::setLine(QFrame *parent)
{
    auto line = new QFrame(parent);
    line->setFixedHeight(1);
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

void PanelUkccPlugin::initData()
{
    if(m_configReplica->panelType() == GeneralConfigDefine::PanelType::Version4) {
        m_classicType->setChecked(true);
    } else if(m_configReplica->panelType() == GeneralConfigDefine::PanelType::Version5) {
        m_threeIslandType->setChecked(true);
    }
    m_panelType = m_configReplica->panelType();

    m_threeIslandType->setEnabled(true);

    m_mergeIcons->setCurrentIndex(m_configReplica->mergeIcons());

    m_mergeIcons->setEnabled(m_panelType == GeneralConfigDefine::PanelType::Version4);

    if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
        m_panelLocation->setCurrentIndex(m_configReplica->panelLocation());
    } else {
        m_panelLocation->setCurrentIndex(m_configReplica->panelLocationNext());
    }

    m_panelLocation->setEnabled(!m_configReplica->panelLock() && m_panelType == GeneralConfigDefine::PanelType::Version4);

    auto refreshSizePolicyItems = [this](bool allow) {
        auto model = qobject_cast<QStandardItemModel *>(m_panelSizePolicy->model());
        if (model) {
            QStandardItem* item = model->item(3);
            if (item) {
                item->setFlags(allow ? item->flags() | Qt::ItemIsEnabled : item->flags() & ~Qt::ItemIsEnabled);
            }
        }
    };
    if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
        m_panelSizePolicy->setCurrentIndex(m_configReplica->panelSizePolicy());
        refreshSizePolicyItems(m_configReplica->allowCustomSize());
    } else {
        m_panelSizePolicy->setCurrentIndex(m_configReplica->panelSizePolicyNext());
    }

    m_panelSizePolicy->setEnabled(!m_configReplica->panelLock());

    if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
        m_panelAutoHide->setChecked(m_configReplica->panelAutoHide());
    } else {
        m_panelAutoHide->setChecked(m_configReplica->panelAutoHideNext());
    }

    m_panelAutoHide->setEnabled(true);

    if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
        m_panelLock->setChecked(m_configReplica->panelLock());
    } else {
        m_panelLock->setChecked(m_configReplica->panelLockNext());
    }

    m_panelLock->setEnabled(true);

    m_iconsShowedOn->setCurrentIndex(m_configReplica->taskBarIconsShowedOn());

    //show panel on all screens
    m_showPanelOnAllScreens->setChecked(m_configReplica->showPanelOnAllScreens());

    m_showPanelOnAllScreens->setEnabled(true);
    m_iconsShowedOn->setEnabled(m_configReplica->showPanelOnAllScreens());

    m_showSystemTrayOnAllPanels->setChecked(m_configReplica->showSystemTrayOnAllPanels());

    m_showSystemTrayOnAllPanels->setEnabled(m_configReplica->panelType() == GeneralConfigDefine::PanelType::Version4 && m_configReplica->showPanelOnAllScreens());

    if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
        m_showTaskView->setChecked(!m_configReplica->disabledWidgets().contains(QStringLiteral("org.ukui.panel.taskView")));
    } else {
        m_showTaskView->setChecked(!m_configReplica->disabledWidgetsNext().contains(QStringLiteral("org.ukui.panel.taskView")));
    }

    m_showTaskView->setEnabled(true);

    //是否始终显示搜索
    if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
        m_showSearch->setChecked(!m_configReplica->disabledWidgets().contains(QStringLiteral("org.ukui.panel.search")));
    } else {
        m_showSearch->setChecked(!m_configReplica->disabledWidgetsNext().contains(QStringLiteral("org.ukui.panel.search")));
    }

    m_showSearch->setEnabled(true);

    QStringList trayIconList = m_configReplica->trayIconsCanInhibited();
    if (m_systemTrayIconId != trayIconList)
        m_systemTrayIconId = trayIconList;

    int index = 0;
    for(const TrayIconInfo &trayIconInfo : m_systemTrayIconInfo) {
        if (!m_systemTrayIconId.contains(trayIconInfo.id)) {
            continue;
        }
        auto button = new SwitchButton(m_frame3, trayIconInfo.name, trayIconInfo.iconName);
        button->setObjectName(trayIconInfo.id);
        button->setEnabled(true);
        button->setChecked(!m_configReplica->trayIconsInhibited().contains(trayIconInfo.id));
        m_vLayout3->insertWidget(2 * index, button);
        m_vLayout3->insertWidget(2 * index + 1, setLine(m_frame3));
        ++index;

        if (!m_initialized) {
            connect(button, &SwitchButton::stateChanged, this, [&, button](bool show) {
                QStringList icons = m_configReplica->trayIconsInhibited();
                if (!show && !icons.contains(button->objectName())) {
                    m_configReplica->setTrayIconsInhibited(icons << button->objectName());
                } else if (show && icons.contains(button->objectName())) {
                    icons.removeAll(button->objectName());
                    m_configReplica->setTrayIconsInhibited(icons);
                }
            });
        }
    }

    //Virtual Keyboard
    const QByteArray id("org.ukui.virtualkeyboard");
    if (QGSettings::isSchemaInstalled(id)) {
        m_gsetting = new QGSettings(id);
        if (m_gsetting->keys().contains(TRAY_ICON_SHOW_POLICY)) {
            //若旧配置中已经禁用虚拟键盘，需要取消禁用
            QStringList icons = m_configReplica->trayIconsInhibited();
            if (icons.contains("kylin-virtual-keyboard")) {
                icons.removeAll("kylin-virtual-keyboard");
                m_configReplica->setTrayIconsInhibited(icons);
            }

            m_virtualKeyboardPolicy->setEnabled(true);
            m_virtualKeyboardPolicy->setCurrentIndex(virtualKeyboardCurrentIndex());
            if (!m_initialized) {
                connect(m_virtualKeyboardPolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
                        [&](int index) {
                            switch (index) {
                                case VirtualKeyboardPolicy::NeverShow:
                                    m_gsetting->set(TRAY_ICON_SHOW_POLICY, "NeverShow");
                                    break;
                                case VirtualKeyboardPolicy::AlwaysSHow:
                                    m_gsetting->set(TRAY_ICON_SHOW_POLICY, "AlwaysShow");
                                    break;
                                case VirtualKeyboardPolicy::ShowWhenKeyboardIsConnected:
                                    m_gsetting->set(TRAY_ICON_SHOW_POLICY, "ShowWhenKeyboardIsConnected");
                                    break;
                                default:
                                    break;
                            }
                        });
                connect(m_gsetting, &QGSettings::changed, this, [&](const QString&key) {
                    if (key == TRAY_ICON_SHOW_POLICY) {
                        m_virtualKeyboardPolicy->setCurrentIndex(virtualKeyboardCurrentIndex());
                    }
                });
            }
        }
    }

    //初始化功能管控配置
    onFunctionControlChanged(QStringLiteral("ukui-panel-v11"), false);
    onFunctionControlChanged(QStringLiteral("ukui-panel-taskManager-v11"), false);

    if (m_controlConfig.contains(QStringLiteral("panelLocation"))) {
        m_panelLocation->blockSignals(true);
        m_panelLocation->setCurrentIndex(m_configReplica->panelLocation());
        m_panelLocation->blockSignals(false);
    }
    if (m_controlConfig.contains(QStringLiteral("panelSizePolicy"))) {
        m_panelSizePolicy->blockSignals(true);
        m_panelSizePolicy->setCurrentIndex(m_configReplica->panelSizePolicy());
        m_panelSizePolicy->blockSignals(false);
    }
    if (m_controlConfig.contains(QStringLiteral("panelAutoHide"))) {
        m_panelAutoHide->blockSignals(true);
        m_panelAutoHide->setChecked(m_configReplica->panelAutoHide());
        m_panelAutoHide->blockSignals(false);
    }
    if (m_controlConfig.contains(QStringLiteral("panelLock"))) {
        m_panelLock->blockSignals(true);
        m_panelLock->setChecked(m_configReplica->panelLock());
        m_panelLock->blockSignals(false);
    }
    if (!m_initialized) {
        connect(m_configReplica, &RemoteConfigReplica::panelTypeChanged, this, [&](GeneralConfigDefine::PanelType panelType) {
        m_panelType = panelType;
        if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
            m_classicType->setChecked(true);
        } else if(m_panelType == GeneralConfigDefine::PanelType::Version5) {
            m_threeIslandType->setChecked(true);
        }
        updateData();
        });
        connect(m_classicType, &QPushButton::clicked, this, [&] {
            if(m_panelType != GeneralConfigDefine::PanelType::Version4) {
                m_panelType = GeneralConfigDefine::PanelType::Version4;
                m_configReplica->setPanelType(GeneralConfigDefine::PanelType::Version4);
            }
        });
        m_classicType->setEnabled(true);
        connect(m_threeIslandType, &QPushButton::clicked, this, [&] {
            if(m_panelType != GeneralConfigDefine::PanelType::Version5) {
                m_panelType = GeneralConfigDefine::PanelType::Version5;
                m_configReplica->setPanelType(GeneralConfigDefine::PanelType::Version5);
            }
        });
        connect(m_mergeIcons, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index){
            m_configReplica->setMergeIcons(static_cast<GeneralConfigDefine::MergeIcons>(index));
        });
        connect(m_configReplica, &RemoteConfigReplica::mergeIconsChanged, m_mergeIcons, &QComboBox::setCurrentIndex);
        connect(m_panelLocation, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index){
        if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
            m_configReplica->setPanelLocation(static_cast<GeneralConfigDefine::PanelLocation>(index));
        } else {
            m_configReplica->setPanelLocationNext(static_cast<GeneralConfigDefine::PanelLocation>(index));
        }
        });
        connect(m_configReplica, &RemoteConfigReplica::panelLocationChanged, m_panelLocation, &QComboBox::setCurrentIndex);
        connect(m_configReplica, &RemoteConfigReplica::panelLocationNextChanged, m_panelLocation, &QComboBox::setCurrentIndex);
        connect(m_panelSizePolicy, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index){
            if (m_panelType == GeneralConfigDefine::PanelType::Version4) {
                m_configReplica->setPanelSizePolicy(static_cast<GeneralConfigDefine::PanelSizePolicy>(index));
            } else {
                m_configReplica->setPanelSizePolicyNext(static_cast<GeneralConfigDefine::PanelSizePolicy>(index));
            }
        });
        connect(m_configReplica, &RemoteConfigReplica::panelSizePolicyChanged, m_panelSizePolicy, [&](GeneralConfigDefine::PanelSizePolicy panelSize){
            if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
                m_panelSizePolicy->setCurrentIndex(panelSize);
            }
        });
        connect(m_configReplica, &RemoteConfigReplica::allowCustomSizeChanged, m_panelSizePolicy, [this, refreshSizePolicyItems](bool allowCustomSize) {
            refreshSizePolicyItems(allowCustomSize);
        });
        connect(m_configReplica, &RemoteConfigReplica::panelSizePolicyNextChanged, m_panelSizePolicy, [&](GeneralConfigDefine::PanelSizePolicy panelSize) {
            if(m_panelType == GeneralConfigDefine::PanelType::Version5) {
                m_panelSizePolicy->setCurrentIndex(panelSize);
            }
        });
        connect(m_panelAutoHide, &kdk::KSwitchButton::stateChanged, m_configReplica, [&](bool state) {
        if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
            m_configReplica->setPanelAutoHide(state);
        } else {
            m_configReplica->setPanelAutoHideNext(state);
        }
        });
        connect(m_configReplica, &RemoteConfigReplica::panelAutoHideChanged, m_panelAutoHide, [&](bool autoHide) {
            if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
                m_panelAutoHide->setChecked(autoHide);
            }
        });
        connect(m_configReplica, &RemoteConfigReplica::panelAutoHideNextChanged, m_panelAutoHide, [&](bool autoHide) {
            if(m_panelType == GeneralConfigDefine::PanelType::Version5) {
                m_panelAutoHide->setChecked(autoHide);
            }
        });
        connect(m_panelLock, &kdk::KSwitchButton::stateChanged, m_configReplica, [&](bool state) {
        if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
            m_configReplica->setPanelLock(state);
        } else {
            m_configReplica->setPanelLockNext(state);
        }

        });
        connect(m_configReplica, &RemoteConfigReplica::panelLockChanged, m_panelLock, [&](bool panelLock) {
            if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
                m_panelLock->setChecked(panelLock);
                if (!m_controlConfig.contains(QStringLiteral("panelLocation"))) {
                    m_panelLocation->setEnabled(!panelLock);
                }
                if (!m_controlConfig.contains(QStringLiteral("panelSizePolicy"))) {
                    m_panelSizePolicy->setEnabled(!panelLock);
                }
            }
        });
        connect(m_configReplica, &RemoteConfigReplica::panelLockNextChanged, m_panelLock, [&](bool panelLock) {
            if(m_panelType == GeneralConfigDefine::PanelType::Version5) {
                m_panelLock->setChecked(panelLock);
                m_panelSizePolicy->setEnabled(!panelLock);
            }
        });
        connect(m_iconsShowedOn, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index){
        m_configReplica->setTaskBarIconsShowedOn(static_cast<GeneralConfigDefine::TaskBarIconsShowedOn>(index));
        });
        connect(m_configReplica, &RemoteConfigReplica::taskBarIconsShowedOnChanged, m_iconsShowedOn, &QComboBox::setCurrentIndex);
        connect(m_showPanelOnAllScreens, &kdk::KSwitchButton::stateChanged, m_configReplica, &RemoteConfigReplica::setShowPanelOnAllScreens);
        connect(m_configReplica, &RemoteConfigReplica::showPanelOnAllScreensChanged, this, [&](bool showPanelOnAllScreens) {
            m_showPanelOnAllScreens->setChecked(showPanelOnAllScreens);
            //不在所有屏幕显示任务栏时，将任务栏图标选项设置为在主屏和打开了窗口的任务栏显示
            if(!showPanelOnAllScreens) {
                if(m_iconsShowedOn->currentIndex() == GeneralConfigDefine::TaskBarIconsShowedOn::PanelWhereWindowIsOpen) {
                    m_iconsShowedOn->setCurrentIndex(GeneralConfigDefine::TaskBarIconsShowedOn::AllPanels);
                }
            }
            if (m_controlConfig.contains(QStringLiteral("taskbarIconsShowedOn"))) {
                m_iconsShowedOn->setEnabled(false);
            } else {
                m_iconsShowedOn->setEnabled(showPanelOnAllScreens);
            }
            if (m_controlConfig.contains(QStringLiteral("showSystemTrayOnAllPanels"))) {
                m_showSystemTrayOnAllPanels->setEnabled(false);
            } else {
                m_showSystemTrayOnAllPanels->setEnabled(showPanelOnAllScreens);
            }

        });
        connect(m_showSystemTrayOnAllPanels, &kdk::KSwitchButton::stateChanged, m_configReplica, &RemoteConfigReplica::setShowSystemTrayOnAllPanels);
        connect(m_configReplica, &RemoteConfigReplica::showSystemTrayOnAllPanelsChanged, m_showSystemTrayOnAllPanels, &kdk::KSwitchButton::setChecked);
        connect(m_configReplica, &RemoteConfigReplica::disabledWidgetsChanged, m_showTaskView,
                [this](const QStringList&list) {
                    if (m_panelType == GeneralConfigDefine::PanelType::Version4) {
                        m_showTaskView->setChecked(!list.contains(QStringLiteral("org.ukui.panel.taskView")));
                    }
                });
        connect(m_configReplica, &RemoteConfigReplica::disabledWidgetsNextChanged, m_showTaskView,
                [this](const QStringList&list) {
                    if (m_panelType == GeneralConfigDefine::PanelType::Version5) {
                        m_showTaskView->setChecked(!list.contains(QStringLiteral("org.ukui.panel.taskView")));
                    }
                });
        connect(m_showTaskView, &kdk::KSwitchButton::stateChanged, this, [&](bool show) {
            if (m_panelType == GeneralConfigDefine::PanelType::Version4) {
                m_configReplica->disableWidget(QStringLiteral("org.ukui.panel.taskView"), !show);
            } else {
                m_configReplica->disableWidgetNext(QStringLiteral("org.ukui.panel.taskView"), !show);
            }
        });
        connect(m_configReplica, &RemoteConfigReplica::disabledWidgetsChanged, m_showSearch,
                [this](const QStringList&list) {
                    if (m_panelType == GeneralConfigDefine::PanelType::Version4) {
                        m_showSearch->setChecked(!list.contains(QStringLiteral("org.ukui.panel.search")));
                    }
                });
        connect(m_configReplica, &RemoteConfigReplica::disabledWidgetsNextChanged, m_showSearch,
                [this](const QStringList&list) {
                    if (m_panelType == GeneralConfigDefine::PanelType::Version5) {
                        m_showSearch->setChecked(!list.contains(QStringLiteral("org.ukui.panel.search")));
                    }
                });
        connect(m_showSearch, &kdk::KSwitchButton::stateChanged, this, [&](bool show) {
            if (m_panelType == GeneralConfigDefine::PanelType::Version4) {
                m_configReplica->disableWidget(QStringLiteral("org.ukui.panel.search"), !show);
            } else {
                m_configReplica->disableWidgetNext(QStringLiteral("org.ukui.panel.search"), !show);
            }
        });
        connect(m_configReplica, &RemoteConfigReplica::trayIconsCanInhibitedChanged, this,
                [this](const QStringList&list) {
                    onTrayIconsCanInhibitedChanged(list);
                });
    }
    m_initialized = true;
}

void PanelUkccPlugin::sourceStateChanged(QRemoteObjectReplica::State state, QRemoteObjectReplica::State oldState)
{
    qDebug() << "PanelUkccPlugin sourceStateChanged" << state;

    if(state == QRemoteObjectReplica::State::Suspect && oldState == QRemoteObjectReplica::State::Valid) {
        qWarning() << "Error  occurs after remote object initialized";
        m_mergeIcons->setEnabled(false);
        m_panelLocation->setEnabled(false);
        m_panelSizePolicy->setEnabled(false);
        m_panelAutoHide->setEnabled(false);
        m_panelLock->setEnabled(false);
        m_showTaskView->setEnabled(false);
        m_showPanelOnAllScreens->setEnabled(false);
        m_showSystemTrayOnAllPanels->setEnabled(false);
        m_iconsShowedOn->setEnabled(false);
        m_threeIslandType->setChecked(false);
        m_threeIslandType->setEnabled(false);
        m_classicType->setChecked(false);
        m_classicType->setEnabled(false);
        m_showSearch->setEnabled(false);
        QStringList trayIconList = m_systemTrayIconId;
        for(const QString &id : trayIconList) {
            removeSystemTrayIcon(id);
        }
    }
}

void PanelUkccPlugin::updateData()
{
    if(m_panelType == GeneralConfigDefine::PanelType::Version4) {
        m_panelSizePolicy->setCurrentIndex(m_configReplica->panelSizePolicy());
        m_panelLocation->setCurrentIndex(m_configReplica->panelLocation());
        m_panelAutoHide->setChecked(m_configReplica->panelAutoHide());
        m_panelLock->setChecked(m_configReplica->panelLock());
        m_panelSizePolicy->setEnabled(!m_configReplica->panelLock());
        m_showTaskView->setChecked(!m_configReplica->disabledWidgets().contains(QStringLiteral("org.ukui.panel.taskView")));
        m_showSearch->setChecked(!m_configReplica->disabledWidgets().contains(QStringLiteral("org.ukui.panel.search")));
    } else {
        m_panelSizePolicy->setCurrentIndex(m_configReplica->panelSizePolicyNext());
        m_panelLocation->setCurrentIndex(m_configReplica->panelLocationNext());
        m_panelAutoHide->setChecked(m_configReplica->panelAutoHideNext());
        m_panelLock->setChecked(m_configReplica->panelLockNext());
        m_panelSizePolicy->setEnabled(!m_configReplica->panelLockNext());
        m_showTaskView->setChecked(!m_configReplica->disabledWidgetsNext().contains(QStringLiteral("org.ukui.panel.taskView")));
        m_showSearch->setChecked(!m_configReplica->disabledWidgetsNext().contains(QStringLiteral("org.ukui.panel.search")));
        m_mergeIcons->setCurrentIndex(GeneralConfigDefine::MergeIcons::Always);
    }
    m_mergeIcons->setEnabled(m_panelType == GeneralConfigDefine::PanelType::Version4);
    m_showSystemTrayOnAllPanels->setEnabled(m_panelType == GeneralConfigDefine::PanelType::Version4);
    m_panelLocation->setEnabled(!m_configReplica->panelLock() && m_panelType == GeneralConfigDefine::PanelType::Version4);
}

int PanelUkccPlugin::virtualKeyboardCurrentIndex()
{
    if (m_gsetting && m_gsetting->keys().contains(TRAY_ICON_SHOW_POLICY)) {
        QString policy = m_gsetting->get(TRAY_ICON_SHOW_POLICY).toString();
        if (policy == "NeverShow")
            return VirtualKeyboardPolicy::NeverShow;
        if (policy == "AlwaysShow")
            return VirtualKeyboardPolicy::AlwaysSHow;
        if (policy == "ShowWhenKeyboardIsConnected")
            return VirtualKeyboardPolicy::ShowWhenKeyboardIsConnected;
    }

    return VirtualKeyboardPolicy::AlwaysSHow;
}

void PanelUkccPlugin::initFunctionControl()
{
    //管控配置
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-v11"));
    UkuiQuick::FunctionControl::instance()->addWatch(QStringLiteral("ukui-panel-taskManager-v11"));

    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlChanged, this, [this](const QString &name) {
        onFunctionControlChanged(name, false);
    });
    connect(UkuiQuick::FunctionControl::instance(), &UkuiQuick::FunctionControl::functionControlExit, this, [this]() {
        onFunctionControlChanged(QStringLiteral("ukui-panel-v11"), true);
        onFunctionControlChanged(QStringLiteral("ukui-panel-taskManager-v11"), true);
    });
}

void PanelUkccPlugin::onFunctionControlChanged(const QString &name, bool exit)
{
    if (name != QStringLiteral("ukui-panel-v11") && name != QStringLiteral("ukui-panel-taskManager-v11")) {
        return;
    }
    m_controlConfig = exit ? QJsonObject() : UkuiQuick::FunctionControl::instance()->functionControlObject(name).value(name).toObject();

    qDebug() << "Panel ukcc plugin function control config:" << m_controlConfig;

    if (name == QStringLiteral("ukui-panel-v11")) {
        //disableWidgets
        QStringList disableWidgets{};
        if (m_controlConfig.contains(QStringLiteral("disableWidgets"))) {
            for (QVariant &disableWidget : m_controlConfig.value(QStringLiteral("disableWidgets")).toArray().toVariantList()) {
                disableWidgets.append(disableWidget.toString());
            }
        }
        //enableWidgets
        QStringList enableWidgets{};
        if (m_controlConfig.contains(QStringLiteral("enableWidgets"))) {
            for (QVariant &enableWidget : m_controlConfig.value(QStringLiteral("enableWidgets")).toArray().toVariantList()) {
                enableWidgets.append(enableWidget.toString());
            }
        }
        m_showTaskView->setEnabled(!(disableWidgets.contains(QStringLiteral("org.ukui.panel.taskView")) ||
                                   enableWidgets.contains(QStringLiteral("org.ukui.panel.taskView"))));
        m_showSearch->setEnabled(!(disableWidgets.contains(QStringLiteral("org.ukui.panel.search")) ||
                                    enableWidgets.contains(QStringLiteral("org.ukui.panel.search"))));
        //showPanelOnAllScreens
        m_showPanelOnAllScreens->setEnabled(!m_controlConfig.contains(QStringLiteral("showPanelOnAllScreens")));

        //showSystemTrayOnAllPanels
        if (m_controlConfig.contains(QStringLiteral("showSystemTrayOnAllPanels"))) {
            m_showSystemTrayOnAllPanels->setEnabled(false);
        } else {
            m_showSystemTrayOnAllPanels->setEnabled(m_configReplica->showPanelOnAllScreens());
        }
    }
    if (name == QStringLiteral("ukui-panel-taskManager-v11")) {
        //mergeIcons
        if (m_controlConfig.contains(QStringLiteral("mergeIcons"))) {
            m_mergeIcons->setEnabled(false);
        } else {
            m_mergeIcons->setEnabled(m_panelType == GeneralConfigDefine::PanelType::Version4);
        }
        //taskbarIconsShowedOn
        if (m_controlConfig.contains(QStringLiteral("taskbarIconsShowedOn"))) {
            m_iconsShowedOn->setEnabled(false);
        } else {
            m_iconsShowedOn->setEnabled(m_configReplica->showPanelOnAllScreens());
        }
    }
}

void PanelUkccPlugin::onTrayIconsCanInhibitedChanged(const QStringList &trayIconList)
{
    QPair<QStringList, QStringList> diff = compareStringLists(m_systemTrayIconId, trayIconList);
    if (!diff.first.isEmpty()) {
        //added
        for (const QString &addedId : diff.first) {
            for (const TrayIconInfo &trayIconInfo : m_systemTrayIconInfo) {
                if (trayIconInfo.id == addedId && !m_systemTrayIconId.contains(addedId)) {
                    m_systemTrayIconId.append(trayIconInfo.id);
                    auto button = new SwitchButton(m_frame3, trayIconInfo.name, trayIconInfo.iconName);
                    button->setObjectName(trayIconInfo.id);
                    button->setChecked(!m_configReplica->trayIconsInhibited().contains(trayIconInfo.id));
                    connect(button, &SwitchButton::stateChanged, this, [&, button](bool show){
                        QStringList icons = m_configReplica->trayIconsInhibited();
                        if(!show && !icons.contains(button->objectName())) {
                            m_configReplica->setTrayIconsInhibited(icons << button->objectName());
                        } else if (show && icons.contains(button->objectName())) {
                            icons.removeAll(button->objectName());
                            m_configReplica->setTrayIconsInhibited(icons);
                        }
                    });
                    button->setEnabled(true);
                    //每添加一个butonn还要添加一条分割线，索引需要乘2
                    m_vLayout3->insertWidget(2 * getTrayIconIndex(trayIconInfo), button);
                    m_vLayout3->insertWidget(2 * getTrayIconIndex(trayIconInfo) + 1, setLine(m_frame3));
                }
            }
        }
    }
    if (!diff.second.isEmpty()) {
        //removed
        for (const QString &removedId : diff.second) {
            removeSystemTrayIcon(removedId);
        }
    }
}

QPair<QStringList, QStringList> PanelUkccPlugin::compareStringLists(const QStringList &oldList, const QStringList &newList)
{
    QSet<QString> oldSet = QSet<QString>(oldList.begin(), oldList.end());
    QSet<QString> newSet = QSet<QString>(newList.begin(), newList.end());

    QSet<QString> addedSet = newSet - oldSet;
    QSet<QString> removedSet = oldSet - newSet;

    return qMakePair(addedSet.values(), removedSet.values());
}

int PanelUkccPlugin::getTrayIconIndex(TrayIconInfo info)
{
    int index;
    for (index = 0; index < m_systemTrayIconInfo.size(); ++index) {
        if (m_systemTrayIconInfo[index].id == info.id)
            break;
    }
    if (index > m_systemTrayIconId.size() - 1)
        index = m_systemTrayIconId.size() - 1;

    return index;
}

void PanelUkccPlugin::removeSystemTrayIcon(const QString &id)
{
    if (m_systemTrayIconId.contains(id)) {
        m_systemTrayIconId.removeAll(id);
        auto button = m_frame3->findChild<SwitchButton *>(id, Qt::FindDirectChildrenOnly);
        QLayoutItem *line = m_vLayout3->itemAt(m_vLayout3->indexOf(button) + 1);
        m_vLayout3->removeItem(line);
        m_vLayout3->removeWidget(button);
        delete line;
        delete button;
    }
}

/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#include "menumodule.h"
#include "kyview.h"
#include "sizedate.h"
menuModule::menuModule(QWidget *parent) : QWidget(parent)
{
    this->setObjectName("Kylin-photo-viewer_QWidget_titleMenu");
    this->setAccessibleName("Kylin-photo-viewer_QWidget_titleMenu");
    this->setAccessibleDescription("title menu");
    init();
}
menuModule::~menuModule()
{
    if (m_menu != nullptr) {
        delete m_menu;
    }
}

void menuModule::init()
{
    initAction();
}

void menuModule::initAction()
{
    menuButton = new QToolButton(this);
    menuButton->setObjectName("Kylin-photo-viewer_QToolButton_titleMenuButton");
    menuButton->setAccessibleName("Kylin-photo-viewer_QToolButton_titleMenuButton");
    menuButton->setAccessibleDescription("title menu button");
    menuButton->setToolTip(tr("Option"));
    menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    menuButton->setProperty("isWindowButton", 0x1);
    menuButton->setProperty("useIconHighlightEffect", 0x2);
    menuButton->setIconSize(MICON_SIZES);
    menuButton->setPopupMode(QToolButton::InstantPopup);
    if (platForm.contains(Variable::platForm)) {
        menuButton->setFixedSize(MBTN_SIZE_INTEL);
    } else {
        menuButton->setFixedSize(MBTN_SIZE);
    }

    menuButton->setAutoRaise(true);
    menuButton->setFocusPolicy(Qt::NoFocus);
    m_menu = new QMenu();
    m_menu->setObjectName("Kylin-photo-viewer_QMenu_titleMenu");
    m_menu->setAccessibleName("Kylin-photo-viewer_QMenu_titleMenu");
    m_menu->setAccessibleDescription("title menu");

    QList<QAction *> actions;
    actionOpen = new QAction(m_menu);
    actionOpen->setText(tr("Open"));
    actionOpen->setObjectName("Kylin-photo-viewer_QAction_open");
    //    actionOpen->setFont(m_ft);

    QAction *actionTheme = new QAction(m_menu);
    actionTheme->setText(tr("Theme"));

    kdk::KMenuButton *kmb = new kdk::KMenuButton();

    actionHelp = new QAction(m_menu);
    actionHelp = kmb->assistAction();
    actionHelp->setObjectName("Kylin-photo-viewer_QAction_help");

    actionAbout = new QAction(m_menu);
    actionAbout = kmb->aboutAction();
    actionAbout->setObjectName("Kylin-photo-viewer_QAction_about");

    actionQuit = new QAction(m_menu);
    actionQuit = kmb->quitAction();
    actionQuit->setObjectName("Kylin-photo-viewer_QAction_quit");

    actionFeedback = new QAction(m_menu);
    actionFeedback = kmb->feedbackAction();
    actionFeedback->setObjectName("Kylin-photo-viewer_QAction_feedback");

    actions << actionOpen /*<<actionTheme*/ << actionHelp << actionAbout  << actionFeedback << actionQuit; //暂时禁掉主题切换按钮
    m_menu->addActions(actions);
    //    互斥按钮组
    QMenu *m_themeMenu = new QMenu;
    QActionGroup *m_themeMenuGroup = new QActionGroup(this);
    QAction *autoTheme = new QAction("Auto", this);
    m_themeMenuGroup->addAction(autoTheme);
    m_themeMenu->addAction(autoTheme);
    autoTheme->setCheckable(true);
    QAction *lightTheme = new QAction("Light", this);
    m_themeMenuGroup->addAction(lightTheme);
    m_themeMenu->addAction(lightTheme);
    lightTheme->setCheckable(true);
    QAction *darkTheme = new QAction("Dark", this);
    m_themeMenuGroup->addAction(darkTheme);
    m_themeMenu->addAction(darkTheme);
    darkTheme->setCheckable(true);
    QList<QAction *> themeActions;
    themeActions << autoTheme << lightTheme << darkTheme;
    actionTheme->setMenu(m_themeMenu);
    menuButton->setMenu(m_menu);
    connect(m_menu, &QMenu::aboutToShow, this, &menuModule::updateMenuActionState);
    connect(m_menu, &QMenu::triggered, this, &menuModule::triggerMenu);
    connect(this, &menuModule::menuModuleClose, Interaction::getInstance(), &Interaction::close);
    connect(this, &menuModule::menuModuleClose, this, &menuModule::exitAppFromMenu);
    //    connect(Interaction::getInstance(),&Interaction::progremExit, KyView::mutual, &KyView::close);
}


void menuModule::triggerMenu(QAction *act)
{
    if (actionQuit == act) {
        Q_EMIT menuModuleClose();
    } else if (actionAbout == act) {
        Q_EMIT aboutShow();
    } else if (actionHelp == act) {
        Q_EMIT helpShow("kylin-photo-viewer");
    } else if (actionOpen == act) {
        Q_EMIT openSignal();
    }
}

void menuModule::updateMenuActionState()
{
    if (operayteMode == OperayteMode::Ocr) {
        for (int i = 0; i < m_menu->actions().length(); i++) {
            if (m_menu->actions().at(i)->text() == tr("Open")) {
                m_menu->actions().at(i)->setEnabled(false);
            }
        }
    } else {
        for (int i = 0; i < m_menu->actions().length(); i++) {
            if (m_menu->actions().at(i)->text() == tr("Open")) {
                if (!m_menu->actions().at(i)->isEnabled()) {
                    m_menu->actions().at(i)->setEnabled(true);
                }
            }
        }
    }
}

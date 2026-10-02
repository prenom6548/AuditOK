/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include <usermanual.h>
#include <QWidget>
#include "controller/core/viewcontrol.h"
#include "windowmanage.hpp"
#include "ukuiwindowhelper.h"
menuModule::menuModule(QWidget *parent) : QWidget(parent)
{
    init();
}

void menuModule::init()
{
    initAction();
}

void menuModule::initAction()
{
    menuButton = new QToolButton(this);
    menuButton->setAccessibleName("kylin-weather_QToolButton_MenuModule_menuButton");
    //    menuButton = new QPushButton(this);
    menuButton->setToolTip(tr("Options"));
    menuButton->setIcon(QIcon::fromTheme("open-menu-symbolic"));
    menuButton->setProperty("isWindowButton", 0x1);
    menuButton->setProperty("useIconHighlightEffect", 0x2);
    menuButton->setProperty("isOptionButton", true);
    menuButton->setIconSize(QSize(16, 16));
    menuButton->setPopupMode(QToolButton::InstantPopup);
    menuButton->setFixedSize(QSize(32, 32));
    menuButton->setAutoRaise(true);
    menuButton->setFocusPolicy(Qt::NoFocus);
    menuButton->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));

    m_menu = new QMenu();
    m_menu->setAccessibleName("kylin-weather_QMenu_MenuModule_menu");

    // Location
    QMenu *actionLocation = new QMenu();
    actionLocation->setAccessibleName("kylin-weather_QMenu_MenuModule_actionLocation");
    actionLocation->setTitle(tr("Location"));
    m_locationYes = new QAction();
    m_locationYes->setText(tr("yes"));
    //    m_locationYes->setIcon(QIcon::fromTheme("emblem-insync-synced"));
    m_locationNo = new QAction();
    m_locationNo->setText(tr("no"));

    if ("1" == CoreVar::getSettings("allow-locate").toString()) {
        //        m_locationYes->setIcon(QIcon::fromTheme("emblem-insync-synced"));
        m_locationYes->setIcon(QIcon());
    } else {
        m_locationNo->setIcon(QIcon());
    }

    actionLocation->addAction(m_locationYes);
    actionLocation->addAction(m_locationNo);

    m_menu->addMenu(actionLocation);

    QAction *actionHelp = new QAction();
    actionHelp->setText(tr("Help"));
    m_menu->addAction(actionHelp);

    QAction *actionAbout = new QAction();
    actionAbout->setText(tr("About"));
    m_menu->addAction(actionAbout);

    KMenuButton *menubutton = new KMenuButton();
    menubutton->setAccessibleName("kylin-weather_KMenuButton_MenuModule_menubutton");
    m_menu->addAction(menubutton->feedbackAction());

    QAction *actionQuit = new QAction();
    actionQuit->setText(tr("Quit"));
    m_menu->addAction(actionQuit);

    //    actions<<actionHelp<<actionLocation<<actionAbout<<actionQuit;//暂时禁掉主题切换按钮
    //    actions<<actionHelp<<actionAbout<<actionQuit;//暂时禁掉主题切换按钮
    //    m_menu->addActions(actions);

    menuButton->setMenu(m_menu);
    initConnect();

    //    connect(this,&menuModule::menuModuleClose,Interaction::getInstance(),&Interaction::close);
}

void menuModule::triggerMenu(QAction *act)
{
    QString str = act->text();
    if (tr("Quit") == str) {
        emit menuModuleClose();
    } else if (tr("About") == str) {
        aboutAction();
    } else if (tr("Help") == str) {
        helpAction();
    } else if (tr("yes") == str) {
        m_locationYes->setIcon(QIcon(":/res/contorl_icons/selected_normal.png"));
        m_locationNo->setIcon(QIcon());
        emit allowLocationYes();
        //        m_locationNo->setIcon(QIcon());
    } else if (tr("no") == str) {
        m_locationNo->setIcon(QIcon(":/res/contorl_icons/selected_normal.png"));
        m_locationYes->setIcon(QIcon());
        emit allowLocationNo();
        //        m_locationYes->setIcon(QIcon());
    }
}

void menuModule::initConnect()
{
    connect(m_menu, &QMenu::triggered, this, &menuModule::triggerMenu);
    connect(this, &menuModule::allowLocationYes, Interaction::getInstance(), &Interaction::allowLocationYes); // 允许定位信号
    connect(this, &menuModule::allowLocationNo, Interaction::getInstance(), &Interaction::allowLocationNo); // 不允许定位信号
    connect(this, &menuModule::menuModuleClose, Interaction::getInstance(), &Interaction::closePrograme); //关闭程序
    //是否允许定位选项及关于服务与支持链接样式跟随主题
    connect(ViewVar::getInstance(), &ViewVar::monitorThemeChanged, this, &menuModule::monitorThemeChanged);
}

void menuModule::monitorThemeChanged(QString themeChange)
{
    if ("1" == CoreVar::getSettings("allow-locate").toString()) {
        if ("ukui-dark" == themeChange || "ukui-black" == themeChange) {
            m_locationYes->setIcon(QIcon(QIcon(":/res/contorl_icons/selected_dark.png")));
        } else {
            m_locationYes->setIcon(QIcon(QIcon(":/res/contorl_icons/selected_normal.png")));
        }
    } else {
        if ("ukui-dark" == themeChange || "ukui-black" == themeChange) {
            m_locationNo->setIcon(QIcon(QIcon(":/res/contorl_icons/selected_dark.png")));
        } else {
            m_locationNo->setIcon(QIcon(QIcon(":/res/contorl_icons/selected_normal.png")));
        }
    }
}

void menuModule::setMenuModuleAllowLocationYes()
{
    m_locationYes->setIcon(QIcon(QIcon(":/res/contorl_icons/selected_normal.png")));
    m_locationNo->setIcon(QIcon());
    emit allowLocationYes();
}

void menuModule::helpAction()
{
    if(viewControl::getInstance()->activeWindow()->windowState() == Qt::WindowFullScreen)
        viewControl::getInstance()->activeWindow()->showMaximized();
    kdk::kabase::UserManualManagement userManualTest;
    if (!userManualTest.callUserManual("kylin-weather")) {
        qCritical() << "user manual call fail!";
    }

    return;
}

void menuModule::aboutAction()
{
    //    关于点击事件处理
    initAbout();
}

int menuModule::executeCMD(QString command, QString &result)
{
    QProcess p;
    QStringList args;
    args << "-c" << command;
    p.start("bash", args);
    p.waitForFinished();
    p.waitForReadyRead();
    result = p.readAll();
    return 0;
}

QString menuModule::getVersion()
{
#ifdef NOTKAIMING
    QString version;
    executeCMD("dpkg -l kylin-weather | grep kylin-weather", version);
    QStringList fields = version.split(QRegularExpression("[ \t]+"));
    if (fields.size() >= 3)
        version = fields.at(2);
    else
        version = "none";
#else
    QString version = QString(qgetenv("APP_VERSION"));
#endif
    return version;
}

void menuModule::initAbout()
{
    kdk::KAboutDialog aboutWindow(this, QIcon::fromTheme("kylin-weather"), tr("Weather"), tr("Version: ").append(menuModule::getVersion()));
    aboutWindow.setBodyText(tr("Weather is a weather information tool, it can browse the weather conditions and living index."));
    aboutWindow.setBodyTextVisiable(false);
    UkuiWindowHelper *helper = new UkuiWindowHelper(this);
    helper->removeTitleBar();
    aboutWindow.exec();
}


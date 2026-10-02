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

#include <QTime>
#include <gsettingmonitor.h>
#include <usermanual.h>
#include "windowmanage.hpp"

#include "titleseting.h"

#include "menumodule.h"
#include "view/kyview.h"
#include "controller/control.h"
#include "ukuiwindowhelper.h"

menuModule::menuModule(QWidget *parent = nullptr) : QWidget(parent)
{
    //失焦仍显示提示信息
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    init();
}

void menuModule::init()
{
    initAction();
    setStyle();
}

void menuModule::initAction()
{
    iconSize = QSize(30, 30);

    m_menu = new QMenu();
    m_menu->setAccessibleName("kylin-ipmsg_menumodule_QMenu_mmenu");
    QList<QAction *> actions;

    QAction *actionSetUp = new QAction(m_menu);
    actionSetUp->setText(tr("Settings"));
    QAction *actionTheme = new QAction(m_menu);
    actionTheme->setText(tr("Theme"));
    QAction *actionHelp = new QAction(m_menu);
    actionHelp->setText(tr("Help"));
    QAction *actionAbout = new QAction(m_menu);
    actionAbout->setText(tr("About"));
    QAction *actionQuit = new QAction(m_menu);
    actionQuit->setText(tr("Quit"));
    KMenuButton* menubutton = new KMenuButton();
    actions << actionSetUp << actionHelp << actionAbout << menubutton->feedbackAction() << actionQuit;
    actionSetUp->setObjectName("kylin-ipmsg_menumodule_QAction_actionSetUp");
    actionTheme->setObjectName("kylin-ipmsg_menumodule_QAction_actionTheme");
    actionHelp->setObjectName("kylin-ipmsg_menumodule_QAction_actionHelp");
    actionAbout->setObjectName("kylin-ipmsg_menumodule_QAction_actionAbout");
    actionQuit->setObjectName("kylin-ipmsg_menumodule_QAction_actionQuit");

    m_menu->addActions(actions);

    //    互斥按钮组
    QMenu *themeMenu = new QMenu;
    themeMenu->setAccessibleName("kylin-ipmsg_menumodule_QMenu_themeMenu");
    QActionGroup *themeMenuGroup = new QActionGroup(this);
    themeMenuGroup->setObjectName("kylin-ipmsg_menumodule_QActionGroup_themeMenuGroup");
    QAction *autoTheme = new QAction(tr("Follow the Theme"), this);
    autoTheme->setObjectName("kylin-ipmsg_menumodule_QAction_autoTheme");
    themeMenuGroup->addAction(autoTheme);
    themeMenu->addAction(autoTheme);
    autoTheme->setCheckable(true);
    QAction *lightTheme = new QAction(tr("Light Theme"), this);
    lightTheme->setObjectName("kylin-ipmsg_menumodule_QAction_lightTheme");
    themeMenuGroup->addAction(lightTheme);
    themeMenu->addAction(lightTheme);
    lightTheme->setCheckable(true);
    QAction *darkTheme = new QAction(tr("Dark Theme"), this);
    darkTheme->setObjectName("kylin-ipmsg_menumodule_QAction_darkTheme");

    themeMenuGroup->addAction(darkTheme);
    themeMenu->addAction(darkTheme);
    darkTheme->setCheckable(true);
    QList<QAction *> themeActions;
    themeActions << autoTheme << lightTheme << darkTheme;
    //    autoTheme->setChecked(true);
    actionTheme->setMenu(themeMenu);
    connect(m_menu, &QMenu::triggered, this, &menuModule::triggerMenu);
    initGsetting();
    setThemeFromLocalThemeSetting(themeActions);
    themeUpdate();
    connect(themeMenu, &QMenu::triggered, this, &menuModule::triggerThemeMenu);
}

void menuModule::setThemeFromLocalThemeSetting(QList<QAction *> themeActions)
{
    // m_pGsettingThemeStatus = new QGSettings(confPath.toLocal8Bit());
    // QString appConf = m_pGsettingThemeStatus->get("thememode").toString();
    // if ("lightonly" == appConf) {
    //     themeStatus = themeLightOnly;
    //     themeActions[1]->setChecked(true); //程序gsetting中为浅色only的时候就给浅色按钮设置checked
    // } else if ("darkonly" == appConf) {
    //     themeStatus = themeBlackOnly;
    //     themeActions[2]->setChecked(true);
    // } else {
    //     themeStatus = themeAuto;
    //     themeActions[0]->setChecked(true);
    // }
}

void menuModule::themeUpdate()
{
    if (themeStatus == themeLightOnly) {
        setThemeLight();
    } else if (themeStatus == themeBlackOnly) {
        setThemeDark();
    } else {
        setStyleByThemeGsetting();
    }
}

void menuModule::setStyleByThemeGsetting()
{
    refreshThemeBySystemConf();
}

void menuModule::triggerMenu(QAction *act)
{
    QString str = act->text();

    if (tr("About") == str) {
        initAbout();
    } else if (tr("Help") == str) {
        helpAction();
    } else if (tr("Theme") == str) {
        // emit menuModuleSetThemeStyle(QString("主题"));
    } else if (tr("Settings") == str) {
        setupAction();
    } else if (tr("Quit") == str) {
        this->window()->close();
    }
}

void menuModule::triggerThemeMenu(QAction *act)
{
    // if (!m_pGsettingThemeStatus) {
    //     m_pGsettingThemeStatus =
    //         new QGSettings(confPath.toLocal8Bit()); // m_pGsettingThemeStatus指针重复使用避免占用栈空间
    // }
    // QString str = act->text();
    // if ("Light" == str) {
    //     themeStatus = themeLightOnly;
    //     disconnect(m_pGsettingThemeData, &QGSettings::changed, this, &menuModule::dealSystemGsettingChange);
    //     m_pGsettingThemeStatus->set("thememode", "lightonly");
    //     //        disconnect()
    //     setThemeLight();
    // } else if ("Dark" == str) {
    //     themeStatus = themeBlackOnly;
    //     disconnect(m_pGsettingThemeData, &QGSettings::changed, this, &menuModule::dealSystemGsettingChange);
    //     m_pGsettingThemeStatus->set("thememode", "darkonly");
    //     setThemeDark();
    // } else {
    //     themeStatus = themeAuto;
    //     m_pGsettingThemeStatus->set("thememode", "auto");
    //     initGsetting();
    //     //        updateTheme();
    //     themeUpdate();
    // }
}

void menuModule::helpAction()
{
    // 帮助点击事件处理
    kdk::UserManual userManualTest;
    if (!userManualTest.callUserManual("kylin-ipmsg")) {
        qCritical() << "user manual call fail!";
    }
}

void menuModule::setupAction()
{
    TitleSeting *titleSetUp = new TitleSeting(this);
    titleSetUp->setAccessibleName("kylin-ipmsg_menumodule_TitleSeting_titleSetUp");
    if (QGuiApplication::platformName().startsWith(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        titleSetUp->show();
        ::kabase::WindowManage::setMiddleOfScreen(titleSetUp);
    }
    else{
        titleSetUp->show();
    }
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(titleSetUp->windowHandle());
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);
    titleSetUp->update();
}

QString menuModule::getfontviewerVersion()
{

#ifdef NOTKAIMING
       FILE *pp = NULL;
       char *line = NULL;
       size_t len = 0;
       ssize_t read;
       char *q = NULL;
       QString version = tr("none");

       pp = popen("dpkg -l kylin-ipmsg", "r");
       if(NULL == pp)
           return version;

       while((read = getline(&line, &len, pp)) != -1){
           q = strrchr(line, '\n');
           *q = '\0';

           QString content = line;
           QStringList list = content.split(" ");

           list.removeAll("");

           if (list.size() >= 3)
               version = list.at(2);
       }

       free(line);
       pclose(pp);   
#else
    QString version = QString(qgetenv("APP_VERSION"));
#endif
    return version;
}

#include <QDialog>
void menuModule::initAbout()
{
    appVersion = getfontviewerVersion();
    kdk::KAboutDialog aboutWindow(this, QIcon::fromTheme("kylin-ipmsg"), tr(appShowingName.toLocal8Bit()), tr("Version: ") + appVersion);
    aboutWindow.setBodyText(tr("Message provides text chat and file transfer functions in the LAN. "
                      "There is no need to build a server. "
                      "It supports multiple people to interact at the same time "
                      "and send and receive in parallel."));
    aboutWindow.setBodyTextVisiable(false);
    aboutWindow.setModal(true);
    aboutWindow.show();
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(aboutWindow.windowHandle());
    windowhelper->setSkipTaskBar(true);
    windowhelper->setSkipSwitcher(true);
    aboutWindow.exec();

}

void menuModule::setStyle()
{
    // m_menuButton->setObjectName("m_menuButton");
    // // qDebug() << "m_menuButton->styleSheet" << m_menuButton->styleSheet();
    // m_menuButton->setStyleSheet("QPushButton::menu-indicator{image:None;}");
}

void menuModule::initGsetting()
{
    // 主题颜色
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]() {
        GlobalData::getInstance()->getSystemTheme();
        refreshThemeBySystemConf();
    });
}

void menuModule::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_F1) {
        emit pullupHelp();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void menuModule::dealSystemGsettingChange(const QString key)
{
    if (key == "styleName") {
        refreshThemeBySystemConf();
    }
}

void menuModule::refreshThemeBySystemConf()
{
    if (GlobalSizeData::THEME_COLOR == GlobalSizeData::UKUILight) {
        setThemeLight();
    } else {
        setThemeDark();
    }
}

void menuModule::setThemeDark()
{
    emit menuModuleSetThemeStyle("dark-theme");
}

void menuModule::setThemeLight()
{
    emit menuModuleSetThemeStyle("light-theme");
}

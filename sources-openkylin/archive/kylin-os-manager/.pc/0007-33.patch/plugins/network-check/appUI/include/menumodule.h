/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef MENUMODULE_H
#define MENUMODULE_H

#include <QApplication>
#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QToolButton>
#include <QDebug>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRect>
#include <QScreen>
#include <QGSettings>
#include <QMetaEnum>

#include "daemonipcdbus.h"
#include "kaboutdialog.h"
using namespace kdk;
//#include "global_variable.h"

class MenuModule : public QWidget
{
    Q_OBJECT
public:
    explicit MenuModule(QWidget *parent = nullptr);
    void themeUpdate();
    void setQSSFontSize(QFont curFont);
signals:
    void menuModuleClose();
    void menuModuleChanged(QString);

    void menuModuleSetThemeStyle(QString);
    void showConfigureWin();

private:
    QToolButton *m_menuButton = nullptr;

private:
    //    程序在实例化的时候需要传递的信息字段,打开debug开关后这些字段会被自动填充
    QString m_appName = tr("NetWork Check Tools");
    QString m_appShowingName = tr("NetWork Check Tools");
    QString m_appVersion = qApp->applicationVersion();
    QString m_appDesc = tr("NetWork Check Tools");
    //    HelpManual *m_helpWindow = nullptr;
    QDialog *m_aboutWindow = nullptr;

private:
    QWidget *m_mainWin;
    QMenu *m_menu = nullptr;
    QMenu *m_themeMenu = nullptr;
    QSize m_iconSize;
    QString m_appPath = ""; //拉起帮助菜单时使用appName字段
    QPushButton *m_titleIconBtn;
    QPushButton *m_bodyIcon = nullptr;

    QLabel *m_bodySupport;
    QLabel *m_titleText;
    QLabel *m_bodyAppName;
    QLabel *m_bodyAppVersion;
    QLabel *m_bodyAppDescribe; //增加介绍

private:
    void init();
    QHBoxLayout *initTitleBar(); //关于窗口标题栏初始化
    QVBoxLayout *initBody();     // 关于窗口body初始化
    void initGsetting();
    void initAction();
    void setStyle();
    void triggerMenu(QAction *act); //主菜单动作4
    void aboutAction();
    void initAbout();    //关于窗口初始化
    void initAboutSDK(); //使用SDK的关于窗口
    void helpAction();
    void setStyleByThemeGsetting(); //通过外部主题配置设置主题
    void setThemeStyle();
    void setThemeLight();
    void setThemeDark();
    void showUserManual();
    //    void updateTheme(); //点击菜单中的主题设置后更新一次主题

    QGSettings *m_themeSettings = nullptr;
    QString m_curStyle = "ukui-default";
    void refreshThemeBySystemConf(); //通过系统配置更改主题
};

#endif // MENUMODULE_H

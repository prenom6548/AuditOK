/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#define DEBUG_MENUMODULE 0  //DEBUG模式开关，代码上线之前务必关掉
#define FITTHEMEWINDOWS "org.ukui.style"
#define FITCONTROLTRANS "org.ukui.control-center.personalise"


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
#include <QDesktopServices>
#include <QUrl>

#include "UIControl/base/daemonipcdbus.h"
#include "UI/base/widgetstyle.h"
#include "kaboutdialog.h"

class menuModule : public QWidget
{
    Q_OBJECT

    friend class unitMainWindow;
public:
    explicit menuModule(QWidget *);
    void themeUpdate();

    enum typeThemeStatus {
        themeAuto = 0,
        themeBlackOnly = 1,
        themeLightOnly = 2
    } themeStatus;

Q_SIGNALS:
    void menuModuleSetThemeStyle(QString);

public:
    QToolButton *menuButton = nullptr;

//    程序在实例化的时候需要传递的信息字段,打开debug开关后这些字段会被自动填充
    QString appName = "kylin-music"; //格式kylin-usb-creator
//    QString appShowingName = tr("kylin music"); //格式kylin usb creator ,用来在前端展示
//    QString appVersion = "1.1.0";
    QString appDesc = "appDesc字段未填充";
    QString iconPath = ":/img/kylin-music.png";
    QString confPath = "org.kylin-music-data.settings";

private:
    QMenu *m_menu = nullptr;
    QMenu *themeMenu = nullptr;
    QSize iconSize;
    QString appPath = "tools/kylin-music"; //拉起帮助菜单时使用appName字段

    QString appShowingName = tr("kylin-music");
    QString appVersion = qApp->applicationVersion();
    kdk::KAboutDialog *m_aboutWindow = nullptr; // 关于界面

    QGSettings *m_pGsettingThemeData = nullptr;
    QGSettings *m_pGsettingThemeStatus = nullptr;

public Q_SLOTS:
    void dealSystemGsettingChange(const QString);
    void slotLableSetFontSize(int size);

private:
    void init();
    void initGsetting();
    void initAction();
    void setStyle();
    void triggerMenu(QAction* act); //主菜单动作4
    void triggerThemeMenu(QAction* act); //主题动作
    void aboutAction();
    void initAbout(); //关于窗口初始化
    void helpAction();
    void setThemeFromLocalThemeSetting(QList<QAction* >); //获取本地主题配置
    void setStyleByThemeGsetting(); //通过外部主题配置设置主题
    void setThemeStyle();
    void setThemeLight();
    DaemonIpcDbus *ipcDbus;

    void setThemeDark();
    QVBoxLayout *mainlyt = nullptr;

    void refreshThemeBySystemConf();    //通过系统配置更改主题
};

#endif // MENUMODULE_H

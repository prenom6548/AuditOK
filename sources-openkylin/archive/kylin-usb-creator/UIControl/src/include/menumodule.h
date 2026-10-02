/*
 * Copyright (C) 2002  KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#ifndef MENUMODULE_H
#define MENUMODULE_H
//#define FITTHEMEWINDOW "org.ukui.style"
#define APPDATA "org.kylin-usb-creator-data.settings"
#define UKUI_STYLE "org.ukui.style"
#define UKUI_FONT_SIZE "systemFontSize"
#include <QObject>
#include <QWidget>
#include <QMenu>
#include <QPushButton>
#include <QKeyEvent>
#include <QToolButton>
#include <QDebug>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDesktopServices>
#include <QRect>
#include <QUrl>
#include <QScreen>
#include <QGSettings>
#include <QMetaEnum>
#include <QTextEdit>
#include "daemonipcdbus.h"

// 测试友元类声明
class TestMenuModule;

class menuModule : public QWidget
{
    Q_OBJECT
    
    // 声明测试类为友元类，以便访问私有成员和函数
    friend class TestMenuModule;

public:
    explicit menuModule(QWidget *);
    bool isWayland=false;
    void themeUpdate();
    void fontSizeInit();
    void initsuccessbackdilog();
    void limitTextEditTextCount(QTextEdit* text_edit,const int& limit_count);
    void setThemeDark();
    bool m_isDarktheme;
    bool isDarkTheme() {
        return m_isDarktheme;
    }

    QWidget *aboutWindow = nullptr;
    QWidget *sucbackWindow = nullptr;

signals:
    void menuModuleClose();
    void menuModuleSetThemeStyle(QString);
    void pullupHelp();
    void iconchange();
    void setFontSize(int);
    void clickabout();
    
public:
    QToolButton *menuButton = nullptr;

public:
    // 程序在实例化的时候需要传递的信息字段,打开debug开关后这些字段会被自动填充
    QString appName = "tools/kylin-usb-creator"; //格式kylin-usb-creator
    QString appVersion = "1.2.0";
    QString appDesc = "appDesc字段未填充!";
    QString iconPath = ":/data/kylin-usb-creator.png";
    QString confPath = "org.kylin-usb-creator-data.settings";

private:
    QMenu *m_menu = nullptr;
    QMenu *themeMenu = nullptr;
    QAction *actionHelp;
    QAction *actionAbout;
    QAction *actionTheme;
    QAction *actionsetting;
    QAction *actionQuit;
    QAction *autoTheme;
    QAction *lightTheme;
    QAction *darkTheme;
    QAction *checkupdatehelp;
    QAction *successbackhelp;
    QAction *officewebsitehelp;
    QSize iconSize;
    QString appPath = "tools/kylin-usb-creator"; //拉起帮助菜单时使用appName字段
    QGSettings *m_pGsettingThemeData = nullptr;
    QGSettings *m_pGsettingThemeStatus = nullptr;
    
    enum typeThemeStatus {
        themeAuto = 0,
        themeBlackOnly = 1,
        themeLightOnly = 2
    } themeStatus;

public slots:
    void dealSystemGsettingChange(const QString);
    void dealSetFontSize(double size);
    void successbackAction();
    void slot_limitTextEditTextCount();
    void initGsetting();
    void triggerMenu(QAction* act); //主菜单动作4
    void triggerThemeMenu(QAction* act); //主题动作
    void aboutAction();
    void helpAction();
    void setStyleByThemeGsetting(); //通过外部主题配置设置主题
    void setThemeLight();
    void uiModeChange(bool);
    
private:
    void keyPressEvent(QKeyEvent *event);
    void init();
    QHBoxLayout* initTitleBar(); //关于窗口标题栏初始化
    QVBoxLayout* initBody();    // 关于窗口body初始化
    QVBoxLayout* initsucebody();
    void initAction();
    void setStyle();
    void triggerHelpMenu(QAction* act);
    void initAbout(); //关于窗口初始化
    void setThemeFromLocalThemeSetting(QList<QAction* >); //获取本地主题配置
    void setThemeStyle();
    DaemonIpcDbus *ipcDbus = nullptr;
    QLabel* titleText = nullptr;
    QLabel* bodyAppName = nullptr;
    QLabel* bodyAppVersion = nullptr;
    QLabel* bodyAppDesc = nullptr;
    QLabel* bodySupport = nullptr;
    QLabel* bodyIcon = nullptr;
    QLabel* titleIcon = nullptr;
    QTextEdit * textedit = nullptr;
    QLabel *toollabel = nullptr;
    QPushButton *submitbtn = nullptr;
    QWidget * body = nullptr;
    bool m_isTabletMode;
    void refreshThemeBySystemConf();    //通过系统配置更改主题

    // 私有辅助函数，供测试使用
    QMenu* getMainMenu() const { return m_menu; }
    QMenu* getThemeMenu() const { return themeMenu; }
    QGSettings* getThemeDataSettings() const { return m_pGsettingThemeData; }
    QGSettings* getThemeStatusSettings() const { return m_pGsettingThemeStatus; }
    bool getIsTabletMode() const { return m_isTabletMode; }
    void setThemeStatusDirect(int status) { themeStatus = static_cast<typeThemeStatus>(status); }
    int getThemeStatus() const { return themeStatus; }
};

#endif // MENUMODULE_H
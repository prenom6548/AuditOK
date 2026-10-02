/* greeterwindow.h
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#ifndef GREETERWINDOW_H
#define GREETERWINDOW_H

#include <QWidget>
#include <QStackedLayout>
#include <QSharedPointer>
#include <QtConcurrent/QtConcurrentRun>
#include <QTranslator>
#include <QLocale>
#include <QMap>
#include <QPluginLoader>
#include <QScrollArea>
#include <QTimer>
#include <QTabWidget>
#include "libinputswitchevent.h"
#include "loginwindow.h"
#include "sessionwindow.h"
#include "usersmodel.h"
#include "greeterwrapper.h"
#include "ktooltip.h"
#include "surewidget.h"
#include "xeventmonitor.h"
#include "agreementwindow.h"
#include "batterywidget.h"
#include "greeterservice.h"
#include <libkydate.h>

class Tooltip;
class KLabel;
class UsersView;
class SessionsView;
class PowerManager;
class QProcess;
class QGraphicsOpacityEffect;
class Configuration;
class VirtualKeyboardWidget;
class LanguageWidget;
class Language;
class KylinNM;
class Interface;

enum dateinfo {
    DATE,
    TIME,
};

enum TABAT {
    EMPTY = -1,
    LINEEDIT = 0,
    ENTERBTN,
    BIOBTN,
    BOTTMBTN,
    POWERMANAGER,
    MESSAGEBTN,
};

enum HORIZONBTN {
    BATTERYBTN = 0,
    SWITCHBTN,
    NETWORKBTN,
    KEYBOARDBTN,
    POWERBTN,
    SESSIONBTN,
};

class GreeterWindow : public QWidget
{
    Q_OBJECT
public:
    GreeterWindow(QWidget *parent = 0);
    void initUI();
    QString guessBackground();
    void setRootWindow(); 
    QString getCurUser();
    QString getBatteryIconName();
    void setCurUserName(QString strUserName);

signals:
    void currentUserChange(QString strUserName);
    void authenticationSucess(QString userName);

protected:
    void resizeEvent(QResizeEvent *event);
    void keyReleaseEvent(QKeyEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);

private slots:
    void onAuthenticationComplete1();
    void onUserSelected(const QModelIndex &);
    void onCurrentUserChanged(const QModelIndex&);
    void timedAutologin();
    void showPowerWnd(bool keynavigation = false);
    void showNetManager(bool keynavigation = false);
    void onClickPassword();
    void onShowMessageBtn(bool is_show);
    void showBattery();
    void toolTipShow(const QString &text);
    void toolTipHide();
    void netInPutStatus();
    void showVirtualKeyboard();
    void showUserMenu();
    void onLanguageChanged(QString languageCode);
    void showSessionWnd();
    void onSessionChanged(const QString &session);
    void onUserChangedByManual(const QString &userName);
    void setWindowVisible();
    void switchToSureDialog(int stateType, int lockType);
    void hideSureDialog();
    void confirmClicked();
    void onGlobalKeyPress(const QString &key);
    void onGlobalkeyRelease(const QString &key);
    void resetNavigation();
    void setBatteryIcon(const QString &str);
    void dealMessage(QDBusMessage);
    void hideBottomPlugins();
    void onPrepareForSleep(bool sleep);

    /**
     * @brief onServiceUserChanged 登录服务用户切换
     * @param willLoginUserInfo 新用户信息
     */
    void onServiceUserChanged(WillLoginUserInfo willLoginUserInfo);

    /**
     * @brief onCustomRequestAccount 请求切换认证用户
     * @param account 用户名
     */
    void onCustomRequestAccount(QString account);
    /**
     * @brief onGetCustomPluginMsg 获取插件消息槽
     * @param strMsg 消息json
     */
    void onGetCustomPluginMsg(QString strMsg);

private:
    void switchWnd(int);
    void setBackground(const QModelIndex &index);
    void setVirkeyboardPos();
    void setWindowPos(QWidget *widget, Qt::Alignment align = Qt::AlignCenter);
    bool sessionIsValid(const QString &session);
    void updateLanguage(QString userName);
    void updateSession(QString userName);
    void setUserWindowVisible(bool visible);
    void refreshTranslate();
    void drawBackground();
    void updateNetIcon(int status);
    QString getAccountBackground(uid_t uid);
    int getNetStatus();
    void loadNetPlugin();
    void unloadNetPlugin();
    void setBottomBtnVisible(bool visible);
    static QString getHibited_tr(int type);
    void key_OB_release(int key);
    void key_tab_release(int key);
    void key_shiftTab_release();
    void key_LR_release(int key);
    void key_enter_release(int key);
    void key_exit_release();
    void setBottomBtnSheet();
    void setCheckedSheet(int type, bool show);
    void netResetLocation();
    QString getLongFormatDate(int type);
    //tooltip
    Tooltip         *kTooltip = nullptr;
    bool            is_Show = false;
    int height1 ;
    int width1 ;

    UsersView               *m_userWnd;
    LoginWindow             *m_loginWnd;
    PowerManager            *m_powerWnd;
    QPushButton             *m_keyboardLB;
    QPushButton             *m_powerLB;
    QPushButton		    *m_nmLB = nullptr;
    QWidget                 *widgetNetworkManager = nullptr;
    QPushButton             *m_btnSwitchUser = nullptr;
    QPushButton             *m_sessionLB;
    QPushButton             *m_batteryiconLB = nullptr;
    VirtualKeyboardWidget         *m_virtualKeyboard;
    QScrollArea             *scrollArea = nullptr;
    QWidget                 *scrollContents = nullptr;
    GreeterService          *m_greeterServive = nullptr;

    // session widget
    QScrollArea             *m_scrollAreaSession = nullptr;
    QWidget                 *m_scrollContentSession = nullptr;
    SessionsView            *m_sessionsWnd = nullptr;

    GreeterWrapper          *m_greeter;
    UsersModel              *m_usersModel;
    QAbstractItemModel      *m_sessionsModel;
    Configuration           *m_configuration;
    int                     timeType;
    QString                 dataType;
    double                  curFontSize;
    bool                     m_sessionHasChanged;

    QTimer 		    *timer = nullptr;
    QWidget	    	    *widgetTime;
    KLabel		    *lblDate;
    KLabel		    *lblTime;
    QLocale                 local;
    QString                 currentUser = "";
    QFuture<void>           future;
    QFuture<void>           futureNM;
    QWidget                 *m_kylinNM = nullptr;
    QTabWidget              *mkylinNM = nullptr;
    QWidget                 *m_NetManagerWidget = nullptr;
    BatteryWidget           *mBatteryWidget = nullptr;

    bool		    isNetFinished = false;

    Surewidget              *sureWidget = nullptr;
    int                   powermanagerType;

    QPluginLoader netloader;
    QPluginLoader wlanloader;

    bool               is_switchBtn = true;
    bool               is_batteryBtn = true;
    bool               m_isSessionBtnShow = false;
    int                tabAt = LINEEDIT;
    int                horAT = SWITCHBTN;
    bool                at_power = false;
    bool               at_plugins = false;

    QDBusInterface *batInterface = nullptr;
    QDBusInterface *iface = nullptr;
    QDBusInterface *dface = nullptr;
    LibinputSwitchEvent *libswitch = nullptr;
    bool isTableMode = false;

    bool in_switchUser = false; //当前是否是切换用户的标志位

    bool m_isShowKeyboard = false; // 打开电源管理界面前虚拟键盘是否已打开
    bool m_isCustomDefault = false;         /** 是否默认使用第三方认证 */
    bool m_isShowNetwork = true;            /** 是否显示网络插件 */
    bool m_isShowUserSwitch = true;         /** 是否显示用户切换 */
    AgreementWindow         *m_agreementWindow = nullptr;
    QMap<QPushButton*,bool> btnVisMap;
};

#endif // GREETERWINDOW_H

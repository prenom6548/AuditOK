/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __KMRE_WINDOW_H__
#define __KMRE_WINDOW_H__

#include <QWidget>
#include <QByteArray>
#include <QKeyEvent>
#include <QThread>
#include <QBoxLayout>
#include <QToolButton>
#include <QMouseEvent>
#include <QSystemTrayIcon>
#include <QTimer>

#include <thread>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "common.h"

class DisplayWidget;
class DisplayWorkManager;
class DisplayManager;
class QStackedLayout;
class AutohideWidget;
class TitleBar;
class TitleBarJoystick;
class TitleMenu;
class SettingsWidget;
class PopupMenu;
class PopupTip;
class QShortcut;
class QGSettings;
class QClipboard;
class EventListener;
class WindowStretchWorker;
class RecordScreenWidget;
class LoadingWidget;
class WarningNotice;
class LockWidget;
class SimpleLockWidget;
class QJoysticks;
class GameKeyStackWidget;
class JoystickManager;
class JoystickGamekeyManager;
class KeyboardGamekeyManager;
class GameKeyManager;
class Notification;
class ScreenCapture;
class Tray;
class CrashDialog;

class KmreWindow : public QWidget
{
    Q_OBJECT

public:
    explicit KmreWindow(int id, int width, int height, const QString& appName, const QString& packageName);
    ~KmreWindow();

    void initResolutionMonitor();
    int  initialize();
    void initMisc();
    void setTrayIcon(QIcon &icon);
    QString getIconPath(){return mIconPath;}
    QString getTitleName();

    void updateDisplay();
    void updateDisplay(QString appName, int displayId, int width, int height);
    int getDisplayId() {return mId;}
    QString getAppName() {return mAppName;}
    DisplayManager* getDisplayManager() {return mDisplayManager;}
    GameKeyManager* getGameKeyManager() {return mGameKeyManager;}
    JoystickGamekeyManager* getJoystickGameKeyManager() {return mJoystickGamekeyManager;}
    JoystickManager* getJoystickManager() {return mJoystickManager;}
    KeyboardGamekeyManager* getKeyboardGamekeyManager() {return mKeyboardGamekeyManager;}

    RecordScreenWidget* createRecordScreenWidget();
    TitleMenu* createMenu();
    TitleBar* createTitlebar();
    TitleBarJoystick* createJoystickTitlebar();
    GameKeyStackWidget* creatGameKeyStackWidget();
    AutohideWidget* createAutoHideWidget();

    void handleNotification(const QString &text, bool stop, bool call, const QString &title);
    void setEscShortBlockSignals(bool b);
    void handleJoysticKey();      //添加游戏摇杆用方法
    bool isGameKeySettingsEnabled() {return mGameKeyEnabled;}
    bool isReadyClose(){return m_readyClose;}
    QString getPackageName() {return mPackageName;}
    void resizeWindow();
    bool isWindowFocused();
    bool isScreenLocked() {return mScreenLocked;}
    void switchMultipler(QString pkgName, int displayId, bool enable);
    bool getShakeStatus();
    void closeWindow(bool force = false);
    void startBootTimer();
    void stopBootTimer();
    bool isOwner(unsigned long winId);
    void limitMouse();
    void forceUpdating();
    void showCrashedHintDialog();
    void appLaunchCompleted() {m_appLaunchCompleted = true;}
    bool isAppLaunchCompleted() {return m_appLaunchCompleted;}

public slots:
    void shakeEvent();
    void onInputMethodCommands(int id, bool enable, int x, int y);
    void handleGameKey();
    void onResponseBack();
    void onGestureBack();
    void onHelpEvent();
    void onClearApkCache();
    void onCloseEnvEvent();
    void onAboutEvent();
    void onGameKey(bool fullscreen);
    void onJoystick();   //添加对应的游戏手柄按键点击槽
    void slotFocusWindowChanged();
    void showMainWindow(bool firstShow = false);
    void minimizeWindow();
    void slotToTop(bool isTop);
    void onShakeWindow();
    void onAppResumed();
    void shareFileToAndroid(const QString &path);
    void shareFileToAndroid(const QString &path, const QString &displayName, int displayId, bool check);
    void onLockScreen(bool lock);
    void onPlasmaWindowAdded();
    void onPlasmaWindowTitleChanged();

protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    bool event(QEvent *e) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void moveEvent(QMoveEvent *event) Q_DECL_OVERRIDE;
    void hideEvent(QHideEvent *event) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;
    bool nativeEvent(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
    void initGameKeyWidgets();
    void sendControlAppCmd(int cmd, bool mainDisplay = false);
    void updateUiAfterRestoreFromHidden();
    void updatePlasmaWindowId();

private:
    int mId;
    int mWidth;
    int mHeight;
    QString mAppName;
    QString mTitleName;
    QString mPackageName;
    QSize mVirtualDisplaySize;
    QSize mPhysicalDisplaySize;
    QTimer* mBootTimer = nullptr;

    DisplayManager *mDisplayManager = nullptr;
    GameKeyManager* mGameKeyManager = nullptr;
    JoystickGamekeyManager* mJoystickGamekeyManager = nullptr;
    JoystickManager* mJoystickManager = nullptr;
    QJoysticks* mJoystick = nullptr;
    KeyboardGamekeyManager* mKeyboardGamekeyManager = nullptr;

    bool mGameKeyEnabled;
    QString mIconPath;
    std::unique_ptr<Notification> mNotifyication;
    QShortcut *m_escShortcut = nullptr;
    bool isEvenHidden = false;

    bool m_wasMinimized;
    bool m_isActiveWindow;//是否是活动窗口
    bool m_readyClose;//准备退出
    bool m_appCrashedOnAndroid;
    bool m_appLaunchCompleted;

    Tray* mTray = nullptr;
    QPoint mPositionBeforeHiden;// 窗口隐藏前的位置
    LockWidget *mMainLockWidget = nullptr;
    SimpleLockWidget *mLockWidget = nullptr;
    ScreenCapture* mScreenCapture = nullptr;
    std::unique_ptr<CrashDialog> mCrashDialog;
    bool mScreenLocked = false;
    bool m_shakeStaus = false;
    bool mMouselimit = false;

    QString mTitle;
    quint32 mPlasmaWindowId = 0;
};

#endif // __KMRE_WINDOW_H__

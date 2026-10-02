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

#include "kmrewindow.h"
#include "android_display/normaldisplaywidget.h"
#include "dbusclient.h"
#include "utils.h"
#include "kmreenv.h"
#include "preferences.h"
#include "common.h"
#include "widgets/about.h"
#include "widgets/popuptip.h"
#include "displaybackend/displaybackend.h"
#include "keyboard/keyevent_handler.h"
#include "widgets/titlebar.h"
#include "widgets/titlebarjoystick.h"
#include "widgets/menu.h"
#include "widgets/warningnotice.h"
#include "widgets/xatom-helper.h"
#include "windowmanager.h"
#include "app_control_manager.h"
#include "displaymanager.h"
#include "windowstretchworker.h"
#include "widgets/recordscreenwidget.h"
#include "widgets/loadingwidget.h"
#include "widgets/autohidewidget.h"
#include "widgets/messagebox.h"
#include "widgets/lockwidget.h"
#include "widgets/crashdialog.h"
#include "gamekey/qjoysticks.h"
#include "gamekey/gamekeystackwidget.h"
#include "gamekey/joystickmanager.h"
#include "gamekey/joystickgamekeymanager.h"
#include "gamekey/gamekeymanager.h"
#include "widgets/scrollsettingwidget.h"
#include "notification.h"
#include "screencapture.h"
#include "tray.h"
#include "appsettings.h"
#include "sessionsettings.h"

#include "wayland/ukui/ukui-decoration-manager.h"
#include "wayland/ukui/plasma-shell-manager.h"
#include "wayland/xdg/XdgManager.h"
#include "wayland/gtk/GtkShellManager.h"

#include <QApplication>
#include <QWindow>
#include <QX11Info>
#include <QDebug>
#include <QTimer>
#include <QShortcut>
#include <QRect>
#include <QFile>
#include <QDir>
#include <QDBusReply>
#include <QScreen>
#include <QProcess>
#include <QMessageBox>
#include <QMouseEvent>

#include <xcb/xcb.h>
#include <xcb/xproto.h>
#include <X11/Xlib.h>

KmreWindow::KmreWindow(int id, int width, int height, const QString& appName, const QString& packageName)
  : QWidget(nullptr)
  , mDisplayManager(nullptr)
  , mId(id)
  , mWidth(width)
  , mHeight(height)
  , mAppName(appName)
  , mTitleName("")
  , mPackageName(packageName)
  , mGameKeyEnabled(false)
  , m_wasMinimized(false)
  , m_isActiveWindow(true)
  , m_readyClose(false)
  , m_appCrashedOnAndroid(false)
  , m_appLaunchCompleted(false)
  , mPositionBeforeHiden(QPoint(0, 0))
  , mScreenLocked(false)
{
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        this->setWindowFlags(Qt::FramelessWindowHint);
    }

#ifdef KYLIN_V10
    this->setWindowFlags(Qt::FramelessWindowHint);
#endif
    this->setMouseTracking(true);
    this->setFocusPolicy(Qt::NoFocus);//ClickFocus
    this->setAttribute(Qt::WA_AcceptTouchEvents);
    this->setAttribute(Qt::WA_DeleteOnClose);
    //disable style window manager
    this->setProperty("useStyleWindowManager", false);//for UKUI 3.0
    // this->setStyleSheet("QWidget{background: palette(base); border-radius: 6px;}");

    QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp.setHeightForWidth(true);
    setSizePolicy(sp);
    setContentsMargins(0, 0, 0, 0);

    //边框阴影
    // this->setAttribute(Qt::WA_TranslucentBackground);
    // this->setProperty("useCustomShadow", true);
    // this->setProperty("customShadowDarkness", 1.0);
    // this->setProperty("customShadowWidth", 40);
    // this->setProperty("customShadowRadius", QVector4D(4, 4, 4, 4));
    // this->setProperty("customShadowMargins", QVector4D(35, 35, 35, 35));

    mIconPath = utils::getIconPath(mPackageName);
    //syslog(LOG_DEBUG, "App(%s) icon: '%s'", mPackageName.toStdString().c_str(), mIconPath.toStdString().c_str());
    if (!mIconPath.isEmpty()) {
        this->setWindowIcon(QIcon(mIconPath));
    }
    this->setWindowTitle(getTitleName());// set title for system taskbar

    KmreConfig::Preferences::getInstance()->reload();
    mGameKeyEnabled = true;// GameKeyManager::initGameSettingsEnable(mPackageName);
    mScreenCapture = new ScreenCapture(this);
    mDisplayManager = new DisplayManager(this, width, height, packageName);
    mDisplayManager->initUI();
    mTray = new Tray(this);
    

    initResolutionMonitor();
    initMisc();

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        PlasmaShellManager* psm = PlasmaShellManager::getInstance();
        connect(psm, SIGNAL(windowAdded()), this, SLOT(onPlasmaWindowAdded()));
        connect(psm, SIGNAL(windowTitleChanged()), this, SLOT(onPlasmaWindowTitleChanged()));
    }

    // 设置在任务栏的窗口组别
    char nameBuf[256] = {0};
    char classBuf[256] = {0};
    snprintf(nameBuf, 256, "%s", getTitleName().toStdString().c_str());
    snprintf(classBuf, 256, "%s", mPackageName.toStdString().c_str());
    //syslog(LOG_DEBUG, "[%s] nameBuf: '%s', classBuf: '%s'", __func__, nameBuf, classBuf);

    XClassHint classhint;
    classhint.res_name = nameBuf;
    classhint.res_class = classBuf;
    syslog(LOG_DEBUG, "[%s] Set window class hint: '%s', '%s'", __func__, classhint.res_name, classhint.res_class);

    Display *display = XOpenDisplay(NULL);
    if (display) {
        XSetClassHint(display, this->winId(), &classhint);
        XCloseDisplay(display);
    }


#ifndef KYLIN_V10
    // 添加窗管协议
    if (SessionSettings::getInstance().windowUsePlatformX11() &&
            QX11Info::isPlatformX11()) {
        XAtomHelper::getInstance()->setUKUIDecoraiontHint(this->winId(), true);
        MotifWmHints hints;
        hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
        hints.functions = MWM_FUNC_ALL;
        hints.decorations = MWM_DECOR_BORDER;
        XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);
    }
#endif
}

KmreWindow::~KmreWindow()
{
    syslog(LOG_DEBUG, "[%s] Destroy app '%s' window.", __func__, mPackageName.toStdString().c_str());
    stopBootTimer();

    auto appConfig = AppSettings::getInstance().getAppConfig(mPackageName);
    appConfig.bootMaxSize = isFullScreen();
    appConfig.bootWidth = mDisplayManager->getDisplayWidth();
    appConfig.bootHeight = mDisplayManager->getDisplayHeight();
    AppSettings::getInstance().setAppConfig(mPackageName, appConfig);
}

void KmreWindow::startBootTimer()
{
    if (!mBootTimer) {
        mBootTimer = new QTimer(this);
        connect(mBootTimer, &QTimer::timeout, this, [&] {
            syslog(LOG_ERR, " App '%s' boot time out! Please restart app.", mPackageName.toStdString().c_str());
            mBootTimer->stop();
            KylinUI::MessageBox::warning(this, tr("Boot time out"), tr("App boot time out! Please restart app."));
            this->closeWindow(true);
        });
        mBootTimer->start(10000);// 10s
    }
}

void KmreWindow::stopBootTimer()
{
    if (mBootTimer) {
        mBootTimer->stop();
        delete mBootTimer;
    }
    mBootTimer = nullptr;
}

void KmreWindow::forceUpdating()
{
    DisplayWidget* displayWidget = mDisplayManager->getMainDisplayWidget();
    if (displayWidget) {
        displayWidget->forceUpdating();
    }

    displayWidget = mDisplayManager->getAccessoryDisplayWidget();
    if (displayWidget) {
        displayWidget->forceUpdating();
    }
}

void KmreWindow::updateDisplay()
{
    if ((mId < 0) || (mWidth <= 0) || (mHeight <= 0)) {
        syslog(LOG_ERR, "[%s] Invalid display information!", __func__);
        return;
    }

    mDisplayManager->updateMainDisplay(mId, mWidth, mHeight);
}

void KmreWindow::updateDisplay(QString appName, int displayId, int width, int height)
{
    if (appName.isEmpty() || (displayId < 0) || (width <= 0) || (height <= 0)) {
        syslog(LOG_ERR, "[%s] Invalid display information!", __func__);
        return;
    }

    if (mId >= 0) {
        syslog(LOG_ERR, "[%s] Display has already updated.", __func__);
        return;
    }

    syslog(LOG_DEBUG, "[%s] appName: '%s', displayId = %d, width = %d, height = %d",
        __func__, appName.toStdString().c_str(), displayId, width, height);

    mId = displayId;
    mAppName = appName;

    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        if (mTitle.isEmpty()) {
            mTitle = appName;
            this->setWindowTitle(mTitle);
            updatePlasmaWindowId();
        }
    }

    mDisplayManager->getTitleBar()->updateTitleName(appName);
    mDisplayManager->updateMainDisplay(displayId, width, height);
}

// 监控系统分辨率的改变
void KmreWindow::initResolutionMonitor()
{
    QScreen *screen = QApplication::primaryScreen();
    Q_ASSERT(screen);
    connect(screen, &QScreen::availableGeometryChanged, this, [=] (const QRect &geometry) {
        mDisplayManager->setDefaultWindowSize(false);
    });
    connect(screen, &QScreen::geometryChanged, this, [=] (const QRect &geometry) {
        syslog(LOG_DEBUG, "GeometryChanged, width = %d, height = %d", geometry.width(), geometry.height());

        if (geometry.width() >= geometry.height()) {
            AppControlManager::getInstance()->controlApp(mId, mPackageName, 12);
        }
        else {
            AppControlManager::getInstance()->controlApp(mId, mPackageName, 13);
        }
    });
}

void KmreWindow::initGameKeyWidgets()
{
    if (!mGameKeyManager) {
        mGameKeyManager = new GameKeyManager(this, mGameKeyEnabled);
        DisplayWidget* mainDisplayWidget = mDisplayManager->getMainDisplayWidget();
        connect(mainDisplayWidget, SIGNAL(walkKeyProcess(WalkDirectionType, bool)), mGameKeyManager, SLOT(onWalkKeyProcess(WalkDirectionType, bool)));    
        connect(mainDisplayWidget, SIGNAL(oneKeyPressed(QString)), mGameKeyManager, SLOT(onKeyPressed(QString)));
        connect(mainDisplayWidget, SIGNAL(oneKeyRelease(QString)), mGameKeyManager, SLOT(onKeyRelease(QString)));

        AutohideWidget* autohideWidget = mDisplayManager->getAutohideWidget();
        connect(autohideWidget, SIGNAL(requestHideSettings()), mGameKeyManager, SLOT(saveGameKeys()));
    }

    if (!mJoystickGamekeyManager) {
        mJoystickGamekeyManager = new JoystickGamekeyManager(this);
    }

    if (!mJoystick) {
        mJoystick = new QJoysticks(mJoystickGamekeyManager, this);
    }

    GameKeyStackWidget* gameKeyStackWidget = mDisplayManager->getGameKeyStackWidget();

    if (!mJoystickManager) {
        mJoystickManager = new JoystickManager(this);
        gameKeyStackWidget->setJoystickManager(mJoystickManager);
        TitleBarJoystick* titleBarJoystick = mDisplayManager->getTitleBarJoystick();
        connect(titleBarJoystick, SIGNAL(exitJoystickSettings()), mJoystickManager, SLOT(exitJoystickSetting()));
    }

    if (!mKeyboardGamekeyManager) {
        mKeyboardGamekeyManager = new KeyboardGamekeyManager(this);
        DisplayWidget* mainDisplayWidget = mDisplayManager->getMainDisplayWidget();
        connect(mainDisplayWidget, SIGNAL(keyboardPress(QKeyEvent*)), mKeyboardGamekeyManager, SLOT(getKeyboardGamekeyPress(QKeyEvent*)));
        connect(mainDisplayWidget, SIGNAL(keyboardRelease(QKeyEvent*)), mKeyboardGamekeyManager, SLOT(getKeyboardGamekeyRelease(QKeyEvent*)));
        connect(mainDisplayWidget, SIGNAL(mousePress(QMouseEvent*)), mKeyboardGamekeyManager, SLOT(getMouseGamekeyPress(QMouseEvent*)));
        connect(mainDisplayWidget, SIGNAL(mouseRelease(QMouseEvent*)), mKeyboardGamekeyManager, SLOT(getMouseGamekeyRelease(QMouseEvent*)));
        connect(mainDisplayWidget, SIGNAL(mouseMove(QMouseEvent*)), mKeyboardGamekeyManager, SLOT(getMouseGamekeyMove(QMouseEvent*)));
        connect(mainDisplayWidget, SIGNAL(focusOut()), mKeyboardGamekeyManager, SLOT(focusOutMethod()));
        connect(mainDisplayWidget, SIGNAL(mouseLeave()), mKeyboardGamekeyManager, SLOT(resetMousePos()));

        connect(mKeyboardGamekeyManager, SIGNAL(isShowMouseSignal(bool)), mainDisplayWidget, SLOT(isShowMouse(bool)));
        connect(mKeyboardGamekeyManager, SIGNAL(settingMousePosSignal(QPoint)), mainDisplayWidget, SLOT(settingMousePos(QPoint)));
    }

    gameKeyStackWidget->readyGameKeyStackWidget();
}

RecordScreenWidget* KmreWindow::createRecordScreenWidget()
{
    RecordScreenWidget *recordScreenWidget = new RecordScreenWidget(this);
    connect(recordScreenWidget, SIGNAL(sigRefreshMainUI()), mDisplayManager, SLOT(onDisplayForceRedraw()));
    
    return recordScreenWidget;
}

GameKeyStackWidget* KmreWindow::creatGameKeyStackWidget()
{
    GameKeyStackWidget* joystickSetWidget = new GameKeyStackWidget(this);
    return joystickSetWidget;
}

TitleMenu* KmreWindow::createMenu()
{
    TitleMenu *titleMenu = new TitleMenu(mPackageName, this);

    connect(titleMenu, &TitleMenu::sigShake, [this]() {this->shakeEvent();});
    connect(titleMenu, &TitleMenu::sigGps, [this]() {
        if (QFile::exists("/usr/bin/kylin-kmre-gps")) {
            QProcess::startDetached("/usr/bin/kylin-kmre-gps");
        }
    });
    connect(titleMenu, &TitleMenu::sigSensor, [this]() {
        if (QFile::exists("/usr/bin/kylin-kmre-sensorwindow")) {
            QProcess::startDetached("/usr/bin/kylin-kmre-sensorwindow");
        }
    });
    connect(titleMenu, &TitleMenu::sigRotate, [this]() {sendControlAppCmd(25,true);});
    connect(titleMenu, SIGNAL(sigManual()), this, SLOT(onHelpEvent()));
    connect(titleMenu, SIGNAL(sigClearApk()), this, SLOT(onClearApkCache()));
    connect(titleMenu, SIGNAL(sigCloseEnv()), this, SLOT(onCloseEnvEvent()));
    connect(titleMenu, &TitleMenu::sigPref, [this]() {
        if (QFile::exists("/usr/bin/kylin-kmre-settings")) {
            QProcess::startDetached("/usr/bin/kylin-kmre-settings");
        }
    });
    connect(titleMenu, SIGNAL(sigJoystick()), this, SLOT(onJoystick()));
    connect(titleMenu, SIGNAL(sigAbout()), this, SLOT(onAboutEvent()));
    connect(titleMenu, &TitleMenu::sigQuit, [this]() {closeWindow(true);});
    connect(titleMenu, &TitleMenu::sigRecordScreen, [this]() {//录屏
        auto recordScreenWidget = mDisplayManager->getRecordScreenWidget(true);
        if (recordScreenWidget) {
            mDisplayManager->resizeRecordScreenWidget();
            recordScreenWidget->showOrHideUI();
        }
    });
    connect(titleMenu, &TitleMenu::sigKeyboard, [this]() {sendControlAppCmd(9);});//虚拟键盘
    connect(titleMenu, &TitleMenu::sigOpenStorage, [this](const QString &path) {//打开存储设备目录
        //QProcess::execute("xdg-open", QStringList() << path);
        kmre::utils::openFolder(path);
    });

    connect(titleMenu, &TitleMenu::sigOpenGallery, [this](const QString &path) {//打开图库
        //QProcess::execute("xdg-open", QStringList() << path);
        kmre::utils::openFolder(path);
    });

    connect(titleMenu, &TitleMenu::sigOpenMMFiles, [this](const QString &path) {//打开微信下载的文件目录
        //QProcess::execute("xdg-open", QStringList() << path);
        kmre::utils::openFolder(path);
    });

    connect(titleMenu, &TitleMenu::sigOpenQQFiles, [this](const QString &path) {//打开QQ接收的文件目录
        //QProcess::execute("xdg-open", QStringList() << path);
        kmre::utils::openFolder(path);
    });

    connect(titleMenu, &TitleMenu::sigLockScreen, [=]() {
        if (mMainLockWidget) {
            mMainLockWidget->close();
            mMainLockWidget = nullptr;
        }
        mMainLockWidget = new LockWidget(this);
        mMainLockWidget->show();
        KmreWindowManager::getInstance()->lockScreen(true);
    });//锁屏

    connect(titleMenu, &TitleMenu::sigscroll, [this]() {
        ScrollSettingWidget *scrollWidget = new ScrollSettingWidget(mPackageName, this);
        scrollWidget->show();
    });

    connect(titleMenu, &TitleMenu::sigMultipleEnabled, [this](bool enable) {// 打开/关闭平行界面
        //设置完后会收到由Android发送的更新命令，那时再更新配置文件
        AppControlManager::getInstance()->controlApp(0, mPackageName, enable ? 15 : 16);
        KylinUI::MessageBox::information(this, tr("Message"), tr("It takes effect after restarting the app"));
    });

    return titleMenu;
}

TitleBar* KmreWindow::createTitlebar()
{
    TitleBar *titleBar = new TitleBar(this);

    titleBar->setIcon(mIconPath);
    connect(titleBar, SIGNAL(sigBack()), this, SLOT(onResponseBack()));
    connect(titleBar, SIGNAL(sigFullscreen()), mDisplayManager, SLOT(onSwitchFullscreen()));
    connect(titleBar, &TitleBar::sigShowMenu, this, [=]() {
        TitleMenu* titleMenu = mDisplayManager->getTitleMenu();
        if (titleMenu) {
            titleMenu->updateUi();
            titleMenu->exec(QPoint(titleBar->getMenuBtnX() + mapToGlobal(QPoint(0,0)).x(),
                                 mapToGlobal(QPoint(0,0)).y() + titleBar->height() - 5));
        }
    });
    connect(titleBar, &TitleBar::sigMiniSize, this, &KmreWindow::minimizeWindow);
    connect(titleBar, &TitleBar::sigClose, [this]() {
        closeWindow(mBootTimer ? true : false);
    });
    connect(titleBar, &TitleBar::sigToTop, [this](bool top) {slotToTop(top);});
    connect(titleBar, SIGNAL(sigToHide(bool)), mScreenCapture, SLOT(onHideWindowDuringScreenShot(bool)));
    connect(titleBar, SIGNAL(sigScreenShot()), mScreenCapture, SLOT(onScreenShot()));

    return titleBar;
}

TitleBarJoystick* KmreWindow::createJoystickTitlebar()
{
    TitleBarJoystick* titleBarj = new TitleBarJoystick(this);
    titleBarj->setIcon(mIconPath);
    return titleBarj;
}

AutohideWidget* KmreWindow::createAutoHideWidget()
{
    AutohideWidget *autohideWidget = new AutohideWidget(mPackageName, this);

    connect(autohideWidget, SIGNAL(sigForceRedraw()), mDisplayManager, SLOT(onDisplayForceRedraw()));
    connect(autohideWidget, &AutohideWidget::requestSettings, this, [=] (const QPoint &pos, bool fullscreen) {
        Q_UNUSED(pos);
        onGameKey(fullscreen);
    });
    
    connect(autohideWidget, SIGNAL(sigBack()), this, SLOT(onResponseBack()));
    connect(autohideWidget, SIGNAL(sigExitFullscreen()), mDisplayManager, SLOT(onSwitchFullscreen()));
    connect(autohideWidget, &AutohideWidget::sigMiniSize, this, &KmreWindow::minimizeWindow);
    connect(autohideWidget, &AutohideWidget::sigClose, this, &KmreWindow::close);

    return autohideWidget;
}

void KmreWindow::initMisc()
{
    m_escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(m_escShortcut, &QShortcut::activated, this, [&]{
        mDisplayManager->onExitFullscreen();
    });
    connect(qApp, &QGuiApplication::focusWindowChanged, this, &KmreWindow::slotFocusWindowChanged);

    mNotifyication = std::make_unique<Notification>(this);
}

int KmreWindow::initialize()
{
    mDisplayManager->initialize();
    showMainWindow(true);

    // wayland 窗口无边框，需要在窗口show()之后执行
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        UKUIDecorationManager::getInstance()->removeHeaderBar(this->windowHandle());
        UKUIDecorationManager::getInstance()->setCornerRadius(this->windowHandle(), 12, 12, 12, 12);
    }

    updateDisplay();

    return 0;
}

void KmreWindow::onLockScreen(bool lock)
{
    syslog(LOG_DEBUG, "[%s] lock = %d", __func__, lock);
    mScreenLocked = lock;
    if (lock) {
        if (!mMainLockWidget) {
            if (mLockWidget) {
                mLockWidget->close();
            }
            mLockWidget = new SimpleLockWidget(this);
            if (isMinimized()) {
                mLockWidget->hide();
            }
            else {
                mLockWidget->show();
            }
        }
    }
    else {
        if (mMainLockWidget) {
            mMainLockWidget->close();
            mMainLockWidget = nullptr;
        }
        if (mLockWidget) {
            mLockWidget->close();
            mLockWidget = nullptr;
        }
    }
}

void KmreWindow::updatePlasmaWindowId()
{
    if (!SessionSettings::getInstance().windowUsePlatformWayland()) {
        return;
    }

    if (mPlasmaWindowId != 0) {
        return;
    }

    PlasmaShellManager* psm = PlasmaShellManager::getInstance();
    mPlasmaWindowId = psm->takeWindowId(mTitle);
    if (mPlasmaWindowId != 0) {
        disconnect(psm, SIGNAL(windowAdded()), this, SLOT(onPlasmaWindowAdded()));
        disconnect(psm, SIGNAL(windowTitleChanged()), this, SLOT(onPlasmaWindowTitleChanged()));
    }
}

void KmreWindow::onPlasmaWindowAdded()
{
    updatePlasmaWindowId();
}

void KmreWindow::onPlasmaWindowTitleChanged()
{
    updatePlasmaWindowId();
}

// b为true时阻塞信号，为false取消信息阻塞
void KmreWindow::setEscShortBlockSignals(bool b)
{
    if (m_escShortcut) {
        m_escShortcut->blockSignals(b);
    }
}

void KmreWindow::onHelpEvent()
{
    kmre::utils::showUserGuide();
}

void KmreWindow::onClearApkCache()
{
    if (KylinUI::MessageBox::question(this, tr("Clear Apk Cache?"), tr("The operation will clean up APK cache in KMRE. Please ensure that no apk is performing installation, Continue?"))) {
        kmre::utils::clearApkFiles();
    }
}

void KmreWindow::onCloseEnvEvent()
{
    if (KylinUI::MessageBox::question(this, tr("Close Kmre?"), tr("After closing Kmre, all mobile application windows will be forced to close. Are you sure you want to close Kmre?"))) {
        kmre::DbusClient::getInstance()->StopContainer(utils::getUserName(), getuid());
        //防止因意外收不到“StopContainer”信号而导致window无法退出
        KmreWindowManager::getInstance()->waitForExit(5000);
    }
}

void KmreWindow::onAboutEvent()
{
    About about(mIconPath, this);
    // 显示到应用中间
    about.move(mapToGlobal(QPoint(0,0)) + QPoint(width() - about.width(), height() - about.height())/2);
    about.show();
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        UKUIDecorationManager::getInstance()->removeHeaderBar(about.windowHandle());
        UKUIDecorationManager::getInstance()->setCornerRadius(about.windowHandle(), 12, 12, 12, 12);
        about.setFixedSize(about.size());
    }
    about.exec();
}

void KmreWindow::shakeEvent()
{
    m_shakeStaus = true;
    mDisplayManager->hideGameKeysWhileMoving();
    onShakeWindow();
    QDBusInterface *p_sensorInterface = new QDBusInterface("com.kylin.Kmre.sensor",
                                                           "/",
                                                           "com.kylin.Kmre.sensor",
                                                           QDBusConnection::sessionBus());
    if (p_sensorInterface->isValid()){
        QDBusReply<QString> reply = p_sensorInterface->call("passAcceKey","");
        if (!reply.isValid()){
            syslog(LOG_ERR, "Failed to call sensor server interface");
        }
        delete p_sensorInterface;
        p_sensorInterface = nullptr;
    }
    else{
        syslog(LOG_ERR, "using DBUS interface failed");
        delete p_sensorInterface;
        p_sensorInterface = nullptr;
    }
}

bool KmreWindow::getShakeStatus()
{
    return m_shakeStaus;
}

void KmreWindow::onGameKey(bool fullscreen)
{
    mDisplayManager->onHidePopupSubWindows();
    
    if (mGameKeyEnabled) {
        this->handleGameKey();
    }
    else {
        mDisplayManager->showTip(tr("This application can only be used after it is added to the white list. \
                For the method of adding white list, see \"user manual - game keyboard white list setting\""));
    }
}

void KmreWindow::onJoystick()
{
    initGameKeyWidgets();
    handleJoysticKey();
}

void KmreWindow::handleGameKey()
{
    if (mGameKeyManager) {
        if (mGameKeyManager->isSettingsPanelVisible()) {
            mGameKeyManager->saveGameKeys();
            mDisplayManager->focusWindow();
            mDisplayManager->onDisplayForceRedraw();//解决在一些非游戏界面没有数据刷新时（比如会见应用），导致隐藏游戏设置界面不消失的问题
        }
        else {
            //界面的可移动控件可以重新设置
            mGameKeyManager->showGameKeys();
            mGameKeyManager->enableGameKeyEdit(true);
        }
    }
}

//唯一的入口
void KmreWindow::handleJoysticKey(){
    if (mJoystickManager) {
        if(!mJoystickManager->isJoystickSetWidgetInitialed()){
            syslog(LOG_DEBUG,"[%s] isJoystickSetWidgetInitialed:false", __func__);
            mJoystickManager->initJoystickSetWidget();
        }
        else{
            syslog(LOG_DEBUG,"[%s] isJoystickSetWidgetInitialed:true", __func__);
            mJoystickManager->setJoystickSetWidgetVisible(true);
        }
    }
}

void KmreWindow::onGestureBack()
{
    if(mDisplayManager->isWindowFocused()) {
        sendControlAppCmd(0);
    }
}

void KmreWindow::onResponseBack()
{
    sendControlAppCmd(0);
}

void KmreWindow::sendControlAppCmd(int cmd, bool mainDisplay)
{
    int displayId = mId;
    QString pkgName = mPackageName;

    if (!mainDisplay) {
        displayId = mDisplayManager->getCurrentFocusedDisplayId();
        pkgName = mDisplayManager->getCurrentFocusedDisplayPkgName();
    }
    syslog(LOG_DEBUG, "[DisplayManager] sendControlAppCmd, displayId = %d, pkgName = %s, cmd = %d",
            displayId, pkgName.toStdString().c_str(), cmd);
    AppControlManager::getInstance()->controlApp(displayId, pkgName, cmd);
}

void KmreWindow::showMainWindow(bool firstShow)
{
    syslog(LOG_DEBUG, "[%s]", __func__);
    if (SessionSettings::getInstance().windowUsePlatformWayland()) {
        if (SessionSettings::getInstance().hasWaylandPlasmaWindowManagementSupport()) {
            PlasmaShellManager::getInstance()->setAppWindowActive(mPlasmaWindowId);//wayland下需要激活窗口后才能让最小化的窗口显示出来
        } else if (SessionSettings::getInstance().hasWaylandGtkShell1Support()) {
            GtkShellManager::getInstance()->activateWindow(this->windowHandle());
        } else if (SessionSettings::getInstance().hasWaylandXdgActivationV1Support()) {
            XdgManager::getInstance()->activateWindow(this->windowHandle());
        }
        if (this->isMinimized()) {
            mDisplayManager->requestRationWorker();
            this->showNormal();
        }
        else {
            this->show();
        }
    } else {
        this->show();
        this->activateWindow();
    }

    this->raise();
    mDisplayManager->show();
    mDisplayManager->onDisplayForceRedraw();
    mNotifyication->stopNotification();

    if (!SessionSettings::getInstance().windowUsePlatformWayland() &&
            !firstShow) {
        updateUiAfterRestoreFromHidden();
    }
}

void KmreWindow::minimizeWindow()
{
    RecordScreenWidget* recordScreenWidget = mDisplayManager->getRecordScreenWidget();
    if (recordScreenWidget) {
        if (recordScreenWidget->isRecordStarted()) {
            recordScreenWidget->slotHide(false);
        }
        else {
            recordScreenWidget->slotHide(true);
        }
    }
    showMinimized();
}

void KmreWindow::setTrayIcon(QIcon &icon)
{
    mTray->setIcon(icon);
}

void KmreWindow::switchMultipler(QString pkgName, int displayId, bool enable)
{
    mDisplayManager->onMultiplierSwitch(pkgName, displayId, enable);
}

void KmreWindow::resizeWindow()
{
    if (isFullScreen()) {
        mDisplayManager->resizeWindowWhileFullScreen(mDisplayManager->getImageRotation(), true);
    }
    else {
        mDisplayManager->resizeWindowWhileNormalScreen(mDisplayManager->getImageRotation(), true);
    }
}

bool KmreWindow::isWindowFocused()
{
    return mDisplayManager->isWindowFocused();
}

void KmreWindow::updateUiAfterRestoreFromHidden()
{
    // shouldn't update immediately, because we can't move or get correct window geometry at this moment! why ?
    QTimer::singleShot(30, [=] {
        if (mPositionBeforeHiden != QPoint(0, 0)) {
            //syslog(LOG_DEBUG, "[%s]", __func__);
            this->move(mPositionBeforeHiden);
            mPositionBeforeHiden = QPoint(0, 0);
            mDisplayManager->showGameKeysAfterShownFromTray();
        }
    });
}

void KmreWindow::closeWindow(bool force)
{
    if (force || (!mTray->isEnabled())) {
        m_readyClose = true;
    }
    else {// window hide in tray
        mDisplayManager->hideGameKeysWhileHidenInTray();
        mPositionBeforeHiden = pos();
    }

    this->hide();
    mDisplayManager->hide();// disable updating
    if (m_readyClose) {
        this->close();
    }
}

void KmreWindow::closeEvent(QCloseEvent *event)
{
    this->hide();
    mDisplayManager->hide();// disable updating

    KmreWindowManager::getInstance()->closeApp(mPackageName);
    if (!m_appCrashedOnAndroid) {
        syslog(LOG_DEBUG, "[%s] Close android app: '%s'", __func__, mPackageName.toStdString().c_str());
        AppControlManager::getInstance()->closeApp(mAppName, mPackageName, true);
        AppControlManager::getInstance()->controlApp(mId, mPackageName, 7);
    }

    QWidget::closeEvent(event);
}

void KmreWindow::handleNotification(const QString &text, bool stop, bool call, const QString &title)
{
    syslog(LOG_DEBUG, "[%s] title = '%s', text = '%s' stop = %d, call = %d, isWindowFocused = %d", 
		    __func__, title.toStdString().c_str(), text.toStdString().c_str(), stop, call, isWindowFocused());
    if (stop) {
        mNotifyication->stopNotification();
    }
    else if (!isWindowFocused()) {
        mNotifyication->startNotification(call, title, text);
    }
}

void KmreWindow::onInputMethodCommands(int id, bool enable, int x, int y)
{
    //qDebug() << "onInputMethodCommands id:" << id << ",pkgName:" << pkgName << ",ret:" << ret << ",x:" << x << ",y:" << y;
    mDisplayManager->enableInputMethod(id, enable, x, y);
}

//窗口置顶
void KmreWindow::slotToTop(bool isTop)
{
    if (SessionSettings::getInstance().windowUsePlatformX11()) {
        Display *display = QX11Info::display();
        XEvent event;
        event.xclient.type = ClientMessage;
        event.xclient.serial = 0;
        event.xclient.send_event = True;
        event.xclient.display = display;
        event.xclient.window  = winId();
        event.xclient.message_type = XInternAtom (display, "_NET_WM_STATE", False);
        event.xclient.format = 32;

        event.xclient.data.l[0] = isTop;
        event.xclient.data.l[1] = XInternAtom (display, "_NET_WM_STATE_ABOVE", False);
        event.xclient.data.l[2] = 0; //unused.
        event.xclient.data.l[3] = 0;
        event.xclient.data.l[4] = 0;

        XSendEvent(display, DefaultRootWindow(display), False,
                    SubstructureRedirectMask|SubstructureNotifyMask, &event);
    } else {
        PlasmaShellManager::getInstance()->setAppWindowKeepAbove(mPlasmaWindowId, isTop);
    }
}

//窗口抖一抖
void KmreWindow::onShakeWindow()
{
    QPropertyAnimation *pAnimation = new QPropertyAnimation(this, "pos");
    connect(pAnimation, &QPropertyAnimation::finished,this,[this](){
        m_shakeStaus = false;
        mDisplayManager->showGameKeysAfterMoving();
    });
    pAnimation->setDuration(500);
    pAnimation->setLoopCount(2);
    pAnimation->setKeyValueAt(0, QPoint(geometry().x() - 3, geometry().y() - 3));
    pAnimation->setKeyValueAt(0.1, QPoint(geometry().x() + 6, geometry().y() + 6));
    pAnimation->setKeyValueAt(0.2, QPoint(geometry().x() - 6, geometry().y() + 6));
    pAnimation->setKeyValueAt(0.3, QPoint(geometry().x() + 6, geometry().y() - 6));
    pAnimation->setKeyValueAt(0.4, QPoint(geometry().x() - 6, geometry().y() - 6));
    pAnimation->setKeyValueAt(0.5, QPoint(geometry().x() + 6, geometry().y() + 6));
    pAnimation->setKeyValueAt(0.6, QPoint(geometry().x() - 6, geometry().y() + 6));
    pAnimation->setKeyValueAt(0.7, QPoint(geometry().x() + 6, geometry().y() - 6));
    pAnimation->setKeyValueAt(0.8, QPoint(geometry().x() - 6, geometry().y() - 6));
    pAnimation->setKeyValueAt(0.9, QPoint(geometry().x() + 6, geometry().y() + 6));
    pAnimation->setKeyValueAt(1, QPoint(geometry().x() - 3, geometry().y() - 3));
    pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void KmreWindow::onAppResumed()
{
    mDisplayManager->appResumed();
}

void KmreWindow::shareFileToAndroid(const QString &path)
{
    shareFileToAndroid(path, mPackageName, mDisplayManager->getMainDisplayId(), false);
}

void KmreWindow::shareFileToAndroid(const QString &path, const QString &displayName, int displayId, bool check)
{
    if (path.isEmpty() || path.isNull()) {
        return;
    }

    const QFileInfo info(path);
    if (!info.isFile()) {
        return;
    }

    const QString mediaDir = "/media/";
    const QString dataDir = "/data/";
    const QString logDir = "/var/log/";
    const QString homeDir = QDir::homePath();// /home/kylin
    const QString androidStoragePath = "/storage/emulated/0/";
    QString covertPath = "";
    if (path.startsWith(homeDir)) {
        covertPath = androidStoragePath + QString("0-麒麟文件/%1").arg(path.mid(homeDir.length() + 1));
    }
    else if (path.startsWith(mediaDir)) {
        covertPath = androidStoragePath + QString("0-麒麟移动存储设备/%1").arg(path.mid(mediaDir.length()));
    }
    else if (path.startsWith(dataDir)) {
        covertPath = androidStoragePath + QString("0-麒麟数据分区/%1").arg(path.mid(dataDir.length()));
    }
    else if (path.startsWith(logDir)) {
        covertPath = androidStoragePath + QString("0-麒麟日志/%1").arg(path.mid(logDir.length()));
    }
    else {
        if (info.size() < (100 * 1024 * 1024)) {// 100MB
            QDir dir;
            QString shareDataPath = KmreEnv::GetAndroidDataPath() + "/share_data";
            if (!dir.exists(shareDataPath)) {
                dir.mkpath(shareDataPath);
            }
            QString fileName = info.fileName();
            QString newPath = shareDataPath + "/" + fileName;
            //syslog(LOG_DEBUG, "[%s] copy file:'%s' to '%s'", 
            //    __func__, path.toStdString().c_str(), newPath.toStdString().c_str());
            if (QFileInfo(newPath).exists()) {
                if (!QFile::remove(newPath)) {
                    syslog(LOG_ERR, "[%s] remove file:'%s' failed!", __func__, newPath.toStdString().c_str());
                }
            }
            if (QFile::copy(path, newPath)) {
                covertPath = androidStoragePath + QString("share_data/%1").arg(fileName);
            }
            else {
                syslog(LOG_ERR, "[%s] copy file:'%s' to '%s' failed!", 
                    __func__, path.toStdString().c_str(), newPath.toStdString().c_str());
                KylinUI::MessageBox::warning(this, tr("Warning"), tr("Share failed ! (copy file failed)"));
            }
        }
        else {
            syslog(LOG_WARNING, "[%s] file:'%s' size is exceed size limition(100MB)!", __func__, path.toStdString().c_str());
            KylinUI::MessageBox::warning(this, tr("Warning"), tr("Share failed! File size is exceed size limition(100MB)!"));
        }
    }

    if (!covertPath.isEmpty()) {
        bool hasDoubleDisplay = check && (mDisplayManager->getCurrentDisplayMode() == DisplayManager::DISPLAY_MODE_MULTI);
        AppControlManager::getInstance()->sendDragFileInfo(covertPath, displayName, displayId, hasDoubleDisplay);
    }
}

QString KmreWindow::getTitleName()
{
    if (mTitleName.isEmpty()) {
        QString locale = QLocale::system().name();
        QString appName_en, appName_zh;

        kmre::utils::getAppNameFromDesktop(mPackageName, appName_en, appName_zh);
        appName_en = appName_en.isEmpty() ? mAppName : appName_en;
        appName_zh = appName_zh.isEmpty() ? mAppName : appName_zh;

        mTitleName = (locale == "en_US") ? appName_en : appName_zh;
        syslog(LOG_DEBUG, "Title name: '%s', appName_en = '%s', appName_zh = '%s'", 
            mTitleName.toStdString().c_str(), appName_en.toStdString().c_str(), appName_zh.toStdString().c_str());
    }

    return mTitleName;
}

void KmreWindow::slotFocusWindowChanged()
{
    QWindow *win = qApp->focusWindow();
    if (win != windowHandle()) {
        TitleMenu* titleMenu = mDisplayManager->getTitleMenu();
        if ((win == nullptr) || ((win != titleMenu->windowHandle()) && 
                                ((!mGameKeyManager) || (!mGameKeyManager->isGameKeyFocused(win))))) {
            mDisplayManager->unFocusWindow();
            return;
        }
    }
    syslog(LOG_DEBUG, "[%s] Focused window app: %s", __func__, mPackageName.toStdString().c_str());
    mDisplayManager->focusWindow();
}

void KmreWindow::showCrashedHintDialog()
{
    syslog(LOG_DEBUG, "[%s] Show crash hint dialog...", __func__);
    mCrashDialog.reset(new CrashDialog(this));
    connect(mCrashDialog.get(), &CrashDialog::sigCrashRestart, this, [&] (bool restart) {
        m_appCrashedOnAndroid = true;
        closeWindow(true);

        if (restart) {
            syslog(LOG_DEBUG, "[%s] Restart app '%s'", __func__, mPackageName.toStdString().c_str());
            QString cmd = "/usr/bin/startapp " + mPackageName;
            if (!QProcess::startDetached(cmd)) {
                syslog(LOG_ERR, "[%s] Restart app '%s' failed!", __func__, mPackageName.toStdString().c_str());
            }
        }
    });
    
    mCrashDialog->open();// 'Window Modality' and nonblock
}

bool KmreWindow::event(QEvent *e)
{
    bool result = QWidget::event(e);
    QEvent::Type type = e->type();
    //syslog(LOG_DEBUG, "[%s] event type = %d", __func__, type);
    switch (type) {
    case QEvent::Move:
        mDisplayManager->saveWindowGeometry();//resize()和setFixedSize()时会多次走到这里
        break;
    case QEvent::WindowActivate:
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            m_isActiveWindow = true;
        }
        break;
    case QEvent::WindowDeactivate:
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            m_isActiveWindow = false;
        }
        break;
    case QEvent::WindowStateChange:
    {
        if (isMinimized()) {
            m_wasMinimized = true;
        }
        mDisplayManager->onHidePopupSubWindows();

        QWindowStateChangeEvent *ce = dynamic_cast<QWindowStateChangeEvent*>(e);
        if (ce) {
            int oldState = ce->oldState();
            if (oldState & Qt::WindowMinimized) {
                mDisplayManager->onDisplayForceRedraw();
            }
        }
        break;
    }
    case QEvent::ActivationChange:
    {
        if (!isMinimized() && m_wasMinimized) {
            m_wasMinimized = false;
        }
        if (this->isActiveWindow()) {//只有实现了通知功能的一些应用走该流程
            mNotifyication->stopNotification();
        }
        break;
    }
    default:
        break;
    }

    return result;
}

void KmreWindow::resizeEvent(QResizeEvent *event)
{
    mDisplayManager->onHidePopupSubWindows();
    mDisplayManager->resizeRecordScreenWidget();
    QWidget::resizeEvent(event);
}

void KmreWindow::moveEvent(QMoveEvent *event)
{
    mDisplayManager->onHidePopupSubWindows();
    mDisplayManager->showGameKeysAfterMoving();// show game keys after moving
    QWidget::moveEvent(event);
}

/**
 * 窗口隐藏，eventType：5 为应用切到后台
 * @param event
 */
void KmreWindow::hideEvent(QHideEvent *event)
{
    Q_UNUSED(event);

    mDisplayManager->hide();//应用切换到后台，disableUpdateBuffer,不要再进行界面实时更新
    sendControlAppCmd(5);//switch app to background
}

/**
 * 窗口显示，eventType：6 为应用切到前台
 * @param event
 */
void KmreWindow::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);

    mDisplayManager->show();//应用切换到前台，enableUpdateBuffer，及时更新界面
    sendControlAppCmd(6);//switch app to frontground

    if (mScreenLocked && mLockWidget) {
        mLockWidget->show();
    }
}

bool KmreWindow::nativeEvent(const QByteArray &eventType, void *message, long *)
{
    Q_UNUSED(eventType);
    xcb_generic_event_t *event = static_cast<xcb_generic_event_t *>(message);
    uint8_t type = event->response_type & ~0x80;
    //syslog(LOG_DEBUG, "[%s] native event type = %d", __func__, type);
    switch(type) {
        case XCB_FOCUS_IN: {
            mNotifyication->stopNotification();
            //emit androidDesktop->signalM->requestLaunchApp(this->m_pkgName);
            break;
        }
        case XCB_FOCUS_OUT: {
            break;
        }
        case XCB_CONFIGURE_WINDOW: {
            // refresh window when system wake up from sleep
            mDisplayManager->onDisplayForceRedraw();
            break;
        }
        default:
            break;
    }
    return false;
}

void KmreWindow::keyPressEvent(QKeyEvent *event)
{
    // F1快捷键打开用户手册
    if (event->key() == Qt::Key_F1) {
        this->onHelpEvent();
    }
    // F11快捷键进入/退出全屏
    else if (event->key() == Qt::Key_F11) {
        mDisplayManager->onSwitchFullscreen();
    }
    // F2快捷键进入/退出鼠标限制模式
    else if (event->key() == Qt::Key_F2) {
        mMouselimit = !mMouselimit;
        if (mMouselimit) {
            mDisplayManager->showTip(tr("Mouse lock is enabled in the window,press F2 to exit"), 3000);
        }
        else {
            mDisplayManager->showTip(tr("Mouse lock is disabled, press F2 to enable."), 3000);
        }
    }

    return QWidget::keyPressEvent(event);
}

bool KmreWindow::isOwner(unsigned long winId)
{
    bool isOwner = false;
    DisplayWidget *widget = mDisplayManager->getMainDisplayWidget();
    if (winId == widget->winId()) {
        isOwner = true;
    }
    else {
        widget = mDisplayManager->getAccessoryDisplayWidget();
        if (widget && (winId == widget->winId())) {
            isOwner = true;
        }
    }

    return isOwner;
}

void KmreWindow::limitMouse()
{
    if (mMouselimit) {
        const QRect &rect = this->geometry();
        QPoint pos = QCursor::pos();
        qint32 x = qBound(rect.left() + MOUSE_MARGINS, pos.x(), rect.right() - MOUSE_MARGINS);
        qint32 y = qBound(rect.top() + DEFAULT_TITLEBAR_HEIGHT, pos.y(), rect.bottom() - MOUSE_MARGINS);

        if (x != pos.x() || y != pos.y()) {
            QCursor::setPos(x, y);
        }
    }
}

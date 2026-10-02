/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#include "fullbackgroundwidget.h"
#include "../lock-dialog/lockdialogmodel.h"
#include "utils.h"
#include "lockwidget.h"
#include "lockmainuianim.h"
#include "agreementwindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QX11Info>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QStringList>
#include <QDebug>
#include <QTimer>
#include <QCryptographicHash>
#include <syslog.h>
#include <QWindow>
#include "grab-x11.h"
#include <unistd.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xrandr.h>
#include <X11/keysym.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <QPainter>
#include <QAbstractAnimation>
#include <QScreen>
#include <QKeyEvent>
#include <QDateTime>
#include <QEasingCurve>
#include "commonfunc.h"
#include "wallpaperrenderer.h"
#include "lockscreentransition.h"
#include "screensavermode.h"
#include "screensaverwidget.h"
#include "displayservice.h"
#include "configuration.h"
#include "global_utils.h"
#ifdef WITHKYSEC
#include <kysec/libkysec.h>
#include <kysec/status.h>
#endif

FullBackgroundWidget::FullBackgroundWidget(LockDialogModel *model, bool isStartup, QWidget *parent)
    : QWidget(parent), m_modelLockDialog(model), m_isStartupMode(isStartup), m_background(nullptr)
{
    initCurrentBackground();
}

void FullBackgroundWidget::init()
{
    if (m_isInited) {
        return;
    }
    m_isInited = true;
    initUI();
    initConnections();
}

FullBackgroundWidget::~FullBackgroundWidget()
{
    if (m_backgrondGradationTimer) {
        if (m_backgrondGradationTimer->isActive())
            m_backgrondGradationTimer->stop();
        m_backgrondGradationTimer->disconnect();
        m_backgrondGradationTimer->deleteLater();
    }
}

void FullBackgroundWidget::initUI()
{
    if (!m_windowHelper) {
        m_windowHelper = new UkuiWindowHelper(this);
        m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::ScreenLock);
        m_windowHelper->grabKeyboard();
    }
    if (QX11Info::isPlatformX11()) {
        /*捕获键盘，如果捕获失败，则可能是由于弹出菜单项已经捕获，那么模拟一次esc按键来退出菜单，如果仍捕获失败，则放弃锁屏，避免密码无法输入*/
        if (establishGrab())
            qDebug() << "establishGrab : true";
        else {
            qDebug() << "establishGrab : false";
            /*检测当前是否存在弹出菜单，只有存在弹出菜单时，才模拟按键esc*/
            if (checkHasPopupMenu()) {
                qDebug() << "XTestFakeKeyEvent Esc!!";
                XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Escape), True, 1);
                XTestFakeKeyEvent(QX11Info::display(), XKeysymToKeycode(QX11Info::display(), XK_Escape), False, 1);
                XFlush(QX11Info::display());
                sleep(1);
            }
            if (!establishGrab()) {
                exit(1);
            }
        }
    }

    setMouseTracking(true);
    QDesktopWidget *desktop = QApplication::desktop();
    if (desktop) {
        m_defaultGeometry = desktop->geometry();
        setGeometry(desktop->geometry());
    }
    if (!m_lockWidget) {
        m_lockWidget = new LockWidget(m_modelLockDialog, this);
        connect(m_lockWidget, &LockWidget::authSucceed, this, &FullBackgroundWidget::onAuthSucceed);
        connect(m_lockWidget, &LockWidget::sessionToolsExit, this, &FullBackgroundWidget::onCloseScreensaver);
        connect(m_lockWidget, &LockWidget::showBlankScreensaver, this, &FullBackgroundWidget::onShowBlankScreensaver);
        connect(m_lockWidget, &LockWidget::startFcitx, this, &FullBackgroundWidget::onStartFcitx);
        connect(m_lockWidget, &LockWidget::moveToCurrentScreen, this, &FullBackgroundWidget::moveToCurrentScreen);
        connect(m_lockWidget, &LockWidget::moveToPrimaryScreen, this, &FullBackgroundWidget::moveToPrimaryScreen);
        connect(m_lockWidget, &LockWidget::setCurrentBackground, [&]() { setCurrentBackground("LockBackground"); });
        moveToPrimaryScreen();
        m_lockWidget->show();
    }
    if (m_modelLockDialog->getAgreementWindowShowLoginPrompt() && (IsStartupMode() || isGreeterMode())) {
        bool hideTitle = m_modelLockDialog->getAgreementWindowHideTitle();
        QString title = m_modelLockDialog->getAgreementWindowPromptTitle();
        QString text = m_modelLockDialog->getAgreementWindowText();

        // 文本内容不为空 且 如果配置了标题，则标题不为空
        if (!text.isEmpty() && (hideTitle || (!hideTitle && !title.isEmpty()))) {
            m_agreementWindow = new AgreementWindow(hideTitle, title, text, this);
            m_lockWidget->hide();
            m_agreementWindow->setGeometry(m_lockWidget->geometry());
            m_agreementWindow->show();

            m_lockWidget->clearFocus();
            m_agreementWindow->setFocusPolicy(Qt::StrongFocus);
            setFocusProxy(m_agreementWindow);
            m_agreementWindow->setFocus();
            // 特别提示窗口抓取键盘，避免焦点在密码框上时输入事件跑到密码框，但会导致我已知晓按钮无法响应回车
            m_agreementWindow->grabKeyboard();
            connect(m_agreementWindow, &AgreementWindow::switchToGreeter, this, [this]() {
                QString strAutologinUser = (Q_EMIT m_modelLockDialog->getAutologinUserHint());
                QString strAuthUser = (Q_EMIT m_modelLockDialog->pamAuthenticateUser());
                bool isAuthenticated = (Q_EMIT m_modelLockDialog->pamIsAuthenticated());
                qDebug() << "switchToGreeter:" << isAuthenticated << strAutologinUser << strAuthUser;
                if (isAuthenticated && !strAutologinUser.isEmpty() && !strAuthUser.isEmpty()) {
                    onAuthSucceed(strAuthUser);
                    return;
                }
                m_agreementWindow->hide();
                m_agreementWindow->releaseKeyboard();
                m_agreementWindow->clearFocus();
                m_lockWidget->show();
                m_lockWidget->setFocus();
                update();
            });
        }
    }
    if (m_modelLockDialog->isUseWayland()) {
        setWindowFlags(Qt::FramelessWindowHint);
    } else {
        setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
        qApp->installNativeEventFilter(this);
    }
    // 登录模式下监听屏幕插拔
    if (isGreeterMode() && !m_modelLockDialog->isUseWayland()) {
        XRRQueryExtension(QX11Info::display(), &m_RREventBase, &m_RRErrorBase);
        XRRSelectInput(QX11Info::display(), QX11Info::appRootWindow(), RRScreenChangeNotifyMask);
        QtConcurrent::run([=]() { RRScreenChangeEvent(true); });
    }
    installEventFilter(this);
    // 初始化队列处理定时器
    m_queueProcessTimer = new QTimer(this);
    m_queueProcessTimer->setSingleShot(true);
    connect(m_queueProcessTimer, &QTimer::timeout, this, &FullBackgroundWidget::processEventQueue);
    initBlurAnimation();
}

void FullBackgroundWidget::setCurrentBackground(QString cmd)
{
    if (cmd == "LockBackground") {
        // 通过会话工具路径回锁屏时，不一定会走 onShowLock。
        // 这里兜底恢复为锁屏稳定终态，避免背景停留在动画中间值（无模糊）。
        if (!m_isEnteringFromScreensaver) {
            applyVisualMode(VisualMode::LockStable);
        }
        onCurrentUserBackground(m_currentUserName);
    } else if (cmd == "ToolsBackground") {
        QString strBackground = getDesktopBackground();
        qDebug() << "picture path = " << strBackground;
        QSharedPointer<Background> background(new Background);
        background->type = BACKGROUND_IMAGE;
        background->image = strBackground;
        setBackground(background);
    }
}

void FullBackgroundWidget::applyVisualMode(VisualMode mode)
{
    if (m_blurAnimation) {
        m_blurAnimation->stop();
    }
    if (m_scaleAnimation) {
        m_scaleAnimation->stop();
    }

    switch (mode) {
        case VisualMode::EnterFromScreensaverStart:
            m_blurProgress = 0.0;
            m_scaleProgress = 0.0;
            if (m_lockWidget) {
                m_lockWidget->prepareMainUiEnterStartState();
            }
            break;
        case VisualMode::SessionToolsStable:
            m_blurProgress = 1.0;
            m_scaleProgress = 1.0;
            if (m_lockWidget) {
                // sessionTools 保持当前几何，仅清理透明度残留
                m_lockWidget->resetMainUiVisualState(false);
            }
            break;
        case VisualMode::LockStable:
            m_blurProgress = 1.0;
            m_scaleProgress = 1.0;
            if (m_lockWidget && !m_modelLockDialog->isSessionTools()) {
                m_lockWidget->resetMainUiVisualState(true);
            }
            break;
    }
    update();
}

QString FullBackgroundWidget::getDesktopBackground()
{
    UserInfoPtr userInfo = m_modelLockDialog->findUserByName(m_currentUserName);
    QString strBackground = "";
    if (userInfo) {
        strBackground = userInfo->backGround();
        if (userInfo->backGround().isEmpty() || !QFileInfo(userInfo->backGround()).exists()) {
            strBackground = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
            QFileInfo fileimage(strBackground);
            if (fileimage.isFile() && fileimage.exists()) {
                strBackground = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
            } else {
                strBackground = "/usr/share/ukui/ukui-session-manager/images/background-ukui.png";
                QFileInfo image(strBackground);
                if (image.isFile() && image.exists()) {
                    strBackground = "/usr/share/ukui/ukui-session-manager/images/background-ukui.png";
                } else {
                    qWarning() << "Can not get the background when login the user:" << m_currentUserName;
                }
            }
        }
    }
    return strBackground;
}

bool FullBackgroundWidget::eventFilter(QObject *obj, QEvent *event)
{
    static bool isFirstActive = true;

    if (!QX11Info::isPlatformX11()) {
        if (event->type() == QEvent::MouseButtonPress) {
            if (screenStatus & SCREEN_SAVER && !m_isBlank) {
                enqueueEvent(EC_ClearScreenSaver);
            }
        }
    }

    if (event->type() == QEvent::WindowDeactivate) {
        QTimer::singleShot(50, this, SLOT(laterActivate()));
    } else if (event->type() == QEvent::WindowActivate) {
        if (!m_modelLockDialog->isSessionTools()) {
            if (isFirstActive) {
                isFirstActive = false;
                syslog(LOG_INFO, "[ukui-screensaver-dialog] window active event!!");
                QTimer::singleShot(50, this, [&]() {
                    if (m_lockWidget) {
                        m_lockWidget->doDelayInit();
                    }
                    sendStartedFinished();
                });
            }
            qDebug() << "Delay setLockState";
            QTimer::singleShot(10, this, SLOT(setLockState()));
        }
        // QTimer::singleShot(200,this,SLOT(killWindow()));
    }
    return QWidget::eventFilter(obj, event);
}

void FullBackgroundWidget::sendStartedFinished()
{
    // 使用QDBusInterface调用org.gnome.SessionManager的startupfinished方法
    QDBusInterface sessionManager(
        "org.gnome.SessionManager",
        "/org/gnome/SessionManager",
        "org.gnome.SessionManager",
        QDBusConnection::sessionBus());
    if (!sessionManager.isValid()) {
        qWarning() << "QDBusInterface org.gnome.SessionManager is not valid!";
        return;
    }
    // 参数与gdbus命令保持一致
    QDBusMessage reply = sessionManager.call(
        QStringLiteral("startupfinished"), QStringLiteral("ukui-screensaver-dialog"), QString(""));
    if (reply.type() == QDBusMessage::ErrorMessage) {
        qWarning() << "Failed to call startupfinished:" << reply.errorMessage();
    }
}

void FullBackgroundWidget::initConnections()
{
    // 监听全屏窗口变化
    QDesktopWidget *desktop = QApplication::desktop();
    connect(desktop, &QDesktopWidget::resized, this, [&, this, desktop]() {
        qDebug() << "resized:" << desktop->geometry();
        this->scheduleDesktopResized();
    });
    connect(desktop, &QDesktopWidget::primaryScreenChanged, this, [&, this, desktop]() {
        qDebug() << "primaryScreenChanged:" << desktop->geometry();
        this->scheduleDesktopResized();
    });
    connect(desktop, &QDesktopWidget::screenCountChanged, this, [&, this, desktop]() {
        qDebug() << "screenCountChanged:" << desktop->geometry();
        this->scheduleDesktopResized();
    });
    // 监听休眠、睡眠、唤醒
    connect(m_modelLockDialog, SIGNAL(prepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));
    connect(m_modelLockDialog, &LockDialogModel::currentUserChanged, this, &FullBackgroundWidget::onCurUserChanged);
    connect(m_modelLockDialog, &LockDialogModel::showLock, this, &FullBackgroundWidget::onShowLock);
    connect(
        m_modelLockDialog, &LockDialogModel::showBlankScreensaver, this, &FullBackgroundWidget::onShowBlankScreensaver);
    connect(
        m_modelLockDialog, &LockDialogModel::showLockScreensaver, this, &FullBackgroundWidget::onShowLockScreensaver);
    connect(
        m_modelLockDialog,
        &LockDialogModel::showShutdownAnimation,
        this,
        &FullBackgroundWidget::onShowShutdownAnimation);
    connect(m_modelLockDialog, &LockDialogModel::showScreensaver, this, &FullBackgroundWidget::onShowScreensaver);
    connect(m_modelLockDialog, &LockDialogModel::showSessionIdle, this, &FullBackgroundWidget::onShowSessionIdle);
    connect(m_modelLockDialog, &LockDialogModel::SecondRunParam, this, &FullBackgroundWidget::onSecondRunParam);
    connect(m_modelLockDialog, &LockDialogModel::showSessionTools, this, &FullBackgroundWidget::onShowSessionTools);
    connect(m_modelLockDialog, &LockDialogModel::showAppBlockWindow, this, &FullBackgroundWidget::onShowAppBlockWindow);
    connect(
        m_modelLockDialog,
        &LockDialogModel::showMultiUsersBlockWindow,
        this,
        &FullBackgroundWidget::onShowMultiUsersBlockWindows);
    connect(m_modelLockDialog, &LockDialogModel::showSwitchUserLock, this, &FullBackgroundWidget::onShowSwitchUserLock);
    connect(m_modelLockDialog, &LockDialogModel::showSwitchToUser, this, &FullBackgroundWidget::onShowSwitchToUser);
}

void FullBackgroundWidget::setLockState()
{
    qDebug() << " hideen status = " << this->isHidden();
    if (m_bIsLockState || this->isHidden())
        return;
    m_bIsLockState = true;
    if (screenStatus & SCREEN_SAVER) {
        Q_EMIT m_modelLockDialog->lockStateChanged(true, false, true);
    } else {
        Q_EMIT m_modelLockDialog->lockStateChanged(true, false);
    }
}

void FullBackgroundWidget::clearLockState()
{
    if (!m_bIsLockState)
        return;

    m_bIsLockState = false;
    Q_EMIT m_modelLockDialog->clearLockState();
}

void FullBackgroundWidget::onSecondRunParam(const QString &str)
{
    qDebug() << "onSecondRunParam" << str;
    if (str == "SleepLock") {
        onShowBlankScreensaver(0, true);
    } else if (str == "ScreensaverLock") {
        onShowLockScreensaver();
    } else if (str == "StartupLock") {
        setIsStartupMode(true);
        onShowLock(true);
    } else if (str == "CmdLock") {
        onShowLock(false);
    } else if (str.startsWith("CloseLock")) {
        int nDelayTime = 1000;
        bool needLock = false;
        QStringList listParam = str.split('&');
        for (const QString &param : listParam) {
            if (param.startsWith("delay")) {
                QStringList listDelay = param.split('=');
                if (listDelay.size() >= 2) {
                    nDelayTime = listDelay[1].toInt();
                }
            } else if (param.startsWith("hasLock")) {
                QStringList listLock = param.split('=');
                if (listLock.size() >= 2) {
                    needLock = (listLock[1].toInt() ? true : false);
                }
            }
        }
        onShowBlankScreensaver(nDelayTime, needLock);
    } else if (str == "SessionIdle") {
        onShowSessionIdle();
    } else if (str == "Screensaver") {
        onShowScreensaver();
    } else if (str == "CmdSessionTools") {
        onShowSessionTools();
    } else if (str == "CmdSwitchUser") {
        onShowSwitchUserLock();
    } else if (str.startsWith("CmdSwitchToUser")) {
        QStringList listParam = str.split('&');
        if (listParam.size() > 1) {
            onShowSwitchToUser(listParam[1]);
        }
    } else if (str == "CmdRestartAppBlock") {
        onShowAppBlockWindow(0);
    } else if (str == "CmdPowerOffAppBlock") {
        onShowAppBlockWindow(1);
    } else if (str == "CmdSuspendAppBlock") {
        onShowAppBlockWindow(2);
    } else if (str == "CmdHibernateAppBlock") {
        onShowAppBlockWindow(3);
    } else if (str == "CmdLogoutAppBlock") {
        onShowAppBlockWindow(4);
    } else if (str == "CmdRestartAppSoftBlock") {
        onShowAppBlockWindow(5);
    } else if (str == "CmdPowerOffAppSoftBlock") {
        onShowAppBlockWindow(6);
    } else if (str == "CmdLogoutAppSoftBlock") {
        qDebug() << "CmdLogoutAppSoftBlock show";
        onShowAppBlockWindow(7);
    } else if (str == "CmdRestartMulTiUserBlock") {
        onShowMultiUsersBlockWindows(0);
    } else if (str == "CmdPowerOffMulTiUserBlock") {
        onShowMultiUsersBlockWindows(1);
    } else if (str == "CmdRestartAnimation") {
        onShowShutdownAnimation(0);
    } else if (str == "CmdPowerOffAnimation") {
        onShowShutdownAnimation(1);
    } else if (str == "CmdLogoutAnimation") {
        onShowShutdownAnimation(2);
    }
}

void FullBackgroundWidget::onShowShutdownAnimation(int power)
{
    QString animationText;
    if (power == 0) {
        m_command = "Reboot";
        animationText = tr("rebooting");
    } else if (power == 1) {
        m_command = "PowerOff";
        animationText = tr("shutdowning");
    } else if (power == 2) {
        m_command = "Logout";
        animationText = tr("logouting");
    }
    // 转场界面须满模糊背景，避免动效中间态或壁纸快照残留
    applyVisualMode(VisualMode::LockStable);
    endAuthUiWallpaperSnapshot();
    m_modelLockDialog->setIsSessionTools(true);
    qDebug() << "onShowShutdownAnimation" << power << m_command;

    if (m_lockWidget && !m_lockWidget->isHidden())
        screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    Q_EMIT m_modelLockDialog->lockStateChanged(true, true);

    // 由于客户端响应close需要时间，先判断一次是否还有窗口，还存在窗口则开启定时器等3s再判断是否有窗口
    // 同时connect了windowremove信号，当最后一个窗口被关闭时，执行电源操作
    QVector<InhibitorInfo> shutdownLockcheck = m_modelLockDialog->getShutdownLockcheck();
    QVector<InhibitorInfo> logoutLockcheck = m_modelLockDialog->getLogoutLockCheck();
    QVector<WindowInfo::WindowInfo> windows = m_modelLockDialog->getWindowList();
    int count = shutdownLockcheck.count() + logoutLockcheck.count() + windows.count();
    qDebug() << "count" << count;
    if (count == 0) {
        m_lockWidget->onDoPowerAction(m_command, false);
    } else {
        if (!m_animationTimer) {
            m_animationTimer = new QTimer(this);
            m_animationTimer->setSingleShot(true);
            connect(m_animationTimer, &QTimer::timeout, this, &FullBackgroundWidget::onAnimationTimeout);
        }
        if (!m_stopAnimationTimer) {
            m_animationTimer->start(1000 * 3);
        }
    }
}

void FullBackgroundWidget::onAnimationTimeout()
{
    QVector<InhibitorInfo> shutdownLockcheck = m_modelLockDialog->getShutdownLockcheck();
    QVector<InhibitorInfo> logoutLockcheck = m_modelLockDialog->getLogoutLockCheck();
    QVector<WindowInfo::WindowInfo> windows = m_modelLockDialog->getWindowList();
    int count = shutdownLockcheck.count() + logoutLockcheck.count() + windows.count();
    qDebug() << "shutdown animation time out" << m_command << count;
    if (count == 0) {
        m_lockWidget->onDoPowerAction(m_command, false);
    } else {
        show();
        if (m_lockWidget) {
            m_lockWidget->onDoPowerAction(m_command, false);
        }
    }
    qDebug() << "onAnimationTimeout end";
}

void FullBackgroundWidget::onShowBlankScreensaver(int nDelay, bool isHasLock)
{
    if (screenStatus & SCREEN_SAVER) {
        return;
    }
    qDebug() << "onShowBlankScreensaver:" << nDelay << "," << isHasLock << m_isBlank
             << m_modelLockDialog->isSessionTools();
    bool isFromSessionTools = false;
    if (m_modelLockDialog->isSessionTools()) {
        isFromSessionTools = true;
    }
    if (isHasLock) {
        screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER | SCREEN_LOCK);
    } else {
        screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER);
    }

    if (!m_isBlank) {
        for (auto screen : effectiveApplicationScreens()) {
            ScreenSaver *saver = m_modelLockDialog->getScreensaver();
            saver->mode = SaverMode(SAVER_BLANK_ONLY);
            ScreenSaverWidget *saverWidget = new ScreenSaverWidget(saver, nullptr, this);
            widgetXScreensaverList.push_back(saverWidget);
            saverWidget->setGeometry(screen->geometry());
        }

        setCursor(Qt::BlankCursor);
        m_isBlank = true;
        if (isHasLock) {
            m_lockEnterFromBlankPending = true;
        }

        if (0 != nDelay) {
            QTimer::singleShot(nDelay, this, [=]() {
                if (m_isBlank) {
                    m_isBlank = false;
                    onClearScreensaver();
                }
            });
        }
    }
    show();

    if (m_modelLockDialog->isSessionTools()) {
        m_modelLockDialog->setIsSessionTools(false);
        setLockState();
    }

    if (m_lockWidget && !m_lockWidget->isHidden()) {
        m_lockWidget->exitSubWidget(true);
    }
}

void FullBackgroundWidget::onShowLock(bool isStartup)
{
    /* Greeter / 开机启动：不播入场。屏保退出、黑色空白屏保（带锁）回到锁屏时播进场动效。 */
    const bool skipEnterEffects = isGreeterMode() || isStartup;
    const bool playEnterEffects
        = !skipEnterEffects && (m_lockEnterFromBlankPending || m_isEnteringFromScreensaver);
    m_lockEnterFromBlankPending = false;

    if (playEnterEffects) {
        applyVisualMode(VisualMode::EnterFromScreensaverStart);
    } else {
        applyVisualMode(VisualMode::LockStable);
    }

    if (m_modelLockDialog->isSessionTools()) {
        if (m_lockWidget && !m_lockWidget->isHidden()) {
            m_lockWidget->exitSubWidget();
        }
        m_modelLockDialog->setIsSessionTools(false);
        qDebug() << "onShowLock:" << isStartup;
        setLockState();
    } else {
        onCurrentUserBackground("LockBackground");
        moveToPrimaryScreen();
    }
    if (!playEnterEffects && m_lockWidget && !m_modelLockDialog->isSessionTools()) {
        applyVisualMode(VisualMode::LockStable);
    }
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    show();
    syslog(LOG_INFO, "[ukui-screensaver-dialog] set current user!!");
    qDebug() << m_modelLockDialog->defaultUserName() << m_modelLockDialog->currentUserName();
    if (!m_modelLockDialog->currentUserName().isEmpty()) {
        Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->currentUserName());
    } else {
        Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    }
    Q_EMIT m_modelLockDialog->lockStateChanged(true, false);
    if (m_lockWidget && !m_lockWidget->isHidden()) {
        m_lockWidget->show();
    }
    if (playEnterEffects) {
        startBlurTransition();
        if (m_lockWidget && !m_lockWidget->isHidden()) {
            m_lockWidget->startMainUiEnterAnimation();
        }
    }
}

namespace {

constexpr int kWallpaperRepaintMinMs = 33;

} // namespace

void FullBackgroundWidget::setWallpaperAnimActive(bool active)
{
    m_wallpaperAnimActive = active;
    if (!active) {
        m_wallpaperRepaintScheduled = false;
    }
}

bool FullBackgroundWidget::shouldPaintLockSubtreeInBackgroundPass() const
{
    // 壁纸动效期间仍须同帧绘制 Lock 子树，避免子控件单独重绘时底层壁纸陈旧露出黑块
    Q_UNUSED(m_wallpaperAnimActive);
    return true;
}

void FullBackgroundWidget::scheduleWallpaperRepaint()
{
    if (!m_wallpaperAnimActive) {
        update();
        return;
    }
    if (!m_wallpaperRepaintClock.isValid()) {
        m_wallpaperRepaintClock.start();
        update();
        return;
    }
    const qint64 elapsed = m_wallpaperRepaintClock.elapsed();
    if (elapsed >= kWallpaperRepaintMinMs) {
        m_wallpaperRepaintClock.restart();
        update();
        return;
    }
    if (!m_wallpaperRepaintScheduled) {
        m_wallpaperRepaintScheduled = true;
        QTimer::singleShot(static_cast<int>(kWallpaperRepaintMinMs - elapsed), this, [this]() {
            m_wallpaperRepaintScheduled = false;
            if (m_wallpaperAnimActive) {
                m_wallpaperRepaintClock.restart();
                update();
            }
        });
    }
}

void FullBackgroundWidget::onWallpaperAnimationFinished()
{
    if (m_blurAnimation && m_blurAnimation->state() == QAbstractAnimation::Running) {
        return;
    }
    if (m_scaleAnimation && m_scaleAnimation->state() == QAbstractAnimation::Running) {
        return;
    }
    setWallpaperAnimActive(false);
    endAuthUiWallpaperSnapshot();
    if (m_lockWidget) {
        m_lockWidget->update();
    }
    Q_EMIT wallpaperTransitionFinished();
}

void FullBackgroundWidget::beginAuthUiWallpaperSnapshot()
{
    m_authUiSnapshotActive = true;
    m_wallpaperSnapshots.clear();
    for (QScreen *screen : QApplication::screens()) {
        const QRect screenRect = screen->geometry();
        if (!screenRect.isValid()) {
            continue;
        }
        const QString key = QStringLiteral("%1x%2").arg(screenRect.width()).arg(screenRect.height());
        QPixmap snap(screenRect.size());
        snap.fill(QColor(QStringLiteral("#000000")));
        QPainter snapPainter(&snap);
        if (m_transition.started) {
            drawBackground(snapPainter, m_transition.to, m_transition.from, QRect(QPoint(0, 0), screenRect.size()),
                           m_transition.stage);
        } else if (m_background) {
            drawBackground(snapPainter, m_background, m_background, QRect(QPoint(0, 0), screenRect.size()));
        }
        snapPainter.end();
        m_wallpaperSnapshots.insert(key, snap);
    }
}

void FullBackgroundWidget::endAuthUiWallpaperSnapshot()
{
    if (!m_authUiSnapshotActive) {
        return;
    }
    m_authUiSnapshotActive = false;
    m_wallpaperSnapshots.clear();
    update();
}

void FullBackgroundWidget::invalidateStableWallpaperCache()
{
    m_stableWallpaperCache.clear();
}

bool FullBackgroundWidget::drawStableCompositedWallpaper(QPainter &painter, const QPixmap &sharpPixmap,
                                                         const QPixmap &blurPixmap, const QRect &rect,
                                                         qreal blurProgress, qreal scaleProgress, float layerAlpha)
{
    if (m_wallpaperAnimActive || !rect.isValid() || sharpPixmap.isNull()) {
        return false;
    }

    const bool lockedStatic = qFuzzyCompare(blurProgress, 1.0) && qFuzzyCompare(scaleProgress, 1.0);
    const bool desktopStatic = qFuzzyIsNull(blurProgress) && qFuzzyIsNull(scaleProgress);
    if (!lockedStatic && !desktopStatic) {
        return false;
    }

    QString imagePath;
    if (m_background) {
        imagePath = m_background->image;
    }
    const QString key = QStringLiteral("%1|%2x%3|b%4|s%5|a%6")
                            .arg(imagePath)
                            .arg(rect.width())
                            .arg(rect.height())
                            .arg(blurProgress, 0, 'f', 3)
                            .arg(scaleProgress, 0, 'f', 3)
                            .arg(static_cast<double>(layerAlpha), 0, 'f', 3);

    if (!m_stableWallpaperCache.contains(key)) {
        QPixmap composed(rect.size());
        composed.fill(QColor(QStringLiteral("#000000")));
        QPainter cachePainter(&composed);
        WallpaperRenderer::drawLockWallpaperLayers(cachePainter, sharpPixmap, blurPixmap,
                                                   QRect(QPoint(0, 0), rect.size()), blurProgress, scaleProgress,
                                                   layerAlpha);
        cachePainter.end();
        m_stableWallpaperCache.insert(key, composed);
    }

    painter.drawPixmap(rect.topLeft(), m_stableWallpaperCache.value(key));
    return true;
}

void FullBackgroundWidget::drawWallpaperLayers(QPainter &painter, const QPixmap &sharpPixmap, const QPixmap &blurPixmap,
                                                 const QRect &rect, qreal blurProgress, qreal scaleProgress,
                                                 float layerAlpha)
{
    if (drawStableCompositedWallpaper(painter, sharpPixmap, blurPixmap, rect, blurProgress, scaleProgress, layerAlpha)) {
        return;
    }

    const bool lockedStatic = qFuzzyCompare(blurProgress, 1.0) && qFuzzyCompare(scaleProgress, 1.0);
    const bool desktopStatic = qFuzzyIsNull(blurProgress) && qFuzzyIsNull(scaleProgress);
    const bool useComposite = m_wallpaperAnimActive || (!lockedStatic && !desktopStatic);

    if (!useComposite) {
        WallpaperRenderer::drawLockWallpaperLayers(
            painter, sharpPixmap, blurPixmap, rect, blurProgress, scaleProgress, layerAlpha);
        return;
    }

    if (m_layerCompositeCacheSize != rect.size()) {
        m_layerCompositeCache = QPixmap(rect.size());
        m_layerCompositeCacheSize = rect.size();
    }
    m_layerCompositeCache.fill(QColor(QStringLiteral("#000000")));
    QPainter compositePainter(&m_layerCompositeCache);
    WallpaperRenderer::drawLockWallpaperLayers(compositePainter, sharpPixmap, blurPixmap,
                                               QRect(QPoint(0, 0), rect.size()), blurProgress, scaleProgress, layerAlpha);
    compositePainter.end();
    painter.drawPixmap(rect.topLeft(), m_layerCompositeCache);
}

void FullBackgroundWidget::initBlurAnimation()
{
    if (!m_blurAnimation) {
        m_blurAnimation = new QVariantAnimation(this);
        connect(m_blurAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            m_blurProgress = value.toReal();
            scheduleWallpaperRepaint();
        });
        connect(m_blurAnimation, &QAbstractAnimation::finished, this,
                &FullBackgroundWidget::onWallpaperAnimationFinished);
    }
    if (!m_scaleAnimation) {
        m_scaleAnimation = new QVariantAnimation(this);
        connect(m_scaleAnimation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            m_scaleProgress = value.toReal();
            if (!m_blurAnimation || m_blurAnimation->state() != QAbstractAnimation::Running) {
                scheduleWallpaperRepaint();
            }
        });
        connect(m_scaleAnimation, &QAbstractAnimation::finished, this,
                &FullBackgroundWidget::onWallpaperAnimationFinished);
    }
}

void FullBackgroundWidget::startBlurTransition()
{
    if (!m_blurAnimation) {
        initBlurAnimation();
    }
    if (!m_blurAnimation || !m_scaleAnimation) {
        m_blurProgress = 1.0;
        m_scaleProgress = 1.0;
        return;
    }

    invalidateStableWallpaperCache();

    const QEasingCurve enterScaleCurve = LockMainUiAnim::enterEasingCurve();
    const QEasingCurve enterBlurCurve = LockMainUiAnim::exitEasingCurve();

    m_blurAnimation->stop();
    m_scaleAnimation->stop();
    m_blurProgress = 0.0;
    m_scaleProgress = 0.0;
    m_blurAnimation->setDuration(LockMainUiAnim::kFrostedGlassTransitionMs);
    m_blurAnimation->setEasingCurve(enterBlurCurve);
    m_blurAnimation->setStartValue(0.0);
    m_blurAnimation->setEndValue(1.0);
    m_scaleAnimation->setDuration(LockMainUiAnim::kWallpaperEnterScaleDurationMs);
    m_scaleAnimation->setEasingCurve(enterScaleCurve);
    m_scaleAnimation->setStartValue(0.0);
    m_scaleAnimation->setEndValue(1.0);
    setWallpaperAnimActive(true);
    m_wallpaperRepaintClock.invalidate();
    m_blurAnimation->start();
    m_scaleAnimation->start();
}

void FullBackgroundWidget::startUnlockTransition()
{
    if (!m_blurAnimation) {
        initBlurAnimation();
    }
    if (!m_blurAnimation || !m_scaleAnimation) {
        m_blurProgress = 0.0;
        m_scaleProgress = 0.0;
        Q_EMIT wallpaperTransitionFinished();
        return;
    }

    invalidateStableWallpaperCache();

    const QEasingCurve exitCurve = LockMainUiAnim::unlockBackgroundExitEasingCurve();

    m_blurAnimation->stop();
    m_scaleAnimation->stop();
    m_blurProgress = 1.0;
    m_scaleProgress = 1.0;
    m_blurAnimation->setDuration(LockMainUiAnim::kFrostedGlassTransitionMs);
    m_blurAnimation->setEasingCurve(exitCurve);
    m_blurAnimation->setStartValue(1.0);
    m_blurAnimation->setEndValue(0.0);
    /* 摹客登录→桌面：缩放 1.15→1 为 260ms，毛玻璃 400ms，并行 */
    m_scaleAnimation->setDuration(LockMainUiAnim::kWallpaperUnlockScaleDurationMs);
    m_scaleAnimation->setEasingCurve(exitCurve);
    m_scaleAnimation->setStartValue(1.0);
    m_scaleAnimation->setEndValue(0.0);
    setWallpaperAnimActive(true);
    m_wallpaperRepaintClock.invalidate();
    m_blurAnimation->start();
    m_scaleAnimation->start();
}

void FullBackgroundWidget::applyUnlockWallpaperVisualFinishedState()
{
    if (m_blurAnimation) {
        m_blurAnimation->stop();
    }
    if (m_scaleAnimation) {
        m_scaleAnimation->stop();
    }
    setWallpaperAnimActive(false);
    m_blurProgress = 0.0;
    m_scaleProgress = 0.0;
    update();
}

void FullBackgroundWidget::persistAuthSuccessLoginInfo(const QString &strUserName)
{
    if (!UniAuthService::instance()) {
        return;
    }
    qInfo() << "SaveLastLoginUser!";
    UniAuthService::instance()->SaveLastLoginUser(strUserName);
#ifdef WITHKYSEC
    if (kysec_is_disabled() || !kysec_get_3adm_status()) {
#endif
        UniAuthService::instance()->SaveQuickLoginUser(strUserName);
#ifdef WITHKYSEC
    }
#endif
}

void FullBackgroundWidget::completeUnlockAfterAnimation(const QString &strUserName)
{
    endAuthUiWallpaperSnapshot();
    applyUnlockWallpaperVisualFinishedState();
    qInfo() << "onAuthSucceed hide window";
    onCloseScreensaver();
    clearLockState();
    if (m_lockWidget) {
        m_lockWidget->drawRootBackground();
    }
    persistAuthSuccessLoginInfo(strUserName);
    Q_EMIT m_modelLockDialog->CheckTurnOnScreen();
    m_isUnlockAnimating = false;
}

void FullBackgroundWidget::onShowSwitchUserLock()
{
    if (m_modelLockDialog->isSessionTools()) {
        if (m_lockWidget && !m_lockWidget->isHidden()) {
            m_lockWidget->exitSubWidget();
        }
        m_modelLockDialog->setIsSessionTools(false);
        setLockState();
    }
    moveToPrimaryScreen();
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    show();
    Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    Q_EMIT m_modelLockDialog->lockStateChanged(true, false);
    if (m_lockWidget && !m_lockWidget->isHidden()) {
        m_lockWidget->show();
        m_lockWidget->onShowUserListWidget(true);
    }
}

void FullBackgroundWidget::onShowSwitchToUser(QString strUserName)
{
    if (strUserName.isEmpty() || !m_modelLockDialog->findUserByName(strUserName)) {
        qInfo() << "Switch to user failed, name invalid or not exist:" << strUserName;
        return;
    }

    if (m_lockWidget && !m_lockWidget->isHidden()) {
        m_lockWidget->exitSubWidget();
    }
    if (m_modelLockDialog->isSessionTools()) {
        m_modelLockDialog->setIsSessionTools(false);
        setLockState();
    }

    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    Q_EMIT m_modelLockDialog->lockStateChanged(true, false);
    show();
    QTimer::singleShot(0, this, [=]() {
        if (m_lockWidget) {
            m_lockWidget->SwitchToUser(strUserName);
        }
    });
}

void FullBackgroundWidget::onShowSessionTools()
{
    if (m_modelLockDialog->isSessionTools()) {
        return;
    }
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    m_modelLockDialog->setIsSessionTools(true);
    show();
    Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    Q_EMIT m_modelLockDialog->lockStateChanged(true, true);
    if (m_modelLockDialog->isSessionTools()) {
        applyVisualMode(VisualMode::SessionToolsStable);
        setCurrentBackground("ToolsBackground");
    }
    if (m_lockWidget && !m_lockWidget->isHidden()) {
        m_lockWidget->onShowPowerListWidget();
    }
}

void FullBackgroundWidget::onShowAppBlockWindow(int actionType)
{
    if (!m_modelLockDialog->getIsLegacy()) {
        QVector<InhibitorInfo> lockcheck;
        QVector<WindowInfo::WindowInfo> windows;
        bool isSleepLock = false;
        if (actionType == 0 || actionType == 1 || actionType == 4) {
            lockcheck = m_modelLockDialog->getShutdownLockcheck();
            lockcheck.append(m_modelLockDialog->getLogoutLockCheck());
            windows = m_modelLockDialog->getWindowList();
        } else if (actionType == 2 || actionType == 3) {
            lockcheck = m_modelLockDialog->getSleepLockcheck();
            isSleepLock = true;
        }
        if (!lockcheck.isEmpty() || !windows.isEmpty()) {
            show();
            if (m_lockWidget && !m_lockWidget->isHidden())
                screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
            Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
            Q_EMIT m_modelLockDialog->lockStateChanged(true, true);
            if (m_lockWidget && !m_lockWidget->isHidden()) {
                m_lockWidget->onShowInhibitWarning(lockcheck, windows, actionType, isSleepLock);
            }
            if (m_modelLockDialog->isSessionTools()) {
                setCurrentBackground("ToolsBackground");
            }
        }
    } else {
        QVector<InhibitorInfo> lockcheck;
        if (actionType == 0 || actionType == 1 || actionType == 4) {
            lockcheck = m_modelLockDialog->getShutdownLockcheck();
        } else if (actionType == 2 || actionType == 3) {
            lockcheck = m_modelLockDialog->getSleepLockcheck();
        } else if (actionType == 5 || actionType == 6 || actionType == 7) {
            lockcheck = m_modelLockDialog->getLogoutLockCheck();
        }
        if (!lockcheck.isEmpty()) {
            show();
            if (m_lockWidget && !m_lockWidget->isHidden())
                screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
            Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
            Q_EMIT m_modelLockDialog->lockStateChanged(true, true);
            if (m_lockWidget && !m_lockWidget->isHidden()) {
                m_lockWidget->onShowInhibitWarning(lockcheck, actionType);
            }
            if (m_modelLockDialog->isSessionTools()) {
                setCurrentBackground("ToolsBackground");
            }
        }
    }
}

void FullBackgroundWidget::onShowMultiUsersBlockWindows(int actionType)
{
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    show();
    if (m_lockWidget && !m_lockWidget->isHidden())
        Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    Q_EMIT m_modelLockDialog->lockStateChanged(true, true);
    if (m_lockWidget && !m_lockWidget->isHidden()) {
        m_lockWidget->onMulUsersLogined(actionType);
    }
    if (m_modelLockDialog->isSessionTools()) {
        setCurrentBackground("ToolsBackground");
    }
}

void FullBackgroundWidget::onShowSessionIdle()
{
    // if screensaver is never do nothing
    if (m_modelLockDialog->getIdleDelay() == -1) {
        return;
    }
    onShowScreensaver();
    delayLockScreen();
    Q_EMIT m_modelLockDialog->lockStateChanged(true, true, true);
}

void FullBackgroundWidget::onShowLockScreensaver()
{
    onShowLock(false);
    onShowScreensaver();
}

void FullBackgroundWidget::onShowScreensaver()
{
    if (screenStatus & SCREEN_SAVER) {
        return;
    }
    if (m_modelLockDialog->isSessionTools()) {
        if (m_lockWidget && !m_lockWidget->isHidden()) {
            m_lockWidget->exitSubWidget(true);
        }
        m_modelLockDialog->setIsSessionTools(false);
        setLockState();
    }

    screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER);
    // 当前用户未变化时不触发认证，会在退出屏保后触发认证
    if (m_modelLockDialog->currentUserName().isEmpty()
        || m_modelLockDialog->currentUserName() != m_modelLockDialog->defaultUserName()) {
        Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    }
    for (auto screen : effectiveApplicationScreens()) {
        ScreenSaver *saver = m_modelLockDialog->getScreensaver();
        /*锁屏设置的Qt::WA_TranslucentBackground属性会导致第三方屏保变得透明，因此在使用第三方屏保时
         * 取消该属性，清除屏保时再设置回来*/
        if (saver->path != "/usr/lib/ukui-screensaver/ukui-screensaver-default") {
            //	    setAttribute(Qt::WA_TranslucentBackground,false);
        }

        ScreenSaverWidget *saverWidget
            = new ScreenSaverWidget(saver, getSharpBackground(m_background->image, screen->geometry()), this);
        qDebug() << " new ScreenSaverWidget";
        widgetXScreensaverList.push_back(saverWidget);
        // 深色模式有一像素的白边，所以主屏幕向左，向右移一个像素点;这种操作后，外显上方仍旧会有一个像素的白边，暂时不对外显做偏移处理
        if (screen == qApp->primaryScreen()) {
            saverWidget->setGeometry(
                screen->geometry().x() - 1,
                screen->geometry().y() - 1,
                screen->geometry().width() + 1,
                screen->geometry().height() + 1);
        } else {
            saverWidget->setGeometry(screen->geometry());
        }
    }
    show();
    if (screenStatus & SCREEN_SAVER) {
        Q_EMIT m_modelLockDialog->lockStateChanged(true, false, true);
    } else {
        Q_EMIT m_modelLockDialog->lockStateChanged(true, false);
    }
    if (m_lockWidget) {
        m_lockWidget->stopAuth();
    }
}

void FullBackgroundWidget::onClearScreensaver()
{
    screenStatus = (ScreenStatus)(screenStatus & ~SCREEN_SAVER);
    bool isStartAuth = false;
    if (screenStatus & SCREEN_LOCK) {
        if (m_lockWidget) {
            m_lockWidget->startAuth();
            isStartAuth = true;
        }
    }

    for (auto widget : widgetXScreensaverList) {
        widget->close();
        // 这里使用deleteLater会导致在锁屏界面快速按esc和空格引发锁屏内存占用达到几个g的内存问题，原因未知
        // widget->deleteLater();
        delete widget;
        widget = nullptr;
    }
    widgetXScreensaverList.clear();

    unsetCursor();

    if (screenStatus == UNDEFINED) {
        onCloseScreensaver();
    } else {
        m_isEnteringFromScreensaver = true;
        onShowLock(false);
        m_isEnteringFromScreensaver = false;
        if (m_lockWidget) {
            if (!isStartAuth) {
                m_lockWidget->startAuth();
            }
        }
    }
}

void FullBackgroundWidget::delayLockScreen()
{
    qDebug() << "delayLockScreen" << m_modelLockDialog->getLockTimeout() << m_modelLockDialog->getLockEnabled();

    if (!m_timerLock) {
        m_timerLock = new QTimer(this);
        connect(m_timerLock, &QTimer::timeout, this, &FullBackgroundWidget::onLockScreenTimeout);
    }

    if (m_modelLockDialog->getLockTimeout() != -1 && m_modelLockDialog->getLockEnabled()) {
        stopDelayLockScreen();
        m_timerLock->start(m_modelLockDialog->getLockTimeout() * 1000);
    }
}

void FullBackgroundWidget::stopDelayLockScreen()
{
    if (m_timerLock && m_timerLock->isActive()) {
        m_timerLock->stop();
    }
}

void FullBackgroundWidget::onLockScreenTimeout()
{
    qDebug() << "onLockScreenTimeout:" << m_modelLockDialog->getLockEnabled();
    if (m_modelLockDialog->getLockEnabled()) {
        screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    }
    m_timerLock->stop();
}

void FullBackgroundWidget::onCloseScreensaver()
{
    m_lockEnterFromBlankPending = false;
    if (m_lockWidget) {
        // 关闭锁屏时立即清理透明度/动画残留，避免影响后续任意入口
        m_lockWidget->resetMainUiVisualState(true);
        m_lockWidget->exitSubWidget(false, false);
    }
    hide();
    Q_EMIT m_modelLockDialog->lockStateChanged(false, false);
    stopDelayLockScreen();
    screenStatus = UNDEFINED;
    for (auto widget : widgetXScreensaverList) {
        widget->close();
        // 这里使用deleteLater会导致在锁屏界面快速按esc和空格引发锁屏内存占用达到几个g的内存问题，原因未知
        // widget->deleteLater();
        delete widget;
        widget = nullptr;
    }
    widgetXScreensaverList.clear();
    if (m_isBlank) {
        m_isBlank = false;
        unsetCursor();
    }
}

void FullBackgroundWidget::scheduleDesktopResized()
{
    if (!m_desktopResizeDebounceTimer) {
        m_desktopResizeDebounceTimer = new QTimer(this);
        m_desktopResizeDebounceTimer->setSingleShot(true);
        m_desktopResizeDebounceTimer->setInterval(50);
        connect(m_desktopResizeDebounceTimer, &QTimer::timeout, this, &FullBackgroundWidget::onDesktopResized);
    }
    m_desktopResizeDebounceTimer->start();
}

void FullBackgroundWidget::invalidateWallpaperAnimCaches()
{
    m_layerCompositeCache = QPixmap();
    m_layerCompositeCacheSize = QSize();
    m_wallpaperSnapshots.clear();
    m_authUiSnapshotActive = false;
    invalidateStableWallpaperCache();
}

void FullBackgroundWidget::onDesktopResized()
{
    invalidateWallpaperAnimCaches();
    QDesktopWidget *desktop = QApplication::desktop();
    qInfo() << "[FullBackgroundWidget] [onDesktopResized]:" << desktop->geometry();
    if (desktop->geometry().width() > m_defaultGeometry.width()
        || desktop->geometry().height() > m_defaultGeometry.height()) {
        setGeometry(desktop->geometry());
    } else {
        setGeometry(m_defaultGeometry);
    }
    // 单屏到多屏时需添加一个多new一个屏保的逻辑，不好处理，改为在屏保时监听到屏幕变化，退出屏保，与sp1保持一致。
    if (widgetXScreensaverList.size()) {
        onClearScreensaver();
    }
    if (m_lockWidget) {
        if (m_modelLockDialog->isSessionTools()) {
            moveToCurrentScreen();
        } else {
            moveToPrimaryScreen();
        }
        m_lockWidget->reloadRootBackground();
    }

    if (m_windowHelper) {
        QScreen *screen = m_windowHelper->getCurrentScreenBlocking();
        if (screen) {
            if (m_animation) {
                m_animation->setGeometry(screen->geometry());
            }
        }
    }
    repaint();
}

void FullBackgroundWidget::onPrepareForSleep(bool sleep)
{
    /// 系统休眠时，会关闭总线，导致设备不可用，发生错误
    /// 在系统休眠之前停止认证，在系统唤醒后重新开始认证
    qInfo() << "onPrepareForSleep begin:" << sleep;

    if (!isVisible()) {
        qInfo() << "onPrepareForSleep done:" << sleep;
        return;
    }
    if (sleep) {
        if (m_lockWidget)
            m_lockWidget->stopAuth();
        // uninhibit();
    } else {
        if (screenStatus & SCREEN_SAVER) {
            m_isBlank = false;
            onClearScreensaver();
        } else {
            repaint();
            if (m_lockWidget) {
                if (m_lockWidget->isHidden()) {
                    m_lockWidget->show();
                    m_lockWidget->activateWindow();
                }
                m_lockWidget->startAuth();
            }
            // inhibit();
        }
    }
    qInfo() << "onPrepareForSleep done:" << sleep;
}

void FullBackgroundWidget::mousePressEvent(QMouseEvent *e)
{
    // close();
}

QString FullBackgroundWidget::getFocusWindowName()
{
    Window focus = 0;
    int rev = 0;

    XGetInputFocus(QX11Info::display(), &focus, &rev);
    return getWindowNameFromWid(focus);
}

QString FullBackgroundWidget::getWindowNameFromWid(WId window)
{
    XClassHint ch;
    ch.res_name = NULL;
    ch.res_class = NULL;
    XGetClassHint(QX11Info::display(), window, &ch);

    QString nameStr = QString(ch.res_name);

    if (ch.res_name)
        XFree(ch.res_name);
    if (ch.res_class)
        XFree(ch.res_class);

    return nameStr;
}

int FullBackgroundWidget::RegisteSubWnd(quint64 uWndId)
{
    if (!m_listWndIds.contains(uWndId) && m_listWndIds.size() < SUBWND_COUNT_MAX) {
        m_listWndIds.append(uWndId);
        qDebug() << "RegisterSubWnd:" << uWndId;
        // QTimer::singleShot(50,this,SLOT(laterActivate()));
        return m_listWndIds.size();
    } else {
        return -1;
    }
}

int FullBackgroundWidget::UnRegisteSubWnd(quint64 uWndId)
{
    if (m_listWndIds.contains(uWndId)) {
        m_listWndIds.removeAll(uWndId);
        qDebug() << "UnRegisterSubWnd:" << uWndId;
        // QTimer::singleShot(50,this,SLOT(laterActivate()));
        return m_listWndIds.size();
    } else {
        return -1;
    }
}

QList<quint64> FullBackgroundWidget::GetSubWndIds()
{
    return m_listWndIds;
}

void FullBackgroundWidget::moveToCurrentScreen()
{
    if (m_windowHelper) {
        qDebug() << "move to current screen";
        QScreen *screen = m_windowHelper->getCurrentScreenBlocking();
        qDebug() << "screen" << screen->geometry();
        if (m_lockWidget) {
            m_lockWidget->setGeometry(screen->geometry());
        }
    }
}

void FullBackgroundWidget::moveToPrimaryScreen()
{
    if (!m_lockWidget) {
        return;
    }

    const QList<QScreen *> allScreens = QGuiApplication::screens();
    const QList<QScreen *> screens = effectiveApplicationScreens();
    qInfo() << "[ScreenGeom] moveToPrimaryScreen raw screens count:" << allScreens.size();
    for (QScreen *screen : allScreens) {
        if (!screen) {
            continue;
        }
        qInfo() << "[ScreenGeom]   raw name:" << screen->name() << "geometry:" << screen->geometry()
                << "primary:" << (screen == qApp->primaryScreen());
    }

    QScreen *primaryScreen = qApp->primaryScreen();
    if (primaryScreen && !screens.contains(primaryScreen) && !screens.isEmpty()) {
        primaryScreen = screens.first();
    }

    bool chkInOneScreen = false;
    for (auto screen : screens) {
        if (screen == primaryScreen) {
            qInfo() << "LockWidget:" << screen->geometry() << m_lockWidget->geometry() << geometry();
            if (m_agreementWindow && m_agreementWindow->isVisible()) {
                m_agreementWindow->setGeometry(screen->geometry());
            }
            if (m_lockWidget->geometry() != screen->geometry()) {
                if (!m_lockWidget->isHidden()) {
                    m_lockWidget->hide();
                    m_lockWidget->setGeometry(screen->geometry());
                    m_lockWidget->show();
                } else {
                    m_lockWidget->setGeometry(screen->geometry());
                }
            }
            chkInOneScreen = true;
            break;
        }
    }
    if (!chkInOneScreen) {
        for (auto screen : screens) {
            if (screen) {
                /*避免切换时闪烁*/
                qInfo() << "LockWidget:" << screen->geometry() << m_lockWidget->geometry() << "," << screen
                        << geometry();
                if (m_agreementWindow && m_agreementWindow->isVisible()) {
                    m_agreementWindow->setGeometry(screen->geometry());
                }
                if (m_lockWidget->geometry() != screen->geometry()) {
                    if (!m_lockWidget->isHidden()) {
                        m_lockWidget->hide();
                        m_lockWidget->setGeometry(screen->geometry());
                        m_lockWidget->show();
                    } else {
                        m_lockWidget->setGeometry(screen->geometry());
                    }
                }
                chkInOneScreen = true;
                break;
            }
        }
    }
}

void FullBackgroundWidget::fakeFocusIn(WId window)
{
    // We have keyboard grab, so this application will
    // get keyboard events even without having focus.
    // Fake FocusIn to make Qt realize it has the active
    // window, so that it will correctly show cursor in the dialog.
    XEvent ev;
    memset(&ev, 0, sizeof(ev));
    ev.xfocus.display = QX11Info::display();
    ev.xfocus.type = FocusIn;
    ev.xfocus.window = window;
    ev.xfocus.mode = NotifyNormal;
    ev.xfocus.detail = NotifyAncestor;
    XSendEvent(QX11Info::display(), window, False, NoEventMask, &ev);
    XFlush(QX11Info::display());
}

void FullBackgroundWidget::laterActivate()
{
    if (!QX11Info::isPlatformX11()) {
        activateWindow();
        return;
    }
    XSetInputFocus(QX11Info::display(), this->winId(), RevertToParent, CurrentTime);
    activateWindow();
}

void FullBackgroundWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    if (QX11Info::isPlatformX11()) {
        tryGrabKeyboard();
    }
    QTimer::singleShot(100, this, [=]() {
        m_lockWidget->updateFont();
        m_lockWidget->updateFontSize();
    });
}

void FullBackgroundWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (!QX11Info::isPlatformX11()) {
        if (e->key() == Qt::Key_Escape && screenStatus == SCREEN_LOCK) { // "escape"
            enqueueEvent(EC_ShowScreenSaver);
        } else if (screenStatus & SCREEN_SAVER && !m_isBlank) {
            enqueueEvent(EC_ClearScreenSaver);
        }
    }
}

void FullBackgroundWidget::tryGrabKeyboard()
{
    if (window()->windowHandle() && window()->windowHandle()->setKeyboardGrabEnabled(true)) {
        m_tryGrabTimes = 0;
        return;
    }

    m_tryGrabTimes++;

    if (m_tryGrabTimes == 15) {
        qWarning() << "setKeyboardGrabEnable failed! focus Window is" << getFocusWindowName();
        m_tryGrabTimes = 0;
        return;
    }

    QTimer::singleShot(100, this, &FullBackgroundWidget::tryGrabKeyboard);
}

bool FullBackgroundWidget::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (qstrcmp(eventType, "xcb_generic_event_t") != 0) {
        return false;
    }
    xcb_generic_event_t *event = reinterpret_cast<xcb_generic_event_t *>(message);
    const uint8_t responseType = event->response_type & ~0x80;

    if (responseType == XCB_FOCUS_OUT) {
        // 丢焦点时，检测抢焦点的窗口是否为锁屏的子窗口，比如QMenu,QCombobox等，
        // 如果不是子窗口，则重新给锁屏设置一次焦点
        QString focusWindow = getFocusWindowName();
        if (focusWindow == "ukui-screensaver-dialog") {
            return false;
        } else {
            qDebug() << "focus out. focus window is" << focusWindow;
        }
        laterActivate();
        qDebug() << "XCB_FOCUS_OUT !";
    } else if (responseType == XCB_FOCUS_IN) {
        // 因为弹出菜单会释放键盘抓取，所以在锁屏窗口重新获取到焦点时，重新抓取一次键盘焦点
        qDebug() << "XCB_FOCUS_IN !";
    } else if (responseType == XCB_GE_GENERIC) {
        // 因为锁屏抓取了键盘，所以锁屏窗口不管有没有焦点，都能够输入，但仍然需要发送一个
        // 虚拟的focusin事件，来激活窗口焦点,同时点击锁屏界面，也能触发锁屏键盘抓取
        xcb_ge_generic_event_t *xc = reinterpret_cast<xcb_ge_generic_event_t *>(event);
        if (xc->event_type == XCB_BUTTON_PRESS) {
            if (this->windowHandle()) {
                bool grabed = this->windowHandle()->setKeyboardGrabEnabled(true);
                if (!grabed) {
                    qDebug() << "setKeyboardGrabEnable failed! focus Window is" << getFocusWindowName();
                }
            }
            onGlobalButtonPressed(QCursor::pos().x(), QCursor::pos().y());
        } else if (xc->event_type == XCB_BUTTON_RELEASE) {
        } else if (xc->event_type == XCB_MOTION_NOTIFY) {
            onGlobalButtonDrag(QCursor::pos().x(), QCursor::pos().y());
        }
    } else if (responseType == XCB_BUTTON_PRESS) {
        xcb_button_press_event_t *xc = reinterpret_cast<xcb_button_press_event_t *>(event);
        int x = xc->root_x;
        int y = xc->root_y;
        onGlobalButtonPressed(x, y);
        qDebug() << "XCB_BUTTON_PRESS:" << x << "," << y;
    } else if (responseType == XCB_BUTTON_RELEASE) {

    } else if (responseType == XCB_MOTION_NOTIFY) {
        xcb_motion_notify_event_t *xc = reinterpret_cast<xcb_motion_notify_event_t *>(event);
        int x = xc->root_x;
        int y = xc->root_y;
        onGlobalButtonDrag(x, y);
        qDebug() << "XCB_MOTION_NOTIFY:" << x << "," << y;
    } else if (responseType == XCB_KEY_PRESS) {
        xcb_key_press_event_t *xc = reinterpret_cast<xcb_key_press_event_t *>(event);
        // qDebug()<<"XCB_KEY_PRESS:"<<xc->detail;
        onGlobalKeyPress(xc->detail);
    } else if (responseType == XCB_KEY_RELEASE) {
        xcb_key_release_event_t *xc = reinterpret_cast<xcb_key_release_event_t *>(event);
        qDebug() << "XCB_KEY_RELEASE:" << xc->detail;
        onGlobalKeyRelease(xc->detail);
    } else if (responseType == m_RREventBase + RRScreenChangeNotify) {
        if (isGreeterMode() && !m_modelLockDialog->isUseWayland()) {
            RRScreenChangeEvent(false);
        }
    }
    return false;
}

void FullBackgroundWidget::onCurUserChanged(const QString &strUserName)
{
    if (isGreeterMode() && !m_modelLockDialog->isUseWayland()) {
        DisplayService::instance(m_modelLockDialog)->setCurUserName(strUserName);
    }
}

void FullBackgroundWidget::onAuthSucceed(QString strUserName)
{
    if (IsStartupMode()) {
        setIsStartupMode(false);
    }
    if (getenv("USER") == strUserName) {
        if (!m_isUnlockAnimating) {
            m_isUnlockAnimating = true;
            const bool greeterNoAnim = isGreeterMode();
            if (greeterNoAnim) {
                initBlurAnimation();
                if (m_blurAnimation) {
                    m_blurAnimation->stop();
                }
                if (m_scaleAnimation) {
                    m_scaleAnimation->stop();
                }
                m_blurProgress = 0.0;
                m_scaleProgress = 0.0;
                update();
                if (m_lockWidget) {
                    m_lockWidget->applyMainUiExitFinishedState();
                }
                QTimer::singleShot(0, this, [this, strUserName]() { completeUnlockAfterAnimation(strUserName); });
            } else {
                if (!m_unlockTransition) {
                    m_unlockTransition = new LockScreenTransitionController(this);
                    connect(m_unlockTransition, &LockScreenTransitionController::unlockFinished, this,
                            [this](const QString &userName) { completeUnlockAfterAnimation(userName); });
                }
                m_unlockTransition->playUnlock(m_lockWidget, this, strUserName);
            }
        }
    } else {
        clearLockState();
        if (m_lockWidget) {
            m_lockWidget->drawRootBackground();
        }
        if (m_modelLockDialog) {
            Q_EMIT m_modelLockDialog->startSession();
        }
    }
}

void FullBackgroundWidget::onStartFcitx()
{
    if (isGreeterMode()) {
        startFcitx();
        qInfo() << " QT_IM_MODULE = " << getenv("QT_IM_MODULE");
    }
}

void FullBackgroundWidget::onGlobalKeyPress(const quint8 &key) {}

void FullBackgroundWidget::onGlobalKeyRelease(const quint8 &key)
{
    //    if (m_lockWidget && m_lockWidget->isVisible()) {
    //        m_lockWidget->onGlobalkeyRelease(key);
    //    }
    if (key == 9 && screenStatus == SCREEN_LOCK) { // "escape"
        enqueueEvent(EC_ShowScreenSaver);
    } else if (screenStatus & SCREEN_SAVER /* && !isBlank*/) {
        qDebug() << "clearclear";
        enqueueEvent(EC_ClearScreenSaver);
    }
}

void FullBackgroundWidget::onGlobalButtonDrag(int xPos, int yPos)
{
    if (screenStatus & SCREEN_SAVER && !m_isBlank) {
        qDebug() << "clearclear";
        enqueueEvent(EC_ClearScreenSaver);
    }
}

void FullBackgroundWidget::onGlobalButtonPressed(int xPos, int yPos)
{
    if (screenStatus & SCREEN_SAVER && !m_isBlank) {
        enqueueEvent(EC_ClearScreenSaver);
    }
}

//////////////////////////////////
void FullBackgroundWidget::initCurrentBackground()
{
    m_background = QSharedPointer<Background>(new Background);
    UserInfoPtr userInfo = m_modelLockDialog->findUserByName(m_modelLockDialog->defaultUserName());
    if (userInfo) {
        if (!userInfo->backGroundColor().isEmpty()) {
            if (!m_modelLockDialog->isSessionTools()) {
                QString strBackground = getDesktopBackground();
                m_background->type = BACKGROUND_IMAGE;
                m_background->image = strBackground;
            } else {
                m_background->type = BACKGROUND_COLOR;
                m_background->color = userInfo->backGroundColor();
            }
        } else {
            m_background->type = BACKGROUND_IMAGE;
            m_background->image = getUserBackgroundPath(m_modelLockDialog->defaultUserName());
            if (m_background->image.isEmpty() || !QFileInfo(m_background->image).isReadable()) {
                m_background->image = getDefaultBackgroundPath();
            }
        }
        m_currentUserName = userInfo->name();
    }
    m_transition.started = false;
    m_loadingOneBackgroundFuture = QtConcurrent::run([=]() {
        QString strBackground = getUserBackgroundPath(m_modelLockDialog->defaultUserName());
        addBackgroundData(strBackground);
    });

    connect(m_modelLockDialog, &LockDialogModel::setCurrentUser, this, &FullBackgroundWidget::onCurrentUserBackground);
    connect(
        m_modelLockDialog, &LockDialogModel::currentUserChanged, this, &FullBackgroundWidget::onUpdateUserBackground);
}

QString FullBackgroundWidget::getDefaultBackgroundPath()
{
    QString default_background_path = "/usr/share/backgrounds/1-warty-final-ubuntukylin.jpg";
    QFile file(default_background_path);
    if (!file.exists()) {
        default_background_path = "/usr/share/backgrounds/1-openkylin.jpg";
        QFile file(default_background_path);
        if (!file.exists()) {
            return "";
        }
    }
    return default_background_path;
}

void FullBackgroundWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), QColor("#000000"));

    for (QScreen *screen : effectiveApplicationScreens()) {
        // 在每个屏幕上绘制背景
        const QRect screenRect = screen->geometry();
        if (!screenRect.isValid()) {
            continue;
        }
        const QString snapKey = QStringLiteral("%1x%2").arg(screenRect.width()).arg(screenRect.height());
        const bool useSnapshot = m_authUiSnapshotActive && !m_wallpaperAnimActive
            && m_wallpaperSnapshots.contains(snapKey);
        if (useSnapshot) {
            painter.drawPixmap(screenRect.topLeft(), m_wallpaperSnapshots.value(snapKey));
        } else if (m_transition.started) {
            drawBackground(painter, m_transition.to, m_transition.from, screenRect, m_transition.stage);
        } else {
            drawBackground(painter, m_background, m_background, screenRect);
        }
    }
    if (shouldPaintLockSubtreeInBackgroundPass()) {
        QWidget::paintEvent(event);
    }
}

bool FullBackgroundWidget::isOpenGradation()
{
    bool isOpenGradation = false;
    //    return true;//test 一直触发渐变效果
    if (m_currentUserName.isEmpty() || m_oldUserName.isEmpty())
        return isOpenGradation;
    if (m_currentUserName == m_oldUserName)
        return isOpenGradation;

    if (getUserBackgroundPath(m_oldUserName) != getUserBackgroundPath(m_currentUserName)) {
        isOpenGradation = true;
    }

    return isOpenGradation;
}

void FullBackgroundWidget::onUpdateUserBackground(const QString &strUserName)
{
    QString strBackground = getUserBackgroundPath(strUserName);
    qDebug() << "UpdateUserBackground:" << strUserName << strBackground;
    if (m_currentUserName == strUserName && m_background->image == strBackground)
        return;
    m_oldUserName = m_currentUserName;
    m_currentUserName = strUserName;
    if (m_modelLockDialog->isSessionTools())
        return;
    QSharedPointer<Background> background(new Background);
    background->type = BACKGROUND_IMAGE;
    background->image = strBackground;
    setBackground(background);
}

void FullBackgroundWidget::onTransition()
{
    m_transition.stage += 0.05; //= (1 - cos(M_PI * m_transition.stage)) / 2;

    if (m_transition.stage >= 1.0)
        stopTransition();

    // repaint();
    update();
}

void FullBackgroundWidget::startTransition(QSharedPointer<Background> &from, QSharedPointer<Background> &to)
{
    if (!m_backgrondGradationTimer) {
        m_backgrondGradationTimer = new QTimer(this);
        connect(m_backgrondGradationTimer, &QTimer::timeout, this, &FullBackgroundWidget::onTransition);
    }
    stopTransition();

    m_transition.from = from;
    m_transition.to = to;
    m_transition.stage = 0.0;
    m_transition.started = true;

    m_backgrondGradationTimer->start(10);
}

void FullBackgroundWidget::stopTransition()
{
    if (m_backgrondGradationTimer && m_backgrondGradationTimer->isActive()) {
        m_backgrondGradationTimer->stop();
    }
    m_transition.stage = 1.0;
    m_transition.started = false;
}

void FullBackgroundWidget::setBackground(QSharedPointer<Background> &background)
{
    if (background) {
        qDebug() << "FullBackgroundWidget::setBackground" << background->image;
    }

    stopTransition();

    if (background) {
        if (m_modelLockDialog->isSessionTools()) {
            m_background = background;
        } else {
            if (background->image.isEmpty() || !QFileInfo(background->image).isReadable()) {
                background->image = getDefaultBackgroundPath();
            }
        }
    }

    if (m_background && background && m_background->image == background->image) {
        repaint();
        return;
    }

    invalidateStableWallpaperCache();

    // 如果是第一次绘制背景，则不需要渐变
    // 现在由于绘制模糊背景，不需要渐变了
    if (!m_background.isNull()) {
        startTransition(m_background, background);
    }
    m_background = background;
    update();
}

void FullBackgroundWidget::drawBackground(QPainter &painter, QSharedPointer<Background> &backgroundBack,
                                          QSharedPointer<Background> &backgroundFront, const QRect &rect,
                                          float alpha)
{
    if (backgroundBack == nullptr) {
        return;
    }

    switch (backgroundBack->type) {
        case BACKGROUND_IMAGE: {
            const bool sameWallpaper
                = backgroundFront && !backgroundBack->image.isEmpty()
                  && backgroundBack->image == backgroundFront->image;
            const bool fullOpacity = alpha >= 0.999f;

            if (sameWallpaper && fullOpacity) {
                QPixmap *pixmapBlur = getBackground(backgroundBack->image, rect);
                QPixmap *pixmapSharp = getSharpBackground(backgroundBack->image, rect);
                if (!pixmapBlur || pixmapBlur->isNull() || !pixmapSharp || pixmapSharp->isNull()) {
                    painter.fillRect(rect, QColor("#035290"));
                } else {
                    drawWallpaperLayers(
                        painter, *pixmapSharp, *pixmapBlur, rect, m_blurProgress, m_scaleProgress, alpha);
                }
            } else {
                QPixmap *pixmapFront = getBackground(backgroundFront->image, rect);
                QPixmap *pixmapBack = getBackground(backgroundBack->image, rect);
                QPixmap *sharpFront = getSharpBackground(backgroundFront->image, rect);
                QPixmap *sharpBack = getSharpBackground(backgroundBack->image, rect);
                if (!pixmapFront || pixmapFront->isNull() || !pixmapBack || pixmapBack->isNull() || !sharpFront
                    || sharpFront->isNull() || !sharpBack || sharpBack->isNull()) {
                    painter.setOpacity(alpha);
                    painter.fillRect(rect, QColor("#035290"));
                    painter.setOpacity(1.0);
                } else {
                    drawWallpaperLayers(
                        painter, *sharpFront, *pixmapFront, rect, m_blurProgress, m_scaleProgress, 1.0f);
                    drawWallpaperLayers(
                        painter, *sharpBack, *pixmapBack, rect, m_blurProgress, m_scaleProgress, alpha);
                }
            }
            break;
        }
        case BACKGROUND_COLOR: {
            painter.setOpacity(alpha);
            painter.fillRect(rect, backgroundFront ? backgroundFront->color : backgroundBack->color);
            painter.setOpacity(1.0);
            break;
        }
    }
}

void FullBackgroundWidget::onRemoveUserBackground(const QString &strUserName)
{
    if (m_allBackgroundsMap.contains(strUserName))
        m_allBackgroundsMap.remove(strUserName);
}

QString FullBackgroundWidget::getUserBackgroundPath(const QString &strUserName)
{
    for (UserInfoPtr userInfo : m_modelLockDialog->usersInfo()) {
        if (strUserName == userInfo->name()) {
            if (!userInfo->greeterBackGround().isEmpty() && QFile(userInfo->greeterBackGround()).exists()) {
                return userInfo->greeterBackGround();
            } else {
                return userInfo->backGround();
            }
        }
    }
    return QString("");
}

void FullBackgroundWidget::addBackgroundData(const QString &bgPath)
{
    if (bgPath.isEmpty()) {
        qDebug() << __LINE__ << __FUNCTION__ << bgPath << ":Path is Null";
        return;
    }
    QFile file(bgPath);
    if (!file.exists()) {
        qDebug() << __LINE__ << __FUNCTION__ << "Add background file isn't exists";
        return;
    }
    for (QScreen *screen : effectiveApplicationScreens()) {
        int width = screen->geometry().width();
        int height = screen->geometry().height();

        QFileInfo fileInfo(bgPath);
        QString lastModified;
        if (fileInfo.exists()) {
            lastModified = QString::number(fileInfo.lastModified().toSecsSinceEpoch());
        }
        QString resolution = lastModified + "_" + QString("%1x%2").arg(width).arg(height);
        QPair<QString, QString> key(bgPath, resolution);
        if (!m_allBackgroundsDataMap.contains(key)) {
            syslog(LOG_DEBUG, "[ukui-screensaver-dialog] load background begin");
            QPixmap *pixmap = nullptr;
            QCryptographicHash pixmapHash(QCryptographicHash::Md5);
            QFile pixmapFile(bgPath);
            if (pixmapFile.open(QIODevice::ReadOnly) && pixmapHash.addData(&pixmapFile)) {
                QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
                QString bgThumbPath = QString("%1/bg_thumbnail/").arg(cachePath);
                QString bgThumbFileName
                    = QString("%1-%2-scaled.jpg").arg(QString(pixmapHash.result().toHex())).arg(resolution);
                QString bgThumbFile = QString("%1%2").arg(bgThumbPath).arg(bgThumbFileName);
                if (bgThumbFileName == "27ae8eb54648e4335a8cb3fb13fe4f7c-1920x1080.jpg") {
                    pixmap = new QPixmap(":/image/assets/defaultbackground.jpg");
                } else {
                    pixmap = new QPixmap(bgThumbFile);
                    if (pixmap->isNull()) {
                        delete pixmap;
                        pixmap = nullptr;
                        pixmap = new QPixmap(scaleBlurPixmap(width, height, bgPath));
                        QDir bgThumbDir(bgThumbPath);
                        if (!bgThumbDir.exists()) {
                            bgThumbDir.mkpath(".");
                        }
                        if (pixmap->save(bgThumbFile, "JPEG", 90)) {
                            qInfo() << "save bg thumb file:" << bgThumbFile;
                        }
                    }
                }
            } else {
                pixmap = new QPixmap(scaleBlurPixmap(width, height, bgPath));
            }
            syslog(LOG_DEBUG, "[ukui-screensaver-dialog] load background end");
            m_allBackgroundsDataMap[key] = pixmap;
        }
    }
}

void FullBackgroundWidget::onAddUserBackground(const QString &strUserName)
{
    QString strBackground = getUserBackgroundPath(strUserName);
    addBackgroundData(strBackground);
}

void FullBackgroundWidget::onUserBackgroundChanged(const QString &strUserName) {}

void FullBackgroundWidget::onCurrentUserBackground(const QString &strUserName)
{
    QString strBackground = getUserBackgroundPath(strUserName);
    qDebug() << "CurrentUserBackground:" << strUserName << strBackground;
    if (m_currentUserName == strUserName && m_background->image == strBackground)
        return;
    m_oldUserName = m_currentUserName;
    m_currentUserName = strUserName;
    if (m_modelLockDialog->isSessionTools())
        return;
    QSharedPointer<Background> background(new Background);
    background->type = BACKGROUND_IMAGE;
    background->image = strBackground;
    stopTransition();
    if (background) {
        if (background->image.isEmpty() || !QFileInfo(background->image).isReadable()) {
            background->image = getDefaultBackgroundPath();
        }
    }
    m_background = background;
    repaint();
}

QPixmap *FullBackgroundWidget::getBackground(const QString &path, const QRect &rect)
{
    if (path.isEmpty() || !QFile(path).exists())
        return nullptr;

    QFileInfo fileInfo(path);
    QString lastModified;
    if (fileInfo.exists()) {
        lastModified = QString::number(fileInfo.lastModified().toSecsSinceEpoch());
    }
    QString resolution = lastModified + "_" + QString("%1x%2").arg(rect.width()).arg(rect.height());
    QPair<QString, QString> key(path, resolution);

    if (m_allBackgroundsDataMap.isEmpty() && m_loadingOneBackgroundFuture.isRunning()) {
        m_loadingOneBackgroundFuture.waitForFinished();
    } else {
        if (!m_loadingOneBackgroundFuture.isFinished() && m_loadingOneBackgroundFuture.isStarted()) {
            m_loadingOneBackgroundFuture.waitForFinished();
        }
    }
    if (!m_allBackgroundsDataMap.contains(key)) {
        QPixmap *pixmap = new QPixmap(scaleBlurPixmap(rect.width(), rect.height(), path));
        m_allBackgroundsDataMap[key] = pixmap;
    }
    return m_allBackgroundsDataMap[key];
}

QPixmap *FullBackgroundWidget::getSharpBackground(const QString &path, const QRect &rect)
{
    if (path.isEmpty() || !QFile(path).exists()) {
        return nullptr;
    }

    QFileInfo fileInfo(path);
    QString lastModified;
    if (fileInfo.exists()) {
        lastModified = QString::number(fileInfo.lastModified().toSecsSinceEpoch());
    }
    QString resolution = lastModified + "_" + QString("%1x%2").arg(rect.width()).arg(rect.height());
    QPair<QString, QString> key(path, resolution);

    if (!m_allBackgroundsSharpDataMap.contains(key)) {
        QPixmap source = loadFromFile(path);
        if (source.isNull()) {
            return nullptr;
        }
        /* 按屏尺寸预缩放，动效帧用 drawPixmap 贴图，避免每帧全图 SCALED 填充 */
        m_allBackgroundsSharpDataMap[key] = new QPixmap(scalePixmapWallpaperFill(source, rect.width(), rect.height()));
    }
    return m_allBackgroundsSharpDataMap[key];
}

void FullBackgroundWidget::RRScreenChangeEvent(bool isFirst)
{
    XRRScreenResources *screen;
    screen = XRRGetScreenResources(QX11Info::display(), QX11Info::appRootWindow());
    XRROutputInfo *info;
    QList<QString> listMonitors;

    for (int i = 0; i < screen->noutput; i++) {
        info = XRRGetOutputInfo(QX11Info::display(), screen, screen->outputs[i]);
        if (info->connection == RR_Connected) {
            listMonitors.push_back(info->name);
        }
        XRRFreeOutputInfo(info);
    }

    qDebug() << "monitors = " << listMonitors;
    if (isFirst && DisplayService::instance()->isSaveParamInUsed()) {
        m_listMonitors = listMonitors;
    } else {
        onScreensChanged(listMonitors);
    }
    XRRFreeScreenResources(screen);
}

void FullBackgroundWidget::onScreensChanged(QList<QString> listMonitors)
{
    qDebug() << "newList = " << listMonitors << " listMonitors = " << m_listMonitors
             << DisplayService::instance()->isJJW7200();
    if (DisplayService::instance()->isJJW7200() == 0 && m_listMonitors.size() == listMonitors.size()) {
        bool isAllFound = true;
        for (auto monitor : listMonitors) {
            if (!m_listMonitors.contains(monitor)) {
                isAllFound = false;
                break;
            }
        }
        if (isAllFound) {
            return;
        }
    }

    m_listMonitors = listMonitors;
    if (m_listMonitors.size() < 2) {
        // 默认设置显示最大分辨率
        DisplayService::instance()->setOneDisplayMode();
    } else {
        int mode = Configuration::instance()->getValue("display-mode").toInt();
        DisplayService::instance()->switchDisplayMode((DisplayMode)mode);
    }
    // 在调用xrandr打开显示器以后，不能马上设置窗口大小，会设置不正确的
    // 分辨率，延时1000ms正常。
    QTimer::singleShot(1000, this, &FullBackgroundWidget::scheduleDesktopResized);
}

void FullBackgroundWidget::setIsStartupMode(bool isStartup)
{
    m_isStartupMode = isStartup;
    Q_EMIT StartupModeChanged(m_isStartupMode);
}

void FullBackgroundWidget::enqueueEvent(quint32 eventCode, QString param /* = ""*/)
{
    // 将按键按下事件异步存入队列
    QMutexLocker locker(&m_queueMutex);
    m_keyEventQueue.enqueue(EventInfo(eventCode, param, QDateTime::currentMSecsSinceEpoch()));
    // 启动队列处理定时器（如果还没启动）
    if (!m_queueProcessTimer->isActive()) {
        m_queueProcessTimer->start(0);
    }
}

void FullBackgroundWidget::processEventQueue()
{
    QMutexLocker locker(&m_queueMutex);

    if (m_keyEventQueue.isEmpty()) {
        return;
    }

    // 取出队列中的第一个事件进行处理
    EventInfo eventInfo = m_keyEventQueue.dequeue();
    locker.unlock(); // 释放锁，避免在处理事件时阻塞队列操作

    // 处理事件
    switch (eventInfo.eventCode) {
        case EC_ShowScreenSaver: {
            bool canShow = true;
            if (m_lockWidget && !m_lockWidget->exitSubWidget())
                canShow = false;
            if (canShow && !isGreeterMode() && m_modelLockDialog->getIdleDelay() != -1
                && !m_modelLockDialog->isSessionTools()) {
                onShowScreensaver();
            }
        } break;
        case EC_ClearScreenSaver: {
            onClearScreensaver();
        }
        default:
            break;
    }

    // 处理完一个事件后，清除队列中的相同事件
    locker.relock();
    QQueue<EventInfo> filteredQueue;
    while (!m_keyEventQueue.isEmpty()) {
        EventInfo remainingEvent = m_keyEventQueue.dequeue();
        // 只保留与当前处理事件不同的事件
        if (remainingEvent.eventCode != eventInfo.eventCode || remainingEvent.strParam != eventInfo.strParam) {
            filteredQueue.enqueue(remainingEvent);
        }
    }
    m_keyEventQueue = filteredQueue;

    // 如果队列中还有事件，继续处理
    if (!m_keyEventQueue.isEmpty()) {
        m_queueProcessTimer->start(0);
    }
}

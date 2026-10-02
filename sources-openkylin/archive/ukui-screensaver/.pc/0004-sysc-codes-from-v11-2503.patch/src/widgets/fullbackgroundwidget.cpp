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
#include "agreementwindow.h"
#include <QDesktopWidget>
#include <QApplication>
#include <QX11Info>
#include <QTimer>
#include <QPainterPath>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QStringList>
#include <QDebug>
#include <QTimer>
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
#include <QScreen>
#include <KWindowSystem>
#include <QKeyEvent>
#include "commonfunc.h"
#include "screensavermode.h"
#include "screensaverwidget.h"
#include "displayservice.h"
#include "configuration.h"
#include <windowmanager/windowmanager.h>
using namespace kdk;

FullBackgroundWidget::FullBackgroundWidget(LockDialogModel *model, QWidget *parent)
    : QWidget(parent), m_modelLockDialog(model)
{
    initCurrentBackground();
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
    m_defaultGeometry = QRect(0, 0, 7680, 4320);
    QDesktopWidget *desktop = QApplication::desktop();
    if (desktop->geometry().width() > m_defaultGeometry.width()
        || desktop->geometry().height() > m_defaultGeometry.height()) {
        setGeometry(desktop->geometry());
    } else {
        setGeometry(m_defaultGeometry);
    }

    if (!m_lockWidget) {
        m_lockWidget = new LockWidget(m_modelLockDialog, this);
        connect(m_lockWidget, &LockWidget::authSucceed, this, &FullBackgroundWidget::onAuthSucceed);
        connect(m_lockWidget, &LockWidget::sessionToolsExit, this, &FullBackgroundWidget::onCloseScreensaver);
        connect(m_lockWidget, &LockWidget::showBlankScreensaver, this, &FullBackgroundWidget::onShowBlankScreensaver);
        connect(m_lockWidget, &LockWidget::startFcitx, this, &FullBackgroundWidget::onStartFcitx);
        moveToPrimaryScreen();
    }

    if (m_modelLockDialog->getAgreementWindowShowLoginPrompt()
        && (/*IsStartupMode() || */ (qgetenv("USER") == "lightdm"))) {
        bool hideTitle = m_modelLockDialog->getAgreementWindowHideTitle();
        QString title = m_modelLockDialog->getAgreementWindowPromptTitle();
        QString text = m_modelLockDialog->getAgreementWindowText();

        // 文本内容不为空 且 如果配置了标题，则标题不为空
        if (!text.isEmpty() && (hideTitle || (!hideTitle && !text.isEmpty()))) {
            m_agreementWindow = new AgreementWindow(hideTitle, title, text, this);
            m_lockWidget->hide();
            m_agreementWindow->setGeometry(this->geometry());
            m_agreementWindow->show();

            m_lockWidget->clearFocus();
            m_agreementWindow->setFocusPolicy(Qt::StrongFocus);
            setFocusProxy(m_agreementWindow);
            m_agreementWindow->setFocus();
            // 特别提示窗口抓取键盘，避免焦点在密码框上时输入事件跑到密码框，但会导致我已知晓按钮无法响应回车
            m_agreementWindow->grabKeyboard();
            connect(m_agreementWindow, &AgreementWindow::switchToGreeter, this, [this]() {
                m_agreementWindow->hide();
                m_agreementWindow->releaseKeyboard();
                m_agreementWindow->clearFocus();
                m_lockWidget->show();
                m_lockWidget->setFocus();
                update();
            });
        }
    }

    setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);

    // 登录模式下监听屏幕插拔
    if (isGreeterMode() && QString(qgetenv("XDG_SESSION_TYPE")) != "wayland") {
        XRRQueryExtension(QX11Info::display(), &m_RREventBase, &m_RRErrorBase);
        XRRSelectInput(QX11Info::display(), QX11Info::appRootWindow(), RRScreenChangeNotifyMask);
        QtConcurrent::run([=]() { RRScreenChangeEvent(true); });
    }

    qApp->installNativeEventFilter(this);
    installEventFilter(this);
}

bool FullBackgroundWidget::eventFilter(QObject *obj, QEvent *event)
{
    static bool isFirstActive = true;

    if (!QX11Info::isPlatformX11()) {
        if (event->type() == QEvent::MouseButtonPress
            || event->type() == QEvent::MouseMove /* || event->type() == 7*/) {
            if (screenStatus & SCREEN_SAVER && !m_isBlank) {
                onClearScreensaver();
            }
        }
    }

    if (event->type() == QEvent::WindowDeactivate) {
        QTimer::singleShot(50, this, SLOT(laterActivate()));
    } else if (event->type() == QEvent::WindowActivate) {
        if (isFirstActive) {
            isFirstActive = false;
            syslog(LOG_INFO, "[ukui-screensaver-dialog] window active event!!");
        }
        QTimer::singleShot(500, this, SLOT(setLockState()));
        // QTimer::singleShot(200,this,SLOT(killWindow()));
    } else if (event->type() == QEvent::Show || event->type() == QEvent::UpdateRequest) {
        WindowManager::setWindowLayer(this->windowHandle(), WindowLayer::ScreenLock);
        // qDebug()<<"Set window layer ScreenLock";
    }
    return QWidget::eventFilter(obj, event);
}

void FullBackgroundWidget::initConnections()
{
    // 监听全屏窗口变化
    QDesktopWidget *desktop = QApplication::desktop();
    connect(desktop, &QDesktopWidget::resized, this, [&, this, desktop]() {
        qDebug() << "resized:" << desktop->geometry();
        this->onDesktopResized();
    });
    connect(desktop, &QDesktopWidget::workAreaResized, this, [&, this, desktop]() {
        qDebug() << "workAreaResized:" << desktop->geometry();
        this->onDesktopResized();
    });
    connect(desktop, &QDesktopWidget::primaryScreenChanged, this, [&, this, desktop]() {
        qDebug() << "primaryScreenChanged:" << desktop->geometry();
        this->onDesktopResized();
    });
    connect(desktop, &QDesktopWidget::screenCountChanged, this, [&, this, desktop]() {
        qDebug() << "screenCountChanged:" << desktop->geometry();
        this->onDesktopResized();
    });
    // 监听休眠、睡眠、唤醒
    connect(m_modelLockDialog, SIGNAL(prepareForSleep(bool)), this, SLOT(onPrepareForSleep(bool)));
    connect(m_modelLockDialog, &LockDialogModel::currentUserChanged, this, &FullBackgroundWidget::onCurUserChanged);
    connect(m_modelLockDialog, &LockDialogModel::showLock, this, &FullBackgroundWidget::onShowLock);
    connect(
        m_modelLockDialog, &LockDialogModel::showBlankScreensaver, this, &FullBackgroundWidget::onShowBlankScreensaver);
    connect(
        m_modelLockDialog, &LockDialogModel::showLockScreensaver, this, &FullBackgroundWidget::onShowLockScreensaver);
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
}

void FullBackgroundWidget::setLockState()
{
    if (m_bIsLockState == true)
        return;

    m_bIsLockState = true;
    QString displayNum = QString(qgetenv("DISPLAY")).replace(":", "").replace(".", "_");
    QString sessionDbus = QString("%1%2").arg(QString(SS_DBUS_SERVICE)).arg(displayNum);
    QDBusInterface *interface = new QDBusInterface(sessionDbus, SS_DBUS_PATH, SS_DBUS_INTERFACE);
    if (!interface->isValid()) {
        delete interface;
        interface = new QDBusInterface(SS_DBUS_SERVICE, SS_DBUS_PATH, SS_DBUS_INTERFACE);
    }
    QDBusMessage msg = interface->call("SetLockState");
    if (msg.type() == QDBusMessage::ErrorMessage)
        qDebug() << msg.errorMessage();
    interface->deleteLater();
}

void FullBackgroundWidget::onSecondRunParam(const QString &str)
{
    qDebug() << "onSecondRunParam" << str;
    if (str == "SleepLock") {
        onShowBlankScreensaver(0, true);
    } else if (str == "ScreensaverLock") {
        onShowLockScreensaver();
    } else if (str == "StartupLock") {
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
    } else if (str == "CmdRestartMulTiUserBlock") {
        onShowMultiUsersBlockWindows(0);
    } else if (str == "CmdPowerOffMulTiUserBlock") {
        onShowMultiUsersBlockWindows(1);
    }
}

void FullBackgroundWidget::onShowBlankScreensaver(int nDelay, bool isHasLock)
{
    qDebug() << "onShowBlankScreensaver:" << nDelay << "," << isHasLock;
    Q_EMIT m_modelLockDialog->lockStateChanged(true);

    if (isHasLock) {
        screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER | SCREEN_LOCK);
    } else {
        screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER);
    }

    for (auto screen : QGuiApplication::screens()) {
        ScreenSaver *saver = m_modelLockDialog->getScreensaver();
        saver->mode = SaverMode(SAVER_BLANK_ONLY);
        ScreenSaverWidget *saverWidget = new ScreenSaverWidget(saver, this);
        widgetXScreensaverList.push_back(saverWidget);
        saverWidget->setGeometry(screen->geometry());
    }

    setCursor(Qt::BlankCursor);
    m_isBlank = true;

    if (0 == nDelay) {
        /*在进行压力测试时，可能会出现锁屏界面启动极慢，导致在睡眠之前调用了锁屏，但
         * 锁屏没来得及绑定睡眠唤醒信号，导致唤醒后，锁屏界面没有收到信号从而一直显示黑屏的问题。
         * 因此这里在进入黑色屏保时，通过后台接口，获取一次当前是否应该显示黑色屏保状态*/
        bool ret = Q_EMIT m_modelLockDialog->GetBlankState();
        if (!ret) {
            m_isBlank = false;
            onClearScreensaver();
        }
    } else {
        QTimer::singleShot(nDelay, this, [=]() {
            if (m_isBlank) {
                m_isBlank = false;
                onClearScreensaver();
            }
        });
    }
    show();
}

void FullBackgroundWidget::onShowLock(bool isStartup)
{
    if (m_isSessionTools) {
        m_isSessionTools = false;
        if (m_lockWidget && !m_lockWidget->isHidden())
            m_lockWidget->exitSubWidget();
    }

    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    m_isStartupMode = isStartup;
    show();
    Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    Q_EMIT m_modelLockDialog->lockStateChanged(true);
    if (m_lockWidget && !m_lockWidget->isHidden())
        m_lockWidget->show();
}

void FullBackgroundWidget::onShowSwitchUserLock()
{
    if (m_isSessionTools) {
        m_isSessionTools = false;
        if (m_lockWidget && !m_lockWidget->isHidden())
            m_lockWidget->exitSubWidget();
    }

    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    show();
    Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    Q_EMIT m_modelLockDialog->lockStateChanged(true);
    if (m_lockWidget && !m_lockWidget->isHidden()) {
        m_lockWidget->show();
        m_lockWidget->onShowUserListWidget(true);
    }
}

void FullBackgroundWidget::onShowSessionTools()
{
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    m_isSessionTools = true;
    show();
    Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    Q_EMIT m_modelLockDialog->lockStateChanged(true);
    if (m_lockWidget && !m_lockWidget->isHidden())
        m_lockWidget->onShowPowerListWidget(true);
}

void FullBackgroundWidget::onShowAppBlockWindow(int actionType)
{
    QStringList lockcheck;
    if (actionType == 0 || actionType == 1 || actionType == 4) {
        lockcheck = m_modelLockDialog->getShutdownLockcheck();
    } else if (actionType == 2 || actionType == 3) {
        lockcheck = m_modelLockDialog->getSleepLockcheck();
    }
    if (!lockcheck.isEmpty()) {
        show();
        if (m_lockWidget && !m_lockWidget->isHidden())
            screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
        Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
        Q_EMIT m_modelLockDialog->lockStateChanged(true);
        m_isSessionTools = true;
        if (m_lockWidget && !m_lockWidget->isHidden())
            m_lockWidget->onShowInhibitWarning(lockcheck, actionType, true);
    }
}

void FullBackgroundWidget::onShowMultiUsersBlockWindows(int actionType)
{
    screenStatus = (ScreenStatus)(screenStatus | SCREEN_LOCK);
    m_isSessionTools = true;
    show();
    if (m_lockWidget && !m_lockWidget->isHidden())
        Q_EMIT m_modelLockDialog->setCurrentUser(m_modelLockDialog->defaultUserName());
    Q_EMIT m_modelLockDialog->lockStateChanged(true);
    if (m_lockWidget && !m_lockWidget->isHidden())
        m_lockWidget->onMulUsersLogined(actionType, true);
}

void FullBackgroundWidget::onShowSessionIdle()
{
    onShowScreensaver();
    delayLockScreen();
    Q_EMIT m_modelLockDialog->lockStateChanged(true);
}

void FullBackgroundWidget::onShowLockScreensaver()
{
    onShowLock(false);
    onShowScreensaver();
}

void FullBackgroundWidget::onShowScreensaver()
{
    if (m_isSessionTools) {
        m_isSessionTools = false;
        if (m_lockWidget && !m_lockWidget->isHidden())
            m_lockWidget->exitSubWidget(true);
    }

    screenStatus = (ScreenStatus)(screenStatus | SCREEN_SAVER);

    for (auto screen : QGuiApplication::screens()) {
        ScreenSaver *saver = m_modelLockDialog->getScreensaver();
        /*锁屏设置的Qt::WA_TranslucentBackground属性会导致第三方屏保变得透明，因此在使用第三方屏保时
         * 取消该属性，清除屏保时再设置回来*/
        if (saver->path != "/usr/lib/ukui-screensaver/ukui-screensaver-default") {
            //	    setAttribute(Qt::WA_TranslucentBackground,false);
        }

        ScreenSaverWidget *saverWidget = new ScreenSaverWidget(saver, this);
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
    Q_EMIT m_modelLockDialog->lockStateChanged(true);
}

void FullBackgroundWidget::onClearScreensaver()
{
    screenStatus = (ScreenStatus)(screenStatus & ~SCREEN_SAVER);

    for (auto widget : widgetXScreensaverList) {
        widget->close();
    }
    widgetXScreensaverList.clear();

    unsetCursor();

    if (screenStatus == UNDEFINED) {
        onCloseScreensaver();
    } else {
        onShowLock(false);
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
    hide();
    Q_EMIT m_modelLockDialog->lockStateChanged(false);
    stopDelayLockScreen();
    screenStatus = UNDEFINED;
    m_isBlank = false;
}

void FullBackgroundWidget::onDesktopResized()
{
    QDesktopWidget *desktop = QApplication::desktop();
    qInfo() << "[FullBackgroundWidget] [onDesktopResized]:" << desktop->geometry();
    if (desktop->geometry().width() > m_defaultGeometry.width()
        || desktop->geometry().height() > m_defaultGeometry.height()) {
        setGeometry(desktop->geometry());
    } else {
        setGeometry(m_defaultGeometry);
    }
    if (m_lockWidget) {
        moveToPrimaryScreen();
        m_lockWidget->reloadRootBackground();
    }
    update();
}

void FullBackgroundWidget::onPrepareForSleep(bool sleep)
{
    /// 系统休眠时，会关闭总线，导致设备不可用，发生错误
    /// 在系统休眠之前停止认证，在系统唤醒后重新开始认证
    qDebug() << "onPrepareForSleep:" << sleep;

    if (!isVisible()) {
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

void FullBackgroundWidget::moveToPrimaryScreen()
{
    if (!m_lockWidget) {
        return;
    }
    bool chkInOneScreen = false;
    for (auto screen : QGuiApplication::screens()) {
        if (screen == qApp->primaryScreen()) {
            qInfo() << "LockWidget:" << screen->geometry() << m_lockWidget->geometry() << geometry();
            if (!m_lockWidget->isHidden()) {
                m_lockWidget->hide();
                m_lockWidget->setGeometry(screen->geometry());
                m_lockWidget->show();
            } else {
                m_lockWidget->setGeometry(screen->geometry());
            }
            chkInOneScreen = true;
            break;
        }
    }
    if (!chkInOneScreen) {
        for (auto screen : QGuiApplication::screens()) {
            if (screen) {
                /*避免切换时闪烁*/
                qInfo() << "LockWidget:" << screen->geometry() << m_lockWidget->geometry() << "," << screen
                        << geometry();
                if (!m_lockWidget->isHidden()) {
                    m_lockWidget->hide();
                    m_lockWidget->setGeometry(screen->geometry());
                    m_lockWidget->show();
                } else {
                    m_lockWidget->setGeometry(screen->geometry());
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
    if (!QX11Info::isPlatformX11())
        return;
    XSetInputFocus(QX11Info::display(), this->winId(), RevertToParent, CurrentTime);
    activateWindow();
}

void FullBackgroundWidget::showEvent(QShowEvent *event)
{
    if (QX11Info::isPlatformX11())
        tryGrabKeyboard();
    QTimer::singleShot(100, this, [=]() {
        m_lockWidget->updateFont();
        m_lockWidget->updateFontSize();
    });
}

void FullBackgroundWidget::keyReleaseEvent(QKeyEvent *e)
{
    if (!QX11Info::isPlatformX11()) {
        if (e->key() == Qt::Key_Escape && screenStatus == SCREEN_LOCK) { // "escape"
            bool canShow = true;
            if (m_lockWidget && !m_lockWidget->exitSubWidget())
                canShow = false;
            if (canShow)
                onShowScreensaver();
        } else if (screenStatus & SCREEN_SAVER /* && !isBlank*/) {
            onClearScreensaver();
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
        qDebug() << "---------------------XCB_BUTTON_PRESS:" << x << "," << y;
    } else if (responseType == XCB_BUTTON_RELEASE) {

    } else if (responseType == XCB_MOTION_NOTIFY) {
        xcb_motion_notify_event_t *xc = reinterpret_cast<xcb_motion_notify_event_t *>(event);
        int x = xc->root_x;
        int y = xc->root_y;
        onGlobalButtonDrag(x, y);
        qDebug() << "---------------------XCB_MOTION_NOTIFY:" << x << "," << y;
    } else if (responseType == XCB_KEY_PRESS) {
        xcb_key_press_event_t *xc = reinterpret_cast<xcb_key_press_event_t *>(event);
        // qDebug()<<"---------------------XCB_KEY_PRESS:"<<xc->detail;
        onGlobalKeyPress(xc->detail);
    } else if (responseType == XCB_KEY_RELEASE) {
        xcb_key_release_event_t *xc = reinterpret_cast<xcb_key_release_event_t *>(event);
        qDebug() << "---------------------XCB_KEY_RELEASE:" << xc->detail;
        onGlobalKeyRelease(xc->detail);
    } else if (responseType == m_RREventBase + RRScreenChangeNotify) {
        if (isGreeterMode() && QString(qgetenv("XDG_SESSION_TYPE")) != "wayland") {
            RRScreenChangeEvent(false);
        }
    }
    return false;
}

void FullBackgroundWidget::onCurUserChanged(const QString &strUserName)
{
    if (isGreeterMode())
        DisplayService::instance(m_modelLockDialog)->setCurUserName(strUserName);
}

void FullBackgroundWidget::onAuthSucceed(QString strUserName)
{
    if (getenv("USER") == strUserName) {
        onCloseScreensaver();
    } else {
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
        bool canShow = true;
        if (m_lockWidget && !m_lockWidget->exitSubWidget())
            canShow = false;
        if (canShow)
            onShowScreensaver();
    } else if (screenStatus & SCREEN_SAVER /* && !isBlank*/) {
        qDebug() << "clearclear";
        onClearScreensaver();
    }
}

void FullBackgroundWidget::onGlobalButtonDrag(int xPos, int yPos)
{
    if (screenStatus & SCREEN_SAVER && !m_isBlank) {
        qDebug() << "clearclear";
        onClearScreensaver();
    }
}

void FullBackgroundWidget::onGlobalButtonPressed(int xPos, int yPos)
{
    if (screenStatus & SCREEN_SAVER && !m_isBlank) {
        onClearScreensaver();
    }
}

//////////////////////////////////
void FullBackgroundWidget::initCurrentBackground()
{
    loadingAllUserBackground();
    m_backgrondGradationTimer = new QTimer(this);
    connect(m_backgrondGradationTimer, &QTimer::timeout, this, &FullBackgroundWidget::onTransition);

    connect(m_modelLockDialog, &LockDialogModel::setCurrentUser, this, &FullBackgroundWidget::onCurrentUserBackground);
    connect(
        m_modelLockDialog, &LockDialogModel::currentUserChanged, this, &FullBackgroundWidget::onUpdateUserBackground);

    //    update();
}

QString FullBackgroundWidget::getDefaultBackgroundPath()
{
    qDebug() << __LINE__ << __FUNCTION__;

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

void FullBackgroundWidget::loadingAllUserBackground()
{
    qWarning() << __LINE__ << __FUNCTION__;
    if (!getDefaultBackgroundPath().isEmpty())
        addBackgroundData(getDefaultBackgroundPath());
}

void FullBackgroundWidget::paintEvent(QPaintEvent *event)
{
    // 全屏黑色打底
    QPainter paintGlobal(this);
    QColor color = "#000000";
    paintGlobal.setBrush(color);
    paintGlobal.drawRect(geometry());

    for (QScreen *screen : QApplication::screens()) {
        // 在每个屏幕上绘制背景
        QRect rect = screen->geometry();
        int width = rect.width();
        int height = rect.height();
        QString currentPath = getUserBackgroundPath(m_currentUserName);
        QString resolution = QString("%1x%2").arg(width).arg(height);
        QPair<QString, QString> cunrrentKey(currentPath, resolution);

        QString oldPath = getUserBackgroundPath(m_oldUserName);
        QPair<QString, QString> oldKey(oldPath, resolution);
        if (m_gradualChanging && m_allBackgroundsDataMap.contains(cunrrentKey)) {
            drawBackground(
                m_allBackgroundsDataMap[cunrrentKey],
                m_allBackgroundsDataMap.contains(oldKey) ? m_allBackgroundsDataMap[oldKey] : nullptr,
                rect,
                m_backgroundAlpha);
        } else {
            stopTransition();
            if (m_allBackgroundsDataMap.contains(cunrrentKey)) {
                drawBackground(m_allBackgroundsDataMap[cunrrentKey], m_allBackgroundsDataMap[cunrrentKey], rect);
            } else {
                QPainter painter(this);
                QPixmap *pixmapBg = getBackground(currentPath, rect);
                if (!pixmapBg) {
                    pixmapBg = getBackground(getDefaultBackgroundPath(), rect);
                }
                if (pixmapBg && !pixmapBg->isNull()) {
                    painter.drawPixmap(rect, *pixmapBg);
                }
            }
        }
    }
    return QWidget::paintEvent(event);
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
    qDebug() << __LINE__ << __FUNCTION__ << "==========" << strUserName;
    if (m_currentUserName == strUserName)
        return;
    m_oldUserName = m_currentUserName;
    m_currentUserName = strUserName;
    // 是否需要开启渐变效果
    bool gradualChangeEnable = isOpenGradation();
    qDebug() << __LINE__ << __FUNCTION__ << "==========" << gradualChangeEnable;

    if (gradualChangeEnable)
        startTransition();
    repaint();
}

void FullBackgroundWidget::onTransition()
{
    m_backgroundAlpha += 0.05;

    if (m_backgroundAlpha >= 1.0)
        stopTransition();
    repaint();
}

void FullBackgroundWidget::startTransition()
{
    if (!m_backgrondGradationTimer) {
        m_backgrondGradationTimer = new QTimer(this);
        connect(m_backgrondGradationTimer, &QTimer::timeout, this, &FullBackgroundWidget::onTransition);
    }
    stopTransition();

    m_backgroundAlpha = 0.0;
    m_gradualChanging = true;

    m_backgrondGradationTimer->start(10);
}

void FullBackgroundWidget::stopTransition()
{
    if (m_backgrondGradationTimer && m_backgrondGradationTimer->isActive())
        m_backgrondGradationTimer->stop();
    m_backgroundAlpha = 1.0;
    m_gradualChanging = false;
}

void FullBackgroundWidget::drawBackground(
    QPixmap *backgroundBack, QPixmap *backgroundFront, const QRect &rect, float alpha)
{
    if (!backgroundBack || backgroundBack->isNull()) {
        stopTransition();
    }

    QPainter painter(this);
    if (!backgroundBack || backgroundBack->isNull() || !backgroundFront) {
        painter.setOpacity(alpha);
        QString color = Configuration::instance(this)->getValue("background-color").toString();
        QColor cor;
        if (!color.isEmpty())
            cor = color;
        else
            cor = "#035290";
        painter.setBrush(cor);
        painter.drawRect(rect);
    } else {
        painter.drawPixmap(rect, *backgroundFront);
        painter.setOpacity(alpha);
        painter.drawPixmap(rect, *backgroundBack);
        QPainterPath path;
        QPainter painter1(this);
        painter1.setOpacity(0.25);
        painter1.setRenderHint(QPainter::Antialiasing); // 反锯齿;
        painter1.setClipping(true);
        painter1.setPen(Qt::transparent);
        path.addRect(this->rect());
        path.setFillRule(Qt::WindingFill);
        painter1.setBrush(QColor("#000000"));
        painter1.setPen(Qt::transparent);
        painter1.drawPath(path);
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
    for (QScreen *screen : QApplication::screens()) {
        int width = screen->geometry().width();
        int height = screen->geometry().height();

        QString resolution = QString("%1x%2").arg(width).arg(height);
        QPair<QString, QString> key(bgPath, resolution);
        if (!m_allBackgroundsDataMap.contains(key)) {
            QPixmap *pixmap = new QPixmap(scaleBlurPixmap(width, height, bgPath));
            m_allBackgroundsDataMap[key] = pixmap;
        }
    }
}

void FullBackgroundWidget::onAddUserBackground(const QString &strUserName)
{
    m_loadingOneBackgroundFuture = QtConcurrent::run([=]() {
        QString strBackground = getUserBackgroundPath(strUserName);
        addBackgroundData(strBackground);
    });
    m_loadingOneBackgroundFuture.waitForFinished();
}

void FullBackgroundWidget::onUserBackgroundChanged(const QString &strUserName) {}

void FullBackgroundWidget::onCurrentUserBackground(const QString &strUserName)
{
    qDebug() << __LINE__ << __FUNCTION__ << "===============" << strUserName;
    onAddUserBackground(strUserName);
    repaint();
}

QPixmap *FullBackgroundWidget::getBackground(const QString &path, const QRect &rect)
{
    if (path.isEmpty() || !QFile(path).exists())
        return nullptr;

    QString resolution = QString("%1x%2").arg(rect.width()).arg(rect.height());
    QPair<QString, QString> key(path, resolution);

    if (m_allBackgroundsDataMap.isEmpty() && m_loadingOneBackgroundFuture.isRunning()) {
        m_loadingOneBackgroundFuture.waitForFinished();
    } else {
        if (!m_loadingOneBackgroundFuture.isFinished() && m_loadingOneBackgroundFuture.isStarted()) {
            m_loadingOneBackgroundFuture.waitForFinished();
        }
    }
    if (!m_allBackgroundsDataMap.contains(key)) {
        QPixmap *pixmap = new QPixmap(scaleBlurPixmap(width(), height(), path));
        m_allBackgroundsDataMap[key] = pixmap;
    }
    return m_allBackgroundsDataMap[key];
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
    QTimer::singleShot(1000, this, SLOT(onDesktopResized()));
}

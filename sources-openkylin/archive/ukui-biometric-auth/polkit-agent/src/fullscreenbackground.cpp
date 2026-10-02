/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
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
#include "fullscreenbackground.h"
#include "mainwindow.h"
#include <unistd.h>
#include <QDebug>
#include <QImageReader>
#include <QPixmap>
#include <QApplication>
#include <QScreen>
#include <QCursor>
#include <QTimer>
#include <QDesktopWidget>
#include <QPainter>
#include <QPainterPath>
#include <KWindowSystem>
#include <windowmanager/windowmanager.h>

using namespace kdk;

#define PERSONALSIE_SCHEMA "org.mate.background"
#define PERSONALSIE_TRAN_KEY "picture-filename"

#define USD_MEDIA_KEYS_SCHEMA "org.ukui.SettingsDaemon.plugins.media-keys"
#define USD_MEDIA_KEYS_WINDOW_SWTICH_KEY "ukuiWindowSwitch"
#define USD_MEDIA_KEYS_WINDOW_SWTICH2_KEY "ukuiWindowSwitch2"

FullScreenBackground::FullScreenBackground(QWidget *parent) : QWidget(parent)
{
    m_isWayland = QString(qgetenv("XDG_SESSION_TYPE")) == "wayland";
    initUI();
    initConnections();
}

FullScreenBackground::~FullScreenBackground()
{
    closeDialog();
}

void FullScreenBackground::initMainWindow(
    const QString &strIconName,
    const QString &strMsg,
    const QStringList &listUsers,
    const QString &subjectPid,
    const QString &callerPid,
    const PolkitQt1::ActionDescription &actionDesc,
    const bool &isMavis,
    const bool &isSupportTableMode)
{
    if (!m_wndMain) {
        if (m_isWayland) {
            m_wndMain = new MainWindow();
            m_wndMain->setWindowFlags(windowFlags() | Qt::Tool | Qt::WindowStaysOnTopHint);
            KWindowSystem::setState(m_wndMain->winId(), NET::State::KeepAbove | NET::State::SkipTaskbar);
        } else {
            m_wndMain = new MainWindow(this);
            m_wndMain->setModal(true);
        }
        setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint & ~Qt::WindowMinimizeButtonHint);
        m_wndMain->setWindowTitle(tr("Authentication"));
        WindowManager::setSkipTaskBar(m_wndMain->windowHandle(), true);
        m_wndMain->setIcon(strIconName);
        m_wndMain->setCurProject(isMavis);
        m_wndMain->setHeader(strMsg);
        m_wndMain->setUsers(listUsers);
        m_wndMain->setEditInputMethod(isSupportTableMode);
        m_wndMain->setFixedSize(420, 337);
        /*
         *
        mainWindow->setDetails(subjectPid, callerPid,
                               actionDesc.actionId(),
                               actionDesc.description(),
                               actionDesc.vendorName(),
                               actionDesc.vendorUrl());
        */
        connect(m_wndMain, &MainWindow::finished, this, [=](int nResult) {
            qDebug() << "auth dialog finished:" << nResult;
        });
    }
    m_windowHelper = new UkuiWindowHelper(m_wndMain);
    m_windowHelper->setWindowRole(UkuiWindowHelper::WindowRole::Authentication);
    m_windowHelper->grabKeyboard();

    if (m_isWayland) {
        hide();
        connect(m_windowHelper, &UkuiWindowHelper::currentScreenChanged, this, &FullScreenBackground::onCurrentScreenChanged);
        QTimer::singleShot(0, this, [&, this]() { m_windowHelper->requestGetCurrentScreen();});
    } else {
        show();
    }
}

void FullScreenBackground::initUI()
{
    setWindowTitle(tr("Authentication"));
    if (m_isWayland) {
        setAttribute(Qt::WA_TranslucentBackground);
        setGeometry(0, 0, 0, 0);
    } else {
        setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
        setWindowOpacity(0.25);
        KWindowSystem::setState(winId(), NET::State::KeepAbove | NET::State::SkipTaskbar);
        WindowManager::setSkipTaskBar(this->windowHandle(), true);
        installEventFilter(this);

        if (!isSupportCompositing()) {
            m_pixmapBackground = getBackground();
        }
        setGeometry(0, 0, 7680, 4320);
    }
    qDebug() << "Window geometry:" << geometry();
}

void FullScreenBackground::initConnections()
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (desktop) {
        connect(desktop, &QDesktopWidget::resized, this, &FullScreenBackground::onDesktopResized);
        connect(desktop, &QDesktopWidget::workAreaResized, this, &FullScreenBackground::onWorkAreaResized);
        connect(desktop, &QDesktopWidget::primaryScreenChanged, this, &FullScreenBackground::onPrimaryScreenChanged);
        connect(desktop, &QDesktopWidget::screenCountChanged, this, &FullScreenBackground::onScreenCountChanged);
    }
}

void FullScreenBackground::onScreenCountChanged(int)
{
    qDebug() << "onScreenCountChanged----";
    if (m_isWayland) {
        m_windowHelper->requestGetCurrentScreen();
        update();
        QTimer::singleShot(100, this, [&, this]() { moveToCursorScreen();});
        return;
    }
    QDesktopWidget *desktop = QApplication::desktop();
    if ((desktop->geometry().x() != 0 || desktop->geometry().y() != 0) && this->geometry().contains(desktop->geometry())
        && (QApplication::screens().count() == 1)) {
        return;
    }
    if (desktop->geometry().height() > height() || desktop->geometry().width() > width()) {
        setGeometry(
            desktop->geometry().x(),
            desktop->geometry().y(),
            desktop->geometry().width() + 1,
            desktop->geometry().height() + 1);
    }

    update();
    moveToCursorScreen();
    qDebug() << desktop->geometry();
}

void FullScreenBackground::onWorkAreaResized()
{
    qDebug() << "onWorkAreaResized----";
    update();
}

void FullScreenBackground::onPrimaryScreenChanged()
{
    qDebug() << "onPrimaryScreenChanged----";
    if (m_isWayland) {
        m_windowHelper->requestGetCurrentScreen();
    }
    update();
    QTimer::singleShot(100, this, [&, this]() { moveToCursorScreen();});
}

void FullScreenBackground::onDesktopResized()
{
    qDebug() << "onDesktopResized----";
    if (m_isWayland) {
        m_windowHelper->requestGetCurrentScreen();
        update();
        QTimer::singleShot(100, this, [&, this]() { moveToCursorScreen();});
        return;
    }
    QDesktopWidget *desktop = QApplication::desktop();
    if ((desktop->geometry().x() != 0 || desktop->geometry().y() != 0) && this->geometry().contains(desktop->geometry())
        && (QGuiApplication::screens().count() == 1)) {
        return;
    }
    if (desktop->geometry().height() > height() || desktop->geometry().width() > width()) {
        setGeometry(
            desktop->geometry().x(),
            desktop->geometry().y(),
            desktop->geometry().width() + 1,
            desktop->geometry().height() + 1);
    }

    update();
    moveToCursorScreen();
    qDebug() << desktop->geometry();
}

void FullScreenBackground::showDialog()
{
    if (m_isFirstShow) {
        m_isFirstShow = false;
        blockSomeShortCut(true);
        if (!m_isWayland) {
            show();
        }
        if (m_wndMain) {
            qDebug() << "DialogGeometry:" << m_wndMain->geometry();
            m_wndMain->show();
            m_wndMain->activateWindow();
            // move auth dialog to cursor screen
            moveToCursorScreen();
        }
    }
}

void FullScreenBackground::closeDialog()
{
    blockSomeShortCut(false);
    if (m_wndMain) {
        m_wndMain->hide();
        m_wndMain->stopDoubleAuth();
        m_wndMain->close();
        m_wndMain->deleteLater();
        m_wndMain = nullptr;
    }
}

bool FullScreenBackground::isSupportCompositing()
{
    if (QString(qgetenv("XDG_SESSION_TYPE")) == "wayland") {
        return true;
    }
    QDBusInterface kwinCompositor(
        "org.ukui.KWin", "/Compositor", "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus());
    QDBusReply<QDBusVariant> sessionReply = kwinCompositor.call("Get", "org.kde.kwin.Compositing", "compositingType");
    if (!sessionReply.isValid()) {
        qWarning() << sessionReply.error();
    } else {
        QString strType = sessionReply.value().variant().toString();
        if (strType.isEmpty() || strType == "none")
            return false;
    }
    return true;
}

QPixmap FullScreenBackground::getBackground()
{
    QPixmap pixmapBackground;
    QString strPicName = "";

    // 获取桌面壁纸路径
    QGSettings *personalGSettings = nullptr;
    if (QGSettings::isSchemaInstalled(PERSONALSIE_SCHEMA)) {
        personalGSettings = new QGSettings(PERSONALSIE_SCHEMA, QByteArray(), this);
        QStringList keys = personalGSettings->keys();
        if (keys.contains(PERSONALSIE_TRAN_KEY)) {
            strPicName = personalGSettings->get(PERSONALSIE_TRAN_KEY).toString();
        }
        delete personalGSettings;
        personalGSettings = nullptr;
    }

    // 文件存在且可读则加载图片
    QFileInfo fileInfo(strPicName);
    if (fileInfo.exists() && fileInfo.isReadable()) {
        // do nothing
    } else {
        strPicName = getAccountBackground();
    }

    if (!strPicName.isEmpty()) {
        QImageReader reader;
        reader.setFileName(strPicName);
        reader.setAutoTransform(true);
        reader.setDecideFormatFromContent(true);
        pixmapBackground = QPixmap::fromImageReader(&reader);
    }

    return pixmapBackground;
}

QString FullScreenBackground::getAccountBackground()
{
    QString strTmpPicName = "/usr/share/backgrounds/ubuntukylin-default-settings.jpg";

    uid_t uid = getuid();
    QDBusInterface iface(
        "org.freedesktop.Accounts",
        "/org/freedesktop/Accounts",
        "org.freedesktop.Accounts",
        QDBusConnection::systemBus());

    QDBusReply<QDBusObjectPath> userPath = iface.call("FindUserById", (qint64)uid);
    if (!userPath.isValid()) {
        return strTmpPicName;
    } else {
        QDBusInterface userIface(
            "org.freedesktop.Accounts",
            userPath.value().path(),
            "org.freedesktop.DBus.Properties",
            QDBusConnection::systemBus());
        QDBusReply<QDBusVariant> backgroundReply
            = userIface.call("Get", "org.freedesktop.Accounts.User", "BackgroundFile");
        if (backgroundReply.isValid()) {
            QString strPicName = backgroundReply.value().variant().toString();
            if (!strPicName.isEmpty()) {
                QFileInfo fileInfo(strPicName);
                if (fileInfo.exists() && fileInfo.isReadable()) {
                    return strPicName;
                }
            }
        }
    }

    return strTmpPicName;
}

void FullScreenBackground::moveToScreen(QScreen *screen)
{
    // 将主全屏更新到新屏幕上，并重新将对话框居中
    if (!screen) {
        return;
    }

    if (m_wndMain) {
        QRect rectParent = screen->geometry();
        QRect rectDes = QRect(
            rectParent.x() + (rectParent.width() - m_wndMain->width()) / 2,
            rectParent.y() + (rectParent.height() - m_wndMain->height()) / 2,
            m_wndMain->width(),
            m_wndMain->height());

        m_wndMain->setGeometry(rectDes);
        qDebug() << "new rect:" << rectParent << "," << rectDes << "," << m_wndMain->geometry();
    }
}

void FullScreenBackground::moveToCursorScreen()
{
    bool isFoundScreen = false;
    if (m_isWayland) {
        QString strCurScreen = WindowManager::currentOutputName();
        if(!m_curScreenName.isEmpty()){
            strCurScreen = m_curScreenName; 
        }

        for (auto screen : QApplication::screens()) {
            qDebug() << "Screen:" << screen->name() << "," << screen->geometry();
            if (screen && screen->name() == strCurScreen) {
                isFoundScreen = true;
                moveToScreen(screen);
                break;
            }
        }
    } else {
        QPoint cursor = QCursor::pos();
        for (auto screen : QApplication::screens()) {
            qDebug() << "Screen:" << screen->name() << "," << screen->geometry();
            if (screen && screen->geometry().contains(cursor)) {
                isFoundScreen = true;
                moveToScreen(screen);
                break;
            }
        }
    }
    if (!isFoundScreen) {
        if (QApplication::primaryScreen()) {
            moveToScreen(QApplication::primaryScreen());
        }
        isFoundScreen = true;
    }
}

void FullScreenBackground::paintEvent(QPaintEvent *event)
{
    if (!m_isWayland) {
        for (auto screen : QApplication::screens()) {
            QPainter painter(this);
            if (!m_pixmapBackground.isNull()) {
                painter.drawPixmap(screen->geometry(), m_pixmapBackground);
            }
        }
    }
    return QWidget::paintEvent(event);
}

void FullScreenBackground::closeEvent(QCloseEvent *event)
{
    qDebug() << "FullScreenBackground::closeEvent";
    if (!m_isWayland) {
        // 断开屏幕变化信号
        QDesktopWidget *desktop = QApplication::desktop();
        disconnect(desktop, &QDesktopWidget::resized, this, &FullScreenBackground::onDesktopResized);
        disconnect(desktop, &QDesktopWidget::workAreaResized, this, &FullScreenBackground::onWorkAreaResized);
        disconnect(desktop, &QDesktopWidget::primaryScreenChanged, this, &FullScreenBackground::onPrimaryScreenChanged);
        disconnect(desktop, &QDesktopWidget::screenCountChanged, this, &FullScreenBackground::onScreenCountChanged);
    }

    return QWidget::closeEvent(event);
}

void FullScreenBackground::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    qDebug() << "WndSize:" << this->size();
}

void FullScreenBackground::blockSomeShortCut(bool isBlock /* = false*/)
{
    static bool sIsBlocked = false;
    if (sIsBlocked == isBlock) {
        return;
    }
    QStringList listBlockShortCut = {
        "Minimize All Window",                                               // 最小化所有窗口
        "Minimize Or Unminimize All Window Besides Active Window",           // 最小化或还原非焦点窗口
        "Multitask View show",                                               // 显示工作区
        "Show/Hide Desktop",                                                 // 显示/隐藏桌面
        "ShowDesktopGrid",                                                   // Show Desktop Grid
        "ShowMultitaskView",                                                 // Show Multitask View
        "Switch One Desktop Down",                                           // 切换到下面的桌面
        "Switch One Desktop Up",                                             // 切换到上面的桌面
        "Switch One Desktop to the Left",                                    // 切换到左边的工作区
        "Switch One Desktop to the Right",                                   // 切换到右边的工作区
        "Switch to Desktop 1",                                               // 切换到桌面 1
        "Switch to Desktop 2",                                               // 切换到桌面 2
        "Switch to Desktop 3",                                               // 切换到桌面 3
        "Switch to Desktop 4",                                               // 切换到桌面 4
        "Switch to Next Screen",                                             // 切换到下一屏幕
        "Switch to Previous Screen",                                         // 切换到上一个屏幕
        "Toggle Window Raise/Lower",                                         // 窗口置前或置后
        "Unminimize All Window",                                             // 还原所有最小化窗口
        "Walk Through Desktop List",                                         // 遍历桌面列表
        "Walk Through Desktop List (Reverse)",                               // 遍历桌面列表(反向)
        "Walk Through Desktops",                                             // 遍历桌面
        "Walk Through Desktops (Reverse)",                                   // 遍历桌面(反向)
        "Walk Through Windows",                                              // 遍历窗口
        "Walk Through Windows (Reverse)",                                    // 遍历窗口(反向)
        "Walk Through Windows Alternative",                                  // 遍历窗口候选
        "Walk Through Windows Alternative (Reverse)",                        // 遍历窗口候选(反向)
        "Walk Through Windows of Current Application",                       // 遍历当前应用程序窗口
        "Walk Through Windows of Current Application (Reverse)",             // 遍历当前应用程序窗口(反向)
        "Walk Through Windows of Current Application Alternative",           // 遍历当前应用程序窗口候选
        "Walk Through Windows of Current Application Alternative (Reverse)", // 遍历当前应用程序窗口候选(反向)
        "Walk Through fullscreenWindows",                                    // 遍历所有最大化或全屏窗口
        "Window Lower",                                                      // 降低窗口
        "Window Maximize",                                                   // 最大化窗口
        "Window Maximize Horizontal",                                        // 水平最大化窗口
        "Window Maximize Vertical",                                          // 垂直最大化窗口
        "Window Minimize",                                                   // 最小化窗口
        "Window On All Desktops",                                            // 将窗口放到全部桌面
        "Window One Desktop Down",                                           // 窗口下移一个桌面
        "Window One Desktop Up",                                             // 窗口上移一个桌面
        "Window One Desktop to the Left",                                    // 窗口左移一个桌面
        "Window One Desktop to the Right",                                   // 窗口右移一个桌面
        "Window Operations Menu",                                            // 窗口操作菜单
        "Window Quick Move And Maximize Left Screen",  // 窗口快速移动并最大化到下一个屏幕
        "Window Quick Move And Maximize Right Screen", // 窗口快速移动并最大化到上一个屏幕
        "Window Quick Move Left Screen",               // 窗口快速移动到下一个屏幕
        "Window Quick Move Right Screen",              // 窗口快速移动到上一个屏幕
        "Window to Desktop 1",                         // 窗口到桌面 1
        "Window to Desktop 2",                         // 窗口到桌面 2
        "Window to Desktop 3",                         // 窗口到桌面 3
        "Window to Desktop 4",                         // 窗口到桌面 4
        "Window to Next Desktop",                      // 窗口到下一桌面
        "Window to Next Screen",                       // 窗口到下一屏幕
        "Window to Previous Desktop",                  // 窗口到前一桌面
        "Window to Previous Screen",                   // 窗口到前一屏幕
        "Window to Screen 0",                          // 窗口到屏幕 0
        "Window to Screen 1",                          // 窗口到屏幕 1
        "Window to Screen 2",                          // 窗口到屏幕 2
        "Window to Screen 3",                          // 窗口到屏幕 3
        "Window to Screen 4",                          // 窗口到屏幕 4
    };
    sIsBlocked = isBlock;
    if (sIsBlocked) {
        if (!m_isWayland) {
            // kwin shortcut
            QDBusInterface kwinInterface("org.ukui.KWin", "/KWin", "org.kde.KWin", QDBusConnection::sessionBus());
            for (QString strShortCut : listBlockShortCut) {
                QDBusMessage blockMessage = kwinInterface.call("blockShortcut", strShortCut, true);
                if (blockMessage.type() == QDBusMessage::ErrorMessage) {
                    qWarning() << "Kwin dbus error:" << blockMessage.errorMessage();
                    break;
                }
            }
        }

        //  usd shortcut
        if (!m_usdBlockShortCut) {
            m_usdBlockShortCut = new USDBlockShortCut(this);
        }
    } else {
        if (!m_isWayland) {
            // kwin shortcut
            QDBusInterface kwinInterface("org.ukui.KWin", "/KWin", "org.kde.KWin", QDBusConnection::sessionBus());
            for (QString strShortCut : listBlockShortCut) {
                QDBusMessage blockMessage = kwinInterface.call("blockShortcut", strShortCut, false);
                if (blockMessage.type() == QDBusMessage::ErrorMessage) {
                    qWarning() << "Kwin dbus error1:" << blockMessage.errorMessage();
                    break;
                }
            }
        }

        //  usd shortcut
        if (m_usdBlockShortCut) {
            delete m_usdBlockShortCut;
            m_usdBlockShortCut = nullptr;
        }
    }
}

bool FullScreenBackground::eventFilter(QObject *obj, QEvent *event)
{
    return QWidget::eventFilter(obj, event);
}

void FullScreenBackground::onCurrentScreenChanged(QScreen *screen)
{
    if(screen){
        qDebug()<<"onCurrentScreenChanged "<<screen->geometry();
        m_curScreenName = screen->name();
    }
}

/* Copyright (C) 2020, KylinSoft Co., Ltd.

*
* This file is part of Kylin-Screenshot.
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

#include "controller.h"
#include "src/widgets/capture/capturewidget.h"
#include "src/utils/confighandler.h"
#include "src/config/configwindow.h"
#include "src/widgets/capture/capturebutton.h"
#include "src/utils/systemnotification.h"
#include "src/utils/screengrabber.h"
#include "my_qt.h"
#include "src/utils/systemnotification.h"
#include "src/utils/filenamehandler.h"
#include "kwidget.h"
#include "ukuiwindowhelper/ukuiwindowhelper.h"
#include <dlfcn.h>
#include <cstddef>
#define ICON_THEME_NAME           "iconThemeName"
#define ORG_UKUI_STYLE            "org.ukui.style"
// #include "Logger.h"
#ifdef Q_OS_WIN
#include "src/core/globalshortcutfilter.h"
#endif
typedef void (*Getkdk_dia_upload_program_startupFunc)();
typedef void (*Getkdk_dia_upload_program_exitFunc)();//kdk_dia_upload_program_exit
// Controller is the core component of kylin-screenshot, creates the trayIcon and
// launches the capture widget

Controller::Controller() : m_captureWindow(nullptr)
{
    qApp->setQuitOnLastWindowClosed(false);
    const QByteArray style_id(ORG_UKUI_STYLE);
    if(QGSettings::isSchemaInstalled(style_id)){
        m_styleSettings = new QGSettings(style_id);
    }
    // init tray icon
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (!ConfigHandler().disabledTrayIconValue()) {
        enableTrayIcon();
    }
#elif defined(Q_OS_WIN)
    enableTrayIcon();

    GlobalShortcutFilter *nativeFilter = new GlobalShortcutFilter(this);
    qApp->installNativeEventFilter(nativeFilter);
    connect(nativeFilter, &GlobalShortcutFilter::printPressed,
            this, [this](){
        this->requestCapture(CaptureRequest(CaptureRequest::GRAPHICAL_MODE));
    });
#endif
    isManagerStart = false;
//    QString StyleSheet = CaptureButton::globalStyleSheet();
//    qApp->setStyleSheet(StyleSheet);

    QTimer::singleShot(50, [this]() {
        void *datacollectFd = dlopen("libkysdk-datacollect.so", RTLD_LAZY);;
        if (!datacollectFd) {
            char *err = dlerror();
            qDebug()<<"链接动态库失败：libkysdk-datacollect.so"<< QByteArray(err);
        } else {
            Getkdk_dia_upload_program_startupFunc kdk_dia_upload_program_startup = (Getkdk_dia_upload_program_startupFunc)dlsym(datacollectFd, "kdk_dia_upload_program_startup");
            if (kdk_dia_upload_program_startup == NULL) {
                qDebug()<< "找不到符号:kdk_dia_upload_program_startup";
            } else {
                kdk_dia_upload_program_startup();
            }
            dlclose(datacollectFd);
        }
    });
}

Controller::~Controller(){
    if(m_styleSettings){
        delete m_styleSettings;
    }
    if(kmb){
        delete kmb;
        kmb = nullptr;
    }
}

Controller *Controller::getInstance()
{
    static Controller c;
    return &c;
}

void Controller::enableExports()
{
#ifndef UNUI
    connect(this, &Controller::captureTaken,
            this, &Controller::handleCaptureTaken);
    connect(this, &Controller::captureFailed,
            this, &Controller::handleCaptureFailed);
#endif
}

void Controller::requestCapture(const CaptureRequest &request)
{
#ifndef UNUI
    uint id = request.id();
    m_requestMap.insert(id, request);

    QGSettings *screen = new QGSettings("org.ukui.screenshot");
    QString screenshot = screen->get("screenshot").toString();
    switch (request.captureMode()) {
    case CaptureRequest::FULLSCREEN_MODE:
        if (screenshot.compare("true") == 0) {
            doLater(request.delay(), this, [this, id](){
                this->startFullscreenCapture(id);
            });
        } else {
            disableScreenCut();
        }
        break;
    case CaptureRequest::SCREEN_MODE:
        if (screenshot.compare("true") == 0) {
            int &&number = request.data().toInt();
            doLater(request.delay(), this, [this, id, number](){
                this->startScreenGrab(id, number);
            });
        } else {
            disableScreenCut();
        }
        break;
    case CaptureRequest::GRAPHICAL_MODE:
        if (screenshot.compare("true") == 0) {
            QString &&path = request.path();
            doLater(request.delay(), this, [this, id, path](){
                isManagerStart = false;
                this->startVisualCapture(id, path);
            });
        } else {
            disableScreenCut();
        }
        break;
    case CaptureRequest::OSMANAGER_MODE:
        if (screenshot.compare("true") == 0) {
            isManagerStart = true;
            QString &&path = request.path();
            doLater(request.delay(), this, [this, id, path](){
                this->startVisualCapture(id, path);
            });
        } else {
            disableScreenCut();
        }
        break;
    default:
        emit captureFailed(id);
        break;
    }
#endif
}

// creation of a new capture in GUI mode
void Controller::startVisualCapture(const uint id, const QString &forcedSavePath)
{
#ifdef SUPPORT_UKUI
    QGSettings *screen = new QGSettings("org.ukui.screenshot");
// if (!screen) {
// Logger::LogInfo("[Controller::startVisualCapture] " + Logger::tr("not support org.ukui.screenshot..."));
// exit(1);
// }
    QString screenshot = screen->get("screenshot").toString();
#endif
    if (!m_captureWindow) {
#ifdef SUPPORT_UKUI
        if (screenshot.compare("true") == 0) {
#endif
        QWidget *modalWidget = nullptr;
        do {
            modalWidget = qApp->activeModalWidget();
            if (modalWidget) {
                modalWidget->close();
                modalWidget->deleteLater();
            }
        } while (modalWidget);

        m_captureWindow = new CaptureWidget(id, forcedSavePath);
        QTimer::singleShot(100,this,[=](){
            if (m_onScreenLock)
            {
//                kdk::WindowManager::setWindowLayer(m_captureWindow->windowHandle(),kdk::WindowLayer::OnScreenDisplay);
            }
            else
            {
//                kdk::WindowManager::setWindowLayer(m_captureWindow->windowHandle(),kdk::WindowLayer::SystemWindow);
            }

        });
        // m_captureWindow = new CaptureWidget(id, forcedSavePath, false); // debug
        connect(m_captureWindow, &CaptureWidget::captureFailed,
                this, &Controller::captureFailed);
        connect(m_captureWindow, &CaptureWidget::captureTaken,
                this, &Controller::captureTaken);
        connect(m_captureWindow, &CaptureWidget::captureExit,
                this, &Controller::captureExit);
        connect(m_captureWindow, &CaptureWidget::captureCopy,
                this, &Controller::captureCopy);
#ifdef SUPPORT_UKUI
    } else {
        disconnect(m_captureWindow, &CaptureWidget::captureFailed,
                   this, &Controller::captureFailed);
        disconnect(m_captureWindow, &CaptureWidget::captureTaken,
                   this, &Controller::captureTaken);
        disableScreenCut();
    }
#endif
#ifdef Q_OS_WIN
        m_captureWindow->show();
#else
#ifdef SUPPORT_UKUI
        if (screenshot.compare("true") == 0) {
#endif
#if defined(UNUI) || defined(Q_OS_WIN)
            m_captureWindow->showFullScreen();
#else
            // wayland: showFullScreen 限制窗口至当前输出
            // 改用 show() 保持 resize 的虚拟桌面尺寸（1947x2281）
            m_captureWindow->show();
#endif

#ifdef SUPPORT_UKUI
    }
#endif
         //m_captureWindow->show(); // Debug
#endif
    } else {
        emit captureFailed(id);
    }
}

void Controller::startScreenGrab(const uint id, const int screenNumber)
{
    bool ok = true;
    int n = screenNumber;

    if (n < 0) {
        QPoint globalCursorPos = QCursor::pos();
        n = qApp->desktop()->screenNumber(globalCursorPos);
    }
    QPixmap p(ScreenGrabber().grabScreen(n, ok));
    if (ok) {
        emit captureTaken(id, p, true);
    } else {
        emit captureFailed(id);
    }
}

// creation of the configuration window
void Controller::openConfigWindow()
{
    if (!m_configWindow) {
        m_configWindow = new ConfigWindow();
#ifndef UNUI
        m_configWindow->show();
#endif
    }
}

void Controller::openShotCutWindow()
{
    if (m_shortCutWindow) {
        delete m_shortCutWindow;
    }
    if (!m_shortCutWindow) {
        m_shortCutWindow = new ShortCutWidget();
#ifndef UNUI
        if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
        {
            kdk::UkuiStyleHelper::self()->removeHeader(m_shortCutWindow);
        }
        else
        {
            UkuiWindowHelper *windowhelper = new UkuiWindowHelper(m_shortCutWindow);
            windowhelper->removeTitleBar();
        }
        m_shortCutWindow->show();
        if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
        {
            m_shortCutWindow->move(
                        ((qApp->desktop()->screenGeometry().width()-m_shortCutWindow->width())/2),
                        ((qApp->desktop()->screenGeometry().height()-m_shortCutWindow->height())
                         /2));
            kdk::WindowManager::setGeometry(m_shortCutWindow->windowHandle(),m_shortCutWindow->geometry());
        }
        else
        {
            int screenWidth = qApp->desktop()->screenGeometry().width();
            int screenHeight = qApp->desktop()->screenGeometry().height();
            int windowWidth = m_shortCutWindow->width();
            int windowHeight = m_shortCutWindow->height();
            int x = (screenWidth - windowWidth) / 2;
            int y = (screenHeight - windowHeight) / 2;
            m_shortCutWindow->setGeometry(x,y,windowWidth,windowHeight);
        }
#endif
    }
}

// creation of the window of information
void Controller::openInfoWindow()
{
//    if (m_infoWindow) {
//        delete m_infoWindow;
//    }
//    if (!m_infoWindow) {
//        m_infoWindow = new infoWidget();
//        m_infoWindow->move(
//            ((qApp->desktop()->screenGeometry().width()-m_infoWindow->width())/2),
//            ((qApp->desktop()->screenGeometry().height()-m_infoWindow->height())
//             /2));
//        kdk::UkuiStyleHelper::self()->removeHeader(m_infoWindow);
//        kdk::WindowManager::setGeometry(m_infoWindow->windowHandle(),m_infoWindow->geometry());
//        m_infoWindow->show();
//    }
    if (m_aboutdialog) {
        delete m_aboutdialog;
    }
    if (!m_aboutdialog) {
        m_aboutdialog = new AboutDialog();
#ifndef UNUI
        if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
        {
            kdk::UkuiStyleHelper::self()->removeHeader(m_aboutdialog);
        }
        else
        {
            UkuiWindowHelper *windowhelper = new UkuiWindowHelper(m_aboutdialog);
            windowhelper->removeTitleBar();
        }
        m_aboutdialog->show();
        if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
        {
            m_aboutdialog->move(
                        ((qApp->desktop()->geometry().width()-m_aboutdialog->width())/2),
                        ((qApp->desktop()->geometry().height()-m_aboutdialog->height())/2));
            kdk::WindowManager::setGeometry(m_aboutdialog->windowHandle(),m_aboutdialog->geometry());
        }
        else
        {
            QRect screenGeometry = qApp->primaryScreen()->geometry();
            int dialogWidth = m_aboutdialog->width();
            int dialogHeight = m_aboutdialog->height();
            int x = (screenGeometry.width() - dialogWidth) / 2;
            int y = (screenGeometry.height() - dialogHeight) / 2;
            m_aboutdialog->setGeometry(x,y,dialogWidth,dialogHeight);
        }
#endif
    }
}

void Controller::openLauncherWindow()
{
    QGSettings *screen = new QGSettings("org.ukui.screenshot");
    QString screenshot = screen->get("screenshot").toString();
    if (m_launcherWindow) {
        delete m_launcherWindow;
    }
    if (!m_launcherWindow) {
        m_launcherWindow = new CaptureLauncher();
#ifndef UNUI
        if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
        {
            kdk::UkuiStyleHelper::self()->removeHeader(m_launcherWindow);
        }
        else
        {
            UkuiWindowHelper *windowhelper = new UkuiWindowHelper(m_launcherWindow);
            windowhelper->removeTitleBar();
        }

        if (screenshot.compare("true") == 0) {
            m_launcherWindow->show();
            if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
            {
                m_launcherWindow->move(((qApp->desktop()->geometry().width()
                                         -m_launcherWindow->width())/2),
                                       ((qApp->desktop()->geometry().height()
                                         -m_launcherWindow->height())/2));
                kdk::WindowManager::setGeometry(m_launcherWindow->windowHandle(),m_launcherWindow->geometry());
            }
            else
            {
                QScreen *primaryScreen = QGuiApplication::primaryScreen();
                QRect screenGeometry = qApp->primaryScreen()->geometry();
                int windowWidth = m_launcherWindow->width();
                int windowHeight = m_launcherWindow->height();
                int x = (screenGeometry.width() - windowWidth) / 2;
                int y = (screenGeometry.height() - windowHeight) / 2;
                m_launcherWindow->setGeometry(x,y,windowWidth,windowHeight);
            }
        } else
            disableScreenCut();
#endif
    }
}

void Controller::enableTrayIcon()
{
    if (m_trayIcon) {
        return;
    }
    ConfigHandler().setDisabledTrayIcon(false);
    // Start the screenshot from the start menu and add a delay by zhanghanhuan
    // doLater(400, this, [this](){ this->startVisualCapture(); });
    QAction *captureAction = new QAction(tr("&Take Screenshot"), this);
    captureAction->setIcon(QIcon::fromTheme("kylin-screenshot").pixmap(QSize(24,24)));
    captureAction->setObjectName("Kylin-screenshot_QAction_takeScreenshot");
    connect(captureAction, &QAction::triggered, this, [this](){
        // Wait 400 ms to hide the QMenu
        doLater(400, this, [this](){
            isManagerStart=false;
            this->startVisualCapture();
        });
    });
    QAction *launcherAction = new QAction(tr("&Open Screenshot Option"), this);
    launcherAction->setIcon(QIcon::fromTheme("kylin-screenshot").pixmap(QSize(24,24)));
    launcherAction->setObjectName("Kylin-screenshot_QAction_openScreenshotOption");
    connect(launcherAction, &QAction::triggered, this,
            &Controller::openLauncherWindow);
    QAction *configAction = new QAction(tr("&Configuration"), this);
    configAction->setObjectName("Kylin-screenshot_QAction_openConfigurationInfo");
    connect(configAction, &QAction::triggered, this,
            &Controller::openConfigWindow);
    QAction *infoAction = new QAction(tr("&About"), this);
    infoAction->setIcon(QIcon::fromTheme("help-about-symbolic").pixmap(QSize(24,24)));
    infoAction->setObjectName("Kylin-screenshot_QAction_openAboutInfo");
    connect(infoAction, &QAction::triggered, this,
            &Controller::openInfoWindow);
    QAction *quitAction = new QAction(tr("&Close"), this);
    quitAction->setIcon(QIcon::fromTheme("window-close-symbolic").pixmap(QSize(24,24)));
    quitAction->setObjectName("Kylin-screenshot_QAction_quit");
    connect(quitAction, &QAction::triggered, this,
            &Controller::deletePidfile);
    connect(quitAction, &QAction::triggered, qApp,
            &QCoreApplication::quit);

    QAction *shortcutAction = new QAction(tr("&ShortCut"), this);
    shortcutAction->setIcon(QIcon::fromTheme("ukui-shortcut-key-symbolic").pixmap(QSize(24,24)));
    shortcutAction->setObjectName("Kylin-screenshot_QAction_shortCutConfig");
    connect(shortcutAction, &QAction::triggered, this,
            &Controller::openShotCutWindow);
//    if(kmb == nullptr){
//        kmb = new kdk::KMenuButton();
//    }
//    QAction *actionFeedback;
//    actionFeedback = kmb->feedbackAction();
//    actionFeedback->setIcon(QIcon::fromTheme("dialog-question-symbolic").pixmap(QSize(24,24)));
//    actionFeedback->setObjectName("Kylin-photo-viewer_QAction_feedback");

    QMenu *trayIconMenu = new QMenu();
    trayIconMenu->addAction(captureAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(launcherAction);
//     trayIconMenu->addAction(configAction);
    trayIconMenu->addAction(infoAction);
    trayIconMenu->addAction(shortcutAction);
//    trayIconMenu->addAction(actionFeedback);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    m_trayIcon = new QSystemTrayIcon();
    m_trayIcon->setToolTip(tr("Screenshot Shift+PrtSc"));
    m_trayIcon->setContextMenu(trayIconMenu);
    // QIcon trayicon = QIcon("/usr/share/icons/ukui-icon-theme-default/128x128/apps/kylin-screenshot.png");
    m_trayIcon->setIcon(QIcon::fromTheme("kylin-screenshot").pixmap(QSize(24,24)));

    auto trayIconActivated = [this](QSystemTrayIcon::ActivationReason r){
                                 if (r == QSystemTrayIcon::Trigger) {
                                     isManagerStart = false;
                                     startVisualCapture();
                                 }
                             };
    connect(m_trayIcon, &QSystemTrayIcon::activated, this, trayIconActivated);
    m_trayIcon->show();
    if(m_styleSettings != nullptr){
        connect(m_styleSettings,&QGSettings::changed,this,[=](const QString &key) {
            if(key==ICON_THEME_NAME){
                QIcon trayicon = QIcon::fromTheme("kylin-screenshot").pixmap(QSize(24,24));
                captureAction->setIcon(trayicon);
                launcherAction->setIcon(trayicon);
                m_trayIcon->setIcon(trayicon);
            }
        });
    }
}

void Controller::disableTrayIcon()
{
#ifndef UNUI
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (m_trayIcon) {
        m_trayIcon->deleteLater();
    }
    ConfigHandler().setDisabledTrayIcon(true);
#endif
#endif
}

void Controller::sendTrayNotification(
    const QString &text, const QString &title, const int timeout)
{
#ifndef UNUI
    if (m_trayIcon) {
        m_trayIcon->showMessage(title, text, QSystemTrayIcon::Information, timeout);
    }
#endif
}

void Controller::updateConfigComponents()
{
#ifndef UNUI
    if (m_configWindow) {
        m_configWindow->updateChildren();
    }
#endif
}

void Controller::deletePidfile()
{
#ifndef UNUI
     void *datacollectFd = dlopen("libkysdk-datacollect.so", RTLD_LAZY);;
     if (!datacollectFd) {
         char *err = dlerror();
         qDebug()<<"链接动态库失败：libkysdk-datacollect.so"<< QByteArray(err);
     } else {
         Getkdk_dia_upload_program_exitFunc kdk_dia_upload_program_exit = (Getkdk_dia_upload_program_exitFunc)dlsym(datacollectFd, "kdk_dia_upload_program_exit");
         if (kdk_dia_upload_program_exit == NULL) {
             qDebug()<< "找不到符号:kdk_dia_upload_program_exit";
         } else {
             kdk_dia_upload_program_exit();
         }
         dlclose(datacollectFd);
     }
#endif
}

void Controller::startFullscreenCapture(const uint id)
{
    bool ok = true;
    QPixmap p(ScreenGrabber().grabEntireDesktop(ok));
    if (ok) {
        // 立即开始计时发送通知（延时显示，让用户感觉截图变快）
        // 使用图片尺寸估算大小（32位色深 = 4字节/像素）
        qint64 imageSize = static_cast<qint64>(p.width()) * p.height() * 2;
        int notificationDelay;
        if (imageSize < 100 * 1024) {
            notificationDelay = 50;
        } else if (imageSize < 500 * 1024) {
            notificationDelay = 100;
        } else if (imageSize < 1024 * 1024) {
            notificationDelay = 200;
        } else if (imageSize < 2 * 1024 * 1024) {
            notificationDelay = 400;
        } else if (imageSize < 5 * 1024 * 1024) {
            notificationDelay = 600;
        } else {
            notificationDelay = 800;
        }

        // 预先计算本次截图的保存路径，避免多次截图时路径冲突
        // 使用 FileNameHandler 预先生成文件名，确保通知显示的路径与实际保存路径一致
        QGSettings *ScreenshotGsettings = new QGSettings("org.ukui.screenshot");
        QString savePath = ScreenshotGsettings->get("screenshot-path").toString();
        QString saveType = ScreenshotGsettings->get("screenshot-type").toString();

        // 如果配置为空，使用默认值
        if (savePath.isEmpty()) {
            savePath = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).at(0);
        }
        if (!savePath.endsWith('/')) {
            savePath += "/";
        }
        if (saveType.isEmpty()) {
            saveType = ".png";
        }

        // 确保 ScreenshotSaver 使用相同的保存路径
        ScreenshotGsettings->set("screenshot-path", savePath);

        // 预先生成文件名，确保后续保存时使用相同的名字
        QString preGeneratedName = FileNameHandler().parsedPattern();
        ScreenshotGsettings->set("new-screenshot-name", preGeneratedName);

        // 生成完整的保存路径
        QString completePath = savePath + preGeneratedName + saveType;

        QThread *notificationThread = new QThread();
        connect(notificationThread, &QThread::started, [=]() {
            QThread::msleep(notificationDelay);
            QString message = QObject::tr("Capture saved as ") + completePath;
            QUrl fullPath = QUrl::fromLocalFile(completePath);
            QString saveMessage = "<a href=\"" + fullPath.toString() + "\">" + message + "</a>";
            SystemNotification().sendMessage(saveMessage, completePath);
            notificationThread->quit();
        });
        connect(notificationThread, &QThread::finished, notificationThread, &QThread::deleteLater);
        notificationThread->start();

        qDebug() << "[Controller] BEFORE emit captureTaken, path:" << completePath << QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
        emit captureTaken(id, p, false);
    } else {
        emit captureFailed(id);
    }
}

void Controller::handleCaptureTaken(uint id, QPixmap p, bool seedMsg)
{
#ifndef UNUI
    auto it = m_requestMap.find(id);
    if (it != m_requestMap.end()) {
        CaptureRequest request = it.value();
        m_requestMap.erase(it);

        // 将保存操作放到后台线程中执行
        QThread *saveThread = new QThread();
        QObject *worker = new QObject();
        worker->moveToThread(saveThread);

        connect(saveThread, &QThread::started, worker, [=]() {
            // 创建副本以避免 const 问题
            CaptureRequest req = request;
            req.exportCapture(p, seedMsg);

            saveThread->quit();
            worker->deleteLater();
        });

        connect(saveThread, &QThread::finished, saveThread, &QThread::deleteLater);

        // 检查是否需要关闭应用（在主线程中执行）
        if (ConfigHandler().closeAfterScreenshotValue()) {
            connect(saveThread, &QThread::finished, []() {
                QApplication::quit();
            });
        }

        saveThread->start();
    }
#endif
}

void Controller::handleCaptureFailed(uint id)
{
#ifndef UNUI
    m_requestMap.remove(id);

    if (ConfigHandler().closeAfterScreenshotValue()) {
        QApplication::quit();
    }
#endif
}

void Controller::emitSCreenSaveLock()
{
    m_onScreenLock = true;
}

void Controller::emitSCreenSaveUnLock()
{
    m_onScreenLock = false;
}

void Controller::doLater(int msec, QObject *receiver, lambda func)
{
#ifndef UNUI
    DelayWidget *delaywidget = new DelayWidget();
    delaywidget->hide();
    if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
    {
        kdk::UkuiStyleHelper::self()->removeHeader(delaywidget);
    }
    else
    {
        UkuiWindowHelper *windowhelper = new UkuiWindowHelper(delaywidget);
        windowhelper->removeTitleBar();
    }
    QTimer *timer = new QTimer(receiver);
    if (msec > 1000) {
        QObject::connect(timer, &QTimer::timeout, this,
                         [this, msec, timer, delaywidget, func]()
        {
            static bool isRestart = true;
            static int count;
            if (isRestart == true) {
                count = msec/1000;
            }
            if (count == 0) {
                timer->stop();
                timer->deleteLater();
                count = msec/1000;
                isRestart = true;
                func();
            } else if (count > 0) {
                delaywidget->show();
                if (!(m_info.waylandDectected() && !m_info.isHWWayland() && (m_info.windowManager() == DesktopInfo::UKUI)))
                {
                    delaywidget->move(((qApp->desktop()->geometry().width()
                                        -delaywidget->width())/2),
                                      ((qApp->desktop()->geometry().height()
                                        -delaywidget->height())/2));
                    kdk::WindowManager::setGeometry(delaywidget->windowHandle(),delaywidget->geometry());
                }
                else
                {
                    QScreen *primaryScreen = QGuiApplication::primaryScreen();
                    QRect screenGeometry = qApp->primaryScreen()->geometry();
                    int delayWidth = delaywidget->width();
                    int delayHeight = delaywidget->height();
                    int x = (screenGeometry.width() - delayWidth) / 2;
                    int y = (screenGeometry.height() - delayHeight) / 2;
                    delaywidget->setGeometry(x,y,delayWidth,delayHeight);
                }
                delaywidget->updateText(count -1);
                isRestart = false;
                count--;
                if (count == 0) {
                    delaywidget->hide();
                    delaywidget->deleteLater();
                }
            }
        });
        timer->start(1000);
    } else {
        QObject::connect(timer, &QTimer::timeout, receiver,
                         [timer, func](){
            func();
            timer->deleteLater();
        });
        timer->setInterval(msec);
        timer->start();
    }
#endif
}

void Controller::disableScreenCut()
{
#ifndef UNUI
    SystemNotification().sendMessage(tr("Unable to use kylin-screenshot"));
#endif
}

bool Controller::getScreenSaveLockStatus()
{
    return m_onScreenLock;
}

void Controller::setScreenSaveLockStatus(bool status)
{
    m_onScreenLock = status;
}

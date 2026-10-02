/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "windowhelper.h"

#include <QWindow>
#include <KWindowSystem>
#include <QX11Info>
#include <QMenu>
#include <QApplication>

#include <QDebug>

#include <KWindowEffects>
#include <kwindowsystem_version.h>

#include <xcb/xcb.h>
#include <netwm.h>
#include <xatom-helper.h>
#include <kysdk/applications/ukuistylehelper/ukui-decoration-manager.h>
#ifdef USE_UKUI_SHELL
#include <kysdk/applications/ukuistylehelper/ukui-shell-manager.h>
#endif

#include <kaboutdialog.h>

#include <ukui-quick/platform/window-helper.h>
#include <QGSettings>

#include <QKeyEvent>

#include <file-operation-manager.h>

#define UKUI_PANEL_SETTING          "org.ukui.panel.settings"
#define UKUI_PANEL_POSITION_KEY     "panelposition"
#define UKUI_PANEL_SIZE_KEY         "panelsize"
#define UKUI_PANEL_TYPE             "paneltype"

QString getCurrentVersion()
{
    FILE *pp = NULL;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    char *q = NULL;
    QString version = QObject::tr("Unknown");

    pp = popen("dpkg -l ukui-intelligent-data-manager", "r");
    if(NULL == pp)
        return version;

    while((read = getline(&line, &len, pp)) != -1){
        q = strrchr(line, '\n');
        *q = '\0';

        QString content = line;
        QStringList list = content.split(" ");

        list.removeAll("");

        if (list.size() >= 3)
            version = list.at(2);
    }

    free(line);
    pclose(pp);
    return version;
}

WindowHelper::WindowHelper(QWindow *parent) : QObject(parent)
{
    rootWindow = parent;
    isX11 = KWindowSystem::isPlatformX11();
    isWayland = KWindowSystem::isPlatformWayland();

    proxy = new UkuiQuick::WindowProxy2(parent);

    if (QGSettings::isSchemaInstalled(UKUI_PANEL_SETTING)) {
        settings = new QGSettings(UKUI_PANEL_SETTING, QByteArray(), parent);
        updateSettings();
        connect(settings, &QGSettings::changed, this, [=](){
            updateSettings();
        });
    }

    connect(rootWindow, &QWindow::screenChanged, this, [=](){
        if (autoHide) {
            updateGeometry(panelPos);
        }
    });

    connect(rootWindow->screen(), &QScreen::geometryChanged, this, [=](){
        if (autoHide) {
            updateGeometry(panelPos);
        }
    });

    resetWindowState();

    connect(rootWindow, &QWindow::activeChanged, this, [=](){
        if (autoHide) {
            if (!rootWindow->isActive() && !qApp->activeModalWidget() && !qApp->activePopupWidget() && !qApp->activeWindow()) {
                // wlcom下如果再次打开会报错xdg_wm_base#38: error 3: xdg_surface must not have a buffer at creation
                // 这里使用hide()规避
                //rootWindow->close();
                rootWindow->hide();
            }
        }
    });

    connect(rootWindow, SIGNAL(windowModeTriggered()), this, SLOT(switchToWindowMode()));

    connect(rootWindow, SIGNAL(windowClosed()), this, SLOT(resetWindowState()), Qt::QueuedConnection);

    rootWindow->installEventFilter(this);

    QVariant windowVar;
    windowVar.setValue(rootWindow);
    Peony::FileOperationManager::getInstance()->setProperty("rootWindow", windowVar);
}

void WindowHelper::handleSystemMoveRequest(QPointF pos)
{
    // 三岛模式不支持拖拽移动
    if (autoHide)
        return;

    auto window = qobject_cast<QWindow *>(sender());

    qreal dpiRatio = window->devicePixelRatio();
    int nativeX = pos.x() * dpiRatio;
    int nativeY = pos.y() * dpiRatio;
    if (KWindowSystem::isPlatformX11()) {
        auto connection = QX11Info::connection();
        xcb_ungrab_pointer(connection, XCB_TIME_CURRENT_TIME);
        NETRootInfo(connection, NET::WMMoveResize).moveResizeRequest(window->winId(), nativeX, nativeY, NET::Move);

        xcb_button_release_event_t* event = new xcb_button_release_event_t;
        memset(event, 0x00, sizeof(xcb_button_release_event_t));
        event->response_type = XCB_BUTTON_RELEASE;
        event->event = window->winId();
        event->time = QX11Info::getTimestamp();
        event->same_screen = 1;
        event->root = QX11Info::appRootWindow();
        event->root_x = nativeX;
        event->root_y = nativeY;
        event->event_x = 0;
        event->event_y = 0;
        event->child = 0;
        event->state = 0;
        event->detail = XCB_BUTTON_INDEX_1;

        xcb_send_event(connection, false, window->winId(), XCB_EVENT_MASK_BUTTON_RELEASE, (char *) event);
        delete event;

        xcb_flush(connection);
    } else if (KWindowSystem::isPlatformWayland()) {
#if (QT_VERSION >= QT_VERSION_CHECK(5,15,0))
        window->startSystemMove();
#else
        if (UKUIDecorationManager::getInstance()->supportUKUIDecoration()) {
            UKUIDecorationManager::getInstance()->moveWindow(rootWindow);
        }
#endif
    }

    qDebug() << "movement" << pos;
}

void WindowHelper::handleOptionMenuRequest(QPointF pos)
{
    QMenu menu;
    auto helpAction = menu.addAction(tr("Help"));
    auto aboutAction = menu.addAction(tr("About"));
    auto quitAction = menu.addAction(tr("Quit"));
    connect(aboutAction, &QAction::triggered, getRootWindow(), [=]{
        kdk::KAboutDialog d;
        d.setAppIcon(getRootWindow()->icon());
        d.setAppName(getRootWindow()->title());
        d.setAppSupport(tr("Service & Support: ") + "<a href=\"mailto://support@kylinos.cn\" style=\"color:"
                        + qApp->palette().color(QPalette::ButtonText).name(QColor::HexArgb)
                        + ";\">support@kylinos.cn</a><br/>");
        d.setAppVersion(QString(tr("Version: %1")).arg(getCurrentVersion()));
        d.exec();
    });
    connect(quitAction, &QAction::triggered, getRootWindow(), &QWindow::close);
    menu.exec(pos.toPoint());
}

void WindowHelper::activeWindow()
{
    qDebug() << rootWindow << rootWindow->winId();

    if (autoHide) {
        updateGeometry(panelPos);
        // 如果是数据岛模式，再次激活数据岛按钮应该隐藏数据岛界面
        if (rootWindow->isVisible()) {
            rootWindow->close();
            return;
        }
    }

    enableBlur();
    removeHeaderBarX11();
    rootWindow->show();
    // fix wayland active window
    KWindowSystem::forceActiveWindow(rootWindow->winId());
    removeHeaderBarWayland();

    //KWindowEffects::slideWindow(rootWindow, KWindowEffects::SlideFromLocation::BottomEdge, 0);
}

void WindowHelper::enableBlur()
{
#if KWINDOWSYSTEM_VERSION <= QT_VERSION_CHECK (5, 82, 0)
    KWindowEffects::enableBlurBehind(rootWindow->winId(), true);
#else
    KWindowEffects::enableBlurBehind(rootWindow, true);
#endif
}

void WindowHelper::removeHeaderBarX11()
{
    if (!isX11)
        return;
    MotifWmHints hints;
    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hints.functions = MWM_FUNC_ALL;
    hints.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(rootWindow->winId(), hints);
}

void WindowHelper::removeHeaderBarWayland()
{
    if (!isWayland)
        return;
    if (UKUIDecorationManager::getInstance()->supportUKUIDecoration()) {
        UKUIDecorationManager::getInstance()->removeHeaderBar(rootWindow);
#ifdef USE_UKUI_SHELL
    } else if (UkuiShellManager::getInstance()->ukuishellReady()) {
        UkuiShellManager::getInstance()->removeHeaderBar(rootWindow);
#endif
    } else {
        rootWindow->setFlag(Qt::FramelessWindowHint);
    }
}

void WindowHelper::updateSettings()
{
    int panelType = settings->get(UKUI_PANEL_TYPE).toInt();
    isInslandMode = (panelType == 1);
    panelPos = settings->get(UKUI_PANEL_POSITION_KEY).toInt();
    panelSize = settings->get(UKUI_PANEL_SIZE_KEY).toInt();

    if (!autoHide) {
        proxy->setSlideWindowArgs(UkuiQuick::WindowProxy::SlideFromEdge::NoEdge, 0);
        return;
    }
    int offset = 8 + panelSize;

    if (isInslandMode) {
        proxy->setSlideWindowArgs(UkuiQuick::WindowProxy::SlideFromEdge::BottomEdge, offset);
        updateGeometry(0);
        return;
    }

    if (settings) {
        switch (panelPos) {
        case 0: {
            //下
            proxy->setSlideWindowArgs(UkuiQuick::WindowProxy::SlideFromEdge::BottomEdge, offset);
            break;
        }
        case 1: {
            //上
            proxy->setSlideWindowArgs(UkuiQuick::WindowProxy::SlideFromEdge::TopEdge, offset);
            break;
        }
        case 2: {
            //左
            proxy->setSlideWindowArgs(UkuiQuick::WindowProxy::SlideFromEdge::LeftEdge, offset);
            break;
        }
        case 3: {
            //右
            proxy->setSlideWindowArgs(UkuiQuick::WindowProxy::SlideFromEdge::RightEdge, offset);
            break;
        }
        default:
            break;
        }
        updateGeometry(panelPos);
    }
}

void WindowHelper::updateGeometry(int panelPos)
{
    int offset = 8 + panelSize;
    if (isInslandMode) {
        offset = panelSize + 4;
        if (panelSize < 72) {
            offset += 2;
        }
    }
    geometry.moveCenter(rootWindow->screen()->geometry().center());
    switch (panelPos) {
    case 0: {
        //下
        geometry.moveBottom(rootWindow->screen()->geometry().bottom() - offset);
        break;
    }
    case 1: {
        //上
        geometry.moveTop(rootWindow->screen()->geometry().top() + offset);
        break;
    }
    case 2: {
        //左
        geometry.moveLeft(rootWindow->screen()->geometry().left() + offset);
        break;
    }
    case 3: {
        //右
        geometry.moveRight(rootWindow->screen()->geometry().right() - offset);
        break;
    }
    default:
        break;
    }

    proxy->setPosition(geometry.topLeft());
    qDebug() << "geo" << geometry << "panel pos" << panelPos;
}

void WindowHelper::switchToWindowMode()
{
    setAutoHide(false);
    proxy->setWindowType(UkuiQuick::WindowType::Normal);

    // 禁用滑动效果
    proxy->setSlideWindowArgs(UkuiQuick::WindowProxy::SlideFromEdge::NoEdge, 0);

    // 设置显示在任务栏和窗口切换
    proxy->setSkipSwitcher(false);
    proxy->setSkipTaskBar(false);
}

void WindowHelper::resetWindowState()
{
    setAutoHide(true);
    // 三岛模式不支持拖拽移动
    proxy->setWindowType(UkuiQuick::WindowType::AppletPopup);

    // 恢复窗口位置
    rootWindow->resize(geometry.size());
    proxy->setPosition(geometry.topLeft());
    rootWindow->setProperty("windowMode", false);

    // 设置不显示在任务栏和窗口切换
    proxy->setSkipSwitcher(true);
    proxy->setSkipTaskBar(true);

    // 恢复窗口滑动效果
    updateSettings();
}

bool WindowHelper::getAutoHide() const
{
    return autoHide;
}

void WindowHelper::setAutoHide(bool newAutoHide)
{
    autoHide = newAutoHide;
}

bool WindowHelper::eventFilter(QObject *watched, QEvent *event)
{
    if (autoHide && watched == rootWindow && event->type() == QEvent::KeyPress) {
        auto keyevent = static_cast<QKeyEvent *>(event);
        if (keyevent->key() == Qt::Key_Escape) {
            rootWindow->close();
            return true;
        }
    }
    return false;
}

bool WindowHelper::getIsWayland() const
{
    return isWayland;
}

QWindow *WindowHelper::getRootWindow() const
{
    return rootWindow;
}

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

#include "screengrabber.h"
#include "src/utils/filenamehandler.h"
#include "src/utils/systemnotification.h"
#include <QPixmap>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QWindow>
#include <QDebug>
#include <KF5/KWindowSystem/KWindowSystem>
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#include <QDBusInterface>
#include <QDBusReply>
#include <QDir>
#endif
#include "../../widgets/ukui-window-switch-wlcom/multitaskviewmodelbywlcomhandler.h"
#include "ukuiwindowhelper/ukuiwindowhelper.h"
#include <QProcess>
#include <QThread>
#include <signal.h>

ScreenGrabber::ScreenGrabber(QObject *parent) : QObject(parent)
{
}

QPixmap ScreenGrabber::grabEntireDesktop(bool &ok)
{
    killOtherScreenshotProcesses(); // 用来防止还在显示蒙版特效的截图进程
    ok = true;
#ifndef UNUI
#if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
    if (m_info.waylandDectected()) {
        QPixmap res;
        // handle screenshot based on DE
        switch (m_info.windowManager()) {
        case DesktopInfo::GNOME:
        {
            // https://github.com/GNOME/gnome-shell/blob/695bfb96160033be55cfb5ac41c121998f98c328/data/org.gnome.Shell.Screenshot.xml
            QString path = FileNameHandler().generateAbsolutePath(QDir::tempPath()) + ".png";
            QDBusInterface gnomeInterface(QStringLiteral("org.gnome.Shell"),
                                          QStringLiteral("/org/gnome/Shell/Screenshot"),
                                          QStringLiteral("org.gnome.Shell.Screenshot"));
            QDBusReply<bool> reply = gnomeInterface.call(QStringLiteral(
                                                             "Screenshot"), false, false, path);
            if (reply.value()) {
                res = QPixmap(path);
                QFile dbusResult(path);
                dbusResult.remove();
            } else {
                ok = false;
            }
            break;
        } case DesktopInfo::KDE:
        {
            // https://github.com/KDE/spectacle/blob/517a7baf46a4ca0a45f32fd3f2b1b7210b180134/src/PlatformBackends/KWinWaylandImageGrabber.cpp#L145
            QDBusInterface kwinInterface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/Screenshot"),
                                         QStringLiteral("org.kde.kwin.Screenshot"));
            QDBusReply<QString> reply = kwinInterface.call(QStringLiteral("screenshotFullscreen"));
            res = QPixmap(reply.value());
            if (!res.isNull()) {
                QFile dbusResult(reply.value());
                dbusResult.remove();
            }
            break;
        } case DesktopInfo::UKUI:
        {
            QDBusInterface UkuiInterface(QStringLiteral("org.ukui.KWin"),
                                         QStringLiteral("/Screenshot"),
                                         QStringLiteral("org.ukui.kwin.Screenshot"));
            const auto screenCount = QGuiApplication::screens().size();
            QDBusReply<QString> reply;
            if (screenCount == 1) {
                QScreen *primaryScreen = QGuiApplication::primaryScreen();
                if (primaryScreen->geometry().width() > 0)
                {
                    reply = UkuiInterface.call(QStringLiteral("screenshotFull"), true, false);
                    res = QPixmap(reply.value());
                    res.setDevicePixelRatio(static_cast<double>(res.width())/primaryScreen->geometry().width());
                    qDebug() << "res.devicePixelRatio:" << res.devicePixelRatio();
                }
                else
                {
                    reply = UkuiInterface.call(QStringLiteral("screenshotFull"), false, false);
                    res = QPixmap(reply.value());
                }
            }
            else
            {
                reply = UkuiInterface.call(QStringLiteral("screenshotFull"), false, false);
                res = QPixmap(reply.value());
            }

            if (!res.isNull()) {
                QFile dbusResult(reply.value());
                dbusResult.remove();
            }
            else
            {
                qDebug() << "res is null";
            }
            break;
        }
        default:
            ok = false;
            break;
        }
        if (!ok) {
            SystemNotification().sendMessage(tr("Unable to capture screen"));
        }
        return res;
    }
#endif
#endif

    QRect geometry;
    QVector<QRect> rects;
    for (QScreen * const screen : QGuiApplication::screens()) {
        geometry = geometry.united(screen->geometry());
        rects.append(screen->geometry());
    }
    QPixmap p1(QApplication::primaryScreen()->grabWindow(
                   QApplication::desktop()->winId(),
                   geometry.x(),
                   geometry.y(),
                   geometry.width(),
                   geometry.height())
               );
    QPixmap p(p1.width(), p1.height());
    p.fill(QColor(Qt::black));
    QPainter painter(&p);
    auto screenNumber = QApplication::desktop()->screenNumber();
    QScreen *screen = QApplication::screens()[screenNumber];
    for (QRect const &rect : rects) {
        QPixmap p2 = p1.copy(
            QRect(rect.x() * screen->devicePixelRatio(),
                  rect.y() * screen->devicePixelRatio(),
                  rect.width() * screen->devicePixelRatio(),
                  rect.height() * screen->devicePixelRatio())
            );
        painter.drawPixmap(
            QRect(rect.x() * screen->devicePixelRatio(),
                  rect.y() * screen->devicePixelRatio(),
                  rect.width() * screen->devicePixelRatio(),
                  rect.height() * screen->devicePixelRatio()),
            p2);
    }
    p.setDevicePixelRatio(screen->devicePixelRatio());
    return p;
}

QPixmap ScreenGrabber::grabScreen(int screenNumber, bool &ok)
{
    QPixmap p; 
    QPixmap p1 = this->grabEntireDesktop(ok); 
    if (m_info.waylandDectected())
    {
        MultitaskViewModelByWlcomHandler *wlcomHandler = new MultitaskViewModelByWlcomHandler();
        QWidget *w;
        UkuiWindowHelper *windowHelper = new UkuiWindowHelper(w);
        QScreen *curScreen = windowHelper->getCurrentScreenBlocking(); 
        QRect screenRect = curScreen->geometry();
        p1 = p1.copy(screenRect);
        QList<QScreen*> screens = QGuiApplication::screens();
        int actualScreenNumber = 0;
        for (int i = 0; i < screens.size(); ++i) {
            if (screens[i] == curScreen) {
                actualScreenNumber = i;
                break;
            }
        }
        screenNumber = actualScreenNumber;
        WlcomWindow activeWindow = wlcomHandler->getActiveWindow(wlcomHandler->getCurrentDesktop(), screenNumber);
        QRect rect = QRect(activeWindow.x(),
                           activeWindow.y(),
                           activeWindow.width(),
                           activeWindow.height());
        QRect finalRect;
        if (screens.size() == 1) {
            finalRect = rect;
        } else {
            finalRect = QRect(activeWindow.x() - screenRect.x(),
                             activeWindow.y() - screenRect.y(),
                             activeWindow.width(),
                             activeWindow.height());
        }
        p = p1.copy(finalRect);
		delete wlcomHandler;
        delete windowHelper;
    }
    else
    {
        QWindow *window = QWindow::fromWinId(KWindowSystem::activeWindow());
        if (nullptr == window) {
            qDebug() << "window is null";
        } else {
            KWindowInfo info(KWindowSystem::activeWindow(), NET::WMFrameExtents);
            p = p1.copy(info.frameGeometry());
        }
		delete window;
    }
    ok = true; 

    return p;
}

// 使用pidof命令查找kylin-screenshot进程
void ScreenGrabber::killOtherScreenshotProcesses() {
    // 获取当前进程PID
    qint64 currentPid = QCoreApplication::applicationPid();

    // 使用pidof查找所有kylin-screenshot进程
    QProcess pidofProcess;
    pidofProcess.start("pidof", QStringList() << "kylin-screenshot");
    pidofProcess.waitForFinished();

    QString output = pidofProcess.readAllStandardOutput().trimmed();
    if (!output.isEmpty()) {
        QStringList pids = output.split(' ', QString::SkipEmptyParts);

        for (const QString& pidStr : pids) {
            bool ok;
            qint64 pid = pidStr.toLongLong(&ok);
            if (ok && pid != currentPid) {
                // 先尝试优雅终止
                kill(static_cast<pid_t>(pid), SIGTERM);

                // 等待一段时间
                QThread::msleep(50);

                // 如果还在运行，强制杀死
                if (kill(static_cast<pid_t>(pid), 0) == 0) {
                    kill(static_cast<pid_t>(pid), SIGKILL);
                }
            }
        }
    }
}

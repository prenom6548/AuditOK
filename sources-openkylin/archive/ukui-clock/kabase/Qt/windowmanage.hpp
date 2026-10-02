/*
 * libkysdk-base's Library
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Shengjie Ji <jishengjie@kylinos.cn>
 *
 */
#ifndef QT_WINDOWMANAGE_HPP_
#define QT_WINDOWMANAGE_HPP_

/*
 * windowmanage 类使用了 kysdk-waylandhelper 库，使用时需要链接 kysdk-waylandhelper 库
 */

#include <unistd.h>

#include <QApplication>
#include <QScreen>
#include <QWidget>
#include <QRect>

#include <windowmanager/windowmanager.h>
#include <ukuistylehelper/ukuistylehelper.h>

namespace kabase
{

class WindowManage : public QObject
{
    Q_OBJECT
public:
    WindowManage() = default;
    ~WindowManage() = default;

    static void setScalingProperties(void)
    {
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
            QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
            QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
        #endif
        
        #if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
            QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
        #endif

        return;
    };

    static void setMiddleOfScreen(QWidget *w) {
        int sw = QGuiApplication::primaryScreen()->availableGeometry().width();
        int sh = QGuiApplication::primaryScreen()->availableGeometry().height();
        kdk::WindowManager::setGeometry(w->windowHandle(), QRect((sw - w->width()) / 2, (sh - w->height()) / 2, w->width(), w->height()));

        return;
    };

    static void removeHeader(QWidget *w) {
        kdk::UkuiStyleHelper::self()->removeHeader(w);

        return;
    };

    /* id 的初始值必须为 0 */
    static void getWindowId(QString *id) {
        connect(kdk::WindowManager::self(), &kdk::WindowManager::windowAdded, [=](const kdk::WindowId &windowId) {
            if (getpid() == (QString)kdk::WindowManager::getPid(windowId)) {
                *id = windowId.toString();
            }
        });

        return;
    };

    static void keepWindowAbove(const QString id) {
        kdk::WindowManager::keepWindowAbove(id);

        return;
    };

    static void activateWindow(const QString id) {
        kdk::WindowManager::activateWindow(id);

        return;
    }

    static bool isActive(const QString id) {
        kdk::WindowInfo windowInfo = kdk::WindowManager::getwindowInfo(id);

        return windowInfo.isActive();
    }

    static bool isMinimized(const QString id) {
        kdk::WindowInfo windowInfo = kdk::WindowManager::getwindowInfo(id);

        return windowInfo.isMinimized();
    }
};

}

/*
 * wayland 下使用窗口置顶接口 desktop 文件需要注意以下两点
 *     1. 增加字段: X-KDE-Wayland-Interfaces=org_kde_plasma_window_management,org_kde_plasma_activation_feedback,org_kde_kwin_keystate,org_kde_kwin_fake_input,zkde_screencast_unstable_v1
 *     2. exec字段为绝对路径
 */

#endif

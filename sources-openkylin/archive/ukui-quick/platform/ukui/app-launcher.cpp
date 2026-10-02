/*
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
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

/*!
    \class UkuiQuick::AppLauncher
    \brief 应用启动器单例类，提供应用启动和URI打开功能。

    AppLauncher是一个单例类，提供通过桌面文件启动应用、执行命令行、
    打开URI等功能。该类通过DBus与系统进程管理器交互，支持异步操作。

    \section1 主要功能

    \list
    \li 通过桌面文件启动应用
    \li 启动应用时传递参数
    \li 执行命令行
    \li 打开URI链接
    \endlist

    \section1 使用示例

    在C++中使用：
    \code
    auto launcher = UkuiQuick::AppLauncher::instance();
    launcher->launchApp("/usr/share/applications/peony.desktop");
    launcher->openUri("https://www.kylinos.cn");
    \endcode

    在QML中使用：
    \qml
    import org.ukui.quick.platform 1.0

    Button {
        text: "打开文件管理器"
        onClicked: AppLauncher.launchApp("/usr/share/applications/peony.desktop)
    }
    \endqml
*/

#include "app-launcher.h"
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDebug>
#include <QDBusPendingReply>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>
#include <mutex>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <qt6xdg/XdgDesktopFile>
#else
#include <qt5xdg/XdgDesktopFile>
#endif

static UkuiQuick::AppLauncher *g_appLauncher = nullptr;
static std::once_flag onceFlag;
static const QString KYLIN_APP_MANAGER_NAME("com.kylin.ProcessManager");
static const QString KYLIN_APP_MANAGER_PATH("/com/kylin/ProcessManager/AppLauncher");
static const QString KYLIN_APP_MANAGER_INTERFACE("com.kylin.ProcessManager.AppLauncher");

namespace UkuiQuick {

class AppLauncherPrivate
{
};

AppLauncher *AppLauncher::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_appLauncher = new AppLauncher();
    });
    return g_appLauncher;
}

AppLauncher::AppLauncher(QObject *parent) : QObject(parent), d(new AppLauncherPrivate)
{
}

AppLauncher::~AppLauncher()
{
    if(d) {
        delete d;
        d = nullptr;
    }
    g_appLauncher = nullptr;
}

/*!
    \fn void UkuiQuick::AppLauncher::launchApp(const QString &desktopFile)
    \brief 通过desktop文件启动应用。

    该函数通过DBus调用系统进程管理器(kylin-process-manager)来启动指定的应用。

    \param desktopFile 桌面文件路径，如"/usr/share/applications/peony.desktop"

    \sa launchAppWithArguments(), runCommand()
*/
void AppLauncher::launchApp(const QString &desktopFile)
{
    if (desktopFile.isEmpty()) {
        return;
    }
    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME, KYLIN_APP_MANAGER_PATH, KYLIN_APP_MANAGER_INTERFACE, "LaunchApp");
    message << desktopFile;
    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)), this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [desktopFile] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            qWarning() << "Fail to call " << KYLIN_APP_MANAGER_INTERFACE << self->error();
            XdgDesktopFile xdf;
            xdf.load(desktopFile);
            if(!xdf.startDetached()) {
                qWarning() << "Fail to Launch" << desktopFile;
            }
        }
        self->deleteLater();
    });
}

/*!
    \fn void UkuiQuick::AppLauncher::launchAppWithArguments(const QString &desktopFile, const QStringList &args)
    \brief 通过desktop文件启动应用并传递参数。

    该函数通过DBus调用系统进程管理器(kylin-process-manager)来启动指定的应用，并传递命令行参数。

    \param desktopFile 桌面文件路径
    \param args 要传递给应用的命令行参数列表

    \sa launchApp(), runCommand()
*/
void AppLauncher::launchAppWithArguments(const QString &desktopFile, const QStringList &args)
{
    if (desktopFile.isEmpty()) {
        return;
    }
    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME, KYLIN_APP_MANAGER_PATH, KYLIN_APP_MANAGER_INTERFACE, "LaunchAppWithArguments");
    message << desktopFile << args;
    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)), this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [desktopFile, args] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            qWarning() << "Fail to call " << KYLIN_APP_MANAGER_INTERFACE << self->error();
            XdgDesktopFile xdf;
            xdf.load(desktopFile);
            if(!xdf.startDetached(args)) {
                qWarning() << "Fail to Launch" << desktopFile << "with args " << args;
            }
        }
        self->deleteLater();
    });
}

/*!
    \fn void UkuiQuick::AppLauncher::openUri(const QString &uri, const QString &parentWindow)
    \brief 打开URI。

    该函数打开指定的URI，可以是网页链接、文件路径等。

    \param uri 要打开的URI
    \param parentWindow 父窗口ID（可选）

    \sa launchApp(), runCommand()
*/
void AppLauncher::openUri(const QString &uri, const QString &parentWindow)
{
    if (uri.isEmpty()) {
        return;
    }
    //这个接口调用了org.freedesktop.portal.OpenURI，当uri为本地文件时会调用后端org.freedesktop.impl.portal.AppChooser，
    // 会显示peony的应用选择窗
    if(!QDesktopServices::openUrl(QUrl(uri))) {
        qWarning() << "Fail to open" << uri;
    }
}

/*!
    \fn void UkuiQuick::AppLauncher::runCommand(const QString &cmd)
    \brief 执行命令行。

    该函数执行指定的命令行字符串。

    \param cmd 要执行的命令行字符串

    \sa launchApp(), openUri()
*/
void AppLauncher::runCommand(const QString &cmd)
{
    if (cmd.isEmpty()) {
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME, KYLIN_APP_MANAGER_PATH, KYLIN_APP_MANAGER_INTERFACE, "RunCommand");
    message << cmd;
    auto watcher = new QDBusPendingCallWatcher(QDBusPendingCall(QDBusConnection::sessionBus().asyncCall(message)), this);
    QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [cmd] (QDBusPendingCallWatcher *self) {
        if (self->isError()) {
            qWarning() << "Fail to call " << KYLIN_APP_MANAGER_INTERFACE << self->error();
            QProcess::startDetached(cmd, {});
        }
        self->deleteLater();
    });
}
} // UkuiQuick

/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */

#include "notifymanager.h"
#include "notifications_adaptor.h"
#include "platformInfo.h"

#include <QApplication>
#include <QStringList>
#include <QStandardPaths>
#include <sys/file.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <X11/Xlib.h>
#include <ukuisdk/kylin-com4c.h>
#include <ukuisdk/kylin-com4cxx.h>
#include <ukui-log4qt.h>

int main(int argc, char *argv[])
{
    initUkuiLog4qt("ukui-notification-daemon");
    //获取平台信息 字段文件在 etc/lsb-release
    QString arg = KDKGetPrjCodeName().c_str();
    PLATFORM::g_platformType = arg.toLower();
    qInfo() << "PROJECT_CODENAME:" << PLATFORM::g_platformType;
    if (PLATFORM::g_platformType != PLATFORM::V101 && PLATFORM::g_platformType != PLATFORM::Intel) {
        PLATFORM::g_platformType = PLATFORM::V101;
        qCritical() << "ERROR: PROJECT_CODENAME field error, Show V10-SP1 notifications";
    }
    /* 适配高分屏 */
#if (QT_VERSION >= QT_VERSION_CHECK(5, 12, 0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    /* 单例模式 */
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    int fd = open(
        QString(homePath.at(0) + "/.config/ukui-notifications%1.lock").arg(getenv("DISPLAY")).toUtf8().data(),
        O_WRONLY | O_CREAT | O_TRUNC,
        S_IRUSR | S_IWUSR);
    if (fd < 0)
        exit(1);
    if (lockf(fd, F_TLOCK, 0)) {
        syslog(LOG_ERR, "Can't lock single file, ukui-notifications is already running!");
        qInfo() << "Can't lock single file, ukui-notifications is already running!";
        exit(0);
    }

    setenv("QT_QPA_PLATFORMTHEME", "ukui", true); //设置主题环境变量
    /*移除SESSION_MANAGER环境变量，ukui-notification不会再通过XSMP注册到ukui-session中，
    防止session注销过程中的消息提示出现延迟*/
    qunsetenv("SESSION_MANAGER");

    QApplication a(argc, argv);
    /* 解析启动参数 */
    QString protocolVersion = "Desktop Notifications Specification " + QString::number(PROTOCOLVERSION, 10, 1);
    QCoreApplication::setApplicationVersion(protocolVersion);
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QApplication::translate("main", "ukui-notification-daemon service")); // 设置应用程序描述信息
    parser.addHelpOption();    // 添加帮助选项 （"-h" 或 "--help"）
    parser.addVersionOption(); // 添加版本选项 ("-v" 或 "--version")
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    parser.process(a);

    NotifyManager w;
    NotificationsAdaptor notifyAdaptor(&w);
//    w.m_pTopWidget->setProperty("useSystemStyleBlur", true);
    Q_UNUSED(notifyAdaptor);
    return a.exec();
}

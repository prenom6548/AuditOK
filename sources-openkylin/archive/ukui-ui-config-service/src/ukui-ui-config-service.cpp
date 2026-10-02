/*
 * Copyright © 2016 Red Hat, Inc
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *       Jan Grulich <jgrulich@redhat.com>
 * Modifier:
 *       Yue Lan <lanyue@kylinos.cn>
 */

#include <QApplication>
#include <QDBusConnection>
#include <QDebug>
#include <QLoggingCategory>
#include <QTranslator>
#include "ukui-log4qt.h"

#include "ukuithemeconfig.h"

Q_LOGGING_CATEGORY(UKUIThemeConfigDaemon, "theme-config-daemon")


int main (int argc, char *argv[])
{
//    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
//    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
//    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
//#endif

//    initUkuiLog4qt("themeConfig-daemon");
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (sessionBus.registerService(QStringLiteral("com.kylin.themeConfig"))) {
        auto desktopPortal = new UKUIThemeConfig(&a);
        if (sessionBus.registerObject(QStringLiteral("/com/kylin/ThemeConfig"), desktopPortal, QDBusConnection::ExportAdaptors)) {
            qCDebug(UKUIThemeConfigDaemon) << "Desktop portal registered successfully";
        } else {
            qCDebug(UKUIThemeConfigDaemon) << "Failed to register desktop portal";
        }
    } else {
        qCDebug(UKUIThemeConfigDaemon) << "Failed to register com.kylin.ThemeConfig service";
    }

    return a.exec();
}

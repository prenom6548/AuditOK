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
#include <QProcessEnvironment>
#include "ukui-log4qt.h"

#include "ukuidesktopportal.h"
#include "debug.h"

Q_LOGGING_CATEGORY(XdgDesktopPortalUKUI, "xdp-ukui")

namespace {

void installTranslator(const QString &translateFile)
{
    QTranslator *translator = new QTranslator(qApp);
    if (!translator->load(translateFile)) {
        qWarning() << "load translate failed:" << translateFile;
        return;
    }
    QApplication::installTranslator(translator);
}

void initTranslators()
{
    const QString locale = QLocale::system().name();
    installTranslator("/usr/share/qt5-ukui-platformtheme/qt5-ukui-platformtheme_" + locale);
    installTranslator("/usr/share/libpeony-qt/libpeony-qt_" + locale);
    installTranslator("/usr/share/peony-qt-desktop/peony-qt-desktop_" + locale);
    installTranslator("/usr/share/qt5/translations/qt_" + locale);
    installTranslator("/usr/share/ukui/xdg-desktop-portal-ukui/xdg-desktop-portal-ukui_" + locale);
}
}

int main (int argc, char *argv[])
{
    if(QProcessEnvironment::systemEnvironment().value("USER") == "lightdm")
        return -1;
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    qputenv("QT_QPA_PLATFORMTHEME", QByteArray("ukui"));
    initUkuiLog4qt("xdesktop-ukui");
    QApplication a(argc, argv);
    a.setQuitOnLastWindowClosed(false);
    initTranslators();

    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (sessionBus.registerService(QStringLiteral("org.freedesktop.impl.portal.desktop.ukui"))) {
        auto desktopPortal = new UKUIDesktopPortal(&a);
        if (sessionBus.registerObject(QStringLiteral("/org/freedesktop/portal/desktop"), desktopPortal, QDBusConnection::ExportAdaptors)) {
            qCDebug(XdgDesktopPortalUKUI) << "Desktop portal registered successfully";
        } else {
            qCDebug(XdgDesktopPortalUKUI) << "Failed to register desktop portal";
        }
    } else {
        qCDebug(XdgDesktopPortalUKUI) << "Failed to register org.freedesktop.impl.portal.desktop.ukui service";
    }

    return a.exec();
}

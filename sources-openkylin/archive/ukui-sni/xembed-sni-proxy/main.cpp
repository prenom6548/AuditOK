/*
    Main
    SPDX-FileCopyrightText: 2015 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2023 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include <QGuiApplication>
#include <QSessionManager>

#include "fdo-selection-manager.h"
#include "dbus-types.h"
#include "xcb-utils.h"

#include <QDBusMetaType>

#include <KWindowSystem>
#include <log-utils.h>

namespace Xcb
{
Xcb::Atoms *atoms;
}

int main(int argc, char **argv)
{
    LogUtils::initLogFile("xembed-sni-proxy");
    qInstallMessageHandler(LogUtils::messageOutput);
    // the whole point of this is to interact with X, if we are in any other session, force trying to connect to X
    // if the QPA can't load xcb, this app is useless anyway.
    qputenv("QT_QPA_PLATFORM", "xcb");

    QGuiApplication::setDesktopSettingsAware(false);

    QGuiApplication app(argc, argv);

    if (!KWindowSystem::isPlatformX11()) {
        qFatal("xembed-sni-proxy is only useful XCB. Aborting");
    }

    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };
    QObject::connect(&app, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest, disableSessionManagement);

    app.setQuitOnLastWindowClosed(false);

    qDBusRegisterMetaType<DbusImageStruct>();
    qDBusRegisterMetaType<DbusImageVector>();
    qDBusRegisterMetaType<DbusToolTipStruct>();

    Xcb::atoms = new Xcb::Atoms();

    FdoSelectionManager manager;

    auto rc = app.exec();

    delete Xcb::atoms;
    return rc;
}

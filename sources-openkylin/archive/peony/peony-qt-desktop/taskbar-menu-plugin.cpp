/*
 * Peony-Qt
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
 */

#include "taskbar-menu-plugin.h"
#include "file-info.h"
#include "file-utils.h"

#include <QAction>
#include <QIcon>
#include <QDebug>
#include <QUrl>
#ifdef KY_SDK_SYSINFO
#include <kysdk/kysdk-system/libkysysinfo.h>
#endif

using namespace Peony;

// V10 SP1 D-Bus constants
static const char* V10SP1_DBUS_SERVICE = "org.ukui.panel.daemon";
static const char* V10SP1_DBUS_PATH = "/";
static const char* V10SP1_DBUS_INTERFACE = "com.ukui.panel.desktop";
static const char* V10SP1_METHOD_CHECK = "CheckIfExist";
static const char* V10SP1_METHOD_ADD = "AddToTaskbar";
static const char* V10SP1_METHOD_REMOVE = "RemoveFromTaskbar";

// V11 D-Bus constants
static const char* V11_DBUS_SERVICE = "org.ukui.panel";
static const char* V11_DBUS_PATH = "/taskManager";
static const char* V11_DBUS_INTERFACE = "org.ukui.taskManager";
static const char* V11_METHOD_CHECK = "checkQuickLauncher";
static const char* V11_METHOD_ADD = "addQuickLauncher";
static const char* V11_METHOD_REMOVE = "removeQuickLauncher";

TaskbarMenuPlugin::TaskbarMenuPlugin(QObject *parent) : QObject(parent)
{
    connect(this, &TaskbarMenuPlugin::versionChecked, this, [this](bool isV10) {
        m_isV10SP1 = isV10;
        m_versionChecked.store(true);
    }, Qt::QueuedConnection);

    auto thread = QThread::create([this]() {
        checkVersion();
    });
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);
    thread->start();
}

QList<QAction *> TaskbarMenuPlugin::menuActions(Types types, const QString &uri,
                                                const QStringList &selectionUris)
{
    QList<QAction *> l;

    if (types != MenuPluginInterface::DesktopWindow &&
        types != MenuPluginInterface::DirectoryView) {
        return l;
    }

    if (selectionUris.count() != 1) {
        return l;
    }

    QString selectedUri = selectionUris.first();
    auto info = FileInfo::fromUri(selectedUri);

    if (!info->isDesktopFile()) {
        return l;
    }

    QString desktopFilePath = getDesktopFilePath(selectedUri);
    if (desktopFilePath.isEmpty()) {
        return l;
    }

    if (!m_versionChecked) {
        return l;
    }

    bool isPinned = checkQuickLauncher(desktopFilePath);

    if (isPinned) {
        QAction *removeAction = new QAction(QIcon::fromTheme("ukui-unfixed-symbolic"),
                                            tr("Remove launcher from panel"), nullptr);
        connect(removeAction, &QAction::triggered, this, [=]() {
            removeQuickLauncher(desktopFilePath);
        });
        l << removeAction;
    } else {
        QAction *addAction = new QAction(QIcon::fromTheme("ukui-fixed-symbolic"),
                                         tr("Add launcher to panel"), nullptr);
        connect(addAction, &QAction::triggered, this, [=]() {
            addQuickLauncher(desktopFilePath);
        });
        l << addAction;
    }

    return l;
}

bool TaskbarMenuPlugin::isV10SP1()
{
    return m_isV10SP1;
}

void TaskbarMenuPlugin::checkVersion()
{
    bool result = false;
#ifdef KY_SDK_SYSINFO
    const char* version = kdk_system_get_major_version();
    if (version) {
        QString ver(version);
        result = ver.contains("V10");
    }
#endif
    Q_EMIT versionChecked(result);
}

bool TaskbarMenuPlugin::checkQuickLauncher(const QString &desktopFile)
{
    QDBusInterface interface(m_isV10SP1 ? V10SP1_DBUS_SERVICE : V11_DBUS_SERVICE,
                             m_isV10SP1 ? V10SP1_DBUS_PATH : V11_DBUS_PATH,
                             m_isV10SP1 ? V10SP1_DBUS_INTERFACE : V11_DBUS_INTERFACE,
                             QDBusConnection::sessionBus());

    if (!interface.isValid()) {
        qWarning() << "Taskbar D-Bus interface is not valid";
        return false;
    }

    const char* method = m_isV10SP1 ? V10SP1_METHOD_CHECK : V11_METHOD_CHECK;
    QDBusReply<bool> reply = interface.call(method, desktopFile);
    if (reply.isValid()) {
        return reply.value();
    } else {
        qWarning() << "Failed to check quick launcher:" << reply.error().message();
        return false;
    }
}

bool TaskbarMenuPlugin::addQuickLauncher(const QString &desktopFile)
{
    QDBusInterface interface(m_isV10SP1 ? V10SP1_DBUS_SERVICE : V11_DBUS_SERVICE,
                             m_isV10SP1 ? V10SP1_DBUS_PATH : V11_DBUS_PATH,
                             m_isV10SP1 ? V10SP1_DBUS_INTERFACE : V11_DBUS_INTERFACE,
                             QDBusConnection::sessionBus());

    if (!interface.isValid()) {
        qWarning() << "Taskbar D-Bus interface is not valid";
        return false;
    }

    const char* method = m_isV10SP1 ? V10SP1_METHOD_ADD : V11_METHOD_ADD;
    QDBusReply<bool> reply = interface.call(method, desktopFile);
    if (reply.isValid()) {
        return reply.value();
    } else {
        qWarning() << "Failed to add quick launcher:" << reply.error().message();
        return false;
    }
}

bool TaskbarMenuPlugin::removeQuickLauncher(const QString &desktopFile)
{
    QDBusInterface interface(m_isV10SP1 ? V10SP1_DBUS_SERVICE : V11_DBUS_SERVICE,
                             m_isV10SP1 ? V10SP1_DBUS_PATH : V11_DBUS_PATH,
                             m_isV10SP1 ? V10SP1_DBUS_INTERFACE : V11_DBUS_INTERFACE,
                             QDBusConnection::sessionBus());

    if (!interface.isValid()) {
        qWarning() << "Taskbar D-Bus interface is not valid";
        return false;
    }

    const char* method = m_isV10SP1 ? V10SP1_METHOD_REMOVE : V11_METHOD_REMOVE;
    QDBusReply<bool> reply = interface.call(method, desktopFile);
    if (reply.isValid()) {
        return reply.value();
    } else {
        qWarning() << "Failed to remove quick launcher:" << reply.error().message();
        return false;
    }
}

QString TaskbarMenuPlugin::getDesktopFilePath(const QString &uri)
{
    QUrl url(uri);
    if (url.isLocalFile()) {
        return url.toLocalFile();
    }

    return QString();
}

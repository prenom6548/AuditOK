/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: an-yongtai <anyongtai@kylinos.cn>
 *
 */

#include "notebook-shortcut.h"
#include <QDebug>
#include <QColor>
#include <QProcess>
#include <QDBusInterface>
#include <QFile>
#include <QIcon>

#define KYLIN_APP_MANAGER_NAME         "com.kylin.ProcessManager"
#define KYLIN_APP_MANAGER_PATH         "/com/kylin/ProcessManager/AppLauncher"
#define KYLIN_APP_MANAGER_INTERFACE    "com.kylin.ProcessManager.AppLauncher"
#define UKUI_NOTEBOOK_NAME  "ukui-notebook"
#define UKUI_NOTEBOOK_SYMBOLIC         "notes-app-24-symbolic"
#define UKUI_NOTEBOOK_LOTTIE           "notes-app-24-symbolic-motion.lottie"
#define THEME_LOTTIE_PATH              "/usr/share/icons/ukui-motion-icons"

using namespace UkuiShortcut;
NotebookShortcut::NotebookShortcut(QObject *parent) : Shortcut(parent)
{
    m_currentStatusInfo.setColor(Color::ColorRole::BaseColor);
    m_currentStatusInfo.setName(tr("Notebook"));
    m_currentStatusInfo.setIcon(QFile::exists(THEME_LOTTIE_PATH "/" UKUI_NOTEBOOK_LOTTIE)
                                ? UKUI_NOTEBOOK_LOTTIE
                                : (QIcon::fromTheme(UKUI_NOTEBOOK_SYMBOLIC).isNull()
                                   ? ":/notebookShortcut/ukui-notebook.svg" : UKUI_NOTEBOOK_SYMBOLIC));
    m_currentStatusInfo.setToolTip(tr("Notebook"));
    m_applicationInfo = new ApplicationInfo;
    m_appPath = getDesktopPath(UKUI_NOTEBOOK_NAME);
    m_isEnable = !m_appPath.isEmpty();
    connect(m_applicationInfo, &ApplicationInfo::appDBItems2BDelete, this, &NotebookShortcut::onAppUninstalled);
    connect(m_applicationInfo, &ApplicationInfo::appDBItems2BAdd, this, &NotebookShortcut::onAppInstalled);
    initMetaData();
}

NotebookShortcut::~NotebookShortcut()
{
    if (m_applicationInfo) {
        delete m_applicationInfo;
        m_applicationInfo = nullptr;
    }
}

QString NotebookShortcut::pluginId()
{
    return QStringLiteral("Notebook");
}

void NotebookShortcut::active(PluginMetaType::Action action)
{
    if (action == PluginMetaType::Action::Click) {
        LaunchApp();
    }
}

const StatusInfo NotebookShortcut::currentStatus()
{
    return m_currentStatusInfo;
}

bool NotebookShortcut::isEnable()
{
    return m_isEnable;
}

void NotebookShortcut::setAddedState(bool added)
{
    if (m_isAdded != added) {
        m_isAdded = added;
    }
}

QString NotebookShortcut::getDesktopPath(QString id)
{
    QString path = "";
    m_applicationInfo->desktopFilePathFromName(id, path);
    return path;
}

void NotebookShortcut::LaunchApp()
{
    if (!m_isEnable || !m_isAdded) return;
    QDBusMessage message = QDBusMessage::createMethodCall(KYLIN_APP_MANAGER_NAME,
                                                          KYLIN_APP_MANAGER_PATH,
                                                          KYLIN_APP_MANAGER_INTERFACE,
                                                          "LaunchApp");
    message << m_appPath;
    auto watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message),this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [ = ] (QDBusPendingCallWatcher *self) {
        if (self) {
            if (self->isError()) {
                qDebug()<<"notebook-shortcut: use dbus failed" << self->error().message();
                if (!QProcess::startDetached("ukui-notebook")) {
                    qDebug()<<"notebook-shortcut: ukui-notebook is failed";
                }
            }
            self->deleteLater();
        }
    });
}

QMap<PluginMetaType::SystemMode, PluginMetaData> NotebookShortcut::pluginMetaData()
{
    return m_metaData;
}

void NotebookShortcut::initMetaData()
{
    PluginMetaData pc {true, 11, PluginMetaType::PluginType::Icon};
    pc.setPreAction(PluginMetaType::Hide);
    PluginMetaData tablet {true, 11, PluginMetaType::PluginType::Icon};
    tablet.setPreAction(PluginMetaType::Hide);
    m_metaData.insert(PluginMetaType::SystemMode::PC, pc);
    m_metaData.insert(PluginMetaType::SystemMode::Tablet, tablet);
}

void NotebookShortcut::onAppUninstalled(const QStringList &desktopFiles)
{
    if (std::any_of(desktopFiles.begin(), desktopFiles.end(),
                    [&](const QString &str) {return str.contains(UKUI_NOTEBOOK_NAME);})) {
        checkAppInstallState();
    }
}

void NotebookShortcut::onAppInstalled(const QStringList &desktopFiles)
{
    if (std::any_of(desktopFiles.begin(), desktopFiles.end(),
                    [&](const QString &str) {return str.contains(UKUI_NOTEBOOK_NAME);})) {
        checkAppInstallState();
    }
}

void NotebookShortcut::checkAppInstallState()
{
    m_appPath = getDesktopPath(UKUI_NOTEBOOK_NAME);
    bool enable = !m_appPath.isEmpty();
    if (m_isEnable == enable) return;
    m_isEnable = enable;
    Q_EMIT enableStatusChanged(m_isEnable);
}

NotebookShortcutPlugin::NotebookShortcutPlugin(QObject *parent) : UkuiShortcutPlugin(parent)
{

}

QStringList NotebookShortcutPlugin::translations()
{
    return {QStringLiteral("notebook-shortcut")};
}

Shortcut *NotebookShortcutPlugin::createShortcut()
{
    return new NotebookShortcut;
}

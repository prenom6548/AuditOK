/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "selectionchecker.h"
#include "loghelper.h"
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDebug>
#include <QGuiApplication>
#include <X11/Xlib.h>
#include <kwindowsystem.h>

namespace
{

const char *kwin_service_name = "org.kde.KWin";
const char *kwin_clipboard_path = "/Clipboard";
const char *kwin_clipboard_interface = "org.kde.KWin.Clipboard";

}

SelectionChecker::SelectionChecker()
{
    checkPlatform();
}

bool SelectionChecker::hasSelection(const std::vector<int> &pids) const
{
    const std::vector<int> selectionPids =
        m_isWaylandPlatform ? getWaylandSelectionProcesses() : getX11SelectionProcesses();

    for (const auto &pid : pids) {
        if (std::find(selectionPids.cbegin(), selectionPids.cend(), pid) != selectionPids.cend()) {
            return true;
        }
    }

    return false;
}

void SelectionChecker::checkPlatform()
{
    m_isWaylandPlatform = QGuiApplication::platformName().startsWith(
        QLatin1String("wayland"), Qt::CaseInsensitive);
}

std::vector<int> SelectionChecker::getX11SelectionProcesses() const
{
    std::vector<int> selectionPids;

    const auto selectionOwners = queryX11SelectionOwners();
    for (const auto &owner : selectionOwners) {
        KWindowInfo windowInfo(owner, NET::WMAllProperties);
        selectionPids.emplace_back(windowInfo.pid());
    }

    return selectionPids;
}

std::vector<unsigned long> SelectionChecker::queryX11SelectionOwners() const
{
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        PRINT_NOTICE("Could not open X display.\n");
        return std::vector<unsigned long>();
    }

    const std::vector<std::string> selections{
        "PRIMARY", "SECONDARY", "CLIPBOARD"};

    std::vector<unsigned long> selectionOwners;

    for (const auto &selection : selections) {
        Atom atomSelection = XInternAtom(display, selection.c_str(), False);
        unsigned long owner = XGetSelectionOwner(display, atomSelection);
        selectionOwners.emplace_back(owner);
    }

    XCloseDisplay(display);

    return selectionOwners;
}

std::vector<int> SelectionChecker::getWaylandSelectionProcesses() const
{
    std::vector<int> selectionPids;

    int clipboardOwnerPid = getSelectionPidFromKwin("getClipboardSelectionPid");
    int primarySelectionOwnerPid = getSelectionPidFromKwin("getPrimarySelectionPid");

    if (clipboardOwnerPid != 0) {
        selectionPids.emplace_back(clipboardOwnerPid);
    }

    if (primarySelectionOwnerPid != 0) {
        selectionPids.emplace_back(primarySelectionOwnerPid);
    }

    QString pidsStr;
    for (size_t i = 0; i < selectionPids.size(); ++i) {
        pidsStr += QString::number(selectionPids[i]);
        if (i != selectionPids.size() - 1) {
            pidsStr += ", "; // 元素间用逗号分隔
        }
    }
    PRINT_DBG("Selection PIDs: %s", pidsStr.toUtf8().constData());

    return selectionPids;
}

int SelectionChecker::getSelectionPidFromKwin(const std::string &method) const
{
    QDBusInterface kwinInterface(
        kwin_service_name, kwin_clipboard_path, kwin_clipboard_interface);

    if (!kwinInterface.isValid()) {
        // qWarning() << "KWIn dbus interface is not vaild.";
        PRINT_NOTICE("KWIn dbus interface is not vaild.\n");
        return {};
    }

    QDBusPendingReply<int> reply = kwinInterface.call(QString::fromStdString(method));
    if (!reply.isValid()) {
        // qWarning() << "Get clipboard selection pid from kwin failed: " << reply.error();
        PRINT_NOTICE("Get clipboard selection pid from kwin failed: %s\n", reply.error().message().toStdString().c_str());
        return 0;
    }

    return reply.value();
}

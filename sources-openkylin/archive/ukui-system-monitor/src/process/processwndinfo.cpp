/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "processwndinfo.h"

#include <QDebug>

#include "kysdk/applications/windowmanager/windowmanager.h"
using namespace kdk;

ProcessWndInfo::ProcessWndInfo(QObject* parent)
    : QObject(parent)
{
    kdk::WindowManager::self();
}

ProcessWndInfo::~ProcessWndInfo()
{

}

void ProcessWndInfo::updateWindowInfos()
{
    m_mapProcWndInfos.clear();
    for(kdk::WindowId w : kdk::WindowManager::windows()) {
        ProcWindowInfo wndInfo;
        if (w.isValid()) {
            wndInfo.procId = kdk::WindowManager::getPid(w);
            wndInfo.wndName = kdk::WindowManager::getWindowTitle(w);
            wndInfo.wndIcon = kdk::WindowManager::getWindowIcon(w);
            m_mapProcWndInfos[wndInfo.procId] = wndInfo;
            qDebug() << Q_FUNC_INFO << wndInfo.procId << wndInfo.wndName << wndInfo.wndIcon;
        }
    }
}

QList<pid_t> ProcessWndInfo::getWindowPids()
{
    return m_mapProcWndInfos.keys();
}

WId ProcessWndInfo::getWndIdByPid(pid_t pid)
{
    if (m_mapProcWndInfos.contains(pid)) {
        return m_mapProcWndInfos[pid].wndId;
    }
    return 0;
}

QString ProcessWndInfo::getWndNameByPid(pid_t pid)
{
    if (m_mapProcWndInfos.contains(pid)) {
        return m_mapProcWndInfos[pid].wndName;
    }
    return "";
}

QIcon ProcessWndInfo::getWndIconByPid(pid_t pid)
{
    if (m_mapProcWndInfos.contains(pid)) {
        return m_mapProcWndInfos[pid].wndIcon;
    }
    return QIcon();
}

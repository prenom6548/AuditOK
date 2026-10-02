/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#include "sidebar-main.h"
#include "log-utils.h"

namespace {
QStringList g_calls;
QString g_lastWidgetId;
bool g_lastShowReturnButton = false;
QString g_lastParsedMessage;

QString g_lastInitFileName;
int g_outputCallCount = 0;
}

SidebarMain::SidebarMain(QObject *parent) : QObject(parent)
{
    g_calls.append("construct");
}

void SidebarMain::requestWidgetActive(const QString &id, bool showReturnButton)
{
    g_calls.append("widget");
    g_lastWidgetId = id;
    g_lastShowReturnButton = showReturnButton;
}

void SidebarMain::resetRecords()
{
    g_calls.clear();
    g_lastWidgetId.clear();
    g_lastShowReturnButton = false;
    g_lastParsedMessage.clear();
}

QStringList SidebarMain::calls()
{
    return g_calls;
}

QString SidebarMain::lastWidgetId()
{
    return g_lastWidgetId;
}

bool SidebarMain::lastShowReturnButton()
{
    return g_lastShowReturnButton;
}

QString SidebarMain::lastParsedMessage()
{
    return g_lastParsedMessage;
}

void SidebarMain::parseMessage(const QString &msg)
{
    g_calls.append("parse:" + msg);
    g_lastParsedMessage = msg;
}

void SidebarMain::requestSidebar(SidebarMain::Request request)
{
    g_calls.append(QString("sidebar:%1").arg(static_cast<int>(request)));
}

void SidebarMain::requestShortcut(SidebarMain::Request request)
{
    g_calls.append(QString("shortcut:%1").arg(static_cast<int>(request)));
}

void SidebarMain::requestNotificationCenter(SidebarMain::Request request)
{
    g_calls.append(QString("notice:%1").arg(static_cast<int>(request)));
}

void LogUtils::initLogFile(const QString &fileName)
{
    g_lastInitFileName = fileName;
}

void LogUtils::messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(type)
    Q_UNUSED(context)
    Q_UNUSED(msg)
    ++g_outputCallCount;
}

void LogUtils::reset()
{
    g_lastInitFileName.clear();
    g_outputCallCount = 0;
}

QString LogUtils::lastInitFileName()
{
    return g_lastInitFileName;
}

int LogUtils::outputCallCount()
{
    return g_outputCallCount;
}

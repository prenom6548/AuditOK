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

#include "qtsingleapplication.h"

namespace {
bool g_running = false;
int g_execResult = 0;
int g_execCallCount = 0;
int g_translatorInstallCount = 0;
QString g_lastId;
QString g_lastMessage;
QStringList g_arguments;
}

QtSingleApplication::QtSingleApplication(const QString &id, int &argc, char **argv)
{
    g_lastId = id;
    g_arguments.clear();
    for (int i = 0; i < argc; ++i) {
        g_arguments.append(QString::fromLocal8Bit(argv[i]));
    }
}

bool QtSingleApplication::isRunning() const
{
    return g_running;
}

bool QtSingleApplication::sendMessage(const QString &message, int timeout)
{
    Q_UNUSED(timeout)
    g_lastMessage = message;
    return true;
}

QStringList QtSingleApplication::arguments()
{
    return g_arguments;
}

int QtSingleApplication::exec()
{
    ++g_execCallCount;
    return g_execResult;
}

void QtSingleApplication::installTranslator(QTranslator *translator)
{
    Q_UNUSED(translator)
    ++g_translatorInstallCount;
}

void QtSingleApplication::reset()
{
    g_running = false;
    g_execResult = 0;
    g_execCallCount = 0;
    g_translatorInstallCount = 0;
    g_lastId.clear();
    g_lastMessage.clear();
    g_arguments.clear();
}

void QtSingleApplication::setRunning(bool running)
{
    g_running = running;
}

void QtSingleApplication::setExecResult(int result)
{
    g_execResult = result;
}

QString QtSingleApplication::lastId()
{
    return g_lastId;
}

QString QtSingleApplication::lastMessage()
{
    return g_lastMessage;
}

int QtSingleApplication::execCallCount()
{
    return g_execCallCount;
}

int QtSingleApplication::translatorInstallCount()
{
    return g_translatorInstallCount;
}

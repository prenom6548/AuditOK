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

#ifndef QTSINGLEAPPLICATION_H
#define QTSINGLEAPPLICATION_H

#include <QObject>
#include <QTranslator>
#include <QStringList>

class QtSingleApplication : public QObject
{
    Q_OBJECT
public:
    QtSingleApplication(const QString &id, int &argc, char **argv);

    bool isRunning() const;
    bool sendMessage(const QString &message, int timeout = 5000);

    static QStringList arguments();
    static int exec();
    static void installTranslator(QTranslator *translator);

    static void reset();
    static void setRunning(bool running);
    static void setExecResult(int result);
    static QString lastId();
    static QString lastMessage();
    static int execCallCount();
    static int translatorInstallCount();

Q_SIGNALS:
    void messageReceived(const QString &message);
};

#endif

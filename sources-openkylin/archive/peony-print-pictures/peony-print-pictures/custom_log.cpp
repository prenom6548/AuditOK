/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include <QMutex>
#include <QDateTime>
#include <QIODevice>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "custom_log.h"

CustomLog::CustomLog(const QString &name)
{
    setName(name);
}

CustomLog::~CustomLog()
{

}

void CustomLog::setName(const QString &name)
{
    QString dest = QDir::homePath() + QString("/.log/peony-print-pictures");
    QDir dir;
    if (!dir.exists(dest)) {
        dir.mkpath(dest);
    }

    filePath = QDir::homePath() + QString("/.log/peony-print-pictures/%1_").arg(name);
    // filePath = QDir::homePath() + QString("/.log/%1_").arg(name);
}

void CustomLog::info(const QString &info)
{
    writeLog(LogType::INFO, info);
}

void CustomLog::writeLog(LogType type, const QString &str)
{
    static QMutex mutex;
    mutex.lock();

    QString text;

    switch(type) {
    case LogType::INFO:
        text = QString("Info: ");
        break;
    case LogType::DEBUG:
        text = QString("Debug: ");
        break;
    case LogType::WARNING:
        text = QString("Warning: ");
        break;
    case LogType::CRITICAL:
        text = QString("Critical:");
        break;
    case LogType::FATAL:
        text = QString("Fatal: ");
        break;
    default:
        text = QString("Debug: ");
    }

    QString contextInfo = QString("(%1) L:(%2)").arg(QString(__FILE__)).arg(QString::number(__LINE__));
    QString strDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString strMessage = QString("%1 %2 \t%3 \t%4").arg(text).arg(contextInfo).arg(strDateTime).arg(str);

    QFile file(filePath + QString(QDateTime::currentDateTime().toString("yyyy-MM-dd").append("-log.txt")));
    file.open(QIODevice::ReadWrite | QIODevice::Append);
    QTextStream stream(&file);
    stream << strMessage << "\r\n";
    file.flush();
    file.close();

    mutex.unlock();
}
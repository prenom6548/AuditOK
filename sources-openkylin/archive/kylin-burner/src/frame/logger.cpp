/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "logger.h"
#include "src/po/translation.h"
#include "src/frame/configuration.h"

#include <fcntl.h>
#include <unistd.h>

#include <QDateTime>
#include <QDir>

#define LOG_PATH "/tmp/kylin-burner/log/"

Logger *Logger::m_oInstance = nullptr;
QMutex  Logger::m_oMutex;

static QStringList level;


Logger::Logger(QObject *parent) :
    QObject(parent),
    m_iLevel(-1),
    m_iHandle(-1),
    m_iCurrentLevel(Info)
{
    m_oRegister.clear();
    level << ("info") << ("warn") << ("error") << ("debug");
}

Logger *Logger::instance()
{
    if (nullptr == m_oInstance)
    {
        QMutexLocker locker(&m_oMutex);
        if (nullptr == m_oInstance)
        {
            m_oInstance = new Logger();
        }
    }
    return m_oInstance;
}

void Logger::init()
{
    QString defaultPath;
    QDir logDir;

    defaultPath = LOG_PATH;
    logDir.setPath(LOG_PATH);
    if (!logDir.mkpath(LOG_PATH))
        return;

    defaultPath = CFG->get("logger", "path", defaultPath).toString();
    if (!defaultPath.endsWith("/")) defaultPath.append("/");
    if (!logDir.mkpath(defaultPath))
    {
        defaultPath = LOG_PATH;
        CFG->set("logger", "path", defaultPath);
    }
    defaultPath.append(QString("%1.log").arg(date()));
    m_iHandle = ::open(defaultPath.toUtf8().constData(), O_RDWR | O_CREAT | O_APPEND, 0660);
    if (-1 == m_iHandle) return;
    m_iLevel = CFG->get("logger", "level", Info).toInt();

    if (m_iLevel > Debug)
        m_iLevel = Debug;

    info(_("logger path : %1").arg(defaultPath));
    info(_("Will record under %1 level log.").arg(level[m_iLevel]));
}

QString Logger::registerName(QObject *obj, QString name)
{
    if (m_oRegister.value(obj).isEmpty())
    {
        m_oRegister.insert(obj, name);
    }
    return m_oRegister.value(obj);
}

void Logger::info(QString content)
{
    m_iCurrentLevel = Info;
    log(content, model(sender()));
}

void Logger::warn(QString content)
{
    m_iCurrentLevel = Warning;
    log(content, model(sender()));
}

void Logger::error(QString content)
{
    m_iCurrentLevel = Error;
    log(content, model(sender()));
}

void Logger::debug(QString content)
{
    m_iCurrentLevel = Debug;
    log(content, model(sender()));
}

QString Logger::model(QObject *obj)
{
    if (m_oRegister.value(obj).isEmpty() || nullptr == obj)
        return QString(_("common"));
    return m_oRegister.value(obj);
}

QString Logger::date()
{
    return QDateTime::currentDateTime().toString("yyyyMMdd");
}

QString Logger::dateTime()
{
    return QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
}

void Logger::log(QString content, QString model)
{
    QString w;
    QString m;

    if ((m_iLevel < m_iCurrentLevel) && m_iCurrentLevel != Error) return;

    m = model;
    if (m.isEmpty()) m = this->model();

    w.clear();
    // date time pid appname model level - content
    w.append(dateTime());
    w.append(" ");
    w.append(QString("%1").arg(::getpid()));
    w.append(" ");
    w.append(level[m_iCurrentLevel]);
    w.append(" - ");
    w.append(content);
    w.append("\n");
    if (-1 != m_iHandle)
    {
        lseek(m_iHandle, SEEK_END, 0);
        ::write(m_iHandle, w.toUtf8().constData(), w.toUtf8().length());
    }
    if (m_iCurrentLevel > Info)
    {
        printf("%s", w.toUtf8().constData());
        fflush(0);
    }
}

Logger::~Logger()
{
    if (-1 != m_iHandle) ::close(m_iHandle);
}

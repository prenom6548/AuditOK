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


#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QMutex>
#include <QHash>

class Logger : public QObject
{
    Q_OBJECT
    enum Level
    {
        Info = 0,
        Warning,
        Error,
        Debug
    };

public:
    static Logger *instance();
public:
    Logger(const Logger*)=delete;
    Logger *operator =(const Logger*)=delete;
public:
    void init();
public:
    QString registerName(QObject *, QString);
public:
    void info(QString);
    void warn(QString);
    void error(QString);
    void debug(QString);
private:
    explicit Logger(QObject *parent = nullptr);
    ~Logger();
private:
    QString model(QObject *obj = nullptr);
    QString date();
    QString dateTime();
    void log(QString content, QString model = QString());
private:
    static Logger           *m_oInstance;
    static QMutex            m_oMutex;
private:
    int                      m_iHandle;
    int                      m_iLevel;
    int                      m_iCurrentLevel;
    QHash<QObject*, QString> m_oRegister;

};
#define LOGGER Logger::instance()
#endif // LOGGER_H

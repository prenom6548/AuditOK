/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef SQLITEHANDLE_H
#define SQLITEHANDLE_H


#include <QString>
#include <QVector>

class QSqlDatabase;

class KPlaylistRecord
{
public:
    QString path;
    QString name;
    QString mark;
    QString viewPath;
    int lastTime;
    int duration;
};

struct MarkRecord {
    QString path;
    int     duration;
    int     pos;
    QString desc;
    QString preview;
};


class SqliteHandle
{
public:
    static SqliteHandle *getInstance(QString path = "");

    QVector<KPlaylistRecord> getPlayList();
    double     getLastTime     (QString path);
    int     getDuration     (QString path);
    bool    insertPlayList  (QString path, QString name, int duration, QString view);
    bool    updateLastTime  (QString path, double lasttime);
    bool    updateDuration  (QString path, int duration);
    bool    deletePlayList  (QString path);
    bool    clearPlayList   ();

    bool    insertBookMark  (QString path, int duration, int pos, QString desc, QString view);
    void    deleteBookMark  (QString path, int pos);
    QVector<MarkRecord> getMarks();
    QString getMarkCharacter(QString path);

    QString lastError       (){return m_lastErrorMessage;}

private:
    static SqliteHandle* instance;
    SqliteHandle(QString);
    ~SqliteHandle();

    QString m_lastErrorMessage;
};

#endif // SQLITEHANDLE_H

/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
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
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "sqlitemodel.h"

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlError>

#include <QDebug>

SqliteModel::SqliteModel(QObject *parent) : QSqlQueryModel(parent)
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/home/lanyue/mydata.db");
    if (!db.open()) {
        perror("failed to open database");
        //return -1;
    }

    qDebug()<<db.isValid();

    QSqlQuery query(db);
//    query.exec("CREATE TABLE person (id INTEGER PRIMARY KEY, name TEXT, age INTEGER)");
//    qDebug()<<query.size()<<query.lastError().text();
//    query.exec("INSERT INTO person (name, age) VALUES ('John Doe', 30)");
//    qDebug()<<query.size()<<query.lastError().text();
    query.exec("SELECT * FROM memos;");
    qDebug()<<query.size()<<query.lastError().text();
    setQuery(query);

    //setHeaderData(0, Qt::Horizontal, "name");
    //setHeaderData(1, Qt::Horizontal, "age");
}

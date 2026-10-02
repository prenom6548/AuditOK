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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QDebug>
#include <QSqlTableModel>
#include <QLocale>
#include <QDir>
//创建或打开数据库
//Create or open database
namespace clock_sql {
static QSqlDatabase db;
static bool createConnection()
{
    QString homeLocation = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config";
    if(!QDir(homeLocation + "/app-ukui-clock").exists()) {
        QDir(homeLocation).mkdir("app-ukui-clock");
    }
    QString url_filepath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) +"/.config/app-ukui-clock/Clock_database"+"_sp2.db";
    //SQLite是一款轻量级的开源的嵌入式数据库
    if(!QSqlDatabase::contains(url_filepath)){
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName(url_filepath);
        if(!db.open()){
            return false;
        }
    }

    //建表语句
    QSqlQuery query;
    query.exec(QString(
                   "create table clock (hour int, minute int, bell_id QString, on_or_off int, num int, repeat Qstring,"
                   "monday int, tuesday int, wednesday int, thursday int, friday int, saturday int , sunday int, "
                   "name Qstring, id Qstring,remind_status Qstring, editstatus bool)"));
    query.clear();
    query.exec(QString("create table setup (mute_on int, bell_id QString)"));
    query.clear();
    query.exec(QString("create table watch_time (wt_id QString,time_since_epoch QString)"));
    query.clear();
    query.exec(QString("create table bell (id QString, bell_en QString,"
                       "bell_path QString,create_time int,bell_type int)"));
    return true;
}
static QSqlQuery getQSqlQuery(){
    QSqlQuery query(db);
    query.clear();
    return query;
}
static QSqlTableModel * getTableByName(QObject *parent,QString name){
    QSqlDatabase db = QSqlDatabase::database();
    QSqlTableModel * model = new QSqlTableModel(parent,db);
    model->setTable(name);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    return model;
}

static QSqlTableModel * getClockTable(QObject *parent){
    QSqlTableModel * model = getTableByName(parent,"clock");
    return model;
}

static QSqlTableModel * getSetupTable(QObject *parent){
    QSqlTableModel * model = getTableByName(parent,"setup");
    return model;
}

static QSqlTableModel * getBellTable(QObject *parent){
    QSqlTableModel * model = getTableByName(parent,"bell");
    return model;
}
static QSqlTableModel * getWatchTimeTable(QObject *parent){
    QSqlTableModel * model = getTableByName(parent,"watch_time");
    return model;
}


}

#endif // CONNECTION_H

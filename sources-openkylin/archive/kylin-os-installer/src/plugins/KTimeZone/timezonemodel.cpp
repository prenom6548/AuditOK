/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "timezonemodel.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "../PluginService/ksystemsetting_unit.h"
using namespace KServer;

QString getZoneList()
{
    QString tmp = KServer::ReadSettingIni("config", "language");
    QString lang = "";
     if(tmp.contains("en_US", Qt::CaseInsensitive)) {
        lang = ":/res/file/zone_en";//英文
     } else if(tmp.contains("bo_CN", Qt::CaseInsensitive)) {
        lang = ":/res/file/zone_bo";  //藏文  bug140681时区列表未藏化
     } else if(tmp.contains("zh_HK", Qt::CaseInsensitive)) {
         lang = ":/res/file/zone_hk";//中文繁体
     } else if(tmp.contains("mn_MN", Qt::CaseInsensitive)) {
         lang = ":/res/file/zone_mn";//蒙古语
     } else {
         lang = ":/res/file/zone_ch";//中文简体
      }
    return (QString)lang;
}

TimeZoneStructList loadZoneFile()
{
    TimeZoneStructList list;
    QFile file(getZoneList());
    if(file.open(QFile::ReadOnly)) {
        QTextStream filetext(&file);
        QStringList flist = filetext.readAll().split('\n');
        flist.removeLast();
        for(int j = 0; j < flist.length() - 2; j++) {
            QStringList str = flist.at(j).split(':');

            TimeZoneStruct item;
            item.encity = flist.at(j).split(';').at(1);
            item.country = str.at(0);
            QStringList tmp = str.at(2).split('/');
            item.zone = tmp.at(0);
//            if(str.at(2).split('/').length() >= 3) {
//                item.TZcity = str.at(2).split('/').at(2);
//            } else
//                item.TZcity = str.at(2).split('/').at(1);
            item.TZcity = tmp.at(tmp.length() - 1) + ":" + str.at(str.length() - 1).split(';').at(0);
            item.latitude = str.at(1).split('*').at(0);
            item.longitude = str.at(1).split('*').at(1);
            list.append(item);
        }
    } else {
        qCritical() << "fail to open the file";
    }
    return (TimeZoneStructList)list;
}

TimeZoneModel::TimeZoneModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_TZLists = loadZoneFile();
}

void TimeZoneModel::flushZone()
{
    m_TZLists.clear();
    m_TZLists = loadZoneFile();
}

void TimeZoneModel::setCurZone(int index)
{
    if(index < 0 || index >=m_TZLists.count()) {
        return;
    }
    m_curZone = m_TZLists.at(index);
    emit curZoneChanged();
}

TimeZoneStruct TimeZoneModel::getcurTimeZoneStruct()
{
    return m_curZone;
}

int TimeZoneModel::rowCount(const QModelIndex &parent) const
{
    // For list models only the root node (an invalid parent) should return the list's size. For all
    // other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
    if (!parent.isValid())
        return m_TZLists.length();
    // FIXME: Implement me!
}

QVariant TimeZoneModel::data(const QModelIndex &index, int role) const
{
    if(role != Qt::DisplayRole) {
        return QVariant();
    }
    if (!index.isValid()) {
        return QVariant();
    } else {
        return m_TZLists.at(index.row()).TZcity;
    }
}

QModelIndex TimeZoneModel::getIndexByTZCity(QString tzcity)
{
    for(int i = 0; i < m_TZLists.length(); i++) {
        if(m_TZLists.at(i).TZcity == tzcity) {
            setCurZone(i);
            return index(i);
        }
    }
    return QModelIndex();
}
//QModelIndex TimeZoneModel::getIndexByLatLong(QStringList latlon)
//{
//    for(int i = 0; i < m_TZLists.length(); i++) {
//        if(m_TZLists.at(i).TZcity == tzcity) {
//            return index(i);
//        }
//    }
//    return QModelIndex();
//}

QModelIndex TimeZoneModel::getIndexByCountryCode(QString code)
{
    for(int i = 0; i < m_TZLists.length(); i++) {
        if(m_TZLists.at(i).country == code) {
            setCurZone(i);
            return index(i);
        }
    }
    return QModelIndex();
}

TimeZoneStruct TimeZoneModel::getLatLongByTZCity(QModelIndex index)
{
    if(index.isValid()) {
        return m_TZLists.at(index.row());
    } else {
        return TimeZoneStruct();
    }
}

//QStringList TimeZoneModel::getTZCityByLatLong(QString lat, QString lon)
//{

//}




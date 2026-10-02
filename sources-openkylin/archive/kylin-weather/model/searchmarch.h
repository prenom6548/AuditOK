/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef SEARCHMARCH_H
#define SEARCHMARCH_H

#include <QList>
#include <QObject>
#include <QThread>
#include "global/corevar.h"
#include "controller/core/errorreact.h"
class SearchMarch;

class SearchThread : public QThread
{
    Q_OBJECT

public:
    explicit SearchThread(SearchMarch *parent);
    ~SearchThread() {}

    //从文件china-city-list.csv中查找子串，并将城市数据保存在LocationData结构体中
    void run() Q_DECL_OVERRIDE;
};


class SearchMarch : public QObject
{
    Q_OBJECT
public:
    explicit SearchMarch(QObject *parent = 0);
    ~SearchMarch();
    QString setAutomaticCity(const QString &cityName); //定位当前城市
    QString matchCityName(const QString &cityId); //定位当前城市
    //将匹配到的每个城市加入一个列表中
    QList<LocationData> exactMatchCity(const QString &inputText) const;
    QList<LocationData> m_locatonList;

private:
    friend class WorkerThread;

    SearchThread *m_searchThread = nullptr;
};

#endif // LOCATIONWORKER_H

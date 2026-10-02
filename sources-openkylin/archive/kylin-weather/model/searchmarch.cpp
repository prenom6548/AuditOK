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
#include "searchmarch.h"
#include <QDebug>
#include <QFile>

SearchThread::SearchThread(SearchMarch *parent) : QThread(parent) {}

void SearchThread::run()
{
    SearchMarch *m_worker = qobject_cast<SearchMarch *>(parent());
    if (!m_worker) {
        return;
    }
    // CN101250101,changsha,长沙,CN,China,中国,hunan,湖南,changsha,长沙,28.19409,112.98228,"430101,430100,430000",
    //本地词表--存储各地区的中英文名称，对应的cityId，经纬度等信息
    //在开始时，将本地词表中的所有的地区的信息m_locatonList列表中，元素为LocationData结构体
    QFile file(":/data/data/china-city-list.csv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        //读取行
        QString info = file.readLine();
        info = info.replace("\n", "");
        //去掉首行，没有数据的，遍历整个词表csv，将其存储在m_locatonList列表中
        while (!info.isEmpty()) {
            QStringList resultList = info.split(",");
            if (resultList.length() < 10) {
                info = file.readLine();
                info = info.replace("\n", "");
                continue;
            }

            QString id = resultList.at(0);
            if (!id.startsWith("CN")) {
                info = file.readLine();
                info = info.replace("\n", "");
                continue;
            }

            LocationData locationdata;
            locationdata.id = id.remove(0, 2); // remove "CN"
            locationdata.city_en = resultList.at(1).toLower().remove(QRegExp("\\s")).remove("'");
            locationdata.city = resultList.at(2).toLower().remove(QRegExp("\\s")).remove("'");
            locationdata.country_en = resultList.at(4);
            locationdata.country = resultList.at(5);
            locationdata.province_en = resultList.at(6).toLower().remove(QRegExp("\\s")).remove("'");
            locationdata.province = resultList.at(7).toLower().remove(QRegExp("\\s")).remove("'");
            locationdata.admin_district_en = resultList.at(8);
            locationdata.admin_district = resultList.at(9);
            locationdata.shorthand = resultList.last().toLower().remove(QRegExp("\\s")).remove("'");

            m_worker->m_locatonList << locationdata; //将数据存入m_locatonList中

            info = file.readLine();
            info = info.replace("\n", "");
        }
        file.close();
    }
}


SearchMarch::SearchMarch(QObject *parent) : QObject(parent)
{
    m_searchThread = new SearchThread(this);
    m_searchThread->start();
}

SearchMarch::~SearchMarch()
{
    m_searchThread->deleteLater();
}
//返回搜索结果
QList<LocationData> SearchMarch::exactMatchCity(const QString &inputText) const
{
    QList<LocationData> searchedList;
    if (m_locatonList.isEmpty()) {
        m_searchThread->start();
    }
    //搜索匹配-遍历m_locatonList
    for (const LocationData info : m_locatonList) {
        if (info.shorthand == inputText || info.id == inputText ||            //拼音和ID
            // info.province == inputText || info.province_en == inputText ||    //省
            // info.city.contains(inputText) || info.city_en.contains(inputText) //县
            info.city.startsWith(inputText) || info.city_en.startsWith(inputText) //县
        ) {
            //保证完全匹配的字符串放在显示的第一个
            if (inputText == info.city_en) {
                searchedList.insert(0, info);
            } else {
                searchedList.append(info); //将数据加入列表
            }
        }
    }
    return searchedList;
}
//自动定位-查找当前地区id
QString SearchMarch::setAutomaticCity(const QString &cityName)
{
    QString cityId;

    if (cityName.isEmpty()) {
        return cityId;
    }
    QFile file(":/data/data/china-city-list.csv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString info = file.readLine();

        info = info.replace("\n", "");
        while (!info.isEmpty()) {
            QStringList resultList = info.split(",");
            if (resultList.length() < 10) {
                info = file.readLine();
                info = info.replace("\n", "");
                continue;
            }
            //("CN101340406", "yunlin", "云林", "CN", "China", "中国", "taiwan", "台湾", "taizhong", "台中", "yl",
            //"23.718", "120.538", "710000")
            QString id = resultList.at(0);
            if (!id.startsWith("CN")) {
                info = file.readLine();
                info = info.replace("\n", "");
                continue;
            }
            //对比cityname和词表的city，拿id，去CN
            if (resultList.at(1).compare(cityName, Qt::CaseInsensitive) == 0
                || resultList.at(2).compare(cityName, Qt::CaseInsensitive) == 0
                || QString(resultList.at(2) + "市").compare(cityName, Qt::CaseInsensitive) == 0
                || QString(resultList.at(2) + "区").compare(cityName, Qt::CaseInsensitive) == 0
                || QString(resultList.at(2) + "县").compare(cityName, Qt::CaseInsensitive) == 0) {
                id.remove(0, 2); // remove "CN"
                cityId = id;
                break;
            }

            info = file.readLine();
            info = info.replace("\n", "");
        }
        file.close();
    }
    return cityId;
}

//自动定位-查找当前地区id
QString SearchMarch::matchCityName(const QString &ID)
{
    QString cityId = QString("CN%1").arg(ID);
    QString cityName;

    if (cityId.isEmpty()) {
        return cityName;
    }
    QFile file(":/data/data/china-city-list.csv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString info = file.readLine();

        info = info.replace("\n", "");
        while (!info.isEmpty()) {
            QStringList resultList = info.split(",");
            if (resultList.length() < 10) {
                info = file.readLine();
                info = info.replace("\n", "");
                continue;
            }
            //("CN101340406", "yunlin", "云林", "CN", "China", "中国", "taiwan", "台湾", "taizhong", "台中", "yl",
            //"23.718", "120.538", "710000")
            QString id = resultList.at(0);
            if (!id.startsWith("CN")) {
                info = file.readLine();
                info = info.replace("\n", "");
                continue;
            }
            //对比cityID和词表的city，拿name
            if (resultList.at(0).compare(cityId, Qt::CaseInsensitive) == 0) {
                cityName = resultList.at(2);
                break;
            }

            info = file.readLine();
            info = info.replace("\n", "");
        }
        file.close();
    }
    return cityName;
}

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
#ifndef DATAPARSER_H
#define DATAPARSER_H

#include <QObject>
#include <QJsonParseError>
#include <QJsonObject>
#include <QDebug>
#include <QTime>
#include <QJsonArray>
#include <QJsonValue>
#include "global/corevar.h"
#include "controller/core/errorreact.h"
class DataParser : public QObject
{
    Q_OBJECT
public:
    DataParser();
signals:
    void showLifeStyle(LifeStyle lifeStyle);           //发送信号，显示生活指数信息
    void showCurrent(CurrentWeather currentWeather);   //发送信号，显示当前城市信息
    void showLeftCity(CurrentWeather currentWeather);   //发送信号，显示左侧城市信息
    void showRightCity(CurrentWeather currentWeather);   //发送信号，显示右侧城市信息
    void showForcast(ForecastWeather forecastWeather); //发送信号，显示城市预测信息
    //    void showForcast(QString forecastWeather);//发送信号，显示城市预测信息
    void showCollectCode(QList<CollectItemData> collectItemList); //发送信号，显示收藏城市信息
    void showLAndRCode(QList<CurrentWeather> citysWeather); //发送信号，显示左右城市信息
    void showHourly(QVariant varHour);                            //逐小时天气信息
private:
    void lifeStyle(QJsonObject lifestyleObj);                                   //解析生活指数信息
    CurrentWeather currentWeather(QJsonObject weatherObj); //解析当前城市信息
    void forcastWeather(QJsonObject weatherObj, CurrentWeather currentWeather); //解析预报天气信息
    void setGsettings(CurrentWeather currentWeather);                           //存当前城市信息于配置文件
    void collectWeather(QString weatherList);                                   //解析当前城市信息
    QList<Hourly> hourlyWeather(QJsonObject hourlyObj);          //解析逐小时信息
    void setCollectValue(int i, const QJsonObject &json, ForecastWeather &forecastweather,
                         const QString &dateTime); //给预报天气的结构体的每一天赋值
    void dataAnalysis(QByteArray dataBa);

    CurrentWeather m_currentWeather;
    QList<Hourly> hourlist;
    bool success = true;
public slots:
    void dataReplyL(QByteArray dataBa);     //对左数据进行解析
    void dataReply(QByteArray dataBa);     //对发送来的数据进行解析
    void dataReplyR(QByteArray dataBa);     //对右数据进行解析
    void dataReplyColl(QByteArray dataBa); //对收藏城市数据进行解析
};

#endif // DATAPARSER_H

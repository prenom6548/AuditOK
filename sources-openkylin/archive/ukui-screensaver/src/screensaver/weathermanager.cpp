/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Authors: ZHAI Kang-ning <zhaikangning@kylinos.cn>
 **/
#include "weathermanager.h"
#include "scconfiguration.h"

#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonParseError>
#include <QTimer>
#include <QString>
#include <QEventLoop>
#include <QDebug>
#include <QtNetwork>
#include <QPixmap>

#define NO_CHECK_TIME
const QString httpPrefix = "http";
const QString weatherServerAddr = "://weather.ubuntukylin.com:8001/weather/api/3.0/heweather_data_s6/";
const QString defaultCityId = "101030100";     //"101030100" 默认天津
const int weatherReqInterval = 1000 * 60 * 60; // 定时更新天气,和麒麟天气保持一致

const QByteArray schemaWeather = "org.kylin-weather.settings";

static const QMap<QString, QString> weatherMap{ { "晴", "100" },           { "多云", "101" },
                                                { "少云", "102" },         { "晴间多云", "103" },
                                                { "阴", "104" },           { "有风", "200" },
                                                { "平静", "201" },         { "微风", "202" },
                                                { "和风", "203" },         { "清风", "204" },
                                                { "强风劲风", "205" },     { "疾风", "206" },
                                                { "大风", "207" },         { "烈风", "208" },
                                                { "风暴", "209" },         { "狂暴风", "210" },
                                                { "飓风", "211" },         { "龙卷风", "212" },
                                                { "热带风暴", "213" },     { "阵雨", "300" },
                                                { "强阵雨", "301" },       { "雷阵雨", "302" },
                                                { "强雷阵雨", "303" },     { "雷阵雨伴有冰雹", "304" },
                                                { "小雨", "305" },         { "中雨", "306" },
                                                { "大雨", "307" },         { "极端降雨", "308" },
                                                { "毛毛雨细雨", "309" },   { "暴雨", "310" },
                                                { "大暴雨", "311" },       { "特大暴雨", "312" },
                                                { "冻雨", "313" },         { "小到中雨", "314" },
                                                { "中到大雨", "315" },     { "大到暴雨", "316" },
                                                { "暴雨到大暴雨", "317" }, { "大暴雨到特大暴雨", "318" },
                                                { "雨", "399" },           { "小雪", "400" },
                                                { "中雪", "401" },         { "大雪", "402" },
                                                { "暴雪", "403" },         { "雨夹雪", "404" },
                                                { "雨雪天气", "405" },     { "阵雨夹雪", "406" },
                                                { "阵雪", "407" },         { "小到中雪", "408" },
                                                { "中到大雪", "409" },     { "大到暴雪", "410" },
                                                { "雪", "499" },           { "薄雾", "500" },
                                                { "雾", "501" },           { "霾", "502" },
                                                { "扬沙", "503" },         { "浮尘", "504" },
                                                { "沙尘暴", "507" },       { "强沙尘暴", "508" },
                                                { "大雾", "509" },         { "强浓雾", "510" },
                                                { "中度霾", "511" },       { "重度霾", "512" },
                                                { "严重霾", "513" },       { "大雾", "514" },
                                                { "特强浓雾", "515" },     { "热", "900" },
                                                { "冷", "901" },           { "未知", "999" } };

WeatherManager::WeatherManager(QObject *parent) : QObject(parent), configuration(SCConfiguration::instance())
{
    // 初始化m_net_manager请求天气
    m_net_manager = new QNetworkAccessManager(this);
    QObject::connect(m_net_manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(replyFinished(QNetworkReply *)));
    m_timer = new QTimer(this);
    m_timer->setInterval(weatherReqInterval);
    connect(m_timer, &QTimer::timeout, this, &WeatherManager::weatherRequest);

    m_local_weather_info = new LocalWeatherInfo(this);
}

void WeatherManager::initNetworkStatus()
{
    m_iNetworkConnectState = configuration->getNetworkConnectState();

    onNetworkStateChanged(m_iNetworkConnectState);
    connect(configuration, &SCConfiguration::networkConnectStateChanged, this, &WeatherManager::onNetworkStateChanged);
    connect(configuration, &SCConfiguration::weatherDataChanged, this, [=]() {
        onNetworkStateChanged(m_iNetworkConnectState);
    });
}

void WeatherManager::onNetworkStateChanged(uint state)
{
    qDebug() << state;
    m_iNetworkConnectState = state;
    if (m_iNetworkConnectState == 4)
        getWeather();
    else
        emit onNetworkError();
}

void WeatherManager::getWeather()
{
    weatherRequest();
    if (m_timer != NULL) {
        m_timer->start();
    }
    return;
}

void WeatherManager::weatherRequest()
{
    if (updateLocation()) {
        return;
    }
}

bool WeatherManager::updateLocation()
{
    if (getLogcalWeather()) {
        qDebug() << "getCityName = " << m_local_weather_info->getCityName()
                 << "getCondText = " << m_local_weather_info->getCondText()
                 << "getTemperature = " << m_local_weather_info->getTemperature();
        emit onWeatherUpdate(
            m_local_weather_info->getCityName(),
            m_local_weather_info->getCondText(),
            m_local_weather_info->getTemperature());
        QNetworkRequest request(
            QUrl(QString("%1%2").arg(httpPrefix).arg(weatherServerAddr) + m_local_weather_info->getCityId() + "/"));
        m_net_manager->get(request);
        return true;
    }

    return false;
}

bool WeatherManager::getLogcalWeather()
{
    QString weather = configuration->getWeatherData();
    if (!weather.isEmpty()) {
        //"1920-08-27 10:17:42,101310204,澄迈,小雨,95%，25℃,北风,1级,"
        // 时间，城市编码，城市名称，天气，湿度，温度，风向，风力
        QStringList weatherInfoList = weather.split(",");
        qDebug() << "weather = " << weather;
        if (weatherInfoList.size() < 9)
            return false;
        m_local_weather_info->setTime(weatherInfoList.at(0));

#ifndef NO_CHECK_TIME
        if (!m_local_weather_info->isTimeValid()) {
            emit onGetWeatherFailed();
            return false;
        }
#endif
        m_local_weather_info->setCityId(weatherInfoList.at(1));
        m_local_weather_info->setCityName(weatherInfoList.at(2));
        m_local_weather_info->setCondText(weatherInfoList.at(3));
        m_local_weather_info->setAirHumidity(weatherInfoList.at(4));
        m_local_weather_info->setTemperature(weatherInfoList.at(5));
        m_local_weather_info->setWindDirection(weatherInfoList.at(6));
        m_local_weather_info->setWindForce(weatherInfoList.at(7));
        return true;
    }

    return false;
}

QString WeatherManager::getLogcalCityId()
{
    if (m_settings != nullptr) {
        QString citys = m_settings->get("citylist").toString(); //"101010100,"
        if (citys.isEmpty())
            return "";

        QStringList cityList = citys.split(",");
        if (cityList.size() >= 1) {
            QString s = cityList.at(0);
            qDebug() << "local city id = " << s;
            return s;
        }
    }
    return "";
}

void WeatherManager::replyFinished(QNetworkReply *reply)
{
    if (reply != nullptr && reply->error() != QNetworkReply::NoError) {
        qWarning() << "[WeatherManager][replyFinished] get weather error:(" << reply->error() << ")"
                   << reply->errorString();
        if (m_networkTryNum < 15) {
            m_networkTryNum++;
            QTimer::singleShot(1000, this, [=] { weatherRequest(); });
        } else {
            m_networkTryNum = 0;
        }
        emit onGetWeatherIconFinished();
        return;
    }

    // 注：天气信息只解析了锁屏需要展示的部分
    QByteArray BA;
    QJsonDocument JD;
    QJsonParseError JPE;

    BA = reply->readAll();

    JD = QJsonDocument::fromJson(BA, &JPE);
    if (JPE.error == QJsonParseError::NoError) {
        if (JD.isObject()) {
            QJsonObject kylinWeatherObj = JD.object().value("KylinWeather").toObject();

            QString nowWeather = kylinWeatherObj.value("weather").toObject().value("now").toString();
            m_city_name = kylinWeatherObj.value("weather").toObject().value("location").toString();
            QStringList nowList = nowWeather.split(",");
            for (QString now : nowList) {
                if (now.contains("cond_txt")) {
                    m_cond_txt = now.mid(9);
                }

                if (now.contains("tmp")) {
                    m_temperature = now.mid(4) + "°C";
                }
            }
            if (nowWeather != "" && nowWeather.contains(",", Qt::CaseInsensitive)) {
                QStringList strList = nowWeather.split(",");
                QJsonObject m_json;
                foreach (QString str, strList) {
                    if (str != "") {
                        m_json.insert(str.split("=").at(0), str.split("=").at(1));
                    }
                }

                m_cond_code = m_json.value("cond_code").toString();
                qDebug() << "Function Positioning  解析当前城市信息: " << m_cond_code << m_city_name;
            }
            emit onGetWeatherIconFinished();
        }
    } else {
        qWarning() << "get weather info error : " << JPE.errorString();
        emit onGetWeatherIconFinished();
    }

    reply->deleteLater();
}

QString WeatherManager::getWeatherIcon()
{
    if (!m_cond_code.isEmpty()) {
        return (":/weather/assets/weather-icon/SVG/" + m_cond_code + ".svg");
    } else {
        return (":/weather/assets/weather-icon/SVG/999.svg");
    }
}

QPixmap WeatherManager::getWeatherStateIcon()
{
    QPixmap weatherStateIcon;
    if (m_cond_code.isEmpty()) {
        weatherStateIcon = QPixmap(":/assets/underlying.png");
    } else {
        int weatherNum = m_cond_code.toInt();
        weatherStateIcon = QPixmap(getWeatherIconPath(weatherNum));
    }
    return weatherStateIcon;
}

QString WeatherManager::getWeatherIconPath(int weatherCode)
{
    static const QMap<int, QString> specialCases = { { 100, ":/assets/qing.png" },
                                                     { 150, ":/assets/qing.png" },
                                                     { 103, ":/assets/duoyun.png" },
                                                     { 153, ":/assets/duoyun.png" } };

    if (specialCases.contains(weatherCode)) {
        return specialCases[weatherCode];
    }

    if (weatherCode > 100 && weatherCode < 200)
        return ":/assets/ying.png";
    if (weatherCode > 199 && weatherCode < 300)
        return ":/assets/feng.png";
    if (weatherCode > 299 && weatherCode < 400)
        return ":/assets/yu.png";
    if (weatherCode > 399 && weatherCode < 500)
        return ":/assets/wu.png";
    if (weatherCode > 499 && weatherCode < 600)
        return ":/assets/xue.png";

    return ":/assets/underlying.png";
}

QPixmap WeatherManager::getWeatherIcon(QString cond)
{
    if (cond.isEmpty()) {
        qWarning() << "cond info is unknown";
        return QPixmap(":/weather/assets/weather-icon/999.svg").scaled(32, 32);
    }

    // 根据m_cond_txt
    QString numStr = weatherMap.value(cond);
    if (!numStr.isEmpty()) {
        qDebug() << "numStr=" + numStr;
        return QPixmap(":/weather/assets/weather-icon/" + numStr + ".svg").scaled(32, 32);
    }

    qWarning() << "天气为|" << cond << "|";
    return QPixmap(":/weather/assets/weather-icon/999.svg").scaled(32, 32);
}

QString WeatherManager::getCityName()
{
    return "";
}

QString WeatherManager::getCond()
{
    return "";
}

QString WeatherManager::getTemperature()
{
    return "";
}

LocalWeatherInfo::LocalWeatherInfo(QObject *parent) {}

bool LocalWeatherInfo::isTimeValid()
{
    if (m_update_time != nullptr && !m_update_time.isEmpty()) {
//        QDateTime time = QDateTime::fromString(m_update_time, "yyyy-MM-dd hh:mm");
        QString datetimePart = m_update_time.split(' ').first();  // "2025-12-05T11:15:00+08:00 13:00"  第一个数据为服务器更新的时间
        QDateTime time = QDateTime::fromString(datetimePart, Qt::ISODate);
        QDateTime currentTime = QDateTime::currentDateTime();
        if (!time.isValid())
            return false;

        uint timeInterval = currentTime.toTime_t() - time.toTime_t();
        uint futureInterval = time.toTime_t() - currentTime.toTime_t();
        if ((timeInterval <= 721 * 60) || (futureInterval <= 721 * 60)) {
            // 天气应用自启动后自动刷新的定时器为12h（如果在没有手动点天气应用的情况下），加入1分钟容错机制
            return true;
        }
    }
    return false;
}

void LocalWeatherInfo::setTime(QString time)
{
    m_update_time = time;
}

QString LocalWeatherInfo::getTime()
{
    return m_update_time;
}

void LocalWeatherInfo::setCityId(QString cityId)
{
    m_city_id = cityId;
}

QString LocalWeatherInfo::getCityId()
{
    return m_city_id;
}

void LocalWeatherInfo::setCityName(QString cityName)
{
    m_city_name = cityName;
}

QString LocalWeatherInfo::getCityName()
{
    return m_city_name;
}

void LocalWeatherInfo::setCondText(QString condText)
{
    m_cond_text = condText;
}

QString LocalWeatherInfo::getCondText()
{
    return m_cond_text;
}

void LocalWeatherInfo::setAirHumidity(QString airHumidity)
{
    m_air_humidity = airHumidity;
}

QString LocalWeatherInfo::getAirHumidity()
{
    return m_air_humidity;
}

void LocalWeatherInfo::setTemperature(QString temperature)
{
    m_temperature = temperature;
}

QString LocalWeatherInfo::getTemperature()
{
    return m_temperature;
}

void LocalWeatherInfo::setWindDirection(QString windDirection)
{
    m_wind_direction = windDirection;
}

QString LocalWeatherInfo::getWindDirection()
{
    return m_wind_direction;
}

void LocalWeatherInfo::setWindForce(QString windForce)
{
    m_wind_force = windForce;
}

QString LocalWeatherInfo::getWindForce()
{
    return m_wind_force;
}

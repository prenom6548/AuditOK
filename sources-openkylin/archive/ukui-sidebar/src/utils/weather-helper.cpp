/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#include "weather-helper.h"

#include "app-manager.h"
#include <QVariant>
#include <QDebug>
#include <QProcess>

#define KYLIN_WEATHER_SETTING                 "org.kylin-weather.settings"
#define WEATHER_KEY                           "weather"
using namespace Sidebar;
WeatherHelper::WeatherHelper(QObject *parent) : QObject(parent)
{
    const QByteArray id(KYLIN_WEATHER_SETTING);
    if(QGSettings::isSchemaInstalled(id)) {
        m_gsettings = new QGSettings(id);

        if(m_gsettings->keys().contains(WEATHER_KEY, Qt::CaseInsensitive)) {
            extractWeatherInfo(m_gsettings->get(WEATHER_KEY).toString());
        } else {
            qWarning() << "WeatherHelper:" << "can't find key :" << WEATHER_KEY;
        }

        connect(m_gsettings, &QGSettings::changed, this, [ & ](const QString & key) {
            if(key == WEATHER_KEY) {
                if(extractWeatherInfo(m_gsettings->get(WEATHER_KEY).toString())) {
                    Q_EMIT weatherInfoChanged(m_weather, m_icon);
                }
            }
        });
    } else {
        qWarning() << "WeatherHelper:" << "can't find gsettings :" << KYLIN_WEATHER_SETTING;
    }
}

QString WeatherHelper::getWeather()
{
    return m_weather;
}

QString WeatherHelper::getIcon()
{
    return m_icon;
}

void WeatherHelper::openWeather()
{
    Sidebar::AppManager::getInstance(this)->launchApp("/usr/share/applications/kylin-weather.desktop", "kylin-weather");
}

bool WeatherHelper::extractWeatherInfo(QString weatherInfo)
{
    QStringList weatherInfoList = weatherInfo.split(",");
    if(weatherInfoList.size() < 7) {
        qWarning() << "WeatherHelper:" << "Get Weather info error!";
        return false;
    }
    m_weather = weatherInfoList.at(2) + weatherInfoList.at(3) + weatherInfoList.at(5);
    m_icon = weatherInfoList.at(8);
    return true;
}

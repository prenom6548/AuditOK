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

#ifndef WEATHERHELPER_H
#define WEATHERHELPER_H

#include <QObject>
#include <QGSettings>

namespace Sidebar {

class WeatherHelper : public QObject
{
    Q_OBJECT
public:
    explicit WeatherHelper(QObject *parent = nullptr);
    Q_INVOKABLE QString getWeather();
    Q_INVOKABLE QString getIcon();

public Q_SLOTS:
    void openWeather();

Q_SIGNALS:
    void weatherInfoChanged(QString weather, QString icon);

private:
    bool extractWeatherInfo(QString weatherInfo);

    QGSettings *m_gsettings = nullptr;
    QString m_weather;
    QString m_icon;

};
}
#endif // WEATHERHELPER_H

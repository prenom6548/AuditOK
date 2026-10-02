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
#include "city.h"
#include <QHBoxLayout>
#include <QThread>
#include <QGraphicsOpacityEffect>
extern double detio;

const int TEMP_NUM_FONT_SIZE = 116;
const int TEMP_SIGN_FONT_SIZE = 80;
const int WEATHER_FONT_SIZE = 35;
const int HUMIDITY_FONT_SIZE = 16;
const int TEMP_NUM_FONT_SIZE_PAD = 200;
const int TEMP_SIGN_FONT_SIZE_PAD = 100;
const int WEATHER_FONT_SIZE_PAD = 60;
const int HUMIDITY_FONT_SIZE_PAD = 24;

city::city(QWidget *parent) : QWidget(parent)
{    
    initWidget();
    initWidgetStyle();
    initLayout();
    initWidgetSize();

}

void city::initWidget()
{
    this->setAttribute(Qt::WA_TranslucentBackground);

    m_temperatureNum = new QLabel(this);  // 摄氏度数字
    m_temperatureNum->setAccessibleName("kylin-weather_QLabel_City_temperatureNum");
    m_degreeNotation = new QLabel(this);  // 摄氏度符号
    m_degreeNotation->setAccessibleName("kylin-weather_QLabel_City_degreeNotation");
    m_weatherPicture = new QLabel(this);  // 符号图标
    m_weatherPicture->setAccessibleName("kylin-weather_QLabel_City_weatherPicture");
    m_weather = new QLabel(this);         // 天气状态
    m_weather->setAccessibleName("kylin-weather_QLabel_City_weather");
    m_humidity = new QLabel(this);        // 湿度
    m_humidity->setAccessibleName("kylin-weather_QLabel_City_humidity");
    m_windForce = new QLabel(this);       // 风向
    m_windForce->setAccessibleName("kylin-weather_QLabel_City_windForce");
    m_updateTime = new QLabel(this);      // 更新时间
    m_updateTime->setAccessibleName("kylin-weather_QLabel_City_updatetime");

    // 布局
    m_cityLayout = new QHBoxLayout();            
    m_cityWidgetLayout = new QHBoxLayout();      // 城市信息布局（横）
    m_cityDataVLayout = new QVBoxLayout();       // 城市信息布局（纵）
    m_leftLayout = new QHBoxLayout();            // 左侧天气摄氏度布局
    m_rightLayout = new QVBoxLayout();           // 右侧数据布局
    m_iconAndWeatherLayout = new QHBoxLayout();  // 天气状态布局
    m_humidityAndWindLayout = new QHBoxLayout(); // 天气湿度和风向布局

    return;
}

void city::initWidgetStyle()
{
    m_temperatureNum->setText("≡");
    m_temperatureNum->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_temperatureNum->setStyleSheet("color: white");

    m_degreeNotation->setText("°");
    m_degreeNotation->setAlignment(Qt::AlignTop);
    m_degreeNotation->setStyleSheet("color: white");

    //天气图标
    path = ":/res/weather-icon/PNG48/1x/999.png";

    QPixmap pixmap(path);
    pixmap = pixmap.scaled(m_weatherPicture->width(), m_weatherPicture->height(), Qt::IgnoreAspectRatio,
                           Qt::SmoothTransformation);
    m_weatherPicture->setPixmap(pixmap);
    m_weatherPicture->setAlignment(Qt::AlignCenter);


    m_weather->setText("N/A");
    m_weather->setAlignment(Qt::AlignCenter);
    m_weather->setStyleSheet("color: white");

    m_humidity->setText("N/A");
    m_humidity->setAlignment(Qt::AlignTop);
    m_humidity->setStyleSheet("color: white");

    m_windForce->setText("N/A");
    m_windForce->setAlignment(Qt::AlignTop);
    m_windForce->setStyleSheet("color: white");

    m_updateTime->setText("N/A");
    m_updateTime->setAlignment(Qt::AlignTop);
    m_updateTime->setStyleSheet("color: white");

    return;
}

void city::initLayout()
{
    m_leftLayout->setMargin(0);
    m_leftLayout->setSpacing(0);
    m_leftLayout->addWidget(m_temperatureNum);
    m_leftLayout->addSpacing(5);
    m_leftLayout->addWidget(m_degreeNotation);
    m_leftLayout->setContentsMargins(0, 0, 0, 20);

    m_iconAndWeatherLayout->addWidget(m_weatherPicture);
    m_iconAndWeatherLayout->addSpacing(5);
    m_iconAndWeatherLayout->addWidget(m_weather);
    m_iconAndWeatherLayout->addStretch();
    m_iconAndWeatherLayout->setContentsMargins(0, 0, 0, 0);

    m_humidityAndWindLayout->addWidget(m_humidity);
    m_humidityAndWindLayout->addSpacing(32);
    m_humidityAndWindLayout->addWidget(m_windForce);
    m_humidityAndWindLayout->addStretch();
    m_humidityAndWindLayout->setContentsMargins(0, 15, 0, 0);

    m_rightLayout->addStretch();
    m_rightLayout->addLayout(m_iconAndWeatherLayout);
    m_rightLayout->addSpacing(5);
    m_rightLayout->addLayout(m_humidityAndWindLayout);
    m_rightLayout->addSpacing(-3);
    m_rightLayout->addWidget(m_updateTime);
    m_rightLayout->addStretch();
    m_rightLayout->setContentsMargins(0, 10, 0, 0);

    m_cityWidgetLayout->addStretch();
    m_cityWidgetLayout->addLayout(m_leftLayout);
    m_cityWidgetLayout->addSpacing(16);
    m_cityWidgetLayout->addLayout(m_rightLayout);
    m_cityWidgetLayout->addStretch();
    m_cityWidgetLayout->setContentsMargins(0, 0, 0, 0);

    m_cityDataVLayout->addLayout(m_cityWidgetLayout);
    m_cityDataVLayout->addStretch();
    m_cityDataVLayout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(m_cityDataVLayout);
    
    return;
}

void city::PCModeWidgetStyle()
{
    // 设置 摄氏度 字号大小
    m_font.setPixelSize(TEMP_NUM_FONT_SIZE);
    m_temperatureNum->setFont(m_font);
    QFontMetrics fontmts = QFontMetrics(m_temperatureNum->font());
    m_temperatureNum->setFixedWidth(fontmts.width(m_temperatureNum->text()));
    // 设置显示白色字体
    QPalette temperatureNumPalette = m_temperatureNum->palette();
    temperatureNumPalette.setColor(QPalette::WindowText, Qt::white);
    m_temperatureNum->setPalette(temperatureNumPalette);

    // 设置 摄氏度图标 字号大小
    m_font.setPixelSize(TEMP_SIGN_FONT_SIZE);
    m_degreeNotation->setFont(m_font);
    // 设置显示白色字体
    QPalette degreeNotationPalette = m_degreeNotation->palette();
    degreeNotationPalette.setColor(QPalette::WindowText, Qt::white);
    m_degreeNotation->setPalette(degreeNotationPalette);
        
    // 天气图标
    m_weatherPicture->setFixedSize(ViewVar::CITY_ICON);
//    m_weatherPicture->setIconSize(ViewVar::CITY_ICON);
      
    // 设置 天气状态 字号大小  
    m_font.setPixelSize(WEATHER_FONT_SIZE);
    m_weather->setFont(m_font);
    // 设置显示白色字体
    QPalette weatherPalette = m_weather->palette();
    weatherPalette.setColor(QPalette::WindowText, Qt::white);
    m_weather->setPalette(weatherPalette);

    m_font.setPixelSize(HUMIDITY_FONT_SIZE);

    // 设置显示白色字体
    QPalette humidityPalette = m_humidity->palette();
    humidityPalette.setColor(QPalette::WindowText, Qt::white);
    m_humidity->setPalette(humidityPalette);
    m_humidity->setFont(m_font);
    // 设置字体透明度
//    QGraphicsOpacityEffect *humidityOpacityEffect = new QGraphicsOpacityEffect;
//    humidityOpacityEffect->setOpacity(0.7);
//    m_humidity->setGraphicsEffect(humidityOpacityEffect);

    // 设置显示白色字体
    QPalette windForcePalette = m_windForce->palette();
    windForcePalette.setColor(QPalette::WindowText, Qt::white);
    m_windForce->setPalette(windForcePalette);
    m_windForce->setFont(m_font);
    // 设置字体透明度
//    QGraphicsOpacityEffect *windForceOpacityEffect = new QGraphicsOpacityEffect;
//    windForceOpacityEffect->setOpacity(0.7);
//    m_windForce->setGraphicsEffect(windForceOpacityEffect);

    m_font.setPixelSize(12);
    m_updateTime->setPalette(windForcePalette);
    m_updateTime->setFont(m_font);
    QGraphicsOpacityEffect *updateTimeOpacityEffect = new QGraphicsOpacityEffect;
    updateTimeOpacityEffect->setOpacity(0.7);
    m_updateTime->setGraphicsEffect(updateTimeOpacityEffect);
    return;
}

void city::PadModeWidgetStyle()
{
    // 设置 摄氏度 字号大小
    m_font.setPixelSize(TEMP_NUM_FONT_SIZE_PAD);
    m_temperatureNum->setFont(m_font);
    QFontMetrics fontmts = QFontMetrics(m_temperatureNum->font());
    m_temperatureNum->setFixedWidth(fontmts.width(m_temperatureNum->text()));
    // 设置显示白色字体
    QPalette temperatureNumPalette = m_temperatureNum->palette();
    temperatureNumPalette.setColor(QPalette::WindowText, Qt::white);
    m_temperatureNum->setPalette(temperatureNumPalette);

    // 设置 摄氏度图标 字号大小
    m_font.setPixelSize(TEMP_SIGN_FONT_SIZE_PAD);
    m_degreeNotation->setFont(m_font);
    // 设置显示白色字体
    QPalette degreeNotationPalette = m_degreeNotation->palette();
    degreeNotationPalette.setColor(QPalette::WindowText, Qt::white);
    m_degreeNotation->setPalette(degreeNotationPalette);
        
    // 天气图标
    m_weatherPicture->setFixedSize(ViewVar::CITY_ICON_MAXSIZE);
//    m_weatherPicture->setIconSize(ViewVar::CITY_ICON_MAXSIZE);
      
    // 设置 天气状态 字号大小  
    m_font.setPixelSize(WEATHER_FONT_SIZE_PAD);
    m_weather->setFont(m_font);
    // 设置显示白色字体
    QPalette weatherPalette = m_weather->palette();
    weatherPalette.setColor(QPalette::WindowText, Qt::white);
    m_weather->setPalette(weatherPalette);

    // 设置字体透明度
    QGraphicsOpacityEffect *opacityEffect = new QGraphicsOpacityEffect;
    opacityEffect->setOpacity(0.7);
    m_font.setPixelSize(HUMIDITY_FONT_SIZE_PAD);

    // 设置显示白色字体
    QPalette humidityPalette = m_humidity->palette();
    humidityPalette.setColor(QPalette::WindowText, Qt::white);
    m_humidity->setPalette(humidityPalette);
    m_humidity->setFont(m_font);
    // 设置字体透明度
    QGraphicsOpacityEffect *humidityOpacityEffect = new QGraphicsOpacityEffect;
    humidityOpacityEffect->setOpacity(0.7);
    m_humidity->setGraphicsEffect(humidityOpacityEffect);

    // 设置显示白色字体
    QPalette windForcePalette = m_windForce->palette();
    windForcePalette.setColor(QPalette::WindowText, Qt::white);
    m_windForce->setPalette(windForcePalette);
    m_windForce->setFont(m_font);
    // 设置字体透明度
    QGraphicsOpacityEffect *windForceOpacityEffect = new QGraphicsOpacityEffect;
    windForceOpacityEffect->setOpacity(0.7);
    m_windForce->setGraphicsEffect(windForceOpacityEffect);
    return;
}

void city::resizeEvent(QResizeEvent *event) {}

void city::getCurrentInfor(CurrentWeather m_currentWeather)
{
    code = m_currentWeather.cond_code;
    //天气图标
    path = ":/res/weather-icon/PNG48/1x/" + code + ".png";

    QPixmap pixmap(path);
    pixmap = pixmap.scaled(m_weatherPicture->width(), m_weatherPicture->height(), Qt::IgnoreAspectRatio,
                           Qt::SmoothTransformation);
    m_weatherPicture->setPixmap(pixmap);



    if (m_currentWeather.tmp.length() == 1) {
        m_temperatureNum->setText(m_currentWeather.tmp);
    } else {
        m_temperatureNum->setText(m_currentWeather.tmp);
    }
    
    QFontMetrics fontmts = QFontMetrics(m_temperatureNum->font());
    m_temperatureNum->setFixedWidth(fontmts.width(m_temperatureNum->text()));

    // m_curCity->setText(m_currentWeather.city);
    m_weather->setText(m_currentWeather.cond_txt);
    m_humidity->setText("湿度" + m_currentWeather.hum + "%");
    m_windForce->setText("" + m_currentWeather.wind_dir + m_currentWeather.wind_sc + "级");
    if(CoreVar::HAS_NETWORK){
        QString updateTime = QDateTime::currentDateTime().toString("hh:mm");
        m_updateTime->setText("更新时间：" + updateTime);
    }else{
        m_updateTime->setText("N/A");
    }

    qDebug() << " ===== INFO ===== current Info ===== " <<__FILE__<< ","<<__FUNCTION__<<","<<__LINE__;
}

void city::isFullScreen(int m_isFullScreen)
{
    m_judgeFullScreen = m_isFullScreen;
}

void city::initConnect() 
{
    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPc, this, &city::slotChangePCMode);
    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPadH, this, &city::slotChangePadHMode);
    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPadV, this, &city::slotChangePadVMode);
}

void city::initWidgetSize()
{
    switch (ViewVar::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
            slotChangePCMode();
            break;
        case CurrentMode::PadHMode:
            slotChangePadHMode();
            break;    
        case CurrentMode::PadVMode:
            slotChangePadVMode();
            break;
        default:
            slotChangePCMode();
            break;
    }
    return;
}

void city::slotChangePCMode()
{
    PCModeWidgetStyle();
    m_cityDataVLayout->setContentsMargins(0, 0, 0, 0);

    return;
}

void city::slotChangePadVMode()
{
    PadModeWidgetStyle();
    m_cityDataVLayout->setContentsMargins(0, 0, 0, 0);

    return;
}

void city::slotChangePadHMode()
{
    PadModeWidgetStyle();
    m_cityDataVLayout->setContentsMargins(0, 0, 0, 0);
    
    return;
}

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
#include "information.h"

information::information(QWidget *parent) : QWidget(parent)
{
    m_informationLayout = new QVBoxLayout(this);
    m_perhour = new perhour(this);
    m_perhour->setAccessibleName("kylin-weather_Information_perhour");
    m_weekweather = new weekweather(this);
    m_weekweather->setAccessibleName("kylin-weather_Information_weekweather");
    m_lifestyle = new lifestyle(this);
    m_lifestyle->setAccessibleName("kylin-weather_Information_lifestyle");

    m_hoursTipLabel = new QLabel(this);
    m_hoursTipLabel->setAccessibleName("kylin-weather_QLabel_Information_hoursTipLabel");
    m_dayTipLabel = new QLabel(this);
    m_dayTipLabel->setAccessibleName("kylin-weather_QLabel_Information_dayTipLabel");
    m_lifeTipLabel = new QLabel(this);
    m_lifeTipLabel->setAccessibleName("kylin-weather_QLabel_Information_lifeTipLabel");
    
    // 创建40日天气预报链接
    m_40DayLinkLabel = new QLabel(this);
    m_40DayLinkLabel->setAccessibleName("kylin-weather_QLabel_Information_40DayLinkLabel");
    m_dayTipLayout = new QHBoxLayout();
    
    QPalette hoursTipPalette = m_hoursTipLabel->palette();
    hoursTipPalette.setColor(QPalette::WindowText, Qt::white);
    m_hoursTipLabel->setPalette(hoursTipPalette);

    QPalette dayTipPalette = m_dayTipLabel->palette();
    dayTipPalette.setColor(QPalette::WindowText, Qt::white);
    m_dayTipLabel->setPalette(dayTipPalette);

    QPalette lifeTipPalette = m_lifeTipLabel->palette();
    lifeTipPalette.setColor(QPalette::WindowText, Qt::white);
    m_lifeTipLabel->setPalette(lifeTipPalette);

    m_hoursTipLabel->setText(tr("Hourly Weather")); //  时刻天气
    m_hoursTipLabel->setStyleSheet("color: white");
    m_dayTipLabel->setText(tr("Daily Weather")); // 逐日天气
    m_dayTipLabel->setStyleSheet("color: white");
    m_lifeTipLabel->setText(tr("Life Index")); // 生活指数
    m_lifeTipLabel->setStyleSheet("color: white");
    
    // 设置40日天气预报链接
    m_40DayLinkLabel->setText(tr("Check weather forecast for the next 15/40 days") + " >  ");
    m_40DayLinkLabel->setStyleSheet("color: white");
    m_40DayLinkLabel->setCursor(Qt::PointingHandCursor); // 设置鼠标悬停为手型
    m_40DayLinkLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter); // 右对齐

    m_dataSource = new QLabel(this);
    m_dataSource->setAccessibleName("kylin-weather_QLabel_Information_dataSource");
    m_dataSource->setText(tr("Data Source: Weather China") + "  ");
    m_dataSource->setStyleSheet("color: white");
    m_dataSource->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    initLayout();
    initQss();
    initConnect();
}

void information::initLayout()
{
    // 设置逐日天气标题行的水平布局
    m_dayTipLayout->addWidget(m_dayTipLabel);
    m_dayTipLayout->addStretch(); // 添加弹性空间，将链接推到右侧
    m_dayTipLayout->addWidget(m_40DayLinkLabel);
    m_dayTipLayout->setMargin(0);
    m_dayTipLayout->setSpacing(0);
    m_dayTipLayout->setContentsMargins(0, 0, 0, 0);
    m_informationLayout->addWidget(m_hoursTipLabel);
    m_informationLayout->addSpacing(BELOW_SPACING);
    m_informationLayout->addWidget(m_perhour);
    m_informationLayout->addSpacing(TOP_SPACING);
    m_informationLayout->addLayout(m_dayTipLayout); // 使用水平布局替代单独的label
    m_informationLayout->addSpacing(BELOW_SPACING);
    m_informationLayout->addWidget(m_weekweather);
    m_informationLayout->addSpacing(TOP_SPACING);
    m_informationLayout->addWidget(m_lifeTipLabel);
    m_informationLayout->addSpacing(BELOW_SPACING);
    m_informationLayout->addWidget(m_lifestyle);
    m_informationLayout->addWidget(m_dataSource);
    m_informationLayout->addSpacing(5);
    m_informationLayout->setMargin(0);
    m_informationLayout->setSpacing(0);
    m_informationLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(m_informationLayout);
}

void information::initQss()
{
    //    this->setStyleSheet("QWidget{background:blue;}");
    //        m_perhour->setStyleSheet("QWidget{background:red;}");
}

void information::initConnect()
{
    connect(this, &information::getLifeStyleInfor, m_lifestyle, &lifestyle::getLifeStyleInfor);
    connect(this, &information::sendIsFullScreen, m_perhour, &perhour::sendIsFullScreen);
    connect(this, &information::sendHourlyInfor, m_perhour, &perhour::setHourlyInfor);
    connect(this, &information::sendIsFullScreen, m_perhour, &perhour::setHourlyIcon);
    connect(this, &information::sendIsFullScreen, m_weekweather, &weekweather::sendIsFullScreen);
    // 判断使用普通大小界面图标还是全屏大小图标
    connect(this, &information::sendIsFullScreen, m_weekweather, &weekweather::setWeatherIcon);
    connect(this, &information::sendIsFullScreen, m_lifestyle, &lifestyle::sendIsFullScreen);
    connect(this, &information::sendForcastInfor, m_weekweather, &weekweather::getForcastInfor);
    connect(this, &information::sendIsFullScreen, m_lifestyle, &lifestyle::sendIsFullScreen);
    connect(this, &information::getLifeStyleInfor, m_lifestyle, &lifestyle::getLifeStyleInfor);
    
    // 为40日天气预报链接安装事件过滤器
    m_40DayLinkLabel->installEventFilter(this);
}

void information::resizeEvent(QResizeEvent *event) {}

bool information::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_40DayLinkLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            on40DayLinkClicked();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void information::on40DayLinkClicked()
{
    QString cityId = CoreVar::CURRENT_CITY_ID;
    if(cityId.isEmpty())
        cityId = CoreVar::DEFAULT_CITYID;

    // 打开40日天气预报网页
    QString url = QString("https://www.weather.com.cn/weather40d/%1.shtml").arg(cityId);
    QDesktopServices::openUrl(QUrl(url));
}

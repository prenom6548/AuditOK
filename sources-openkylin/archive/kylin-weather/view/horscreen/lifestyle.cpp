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
#include <QPainter>
#include <QStyleOption>
#include "lifestyle.h"

lifestyle::lifestyle(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::FramelessWindowHint);  //屏蔽窗口，只看widget部分的效果
	setAttribute(Qt::WA_TranslucentBackground, true);
    this->setFixedSize(856, 120);

    m_lifestyleLayout = new QHBoxLayout(this);

    m_ultravioletrays = new ultravioletrays(this);
    m_ultravioletrays->setAccessibleName("kylin-weather_Lifestyle_ultravioletrays");
    m_air = new air(this);
    m_air->setAccessibleName("kylin-weather_Lifestyle_air");
    m_sport = new sport(this);
    m_sport->setAccessibleName("kylin-weather_Lifestyle_sport");
    m_washcar = new washcar(this);
    m_washcar->setAccessibleName("kylin-weather_Lifestyle_washcar");
    m_flu = new flu(this);
    m_flu->setAccessibleName("kylin-weather_Lifestyle_flu");
    m_clothe = new clothe(this);
    m_clothe->setAccessibleName("kylin-weather_Lifestyle_clothe");

    initLayout();
    initConnect();
}

void lifestyle::getLifeStyleInfor(LifeStyle m_lifeStyle, int m_isFullScreen)
{
    m_ultravioletrays->setItemContent(m_lifeStyle.uv_brf);
    m_air->setItemContent(m_lifeStyle.air_brf);
    m_sport->setItemContent(m_lifeStyle.sport_brf);
    m_washcar->setItemContent(m_lifeStyle.cw_brf);
    m_flu->setItemContent(m_lifeStyle.flu_brf);
    m_clothe->setItemContent(m_lifeStyle.drsg_brf);
}

void lifestyle::initLayout()
{
    double m_radio = 1;
    m_lifestyleLayout->addWidget(m_ultravioletrays, 1);
    m_lifestyleLayout->addWidget(m_air, 1);
    m_lifestyleLayout->addWidget(m_sport, 1);
    m_lifestyleLayout->addWidget(m_washcar, 1);
    m_lifestyleLayout->addWidget(m_flu, 1);
    m_lifestyleLayout->addWidget(m_clothe, 1);
    m_lifestyleLayout->setContentsMargins(20, 3, 20, 3);
    this->setLayout(m_lifestyleLayout);
}

void lifestyle::initConnect()
{
    connect(this, &lifestyle::sendIsFullScreen, m_ultravioletrays, &ultravioletrays::isFullScreen);
    connect(this, &lifestyle::sendIsFullScreen, m_air, &air::isFullScreen);
    connect(this, &lifestyle::sendIsFullScreen, m_sport, &sport::isFullScreen);
    connect(this, &lifestyle::sendIsFullScreen, m_washcar, &washcar::isFullScreen);
    connect(this, &lifestyle::sendIsFullScreen, m_flu, &flu::isFullScreen);
    connect(this, &lifestyle::sendIsFullScreen, m_clothe, &clothe::isFullScreen);
}

void lifestyle::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    //设置背景颜色
    QColor color = this->palette().color(QPalette::Dark);
    QPalette pal(this->palette());
    pal.setColor(QPalette::Background, QColor(color));
    this->setPalette(pal);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setOpacity(0.08);
    painter.setPen(Qt::transparent);
    int radius = 3 * ViewVar::SAME_ITEMRADIUS;
    QStyleOption opt;
    opt.init(this);
    painter.drawRoundedRect(opt.rect, radius, radius); // 绘制圆角
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
    QWidget::paintEvent(event);
}

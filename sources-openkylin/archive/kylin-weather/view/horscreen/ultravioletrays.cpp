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
#include "ultravioletrays.h"
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#define UKUI_STYLE "org.ukui.style"
#define UKUI_FONT_SIZE "systemFontSize"
ultravioletrays::ultravioletrays(QWidget *parent) : QWidget(parent)
{
    m_uvLayout = new QVBoxLayout(this);

    m_uvIcon = new QLabel(this);
    m_uvIcon->setAccessibleName("kylin-weather_QLabel_uvIcon");
    m_str = new QLabel(this);
    m_str->setAccessibleName("kylin-weather_QLabel_uvStr");
    m_degree = new QLabel(this);
    m_degree->setAccessibleName("kylin-weather_QLabel_uvDegree");

    QPixmap uvpixmap(":/res/contorl_icons/index_uv.png");
    m_uvIcon->setPixmap(uvpixmap);
    m_uvIcon->setAlignment(Qt::AlignCenter);
    m_str->setText(tr("UV"));
    m_str->setAlignment(Qt::AlignCenter);
    m_degree->setText("N/A");
    m_degree->setAlignment(Qt::AlignCenter);
    initLayout();
    //    initQss();
}

void ultravioletrays::setItemContent(QString text)
{
    m_degree->setText(text);
}

void ultravioletrays::isFullScreen(int m_isFullScreen)
{
    m_judgeFullScreen = m_isFullScreen;
    initQss(14.0);
}

void ultravioletrays::initLayout()
{
    m_uvLayout->addWidget(m_uvIcon);
    m_uvLayout->addWidget(m_str);
    m_uvLayout->addWidget(m_degree);
}

void ultravioletrays::lable2SetFontSizeSlot(int size)
{
    //默认大小12px,换算成pt为9
    double lable2BaseFontSize = 11;                              //魔鬼数字，自行处理
    double nowFontSize = lable2BaseFontSize * double(size) / 11; // 11为系统默认大小，魔鬼数字，自行处理
    //    QFont font;
    //    font.setPointSizeF(nowFontSize);
    //    ui->label2->setFont(font);
    initQss(nowFontSize);
}
void ultravioletrays::initQss(double nowFontSize)
{
    m_uvIcon->setAlignment(Qt::AlignCenter);

    m_uvIcon->resize(30, 30);
    QPixmap uvpixmap(uvpath);
    m_uvIcon->setPixmap(uvpixmap);

    m_str->setStyleSheet("color: white;");
    QFont m_font;
    m_str->setFont(m_font);
    m_str->setAlignment(Qt::AlignCenter);
    //    设置字体透明度
    QGraphicsOpacityEffect *uvOpacityEffect = new QGraphicsOpacityEffect;
    uvOpacityEffect->setOpacity(0.7);
    m_str->setGraphicsEffect(uvOpacityEffect);
    m_degree->setStyleSheet("color: white;");
    m_degree->setAlignment(Qt::AlignCenter);
    m_degree->setFont(m_font);
}

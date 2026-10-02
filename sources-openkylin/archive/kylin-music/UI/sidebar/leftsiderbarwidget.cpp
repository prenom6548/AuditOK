/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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

#include "leftsiderbarwidget.h"
#include <QDebug>
#include <QGSettings>
#include <QStyleOption>
#include <QPainterPath>
#include <QPainter>
#include "kwindoweffects.h"
#include "UI/base/widgetstyle.h"
#define PERSONALSIE_SCHEMA     "org.ukui.control-center.personalise"
#define PERSONALSIE_TRAN_KEY   "transparency"
#define CONTAIN_PERSONALSIE_TRAN_KEY   "transparency"

LeftsiderbarWidget::LeftsiderbarWidget(QWidget *parent)
    : QWidget(parent)
{
    QGSettings *personalQgsettings = nullptr;
    if (QGSettings::isSchemaInstalled(PERSONALSIE_SCHEMA)) {
        personalQgsettings = new QGSettings(PERSONALSIE_SCHEMA, QByteArray(), this);
        transparency = personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;

        connect(personalQgsettings,&QGSettings::changed,this,[=](QString changedKey) {  //监听透明度变化
                        if (changedKey == CONTAIN_PERSONALSIE_TRAN_KEY) {
                           transparency = personalQgsettings->get(PERSONALSIE_TRAN_KEY).toDouble() * 255;
                           this->repaint();
                        }
                });
    } else {
        personalQgsettings = nullptr;
        qDebug()<<PERSONALSIE_SCHEMA<<" not installed";
    }
}

LeftsiderbarWidget::~LeftsiderbarWidget()
{
}
#ifndef WHITE_TEST
void LeftsiderbarWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing); // 反锯齿
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0);
    /* 开启背景模糊效果（毛玻璃） */
//    KWindowEffects::enableBlurBehind(this->winId(), true, QRegion(rectPath.toFillPolygon().toPolygon()));

    p.setPen(Qt::NoPen);

    QColor color = palette().color(QPalette::Window);
    color.setAlpha(transparency);
    QPalette pal(this->palette());

    //this->setStyleSheet("background-color: rgba(128, 128, 128, 0);");
    //this->setPalette(pal);

    if(WidgetStyle::themeColor == 1 ){
	    pal.setColor(QPalette::Window,QColor(35, 36, 38, 0));
	    this->setPalette(pal);
    }
    else if(WidgetStyle::themeColor == 0){
	    pal.setColor(QPalette::Window,QColor(245, 245, 245, 0));
	    this->setPalette(pal);
    }

    QStyleOption opt;
    opt.init(this);

    QBrush brush =QBrush(color);
    p.setBrush(brush);
    p.drawRoundedRect(opt.rect,0,0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}
#endif

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
#include "selectbtn.h"
#include <QVBoxLayout>
#include <QDebug>
#include "clock.h"
#include <QPainterPath>
#include "theme.h"

SelectBtn::SelectBtn(QString name,QWidget *parent) : QPushButton(parent)
{
    QPixmap pixmap = QPixmap(":/image/go-bottom-symbolic.png");
    textLabel = new QLabel(this);
    textLabel->setObjectName("ukui-clock_QLabel_textLabel");
    IconLabel = new QLabel(this);
    IconLabel->setObjectName("ukui-clock_QLabel_IconLabel");
    noName = new QLabel(this);
    noName->setObjectName("ukui-clock_QLabel_noName");
    //num不同，name与text的大小配比不同
    textLabel->setFixedSize(164, 36);
    IconLabel->setFixedSize(27, 36);
    int sep = 3;
    noName->setFixedSize(sep, 36);
    textLabel->move(sep, 0);
    noName->move(0, 0);
    IconLabel->move(173, 0);
    textLabel->setText(name);
    IconLabel->setPixmap(pixmap);
    textLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    textLabel->setStyleSheet("font-size:14px;");
//    this->resize(290,36);
    QPalette palette;
    palette.setColor(QPalette::ButtonText,QColor(148, 148, 148, 255));
    textLabel->setPalette(palette);

}


SelectBtn::~SelectBtn()
{

}

void SelectBtn::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 6, 6); // 左上右下

    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);
    painter.setBrush(opt.palette.color(QPalette::Base));

    QColor mainColor;
    mainColor = theme::selectBtnBackColor;

    p.fillPath(rectPath,QBrush(mainColor));
}
/**
 * @brief 1 向上 0 向下
 * @param status
 */
void SelectBtn::updateIconLabel(int status)
{
     QPixmap pixmap ;
     if(status==1){
         pixmap = QPixmap(":/image/go-up-symbolic.png");
     }else{
         pixmap = QPixmap(":/image/go-bottom-symbolic.png");
     }
     IconLabel->setPixmap(pixmap);
     IconLabel->update();
}

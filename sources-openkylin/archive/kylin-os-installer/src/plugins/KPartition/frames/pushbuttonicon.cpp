/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "pushbuttonicon.h"
#include <QGridLayout>
#include <QEvent>
#include <QMouseEvent>
#include <QDebug>
namespace KInstaller {
PushButtonIcon::PushButtonIcon(QString icon, QString txt, QWidget* parent ):QWidget(parent),
    m_icon(icon), m_txt(txt)
{
   // setAttribute(Qt::WA_Hover,true);
    this->setObjectName("PushButtonIcon");
    QWidget* widget = new QWidget(this);
    hlay = new QGridLayout(widget);
    hlay->setContentsMargins(0, 0, 0, 0);
    hlay->setAlignment(Qt::AlignCenter);
    licon = new mylabelicon(icon,this);
    ltxt = new QLabel;
    licon->setObjectName("licon");
    licon->setFixedHeight(15);
    licon->setMinimumWidth(15);
    licon->installEventFilter(this);
//    hlay->addStretch();
    hlay->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), 0, 1, 1, 1, Qt::AlignCenter);
    hlay->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), 1, 0, 1, 1, Qt::AlignCenter);
    hlay->addWidget(licon, 1, 1, Qt::AlignCenter);
    hlay->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), 1, 2, 1, 1, Qt::AlignCenter);
    hlay->addItem(new QSpacerItem(10, 10, QSizePolicy::Fixed, QSizePolicy::Fixed), 2, 1, 1, 1, Qt::AlignCenter);
//    hlay->setColumnStretch(0,1);
//    hlay->setColumnStretch(1,3);
//    hlay->setColumnStretch(2,1);

 //   ltxt->setMinimumHeight(20);
 //   ltxt->setMaximumHeight(20);
  //  ltxt->setText(m_txt);
  //  ltxt->installEventFilter(this);
  //  ltxt->setObjectName("pushbtnText");
  //  hlay->addWidget(ltxt,  Qt::AlignLeft | Qt::AlignCenter);
    this->setLayout(hlay);
//    this->installEventFilter(this);

//    setFlat(true);
//    setIcon(m_icon);
//    setIconSize(QSize(12, 12));
//    setText("");
    update();
}

void PushButtonIcon::showIconAndText()
{
    ltxt->setText(m_txt);
//    setText(m_txt);

}

void PushButtonIcon::showIcon()
{
    ltxt->setText(m_txt);
}

//void PushButtonIcon::mousePressEvent(QMouseEvent *event)
//{
//    if(event->buttons() == Qt::LeftButton) {
//        emit clicked();
//    }
//    QWidget::mousePressEvent(event);
//}

//PushButtonIcon::PushButtonIcon(QString icon, QString txt, QWidget* parent ):QWidget(parent),
//    m_icon(icon), m_txt(txt)
//{
//    m_txtLabel = new QLabel;
//    m_txtLabel->setObjectName("txtLabel");
//    m_IcomLabel = new QLabel;
//    m_IcomLabel->setObjectName("IcomLabel");
//    QHBoxLayout* layout = new QHBoxLayout(this);
//    this->setLayout(layout);
//    layout->addWidget(m_IcomLabel);
//    layout->addWidget(m_txtLabel);
//    layout->addStretch();

//    m_txtLabel->setText("");
//    m_IcomLabel->setPixmap(QPixmap(m_icon).scaled(QSize(12, 12)));

//    this->installEventFilter(this);

//}

//void PushButtonIcon::showIconAndText()
//{
//    m_txtLabel->setText(m_txt);
//}

//void PushButtonIcon::showIcon()
//{
//    m_txtLabel->setText("");
//}

bool PushButtonIcon::eventFilter(QObject *target, QEvent *event)
{
    QString sr;
    sr.sprintf("type=%d",event->type());
    qDebug()<<target->objectName()+":::"+sr;
    if(target->objectName() == "licon" || target->objectName() == "pushbtnText") {
        if(event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);//将事件强制转换
            if(mouseEvent->button() == Qt::LeftButton)
            {
                emit clicked();

                return true;
            }
        }
        else if(event->type() == QEvent::Enter)
        {
            //licon->Shadow=QColor(255,0,0,200);
            Shadow=QColor(255, 255, 255, 50);
          //  licon->update();
            update();

        }else if(event->type() == QEvent::Leave)
        {
             //licon->Shadow=QColor(255,0,0,0);
             Shadow=QColor(255, 255, 255, 0);
            // licon->update();
             update();
        }
    }
    return QWidget::eventFilter(target,event);//返回父类的事件过滤器
}

void PushButtonIcon::paintEvent(QPaintEvent *ev)
{
    pain.begin(this);
    pain.setPen(QPen(Qt::blue,1,Qt::SolidLine,Qt::FlatCap));
    pain.fillRect(rect(),Shadow);
    pain.end();
}
}

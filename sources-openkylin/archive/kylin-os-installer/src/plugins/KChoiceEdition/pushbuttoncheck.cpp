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

#include "pushbuttoncheck.h"
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <qdebug.h>
PushButtonCheck::PushButtonCheck(QString str, QWidget *parent) : QPushButton(parent)
{
    QHBoxLayout* layout = new QHBoxLayout(this);
    m_labelpix = new QLabel(this);
    //m_labelpix->setPixmap(QPixmap(":/res/png/clicked.svg").scaled(24, 24));
    m_labelpix->setPixmap(QIcon(":/res/png/clicked.svg").pixmap(24,24));
    this->setLayout(layout);
    layout->setAlignment(this, Qt::AlignLeft);
    this->setText(str);
    layout->addStretch();
    layout->addWidget(m_labelpix);
    this->setCheckable(true);

    m_labelpix->setVisible(false);
//    this->setStyleSheet("    width: 320px;\
//                        height: 48px;\
//                        opacity: 1;\
//                        border-radius: 6px;\
//                        border:2px;\
//                        font-size: 14px;\
//                        font-family: Noto Sans CJK SC;\
//                        font-weight: 500;\
//                        line-height: 35px;\
//                        color: #FFFFFF;");
    connect(this,&PushButtonCheck::clicked,this,&PushButtonCheck::clicked_slot);
}

void PushButtonCheck::clicked_slot()
{
    emit clicked_obj(this);
}

//void PushButtonCheck::addStyleSheet()
//{
//    QFile file(":/res/qss/KChoiceLanguage.css");
//    file.open(QFile::ReadOnly);
//    QTextStream filetext(&file);
//    QString stylesheet = QString("");
//    stylesheet = filetext.readAll();
//    file.close();

//    this->setStyleSheet(stylesheet);
//}


void PushButtonCheck::other_clicked_slot(QString objname)
{
    qDebug()<<"objectname===="<<objectName()<<"======"<<objname;
    if(this->objectName().compare(objname)!=0)
    {
        setLabelShow(false);
        //this->setStyleSheet("background-color: rgba(0, 0, 0, 0.2);");
    }
    else
    {
        setLabelShow(true);
       // setChecked(true);
       // this->setStyleSheet("background-color: rgba(255, 255, 255, 0.1);");

    }
}

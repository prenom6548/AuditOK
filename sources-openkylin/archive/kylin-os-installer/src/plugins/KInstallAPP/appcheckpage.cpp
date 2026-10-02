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

#include "appcheckpage.h"
#include <QDebug>

using namespace KServer;
namespace KInstaller {
appcheckpage::appcheckpage(QWidget *parent, int nAPPNum) : QWidget(parent)
{
    this->setObjectName("AppCheckPage");
    m_nAPPNum = nAPPNum;
    QHBoxLayout* clayout=new QHBoxLayout(this);
    this->setLayout(clayout);
    clayout->setContentsMargins(0, 0, 0, 0);
   // setStyleSheet("  background:rgba(255,0,255,0.3);");

    if(m_nAPPNum == 1){
        m_centerCell =  new QWidget(this);
        m_centerLayout=new QVBoxLayout(m_centerCell);
        m_centerCell->setLayout(m_centerLayout);
        m_centerCell->resize(this->size().width()/2,this->size().height());
        m_centerCell->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
//        clayout->addWidget(m_centerCell,0 ,Qt::AlignHCenter|Qt::AlignTop);
        clayout->addWidget(m_centerCell,0 ,Qt::AlignCenter);
    }else{
        LeftCells =  new QWidget(this);
        RightCells = new QWidget(this);
    //    LeftCells->setStyleSheet("  background:rgba(255,0,0,0.3);");
    //    RightCells->setStyleSheet("  background:rgba(255,255,0,0.3);");
        LeftLayout=new QVBoxLayout(LeftCells);
        RightLayout=new QVBoxLayout(RightCells);
        LeftLayout->setSpacing(24);
        RightLayout->setSpacing(24);
        LeftCells->setLayout(LeftLayout);
        RightCells->setLayout(RightLayout);

        LeftCells->resize(this->size().width()/2,this->size().height());
        RightCells->resize(this->size().width()/2,this->size().height());
        LeftCells->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
        RightCells->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
        clayout->addWidget(LeftCells);
        clayout->addWidget(RightCells);
    }
}

qint8 appcheckpage::AddAppCheckBox(QString FliePaht)
{
    QVBoxLayout* p;
    if(m_nAPPNum == 1){
        p = m_centerLayout;
    }else{
        if(lst.count()&1)//从左列开始存放
          p = RightLayout;
        else
          p = LeftLayout;
    }

   QDir dir(FliePaht);
   if(dir.exists()==false){return 0;}
   QFileInfoList flst=dir.entryInfoList();
   foreach (QFileInfo one, flst) {
       if(one.isFile() && one.suffix()=="cfg")
       {
          AppCheckBox* box=new AppCheckBox(one.filePath());
          if(box->init_ok==false) {
              delete box;
              continue;
          }
          p->addWidget(box,0,Qt::AlignLeft);
          lst.append(box);
       }
   }

   return 1;
}

void appcheckpage::transfer(QString lang)
{
    foreach (AppCheckBox* var, lst) {
        var->translater(lang);
    }
}

}

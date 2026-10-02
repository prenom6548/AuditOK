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

#include "modeselectframe.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include "../PluginService/ksystemsetting_unit.h"
#include <QEvent>

//定义选项控件数量
#define MODE_BNT_NUM    2

namespace KInstaller{

using namespace KServer;
modeselect::modeselect(QWidget* parent):QWidget(parent)
{
    initUI();
    translateStr();
    addStyleSheet();
    emit bntList.at(0)->click();
    initFocusPolicy();
}

void modeselect::initUI()
{

    QVBoxLayout *usersetupLayout = new  QVBoxLayout();
    this->setLayout(usersetupLayout);
    usersetupLayout->setContentsMargins(0, 0, 0, 0);
    usersetupLayout->setSpacing(0);

    m_mainTitle= new QLabel(this);
    m_mainTitle->setObjectName("modeselect_m_mainTitle");
    usersetupLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));
    usersetupLayout->addWidget(m_mainTitle, 0, Qt::AlignCenter | Qt::AlignTop);
    usersetupLayout->addItem(new QSpacerItem(10, 28, QSizePolicy::Expanding, QSizePolicy::Minimum));


    for (int  k= 0; k < MODE_BNT_NUM; k++) {
        chiceBtn* bnt=new chiceBtn(this, k+1);
        if(k==0)
        {
            bnt->installEventFilter(this);
        }
        usersetupLayout->addWidget(bnt, 0, Qt::AlignCenter);
        usersetupLayout->addItem(new QSpacerItem(10, 28, QSizePolicy::Expanding, QSizePolicy::Minimum));
        connect(bnt, &chiceBtn::clickedSignal, this, &modeselect::changemodel);
        connect(this, &modeselect::modeSelect, [=] (chiceBtn *nt) {if(nt!=bnt)bnt->setBntValue(false);});
        bnt->bnt_id=k;
        bntList.append(bnt);
    }

    usersetupLayout->addItem(new QSpacerItem(10, 200, QSizePolicy::Expanding, QSizePolicy::Expanding));

}

void modeselect::addStyleSheet()
{
    QFile file(":/res/qss/modeselectframe.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
   QString stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void modeselect::initFocusPolicy()
{
    int count = bntList.count();
    if (count <= 1) return;
    // 设置相邻按钮的焦点顺序
    for (int i = 0; i < count - 1; i++)
    {
        this->setTabOrder(bntList.at(i), bntList.at(i+1));
    }

  //  this->setTabOrder(bntList.last(), this->m_nextBtn);

    bntList.first()->setFocus();
}

void modeselect::translateStr()
{
    m_mainTitle->setText(tr("Register account"));

    int k=0;
    if(k<bntList.count())
    {
        bntList.at(k)->setObjectName("Register_immediately");
        bntList.at(k)->mainTitle->setText(tr("Register immediately"));
//        bntList.at(k)->subTitle->setText("");
        k++;
    }

    if(k<bntList.count())
    {
        bntList.at(k)->setObjectName("Register_later");
        bntList.at(k)->mainTitle->setText(tr("Register later"));
       bntList.at(k)->subTitle->setText(tr("After the system installation is completed, an account will be created when entering the system."));
//oem2阶段不能选择稍后创建
      if(KServer::ReadSettingIni("config","first_mode").compare("oem2")==0)
      {
        bntList.at(k)->setDisabled(true);
        bntList.at(k)->setCheckable(false);
      }
       else{
        bntList.at(k)->setDisabled(false);
        bntList.at(k)->setCheckable(true);
      }
       k++;
    }

//    if(k<bntList.count())
//    {
//        bntList.at(k)->setObjectName("Use_old_later");
//        bntList.at(k)->mainTitle->setText(tr("Use old access"));
//        bntList.at(k)->subTitle->setText(tr("The previous account will be used for login and historical data will be retained. Make sure you remember the password of your previous account."));
//        k++;
//    }

    if(currentBnt!=nullptr)
    {
          emit changeNextButton();
          emit enableNextButton(true);
    }



}

void modeselect::changemodel(chiceBtn *bnt)
{
    currentBnt=bnt;
    emit changeNextButton();
    emit enableNextButton(true);
    emit modeSelect(bnt);

}

void modeselect::checkactive(QWidget *p)
{
     if(p==this)
     {
        enableNextButton(true);
     }
}

bool modeselect::eventFilter(QObject *watched, QEvent *event)
{
    if(bntList.count()>0)
    {
        if(watched==bntList.first()&&event->type()==QEvent::FocusIn)
        {
            if(m_firstfocusget==true)
            {
                bntList.first()->setStyleSheet(QString(
                  "QPushButton#Register_immediately:focus {"
                        "border: 2px solid transparent; "
                        "outline: none;"
                    "}"
                    ));
                m_firstfocusget=false;
            }
            else
            {
                bntList.first()->setStyleSheet(
                    "QPushButton#Register_immediately:focus {"
                    "border: 2px solid #FFFFFF; "
                    "outline: none;"
                    "}"
                    );
            }
        }
    }

    return QObject::eventFilter(watched,event);
}

void modeselect::showEvent(QShowEvent *event)
{
    m_firstfocusget=true;
    bntList.first()->setFocus();
    emit reSetFocusPolicySignal();
}

}


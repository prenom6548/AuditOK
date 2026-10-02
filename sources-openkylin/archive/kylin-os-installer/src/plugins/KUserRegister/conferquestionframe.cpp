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

#include "conferquestionframe.h"
#define QUESTIONNUM 3

namespace KInstaller{

conferquestion::conferquestion(QWidget *parent):QWidget(parent)
{


    initUI();
    initAllConnect();
    addStyleSheet();
    translateStr();

}

conferquestion::~conferquestion()
{

}

void conferquestion::initUI()
{
    QVBoxLayout *usersetupLayout = new  QVBoxLayout();
    this->setLayout(usersetupLayout);
    usersetupLayout->setContentsMargins(0, 0, 0, 0);
    usersetupLayout->setSpacing(0);

    m_mainTitle = new QLabel();
    m_mainTitle->setObjectName("confer_mainTitle");
    m_subTitle=new QLabel();
    m_subTitle->setObjectName("m_subTitle");

    usersetupLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));
    usersetupLayout->addWidget(m_mainTitle, 1, Qt::AlignCenter);
    usersetupLayout->addItem(new QSpacerItem(10, 28, QSizePolicy::Expanding, QSizePolicy::Minimum));
    usersetupLayout->addWidget( m_subTitle, 3, Qt::AlignCenter);
    usersetupLayout->addItem(new QSpacerItem(10, 28, QSizePolicy::Expanding, QSizePolicy::Minimum));



for(int k=0;k<QUESTIONNUM;k++ )
{

    conferUi *cell=new conferUi(this);
    //问题列表
    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->setMargin(0);
    hlayout->setContentsMargins(0,15,0,0);

    hlayout->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayout->addWidget( cell->qlab1,1,Qt::AlignLeft);
    hlayout->addWidget(cell->question1,1,Qt::AlignLeft);
    cell->qlab1->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    cell->question1->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Preferred);
    hlayout->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayout);

    //答案
   QHBoxLayout* hlayoute = new QHBoxLayout;
    cell->context1->setObjectName("confer_context1");
    cell->context1->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Maximum);
    hlayoute->setMargin(0);
    hlayoute->setContentsMargins(0,15,0,0);
    hlayoute->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    hlayoute->addWidget(cell->clab1,1,Qt::AlignLeft);
    hlayoute->addWidget(cell->context1,2,Qt::AlignLeft);
    cell->clab1->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Preferred);
    cell->context1->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Preferred);
    hlayoute->addItem(new QSpacerItem(1000, 34, QSizePolicy::Expanding, QSizePolicy::Preferred));
    usersetupLayout->addLayout(hlayoute);
    usersetupLayout->addItem(new QSpacerItem(10, 28, QSizePolicy::Expanding, QSizePolicy::Minimum));
    Lst.append(cell);


}

usersetupLayout->addItem(new QSpacerItem(10, 28000, QSizePolicy::Minimum, QSizePolicy::Expanding));
    later=new QCheckBox;
    later->setObjectName("set_later");
    usersetupLayout->addWidget(later,1,Qt::AlignHCenter);
    translateStr();
}

void conferquestion::initAllConnect()
{
    foreach (conferUi* one, Lst) {
       connect(one,&conferUi::ClickedSignal,this,&conferquestion::conferCheck);
    }
    connect(later,&QCheckBox::clicked,this,&conferquestion::conferLater);
}

void conferquestion::addStyleSheet()
{
    QFile file(":/res/qss/conferquestionframe.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
   QString stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void conferquestion::translateStr()
{
     m_mainTitle->setText(tr("confer_mainTitle"));
     m_subTitle->setText(tr("You can reset your password by answering a security question when you forget your password. Make sure you remember the answer."));

    questionlist.clear();
    questionlist.append(tr("Which city were you born in?"));
    questionlist.append(tr("What is your childhood nickname?"));
    questionlist.append(tr("Which middle school did you graduate from?"));
    questionlist.append(tr("What is your father's name?"));
    questionlist.append(tr("What is your mother's name?"));
    questionlist.append(tr("When is your spouse's birthday?"));
    questionlist.append(tr("What's your favorite animal?"));

    for(int k=0;k<Lst.count();k++ )
    {
        Lst.at(k)->qlab1->setText(QString(tr("question%1:")).arg(k));
        Lst.at(k)->clab1->setText(QString(tr("answer%1:")).arg(k));
         Lst.at(k)->question1->clear();
        Lst.at(k)->question1->addItems(questionlist);

    }

    later->setText(tr("set later"));
}

void conferquestion::checkactive(QWidget *p)
{
    if(p==this)
    conferLater();
}




void conferquestion::conferCheck(conferUi* confer)
{
    if(confer!=nullptr)
    {

    }
    anserconunt=0;
    qDebug()<<"conferquestion::conferCheck=";
    foreach (conferUi *one, Lst) {
      anserconunt+=one->is_ok;
    }
    if(anserconunt==QUESTIONNUM)//所有问题都填好
        emit enableNextButton(true);
    else
       emit enableNextButton(false);
}

void conferquestion::conferLater()
{   
    if(anserconunt==QUESTIONNUM || later->isChecked())
        emit enableNextButton(true);
    else
        emit enableNextButton(false);
}

void conferquestion::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}


}

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

#include <QApplication>
#include "userframe.h"
#include <QDebug>
#include <QDir>
#include <QKeyEvent>
#include <QHBoxLayout>
#include <QSizePolicy>
#include "../PluginService/ksystemsetting_unit.h"

namespace KInstaller{

using namespace KServer;


UserFrame::UserFrame(QWidget *parent) : MiddleFrameManager(parent)
{
    this->setObjectName("UserFrame");
    this->initNextBnt();
    stack =new QStackedWidget(m_Widget);
    QVBoxLayout *usersetupLayout = new  QVBoxLayout();
    m_Widget->setLayout(usersetupLayout);
    usersetupLayout->setContentsMargins(0, 0, 0, 0);
    usersetupLayout->setSpacing(0);
    usersetupLayout->addWidget(stack, 1, Qt::AlignCenter);
    //add widget
    usregister=new userregisterwidiget(stack);//创建用户子界面
    usregister->setProperty("type","usregister");
    connect(usregister,&userregisterwidiget::enableNextButton,this,&UserFrame::enableNextButtonSlot);
//    connect(usregister,&userregisterwidiget::setNextText,this,&UserFrame::setBtnValue);
    connect(this,&UserFrame::translateStrSignal,usregister,&userregisterwidiget::translateStr);
    connect(this,&UserFrame::currentwidgetchange,usregister,&userregisterwidiget::checkactive);
    connect(usregister,&userregisterwidiget::reSetFocusPolicySignal,this,&UserFrame::reSetFocusPolicy);
    //confer=new conferquestion(stack);
//    connect(confer,&conferquestion::enableNextButton,this,&UserFrame::enableNextButtonSlot);
//    connect(this,&UserFrame::translateStrSignal,confer,&conferquestion::translateStr);
//    connect(this,&UserFrame::currentwidgetchange,confer,&conferquestion::checkactive);
    QString strModle = KServer::ReadSettingIni("config","first_mode");
    if(strModle.compare("normal")==0)//此模式有两个子界面：创建账户，创建用户
    {
        modesel=new modeselect(stack);//创建账户子界面
        modesel->setProperty("type","modesel");
        connect(modesel,&modeselect::enableNextButton,this,&UserFrame::enableNextButtonSlot);
        connect(modesel,&modeselect::changeNextButton,this,&UserFrame::changeNextButtonSlot);
        connect(this,&UserFrame::translateStrSignal,modesel,&modeselect::translateStr);
        connect(this,&UserFrame::currentwidgetchange,modesel,&modeselect::checkactive);
        connect(modesel,&modeselect::reSetFocusPolicySignal,this,&UserFrame::reSetFocusPolicy);
//    connect(stack,&QStackedWidget::currentChanged,[=](){
//        emit currentwidgetchange(stack->currentWidget());
//    });
        stack->addWidget(modesel);
        setButtonNext();
    }
    else if(strModle.compare("oem2")==0)//此模式有两个子界面：创建用户，设置密保
    {
        m_securityquest = new securityquestions(stack);//设置密保子界面
        m_securityquest->setProperty("type","m_securityquest");
        connect(m_securityquest,&securityquestions::enableNextButton,this,&UserFrame::enableNextButtonSlot);
//        connect(m_securityquest,&securityquestions::setNextText,this,&UserFrame::setBtnValue);
        connect(this,&UserFrame::translateStrSignal,m_securityquest,&securityquestions::translateStr);
        connect(this,&UserFrame::currentwidgetchange,m_securityquest,&securityquestions::checkactive);
        connect(m_securityquest,&securityquestions::reSetFocusPolicySignal,this,&UserFrame::reSetFocusPolicy);
        setButtonNext();
    }
    else if(strModle.compare("oem1")==0)//此模式只有一个子界面：创建用户
    {
        setBtnValue();
    }
    stack->addWidget(usregister);
  //  stack->addWidget(confer);
//    m_securityquest = new securityquestions(stack);
//    connect(m_securityquest,&securityquestions::enableNextButton,this,&UserFrame::enableNextButtonSlot);//ok
//    connect(m_securityquest,&securityquestions::setNextText,this,&UserFrame::setBtnValue);
//    connect(this,&UserFrame::translateStrSignal,m_securityquest,&securityquestions::translateStr);//ok
//    connect(this,&UserFrame::currentwidgetchange,m_securityquest,&securityquestions::checkactive);//o

    if(nullptr != m_securityquest)
        stack->addWidget(m_securityquest);
    stack->setCurrentIndex(curIndex);
    stack->currentWidget()->show();
    addStyleSheet();
    initFocusPolicy();
}

UserFrame::~UserFrame()
{

}

void UserFrame::clickNextButton()
{
    if(stack->currentWidget() == m_securityquest)
    {
        if(!m_securityquest->m_Setuplater->isChecked())
            m_securityquest->bindSecurityQuestionsAnswerV3();
    }

    if(curIndex == (stack->count()-1))
    {
        writeSettingIni(1);
        emit signalStackPageChanged(1);
    }
    else if(curIndex==0)//当前显示的是第一个子界面
    {
        if(modesel != nullptr && modesel->currentBnt!=nullptr)//第一个子界面是“创建账户”modesel，表示是处于normal正常安装阶段
        {
            if(modesel->currentBnt->objectName().compare("Register_later")==0)//稍后创建
            {
                writeSettingIni(0);
                emit signalStackPageChanged(1);
            }
            else if(modesel->currentBnt->objectName().compare("Register_immediately")==0)
            {
                setBtnValue();
                curIndex++;
                stack->setCurrentIndex(curIndex);
                stack->currentWidget()->show();
                usregister->initURFocus();//bug215368
                emit currentwidgetchange(stack->currentWidget());
            }
            else if(modesel->currentBnt->objectName().compare("Use_old_later")==0)
            {
                writeSettingIni(1);
                emit signalStackPageChanged(1);
            }
        }

        if(KServer::ReadSettingIni("config","first_mode").contains("oem2"))//处于OOBE阶段
        {
            setBtnValue();
            curIndex++;
            stack->setCurrentIndex(curIndex);
//            usregister->initURFocus();
            emit currentwidgetchange(stack->currentWidget());
        }
    }
    else
    {
        curIndex++;
        stack->setCurrentIndex(curIndex);
    }
    reSetFocusPolicy();
}

void UserFrame::setButtonNext()
{
        m_nextBtn->setText(tr("Next"));
}
void UserFrame::setButtonStart()
{
    if(KServer::ReadSettingIni("config","first_mode").contains("oem2"))//oobe阶段
    {
            m_nextBtn->setText(tr("Start Configuration"));
    }
    else
    {
            m_nextBtn->setText(tr("Start Installation"));
    }
}

void UserFrame::clickPrevButton(QWidget *p)
{
    if(p==this)
    {
        if(stack->currentIndex()==0)
        {
           if(KServer::ReadSettingIni("config","first_mode").contains("oem2")==1)
           {
                WriteSettingToIni("config", "user_register", QString::number(0));
           }

           emit backspacepressed();
        }
        else
        {
            setButtonNext();
            WriteSettingToIni("config", "user_register", QString::number(0));
            curIndex--;
            stack->setCurrentIndex(curIndex);
            emit currentwidgetchange(stack->currentWidget());
        }
    }
}

void UserFrame::getmessage(QWidget *p, QString str)
{
    if(p==this)
    {

       if(str.compare("next")==0)
        {

         QString str=  ReadSettingIni("config","user_register");
         if(str.compare("1")==0)
         {
             emit signalStackPageChanged(1);
         }
        }

    }
}

void UserFrame::enableNextButtonSlot(bool value)
{
    qDebug()<<"enableNextButtonSlot:"<<value;
    m_nextBtn->setEnabled(value);
}

void UserFrame::changeNextButtonSlot()
{
    if(modesel->currentBnt->objectName().compare("Register_later")==0)//稍后创建
    {
        setButtonStart();
        m_startChangeModule = true;
        changeNavigations(true);
    }
    else if(modesel->currentBnt->objectName().compare("Register_immediately")==0)
    {
        setButtonNext();
        changeNavigations(false);
    }
}

void UserFrame::setBtnValue()//设置最后一个子界面中下一步按钮的显示语言
{
        if(KServer::ReadSettingIni("config","first_mode").contains("oem2")==1)
        {
                SourcePath="/opt/apps-third";
        }
        else{
                SourcePath="/cdrom/apps-third";
        }
        qDebug()<<"SourcePath="<<SourcePath;
        QDir dir(SourcePath);
        if( !dir.exists() )
        {
                qDebug()<<"setnextvalue Start Installation";
                setButtonStart();
        }
        else
        {
                setButtonNext();
        }

}

void UserFrame::modeselslots(int index)
{

}

void UserFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        if(curIndex == (stack->count()-1)){
            setBtnValue();
        }
        else{
            setButtonNext();
        }

       emit translateStrSignal();
    } else {
        QWidget::changeEvent(event);
    }
}

void UserFrame::keyPressEvent(QKeyEvent *event)
{
    if (KServer::ReadSettingIni("config","first_mode").compare("normal") == 0) {
        if (curIndex == (stack->count()-2)) {//如果当前显示的是“创建账户”子界面，就设置上下键功能。解决bug156309
            if (event->key() == Qt::Key_Up) {
                int i;
                for (i = 0; i < modesel->bntList.count(); i++) {
                    if (modesel->bntList.at(i) == modesel->currentBnt) {
                        break;
                    }
                }
                if (i == 0) {
                    i = modesel->bntList.count() - 1;
                } else {
                    i = i - 1;
                }
                emit modesel->bntList.at(i)->clicked();
            } else if (event->key() == Qt::Key_Down) {
                int i;
                for (i = 0; i < modesel->bntList.count(); i++) {
                    if (modesel->bntList.at(i) == modesel->currentBnt) {
                        break;
                    }
                }
                if (i == modesel->bntList.count() -1) {
                    i = 0;
                } else {
                    i = i + 1;
                }
                emit modesel->bntList.at(i)->clicked();
            }
        }
    }

    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        if (m_nextBtn->isEnabled() == true) {
            if (curIndex == (stack->count()-1)) {
                writeSettingIni(1);
                emit enterpressed();
            } else if(curIndex == 0) {
                if(modesel != nullptr && modesel->currentBnt!=nullptr){
//                if (modesel->currentBnt != nullptr) {
                    //稍后创建
                    if (modesel->currentBnt->objectName().compare("Register_later") == 0) {
                        writeSettingIni(0);
                        emit signalStackPageChanged(1);
                    } else if(modesel->currentBnt->objectName().compare("Register_immediately") == 0) {
                    	setBtnValue();
                        curIndex++;
                        stack->setCurrentIndex(curIndex);
                        usregister->initURFocus();//bug215368
                        emit currentwidgetchange(stack->currentWidget());
                    } else if(modesel->currentBnt->objectName().compare("Use_old_later") == 0) {
                    	setBtnValue();
                        curIndex++;
                        stack->setCurrentIndex(curIndex);
                        emit currentwidgetchange(stack->currentWidget());
                    }
                }
                if(KServer::ReadSettingIni("config","first_mode").contains("oem2"))//处于OOBE阶段
                {
                    setBtnValue();
                    curIndex++;
                    stack->setCurrentIndex(curIndex);
        //            usregister->initURFocus();
                    emit currentwidgetchange(stack->currentWidget());
                }
            } else {
                curIndex++;
                stack->setCurrentIndex(curIndex);
                stack->currentWidget()->setFocus();
                if(modesel->currentBnt->objectName().compare("Register_immediately") == 0){
                    usregister->initURFocus();
                }
                emit currentwidgetchange(stack->currentWidget());
            }
        }
    } else if(event->key() == Qt::Key_Backspace) {
        if (curIndex) {
            curIndex--;
            stack->setCurrentIndex(curIndex);
            emit currentwidgetchange(stack->currentWidget());
        } else
            emit backspacepressed();
    } else if(event->key() == Qt::Key_Escape) {
        this->setFocus();
    } else
        QWidget::keyPressEvent(event);
}


void UserFrame::writeSettingIni(quint8 flag)
{
    WriteSettingToIni("config", "user_register", QString("%1").arg(flag));
    if(flag==0){
        WriteSettingToIni("config", "oem-config","true");}
    else{
        WriteSettingToIni("config", "oem-config","false"); }

     if(flag==0) return;

    WriteSettingToIni("config", "username", usregister->m_username->getLineEditText());
    WriteSettingToIni("config", "hostname", usregister->m_hostname->getLineEditText());
    WriteSettingToIni("config", "password", usregister->m_passwd->getLineEditText());
    WriteSettingToIni("config", "autologin", QString("%1").arg(usregister->m_autoLoginFlag));

      //  if(confer->later->isChecked()) return;//不设置

//    for(int k=0;k<confer->Lst.count();k++)
//    {
//        if(confer->later->isChecked())
//        {
//            KServer::WriteSettingToIni("config", QString("question%1").arg(k),confer->Lst.at(k)->question1->currentText());
//            KServer::WriteSettingToIni("config", QString("anser%1").arg(k),confer->Lst.at(k)->context1->text());
//        }
//        else
//        {
//            KServer::WriteSettingToIni("config", QString("question%1").arg(k),"");
//            KServer::WriteSettingToIni("config", QString("anser%1").arg(k),"");
//        }
//    }
}

void UserFrame::changeNavigations(bool isIgnor)//通知主框架隐藏或显示最后一个导航图标：实参值true隐藏，否则显示
{
    if(!m_startChangeModule)
        return;

    if(isIgnor){
        if(!m_lastregisterLaterstatus){
            m_lastregisterLaterstatus = true;
            m_lastregisterNowstatus = false;
            //emit signalchangeNavigations(1, "UserFrame");
        }
    }
    else{
        if(!m_lastregisterNowstatus){
            m_lastregisterNowstatus = true;
            m_lastregisterLaterstatus = false;
            //emit signalchangeNavigations(-1, "UserFrame");
        }
    }
}

void UserFrame::addStyleSheet()
{
    QFile file(":/res/qss/userfram.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void UserFrame::initFocusPolicy()
{
    this->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    stack->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    stack->currentWidget()->setFocusPolicy(Qt::FocusPolicy::NoFocus);

    if(modesel!=NULL)
    {
        this->setTabOrder(modesel->bntList.last(), this->m_nextBtn);
        modesel->bntList.first()->setFocus();
    }
    else if(usregister!=NULL)
    {
        this->setTabOrder(usregister->pwLoginCKBox, this->m_nextBtn);
    }

}

void UserFrame::reSetFocusPolicy()
{
    QString type=stack->currentWidget()->property("type").toString();
    if(type=="usregister")
    {
        this->setTabOrder(usregister->pwLoginCKBox, this->m_nextBtn);
    }
    else if(type=="modesel")
    {
        this->setTabOrder(modesel->bntList.last(), this->m_nextBtn);
        modesel->bntList.first()->setFocus();
    }
    else if(type=="m_securityquest")
    {
        this->setTabOrder(m_securityquest->m_Setuplater, this->m_nextBtn);
    }
}

}

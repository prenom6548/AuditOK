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

#include "appcheckframe.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>
#include <QCheckBox>
#include <QEvent>
#include <QKeyEvent>

#define APP_FILE_PATH_NOMAL  "/cdrom/apps-third"
#define APP_FILE_PATH_OOEM   "/opt/apps-third"
namespace KInstaller {

appcheckframe::appcheckframe(QWidget *parent) : MiddleFrameManager(parent)
{
    this->initNextBnt();
    qDebug()<<"oem-config="<<KServer::ReadSettingIni("config","first_mode")<<"+++++++++++++++++++++++++++";
    if(KServer::ReadSettingIni("config","first_mode").contains("oem2")==1)
    {
        SourcePath=APP_FILE_PATH_OOEM;

    }
    else{
            SourcePath=APP_FILE_PATH_NOMAL;
            QDir dir(APP_FILE_PATH_OOEM);
            if(dir.exists()==false)
                dir.mkpath("/opt/apps-third");
    }

    mIndex=0;
    initUI();

    qDebug()<<"find installer app="<<loadItems();

    addStyleSheet();
    initFocusPolicy();
}

appcheckframe::~appcheckframe()
{
}

void appcheckframe::initUI()
{
    this->setObjectName("appcheckframe");
    QVBoxLayout* mlayout=new QVBoxLayout();
    m_Widget->setLayout(mlayout);
    mlayout->setContentsMargins(0, 0, 0, 0);

    m_mainTitle=new QLabel;
    m_subTitle=new QLabel;
    m_mainTitle->setObjectName("appcheckpage_m_mainTitle");
    m_subTitle->setObjectName("appcheckpage_m_subTitle");

     mlayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));
     mlayout->addWidget(m_mainTitle, 1, Qt::AlignCenter);
     mlayout->addItem(new QSpacerItem(10, 16, QSizePolicy::Expanding, QSizePolicy::Minimum));//bug134324预装应用界面实现与设计稿不相符
     mlayout->addWidget( m_subTitle, 3, Qt::AlignCenter);
     mlayout->addItem(new QSpacerItem(10, 28, QSizePolicy::Expanding, QSizePolicy::Minimum));

    QHBoxLayout* clayout=new QHBoxLayout();
    stack=new QStackedWidget();
    stack->setMinimumWidth(2*m_Widget->size().height()/2);
 //   stack->setMaximumWidth(2*m_Widget->size().height()/2);
  //  clayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred));
    clayout->addWidget(stack,0,Qt::AlignHCenter);
    stack->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Expanding);
    //clayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred));
    mlayout->addLayout(clayout);
    mlayout->addItem(new QSpacerItem(100, 1200, QSizePolicy::Expanding, QSizePolicy::Expanding));


}

void appcheckframe::addStyleSheet()
{
    QFile file(":/new/prefix1/res/qss/KInstallAPP.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void appcheckframe::translateStr()
{
    m_mainTitle->setText(tr("Choose your app"));
    m_subTitle->setText(tr("Select the application you want to install to install with one click when using the system."));

    if(KServer::ReadSettingIni("config","first_mode").contains("oem2")==1){
        m_nextBtn->setText(tr("Start Configuration"));
    }
    else{
        m_nextBtn->setText(tr("Start Installation"));
    }

    QString language=KServer::ReadSettingIni("config", "language");
    for(int k=0;k<stack->count();k++)
    {
       appcheckpage* mPage =(appcheckpage*)stack->widget(k);
        mPage->transfer(language);

    }
}


quint16 appcheckframe::loadItems()
{
  // 清空QStackedWidget
        for(int k=stack->count();k>0;k--)
        {
            QWidget *p=stack->widget(k-1);
            stack->removeWidget(stack->widget(k-1));
            delete p;
        }
    QDir dir(SourcePath+"/resources");
    qDebug()<<"the app path="<<SourcePath+"/resources";
    if(!dir.exists()){
        qDebug()<<"this dir path not exist!";
        return 0;
    }
    QFileInfoList flst=dir.entryInfoList();
    int nSubDir = 0;
    QString strSubDirPath;
    foreach (QFileInfo one, flst) {
        if(one.isDir() && !one.isHidden())
        {
            strSubDirPath = one.filePath();
            nSubDir++;
        }
    }

    if(!nSubDir){
        qDebug()<<"not find subdirectory in this dir!";
        return 0;
    }

    if(nSubDir == 1){
        appcheckpage* mPage=new appcheckpage(this, nSubDir);
        mPage->AddAppCheckBox(strSubDirPath);
        stack->addWidget(mPage);

    }else{
        appcheckpage* mPage=new appcheckpage(this);

        foreach (QFileInfo one, flst) {
            qDebug()<<one.filePath();
            if(one.isDir()&&one.isHidden()==false)
            {
                if(mPage->lst.count()<20)
                {
                    mPage->AddAppCheckBox(one.filePath());
                }
                else
                {
                    stack->addWidget(mPage);
                    mPage=new appcheckpage(this);
                }

            }
        }

        if(mPage->lst.count()>0 && mPage->lst.count()<20)
        {
            stack->addWidget(mPage);
            if(mPage->lst.count()&1)//如果是奇数
            {
                int nHigh = mPage->LeftLayout->sizeHint().height();
                mPage->RightLayout->addItem(new QSpacerItem(32, nHigh, QSizePolicy::Expanding, QSizePolicy::Expanding));//加入空行使左右两列对齐(bug144918)
            }
        }
    }

    if(stack->count()>0){
        stack->setCurrentIndex(0);
        stack->currentWidget()->setFocus();
    }

    return stack->count();
}

bool appcheckframe::writeJson()
{
    QJsonDocument jdoc;
    QJsonArray AppList;
    for(int k=0;k<stack->count();k++)
    {
        appcheckpage* mPage=(appcheckpage*)stack->widget(k);
        foreach (AppCheckBox* box, mPage->lst) {
            if(box->isChecked()==true)
            {
                QJsonObject m_app;
                m_app["app_name"]=box->app_name;
                m_app["package_name"]=box->app_packge_name;
                if(box->app_kind.compare("deb")==0)
                    m_app["type"]="1";
                else if(box->app_kind.compare("apk")==0)
                    m_app["type"]="2";

                AppList.append(m_app);

            }
        }
    }
    jdoc.setArray(AppList);

    QString strApp;
    if(KServer::ReadSettingIni("config","first_mode").contains("oem2")){//oobe阶段
            strApp = QDir::homePath() + "/install.list";
    }
    else{
            strApp = QString(APP_FILE_PATH_OOEM) + "/install.list";
    }
    qDebug()<<"write selected app name to json file ["<<strApp<<"]";
    QFile jfile(strApp);
    if(jfile.open(QIODevice::ReadWrite))
        jfile.write(jdoc.toJson(QJsonDocument::Indented));
    else{
        qDebug()<<"Error:Unable to creat or write file ["<<strApp<<"]";
    }
    return 1;
}

void appcheckframe::clickNextButton()
{
  if(mIndex==stack->count()-1)
  {
      writeJson();
      emit signalStackPageChanged(1);
  }
  else
  {
        mIndex++;
        stack->setCurrentIndex(mIndex);
        stack->currentWidget()->setFocus();
  }
}

void appcheckframe::clickPrevButton(QWidget *p)
{
    if(p==this)
    {
        if(mIndex){
            mIndex--;
            qDebug()<<"appcheckframe::clickPrevButton(QWidget *p)="<<mIndex;
            stack->setCurrentIndex(mIndex);
            stack->currentWidget()->setFocus();
        }
        else
        {
	    qDebug()<<"appcheckframe::clickPrevButton(QWidget *p)=backspacepressed()";
            if(KServer::ReadSettingIni("config", "data-unformat")=="true")
            {
                qDebug()<<"1111111111111111111111111configdata-unformat";
                emit signalStackPageChanged(-2);
            }
            else
            {
                    emit backspacepressed();
            }

        }
    }
}

void appcheckframe::getmessage(QWidget *p, QString str)
{
    if(p==this)
    {
        if(stack->count()==0)
        {
            emit signalStackPageChanged(1);
        }
        else if(str.compare("next")==0)
        {
            if(KServer::ReadSettingIni("config","first_mode").contains("normal")==1
//                    && KServer::ReadSettingIni("config", "oem-config")=="true"
                    )
            {
		QString str=  KServer::ReadSettingIni("config","user_register");
                if(str.compare("1")!=0)//在安装阶段没有完成创建用户时直接跳过当前插件
                {
                    if(KServer::ReadSettingIni("config", "data-unformat")=="true")
                    {
                        qDebug()<<"1111111111111111111111111configdata-unformat";
                    }else {
                        KServer::WriteSettingToIni("config", "oem-config","true");//确保下阶段进入OOBE
                        emit signalStackPageChanged(1);
                    }
                }

            }
        }
        else if(str.compare("prext"))
        {
            QString str=  KServer::ReadSettingIni("config","user_register");
            if(str.compare("1")!=0)//在安装阶段没有完成创建用户时直接跳过当前插件
              emit signalStackPageChanged(-1);
        }

        translateStr();
    }
}

void appcheckframe::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        emit enterpressed();
    }
    else
        QWidget::keyPressEvent(event);
}

void appcheckframe::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) { //bug133405英文安装时，选择安装的应用未英化
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}
void appcheckframe::initFocusPolicy()
{
    if(stack->count()>0)
    {
        appcheckpage* tempPage=(appcheckpage*) stack->widget(stack->count()-1);
        this->setTabOrder(tempPage->lst.last(), m_nextBtn);
    }

}
}

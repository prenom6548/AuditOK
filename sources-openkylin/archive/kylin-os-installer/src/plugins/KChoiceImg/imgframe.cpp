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

#include "imgframe.h"
#include "QStyleFactory"
#include <QDebug>

#include "../PluginService/klinguist.h"
#include <QTextCodec>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QListView>
#include <QScrollBar>
#include <QToolTip>

#include <QFileDialog>
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/base_unit/file_unit.h"
#include "../PluginService/kcommand.h"

using namespace KServer;
namespace KInstaller {


ImgFrame::ImgFrame(QWidget *parent) : MiddleFrameManager(parent)
{
    this->setObjectName("ImgFrame");
    //添加下一步按键
    this->initNextBnt();
    
    m_thread=new KyimgOsproberProcess(this);

    initUI();
    m_nextBtn->setEnabled(true);
    initAllConnect();
    addStyleSheet();
    translateStr();
    this->setFocus();
    liveChoice->setChecked(true);
    liveInstallStatus = true;
    ghostInstallStatus = false;
    initFocusPolicy();
}

void ImgFrame::initUI()
{

    QGridLayout *chiceimgupLayout = new QGridLayout();
//    m_Widget->setLayout(chiceimgupLayout);
    m_Widget->setLayout(chiceimgupLayout);
    chiceimgupLayout->setContentsMargins(0, 0, 0, 0);
//    chiceimgupLayout->setSpacing(0);

    chiceimgupLayout->setColumnStretch(0, 3);
    chiceimgupLayout->setColumnStretch(1, 2);
    chiceimgupLayout->setColumnStretch(2, 3);

    //选择安装途径
    m_mainTitle = new QLabel();
    m_mainTitle->setObjectName("mainTitle");

    chiceimgupLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    chiceimgupLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop );
    chiceimgupLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);

    //从Live安装
    liveChoice = new QRadioButton(this);
    liveChoice->setObjectName("liveChoice");
    liveChoice->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    QHBoxLayout *liveChoicelayout = new QHBoxLayout();
    liveChoicelayout->setContentsMargins(0, 0, 0, 0);
//    liveChoicelayout->setSpacing(0);
    liveChoicelayout->addWidget(liveChoice,Qt::AlignLeft| Qt::AlignBottom);
    liveChoicelayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Preferred));
    chiceimgupLayout->addLayout(liveChoicelayout, 3, 1, 1,1,Qt::AlignCenter);
    chiceimgupLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Preferred), 4, 1, 1, 1);
    //从Ghost安装
    ghostChoice = new QRadioButton(this);
    ghostChoice->setObjectName("ghostChoice");
    ghostChoice->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    QHBoxLayout *ghostChoicelayout=new QHBoxLayout();
    ghostChoicelayout->setContentsMargins(0, 0, 0, 0);
//    ghostChoicelayout->setSpacing(0);
    ghostChoicelayout->addWidget(ghostChoice,Qt::AlignLeft| Qt::AlignBottom);
    ghostChoicelayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Preferred));
    chiceimgupLayout->addLayout(ghostChoicelayout, 5, 1, 1, 1, Qt::AlignCenter);
    chiceimgupLayout->addItem(new QSpacerItem(10, 15, QSizePolicy::Expanding, QSizePolicy::Preferred), 6, 1, 1, 1);
 //   filePath = new QLineEdit(this);
    filePath = new QComboBox(this);
    filePath->setObjectName("filePath");
    filePath->setView(new QListView());
    filePath->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    filePath->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    filePath->view()->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);

    //filePath->setMinimumWidth(420);
    filePath->setMinimumWidth(500);
    filePath->setFixedHeight(48);
    filePath->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
    chiceimgupLayout->addWidget( filePath, 7, 1, 1,1,Qt::AlignLeft );
    chiceimgupLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 4, 1, 1, 1);

    filePath->clear();
    filePath->setInsertPolicy(QComboBox::InsertAtBottom);
    
    kyimgList =getKyimgs();

   // QStandardItemModel *model = new QStandardItemModel();//添加提示tootip
   // for(int i = 0; i < kyimgList.size(); ++i){
   //     QStandardItem *item = new QStandardItem(kyimgList.at(i));;
   //     item->setToolTip(kyimgList.at(i));
   //     model->appendRow(item);
   // }
   // filePath->setModel(model);

    filePath->setMaxVisibleItems(kyimgList.count());
    qDebug() << "kyimgList.count()="<<kyimgList.count();
    filePath->addItems(kyimgList);
    filePath->setEditable(true);
    filePath->setStyleSheet("QComboBox{color: rgb(100, 100, 100)}"
		    "QToolTip { background-color:transparent;color: rgb(255, 255, 255); border: 1px solid white;border-radius: 3px;}");
    filePath->setEnabled(false);


    QHBoxLayout *filelayout=new QHBoxLayout();
    filelayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Preferred));

    chiceimgupLayout->addItem(new QSpacerItem(10, 100, QSizePolicy::Expanding, QSizePolicy::Expanding), 8, 1, 1,1 );
}

QStringList ImgFrame::getKyimgs()
{
    QStringList kyimgs = {};
    qDebug() << "kyimgs1="<<kyimgs;
    kyimgs=KServer::KReadFile("/var/log/installer/ghost.log").split("\n");
    qDebug() << "kyimgs2="<<kyimgs;

    for(int i = 0; i < kyimgs.size(); ++i){
        if(kyimgs[i].isEmpty()){
                kyimgs.removeAt(i);
        }
    }

    return (QStringList)kyimgs;
}

void ImgFrame::initFocusPolicy()
{
     this->setTabOrder(liveChoice, this->ghostChoice);
     this->setTabOrder(ghostChoice, this->filePath);
     this->setTabOrder(filePath, this->m_nextBtn);
}

void ImgFrame::proberKyimgs()
{
    kyimgList =getKyimgs();
    if(kyimgList!=oldKyimgs){
    	filePath->clear();
	//QStandardItemModel *model = new QStandardItemModel();//添加提示tootip
        //for(int i = 0; i < kyimgList.size(); ++i){
        //        QStandardItem *item = new QStandardItem(kyimgList.at(i));;
        //        item->setToolTip(kyimgList.at(i));
        //        model->appendRow(item);
        //}
        //filePath->setModel(model);

    	filePath->setMaxVisibleItems(kyimgList.count());
    	qDebug() << "kyimgList.count()="<<kyimgList.count();
    	filePath->addItems(kyimgList);
	oldKyimgs=kyimgList;
    }
}

void ImgFrame::initAllConnect()
{
    connect(filePath, &QComboBox::currentTextChanged, this, &ImgFrame::changefilePath);
    connect(liveChoice,&QRadioButton::clicked,this,&ImgFrame::ChangeLoad1);
    connect(ghostChoice,&QRadioButton::clicked,this,&ImgFrame::ChangeLoad2);
    connect(m_thread, &KyimgOsproberProcess::signalKyimgOsprober, this, &ImgFrame::proberKyimgs);
}
void ImgFrame::addStyleSheet()
{
    QFile file(":/res/qss/KChoiceImg.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}
void ImgFrame::changefilePath(const QString &text)
{
    filePath->setCurrentText(text);
    if(text.length()>=50){
            filePath->setToolTip(text);
    }
    else{
            filePath->setToolTip("");
    }
    if(text.isEmpty()){
	qDebug() << "changefilePath text is isEmpty";
        m_nextBtn->setEnabled(false);
    }else {
	qDebug() << "changefilePath text is not isEmpty";
        m_nextBtn->setEnabled(true);
    }

}

void ImgFrame::ChangeLoad1()
{
    filePath->setEnabled(false);
    filePath->setEditable(true);
    filePath->setStyleSheet("QComboBox{color: rgb(100, 100, 100)}"
		    "QToolTip { background-color:transparent;color: rgb(255, 255, 255); border: 1px solid white;border-radius: 3px;}");
    m_nextBtn->setEnabled(true);
    WriteSettingToIni("setting","ghost","false");
    liveInstallStatus = true;
    ghostInstallStatus = false;
    liveChoice->setChecked(true);
    ghostChoice->setChecked(false);
    
    if(m_thread->isRunning()){
   	m_thread->terminate();
	KServer::KCommand::getInstance()->RunScripCommand("sync", {});
	KServer::KCommand::getInstance()->RunScripCommand("umount", {"/var/log/installer/ghost"});
    }
}

void ImgFrame::ChangeLoad2()
{
   if(filePath->currentText().isEmpty()){
        m_nextBtn->setEnabled(false);
   }
   m_thread->start();

   filePath->setEnabled(true);
   filePath->setStyleSheet("QComboBox{color: rgb(255, 255, 255)}"
		    "QToolTip { background-color:transparent;color: rgb(255, 255, 255); border: 1px solid white;border-radius: 3px;}");
   filePath->setEditable(false);
   ghostInstallStatus = true;
   liveInstallStatus = false;
   liveChoice->setChecked(false);
   ghostChoice->setChecked(true);

}

void ImgFrame::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter) {
        if(m_nextBtn->isEnabled()==false) {
            return;
        }
        emit enterpressed();
    } else if (event->key() == Qt::Key_Up) {
        if (ghostInstallStatus == true && liveInstallStatus == false) {
            emit liveChoice->clicked();
            liveChoice->setFocus();

        } else if (ghostInstallStatus == false && liveInstallStatus == true) {
            emit ghostChoice->clicked();
            ghostChoice->setFocus();
        }
    } else if (event->key() == Qt::Key_Down) {
        if (ghostInstallStatus == true && liveInstallStatus == false) {
           emit liveChoice->clicked();
            liveChoice->setFocus();
        } else if (ghostInstallStatus == false && liveInstallStatus == true) {
            emit ghostChoice->clicked();
            ghostChoice->setFocus();
        }
    } else if (event->key() == Qt::Key_Return) {
        emit enterpressed();
    } else if (event->key() == Qt::Key_Backspace) {
        emit backspacepressed();
    } else
        QWidget::keyPressEvent(event);
}

void ImgFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
       update();
    } else {
        QWidget::changeEvent(event);
    }
}

void ImgFrame::showEvent(QShowEvent *event)
{
    this->setFocus();
}

void ImgFrame::translateStr()
{
    m_mainTitle->setText(tr("Select install way"));//选择安装路径
    ghostChoice->setText(tr("install from ghost"));//ghost安装
    liveChoice->setText(tr("install from live"));//live安装
    m_nextBtn->setText(tr("Next"));
}


void ImgFrame::clickNextButton()
{
    if(m_nextBtn->isEnabled()==false)
        return;
    if(liveChoice->isChecked()==true) 
    {
        if(ReadSettingIni("setting","ghost-path").isEmpty() ==false)
        {
            WriteSettingToIni("setting","ghost","false");
            WriteSettingToIni("setting","ghost-path","");
    	}
    }else if(ghostChoice->isChecked()==true){
         WriteSettingToIni("setting","ghost","true");
         WriteSettingToIni("setting","ghost-path",filePath->currentText());
    }
    if(m_thread->isRunning()){
   	m_thread->terminate();
	KServer::KCommand::getInstance()->RunScripCommand("sync", {});
	KServer::KCommand::getInstance()->RunScripCommand("umount", {"/var/log/installer/ghost"});
    }
      emit signalStackPageChanged(100);
//    else
//    emit signalStackPageChanged(1);
}

}

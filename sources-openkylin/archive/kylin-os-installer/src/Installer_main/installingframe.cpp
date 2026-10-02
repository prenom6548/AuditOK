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

#include "installingframe.h"
#include <QGridLayout>
#include <QEvent>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QApplication>
#include <sys/stat.h>
#include <QThread>
#include <QKeyEvent>
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/kcommand.h"
#include "QProcess"
#define CFG_PATH    "/var/log/installer/kylin-os-installer.log"
#define CFG_DETAIL_PATH    "/var/log/installer/kylin-os-installer-detail.log"
#include "QTextCursor"
#include <QScrollBar>

#define PROGRESS_RATIO 0.8
#define SHOWDETAILINSTALLMSG    1
#define SHOWSIMPLEINSTALLMSG    0
#define SHOWERRORINSTALLMSG     2

namespace KInstaller {
using namespace KServer;
InstallingFrame::InstallingFrame(QWidget *parent) : QWidget(parent)
{
    initUI();
    addStyleSheet();
    progressBarCtrl = new ShowProgressBar(this);
    mywatcher=new QFileSystemWatcher(this);
    mywatcher->addPath(CFG_PATH);
    mywatcherDetail=new QFileSystemWatcher(this);
    mywatcherDetail->addPath(CFG_DETAIL_PATH);
    initAllConnect();
    //恢复鼠标正常状态
    QApplication::restoreOverrideCursor();
    showMsgDetail();
    slide->setFocus();
    translateStr();
}

InstallingFrame::~InstallingFrame()
{

}

void InstallingFrame::initUI()
{
    gridLayout = new QVBoxLayout;
    this->setLayout(gridLayout);

    //1.标题，安装时隐藏
    m_mainTitle=new QLabel(this);
    m_mainTitle->setObjectName("mainTitle");
    gridLayout->addWidget(m_mainTitle,0,Qt::AlignCenter);
    m_mainTitle->setVisible(false);

    //2.轮播框
    slide = new SlideShow(this);
    slide->setFocus();
    gridLayout->addItem(new QSpacerItem(10, 24, QSizePolicy::Expanding, QSizePolicy::Expanding));
    gridLayout->addWidget(slide, 2, Qt::AlignCenter);

    //3.副标题
    littleTitle = new QLabel(this);
    littleTitle->setObjectName("littleTitle");
    littleTitle->setStyleSheet("QLabel#littleTitle{font-size: 24px;font-family: Noto Sans CJK SC;color: #FFFFFF;opacity: 1;}");
    gridLayout->addItem(new QSpacerItem(10, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
    gridLayout->addWidget(littleTitle, 4, Qt::AlignCenter);
    gridLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    //4.进度条
    m_installProgressBar = new myprobar(this);
    QRect rect = KServer::GetScreenRect();
    m_installProgressBar->setFixedWidth(rect.width() * PROGRESS_RATIO);
    m_installProgressBar->setObjectName(QStringLiteral("installProgressBar"));
    m_installProgressBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    gridLayout->addWidget(m_installProgressBar);

    //5.显示日志详情按键及日志简略显示
    textDetail=new QPushButton(this);
    textDetail->setObjectName("textDetail");
    QHBoxLayout *hbox= new QHBoxLayout(this);
    hbox->setContentsMargins(10, 0, 10, 0);
    textDetail->setLayout( hbox);
    stext=new QLabel;
    stext->setObjectName("LogText");
    hbox->addWidget(stext,1,Qt::AlignCenter);
    spic=new QLabel;
//    QImage *img=new QImage;
//    img->load(":/data/png/pan-up-symbolic.svg");
//    spic->setPixmap(QPixmap::fromImage(*img));
    spic->setPixmap(QIcon(":/data/png/pan-up-symbolic.svg").pixmap(11,5));
    hbox->addWidget(spic,0,Qt::AlignRight);
    MsgContext=new QLabel;
    MsgContext->setObjectName("MsgContext");
    MsgContext->setMaximumSize(1000,20);
    MsgContext->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);

    bnt_box= new QHBoxLayout(this);
    bnt_box->addWidget(MsgContext,0,Qt::AlignLeft);
    bnt_box->addWidget(textDetail,1,Qt::AlignRight);
    gridLayout->addLayout(bnt_box);

    textMsg=new QTextBrowser(this);
    textMsg->setFocusPolicy(Qt::TabFocus);
    textMsg->setObjectName("textMsg");
    textMsg->setMinimumSize(920,580);
    textMsg->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
    textMsg->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);//解决bug161596
    textMsg->verticalScrollBar()->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
    gridLayout->addWidget(textMsg,6,Qt::AlignCenter);

    gridLayout->addItem(new QSpacerItem(10, 150, QSizePolicy::Expanding, QSizePolicy::Expanding));

   // fi=new QFile(CFG_PATH);
   // if(!fi->open(QFile::ReadWrite|QFile::Text))
   //     qDebug()<<"log open fail";
   // currline = fi->size();
   // fi->close();
    fi=new QFile(CFG_PATH);
    if(!fi->open(QFile::ReadWrite|QFile::Text))
        qDebug()<<"First attempt to open kylin-os-installer.log failed";
    else{
        currline = fi->size();
        fi->close();
    }

    fiDetail=new QFile(CFG_DETAIL_PATH);
    if(!fiDetail->open(QFile::ReadWrite|QFile::Text))
        qDebug()<<"First attempt to open kylin-os-installer.log failed";
    else{
        currlineDetail = fiDetail->size();
        fiDetail->close();
    }

}

void InstallingFrame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void InstallingFrame::addStyleSheet()
{
    QFile file(":/data/qss/InstallingFrame.css");
    file.open(QFile::ReadOnly|QFile::Text);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void InstallingFrame::translateStr()
{
    //    m_mainTitle->setText((tr("Installing")));//安装进行中
    littleTitle->setText(tr("The system is being installed, please do not turn off the computer"));//系统正在安装，请不要关闭计算机
    stext->setText(tr("Log"));
}

void InstallingFrame::initAllConnect()
{
    connect(progressBarCtrl, &ShowProgressBar::signelProgressValue, this, &InstallingFrame::updateProgressValue);
    connect(textDetail,&QPushButton::clicked,this,&InstallingFrame::showMsgDetail);
    connect(mywatcher,&QFileSystemWatcher::fileChanged,this,&InstallingFrame::flashMsg);
    connect(mywatcherDetail,&QFileSystemWatcher::fileChanged,this,&InstallingFrame::flashMsgDetail);
}

void InstallingFrame::showErrorFrame()
{
    textMsg->setVisible(true);//显示日志
    textMsg->setMinimumWidth(920);
    QRect rect = KServer::GetScreenRect();
    textMsg->setFixedHeight(rect.height() * 0.35);//解决bug156622
    textMsg->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::Fixed);
    textMsg->moveCursor(QTextCursor::End);


    //1.销毁轮播图控件
    slide->setParent(nullptr);
    delete slide;

    //2.销毁进度条
    m_installProgressBar->setParent(nullptr);
    delete m_installProgressBar;

    //3.销毁,按键和单行显示
    textDetail->setParent(nullptr);
    delete textDetail;

    MsgContext->setParent(nullptr);
    delete MsgContext;

    //4.显示主题，安装失败
    m_mainTitle->setText(tr("Install failed"));
    m_mainTitle->setVisible(true);
    littleTitle->setText(tr("Sorry, KylinOS cannot continue the installation. Please feed back the error log below so that we can better solve the problem for you."));

    // 5.添加重启按键
    gridLayout->addItem(new QSpacerItem(32,53, QSizePolicy::Minimum, QSizePolicy::Preferred));
    m_restartBtn = new QPushButton(this);

    m_restartBtn->setObjectName("restartBtn");
    m_restartBtn->setText(tr("Restart"));
    gridLayout->addWidget(m_restartBtn, 8, Qt::AlignCenter | Qt::AlignTop);
    connect(m_restartBtn,&QPushButton::clicked,this,&InstallingFrame::clickRestartBtn);
    is_faild=1;//安装失败标志
}

void InstallingFrame::findFile()
{
    m_installProgressBar->setValue(100);
    emit signalFinished();
}

void InstallingFrame::showError()
{
    showErrorFrame();
}

void InstallingFrame::updateProgressValue(int value)
{
    m_installProgressBar->setValue(value);
    //if (value == 90) {
    //    m_installProgressBar->setValue(96);
    //}
}

void InstallingFrame::closeW()
{
    progressBarCtrl->timer->stop();
}

void InstallingFrame::showMsgDetail()
{
    if(flag)
    {
//        QImage *img=new QImage;
//        img->load(":/data/png/pan-down-symbolic.svg");
//        spic->setPixmap(QPixmap::fromImage(*img));
        spic->setPixmap(QIcon(":/data/png/pan-down-symbolic.svg").pixmap(11,5));
        flag=0;
        textMsg->setVisible(false);
        slide->setVisible(true);
    }
    else
    {
//        QImage *img=new QImage;
//        img->load(":/data/png/pan-up-symbolic.svg");
//        spic->setPixmap(QPixmap::fromImage(*img));
        spic->setPixmap(QIcon(":/data/png/pan-up-symbolic.svg").pixmap(11,5));
        flag=1;
        slide->setVisible(false);
        textMsg->setVisible(true);
    }
}

void InstallingFrame::flashMsg()
{
    if(!fi->open(QFile::ReadOnly|QFile::Text))
        qDebug()<<"fail to open kylin-os-installer.log";
    if(fi->size() == currline)//过滤掉无内容改变的触发
    {
        qDebug()<<"QFileSystemWatcher::fileChanged void!";
        fi->close();
        mywatcher->addPath(CFG_PATH);
        return;
    }

    fi->seek(currline-1);
    currline=fi->size()+1;
    QString strT;
    while(!fi->atEnd())
    {
        //strT = QString::fromLocal8Bit(fi->readLine());
        strT = fi->readLine();
        if(strT.isEmpty() ||strT.size() == 1)//过滤掉内容改变不完整的触发
            continue;
        if('\n' == strT[0])
        {
            strT.remove(0,1);
        }

        MsgContext->setText(strT);
        //textMsg->append(strT);
        //        MsgContext->setText(fi->readLine());
        //        textMsg->append(MsgContext->text());
        //textMsg->moveCursor(QTextCursor::End);
    }
    fi->close();
    mywatcher->addPath(CFG_PATH);
}

void InstallingFrame::flashMsgDetail()
{
    if(!fiDetail->open(QFile::ReadOnly|QFile::Text))
        qDebug()<<"fail to open kylin-os-installer.log";
    if(fiDetail->size() == currlineDetail)//过滤掉无内容改变的触发
    {
        qDebug()<<"QFileSystemWatcher::fileChanged void!";
        fiDetail->close();
        mywatcherDetail->addPath(CFG_DETAIL_PATH);
        return;
    }

    fiDetail->seek(currlineDetail-1);
    currlineDetail=fiDetail->size()+1;
    QString strT;
    while(!fiDetail->atEnd())
    {
        strT = fiDetail->readLine();
        if(strT.isEmpty() ||strT.size() == 1)//过滤掉内容改变不完整的触发
            continue;
        if('\n' == strT[0])
        {
            strT.remove(0,1);
        }

        textMsg->append(strT);
        textMsg->moveCursor(QTextCursor::End);
    }
    fiDetail->close();
    mywatcherDetail->addPath(CFG_DETAIL_PATH);
}

void InstallingFrame::clickRestartBtn()
{
    QProcess process;
    process.startDetached("systemctl reboot");
}

void InstallingFrame::keyPressEvent(QKeyEvent *event)
{
    if((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)&& is_faild)
        clickRestartBtn();
    else
        QWidget::keyPressEvent(event);
}

}

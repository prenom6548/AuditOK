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

#include "installingoemconfigframe.h"
#include <QLabel>
#include <QGridLayout>
#include <QEvent>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include "../PluginService/backendThread/oeminstallerprocess.h"
#include "../PluginService/kthread.h"
#include "ksystemsetting_unit.h"
#include <QFile>
#include "../plugins/KUserRegister/securityquestions.h"
#include "usersecurityquestions.h"
#include <QProcess>

//extern void finishSecurityQuestionBindG();

namespace KInstaller {
using namespace KServer;

InstallingOEMConfigFrame::InstallingOEMConfigFrame(UserFrame* pUF, QWidget *parent) : QWidget(parent)
  ,m_backOEMThread(new QThread(this))
  ,m_oemProcess(new OemInstallerProcess)
  ,var(0)
  ,m_UFplugin(pUF)
{
    initUI();
    addStyleSheet();
    m_oemProcess->moveToThread(m_backOEMThread);
    m_backOEMThread->start();
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, &InstallingOEMConfigFrame::finishLoadDisk);

    qDebug()<<"UserFrame plugin in:"<<pUF;
    //准备密保绑定
    if(m_UFplugin != nullptr && m_UFplugin->m_securityquest != nullptr){
        qDebug()<<"Get UserFrame plugin in OOBE";
//        connect(m_UFplugin->m_securityquest, SIGNAL(securityquestions::lsSetsetUserQestA(bool)), this, SLOT(setsetUserQestAnsw(bool)));
//        connect(m_UFplugin->m_securityquest, &securityquestions::lsSetsetUserQestA, this, &InstallingOEMConfigFrame::setsetUserQestAnsw);
//        connect(this, SIGNAL(setSecurityAnswers()), m_UFplugin->m_securityquest, SLOT(securityquestions::finishSecurityQuestionBind));
//        connect(this, &InstallingOEMConfigFrame::setSecurityAnswers, m_UFplugin->m_securityquest, &securityquestions::finishSecurityQuestionBind);
//        connect(m_UFplugin->m_securityquest, &securityquestions::setSecurityAnswersErro, [=]{emit signalExecError("Set Security Answers Erro");});
//        connect(m_UFplugin->m_securityquest, SIGNAL(securityquestions::setSecurityAnswersErro), this, SLOT(InstallingOEMConfigFrame::sendUserSecurityAnswersErro()));
        if(m_UFplugin->m_securityquest->m_Setuplater->isChecked())
            m_setUserQestAnsw = false;
    }
    else{
        m_setUserQestAnsw = false;
    }

    if(ReadSettingIni("config", "data-unformat")=="true")//由于此时隐藏了用户注册界面，所以不用设置密保
    {
        m_setUserQestAnsw = false;
    }

    if(m_setUserQestAnsw)
        qDebug()<<"Need to set Security answers";
    else
        qDebug()<<"No need to set Security answers";

    timer->start();
}

InstallingOEMConfigFrame::~InstallingOEMConfigFrame()
{
    quitThreadRun(m_backOEMThread);
}

void InstallingOEMConfigFrame::initUI()
{
    layout = new QGridLayout;
    this->setLayout(layout);
    layout->setSpacing(50);
    label = new QLabel;
    label->setStyleSheet("QLabel { font-size:42px; color:#FFFFFF; }");
    layout->setRowStretch(0, 3);
    layout->addWidget(label, 1, 0, 1, 1, Qt::AlignCenter);
//    layout->addItem(new QSpacerItem(10, 36, QSizePolicy::Fixed, QSizePolicy::Fixed), 2, 0, 1, 1);
    m_loadpic = new QLabel;
    m_loadpic->setObjectName("loadpic");
    m_loadpic->setPixmap(QPixmap(":/data/svg/loading/ukui-loading-0.svg").scaled(QSize(48, 48), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadpic->setScaledContents(true);
    layout->addWidget(m_loadpic, 2, 0, 1, 1, Qt::AlignCenter);
//    label1 = new QLabel;
//    label1->setStyleSheet("QLabel { font-size:14px; color:#FFFFFF; }");
//    layout->addWidget(label1, 3, 0, 1, 1, Qt::AlignCenter);
    layout->setRowStretch(4, 4);

    connect(m_backOEMThread, &QThread::started, m_oemProcess, &OemInstallerProcess::exec);
    connect(m_oemProcess, &OemInstallerProcess::signalProcessStatus, [=]{emit signalExecError("oem erro");});
    translateStr();
}


void InstallingOEMConfigFrame::addStyleSheet()
{

}

void InstallingOEMConfigFrame::translateStr()
{
    label->setText(tr("Progressing system configuration"));
//    label1->setText(tr("Please wait patiently......"));
}

void InstallingOEMConfigFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void InstallingOEMConfigFrame::finishLoadDisk()
{
    qDebug() << "finishLoadDisk";
    var++;
    if(var > 7) {
        var = 0;
    }
    QString s = QString(":/data/svg/loading/ukui-loading-%1.svg").arg(var);

    m_loadpic->setPixmap(QPixmap(s).scaled(QSize(48, 48), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadpic->setScaledContents(true);
    m_loadpic->update();

    //密保
    if(!m_lsQADone){//密保没有设置好
        if(m_setUserQestAnsw){//需要设置密保
            QFile file("/var/log/installer/oem-security.cfg");
            file.open(QFile::ReadOnly);
            QTextStream filetext(&file);
            QString strSecurity = filetext.readAll();
            file.close();
    //        QString strSecurity = KServer::ReadSettingIni("config","security");
            qDebug()<<"/var/log/installer/oem-security.cfg:"<<strSecurity<<"  size:"<<strSecurity.size();
            if(!strSecurity.isEmpty() && strSecurity.contains("security=true")){//开始设置密保
                qDebug()<<"Start setting Security answers";
                if(finishSecurityQuestionBindG(m_UFplugin->m_securityquest->m_uniauthService, m_UFplugin->m_securityquest->m_euid,m_UFplugin->m_securityquest->m_encryptedAnswerlist)){
                    WriteSettingToIni("config","security","true");
                }else{
                    emit signalExecError("oem erro:Set Security Answers Erro");
                }

//                int nRet = m_UFplugin->m_securityquest->m_uniauthService->BindUserSecurityAnswers(1000, m_UFplugin->m_securityquest->m_encryptedAnswerlist);
//                qDebug() << "Bind ret:" << nRet;
//                emit setSecurityAnswers();
//                m_UFplugin->m_securityquest->finishSecurityQuestionBind();
//                finishSecurityQuestionBindG();
                m_lsQADone = true;
            }
        } else {//不需要设置密保，就解除后台阻塞
            WriteSettingToIni("config","security","true");
            m_lsQADone = true;
        }
    }
}

//void InstallingOEMConfigFrame::setsetUserQestAnsw(bool lsSet)
//{
//    m_setUserQestAnsw = !lsSet;
//    if(m_setUserQestAnsw)
//        qDebug()<<"Need to set Security answers";
//    else
//        qDebug()<<"No need to set Security answers";
//}

void InstallingOEMConfigFrame::sendUserSecurityAnswersErro()
{
    emit signalExecError("oem erro:Set Security Answers Erro");
}

void InstallingOEMConfigFrame::showOEMErrorFrame()
{
    timer->stop();
    m_loadpic->setVisible(false);
    label->setText(tr("OEM Error"));
    // 添加重启按键
//    layout->addItem(new QSpacerItem(32,53, QSizePolicy::Minimum, QSizePolicy::Preferred));
    layout->addItem(new QSpacerItem(10, 200, QSizePolicy::Fixed, QSizePolicy::Fixed), 3, 0, 1, 1);
    QPushButton* restartBtn = new QPushButton(this);
    restartBtn->setObjectName("restartBtn");
    restartBtn->setText(tr("Restart"));
    layout->addWidget(restartBtn, 4, 0, 1, 1, Qt::AlignCenter);
    connect(restartBtn,&QPushButton::clicked,this,&InstallingOEMConfigFrame::clickRestartBtn);
}

void InstallingOEMConfigFrame::clickRestartBtn()
{
    QProcess process;
    process.startDetached("systemctl reboot");
}

}

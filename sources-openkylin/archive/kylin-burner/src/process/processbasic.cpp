/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include "processbasic.h"
#include "src/frame/signals.h"
#include "src/frame/logger.h"
#include "src/frame/configuration.h"
#include "src/po/translation.h"
#include "src/view/modaldialog.h"
#include "src/view/tip_burnresult.h"
#include "src/model/logicmain.h"

#include <QStandardPaths>
#include <QCoreApplication>
#include <QDebug> 
#include <QEventLoop>

ProcessBasic::ProcessBasic(QObject *parent) :
    QObject(parent),
    m_oProcess(new QProcess(this)),
    m_bCancel(false),
    m_oIcon(QIcon::fromTheme("ukui-loading-0-symbolic"))
{
    connect(MODAL, &ModalDialog::rejected, this, &ProcessBasic::slotsCancel);

    connect(this, SIGNAL(title(QString)), ModalDialog::instance()->getPageByIndex(ModalDialog::Progress), SIGNAL(title(QString)));
    connect(this, SIGNAL(setRange(int,int)), ModalDialog::instance()->getPageByIndex(ModalDialog::Progress), SIGNAL(setRange(int, int)));
    connect(this, SIGNAL(setValue(int,QIcon,QString)), ModalDialog::instance()->getPageByIndex(ModalDialog::Progress), SIGNAL(setValue(int,QIcon,QString)));
    connect(this, SIGNAL(addValue(int,QIcon,QString)), ModalDialog::instance()->getPageByIndex(ModalDialog::Progress), SIGNAL(addValue(int,QIcon,QString)));

    connect(this, &ProcessBasic::success, TIP_BURNRESULT, &Tip_BurnResult::slotsSuccess);
    connect(this, &ProcessBasic::fail, TIP_BURNRESULT, &Tip_BurnResult::slotsFailed);
    connect(this, &ProcessBasic::title, TIP_BURNRESULT, &Tip_BurnResult::slotsTitle);
    connect(this,  &ProcessBasic::umountfail, TIP_BURNRESULT,  &Tip_BurnResult::slotsUmountFailed);

    connect(LogicMain::instance(), &LogicMain::exit, this, &ProcessBasic::slotsExit);
}

void ProcessBasic::exec()
{
    int step;

    m_lastErrorMsg = "";
    m_oKeep = true;
    step = Prepare;
    m_bCancel = false;

    connection();

    while (!m_bCancel && m_oKeep && step <= Worked)
    {
        switch (step++)
        {
        case Prepare:
            m_oKeep = prepare();
            break;
        case Config:
            m_oKeep = config();
            break;
        case Working:
            m_oKeep = working();
            break;
        case Worked:
            m_oKeep = worked();
            break;
        default:
            break;
        }
    }

    disconnect(this, SIGNAL(cancel(bool)), MODAL, SLOT(cancelEnable(bool)));
    QString UserPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    QString strUsr = UserPath.section("/",-1,-1);


    if (m_bCancel)
    {
        QString strDes;
        if(m_oArguments.count() >= 2)
        {
            strDes = m_oArguments.at(1);
        }
        MODAL->hide();
        fail(m_oProcessName);
        eject();
        return;
    }

    if (!m_oKeep)
    {
        QString strDes;
        if(m_oArguments.count() >= 2)
        {
            strDes = m_oArguments.at(1);
        }
        MODAL->hide();
        if (m_lastErrorMsg.isEmpty()) {
            fail(m_oProcessName);
        } else {
            fail(m_lastErrorMsg + m_oProcessName);
            m_lastErrorMsg = "";
        }
        eject();
        return;
    }

    if (m_oProcessName == _("load") || m_oProcessName == _("Media Info")) return;
    QString strDes;
    if(m_oArguments.count() >= 2)
    {
        strDes = m_oArguments.at(1);
    }
    MODAL->hide();
    success(m_oProcessName);
}

bool ProcessBasic::prepare()
{
    return m_oKeep;
}

bool ProcessBasic::config()
{
    return m_oKeep;
}

bool ProcessBasic::working()
{
    if (m_bCancel) {
        LOGGER->info(QString("ProcessBasic::working User cancel operate, m_bCancel = %1").arg(m_bCancel));
        return false;
    }

    QEventLoop eventloop;
    connect(m_oProcess, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished), this, [&](int,QProcess::ExitStatus){
                eventloop.quit();
    });

    m_oProcess->setArguments(m_oArguments);
    m_oProcess->start();

    eventloop.exec();

    return m_oKeep;
}

bool ProcessBasic::worked()
{
    return m_oKeep;
}

void ProcessBasic::slotsCancel()
{
    if (m_bCancel) return;
    m_bCancel = true;
    if(m_oProcessName==_("Media Info")){
        m_bCancel=false;
        return;
    }
    if (m_oProcess->state() != QProcess::NotRunning)
    {
        LOGGER->debug(_("Stop process when user canceled."));
        m_oProcess->kill();
    }
}

void ProcessBasic::slotsExit()
{
    m_oProcess->kill();
}

void ProcessBasic::connection()
{
    connect(m_oProcess, SIGNAL(started()), this, SLOT(slotsStart()));
    connect(m_oProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(slotsOutput()));
    connect(m_oProcess, SIGNAL(readyReadStandardError()), this, SLOT(slotsError()));
    connect(m_oProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(slotsFinished(int,QProcess::ExitStatus)));
}

ProcessBasic::~ProcessBasic()
{}

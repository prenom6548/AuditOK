/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "netcheck.h"
#include <QThread>
#include <QEventLoop>
#include <QFuture>
#include <QtConcurrent>
#include <curl/curl.h>

NetCheck::NetCheck(QObject *parent) : QObject(parent)
{
    // set plugin type
    setPluginType(CheckType::NET_CHECK);
    m_checkTool = NWCheckToolLib::getInstance();
    setCheckKey(getKeyInfoPtr()->getKey());

    //    setProjectName((tr("O/INetCheck")).toStdString());
    m_cur.m_index = static_cast<int>(CheckType::NET_CHECK);
    //    m_cur.m_projectName = QString(tr("O/INetCheck"));
    //    m_cur.m_projectDetail = QString(tr("Can use browse net?"));
}
NetCheck::~NetCheck()
{
    if (mNetCheckThread) {
        if (mThread && mThread->isRunning()) {
            mThread->quit();
            mThread->wait();
        }
        delete mNetCheckThread;
    }
}
void NetCheck::setInitialCheckType(InnerNetCheck &userSettings)
{
    qDebug() << "NetCheck::setInitialCheckType" << userSettings.isInnerCheck;
    m_settings = userSettings;
    if (m_settings.isInnerCheck) {
        setProjectName((tr("InnerNet Check")).toStdString());
        m_cur.m_projectName = QString(tr("InnerNet Check"));
        m_cur.setCurInfo(tr("Can user browse inner net?"), "");
    } else {
        setProjectName((tr("AccessNet Check")).toStdString());
        m_cur.m_projectName = QString(tr("AccessNet Check"));
        m_cur.setCurInfo(tr("Can user browse out net?"), "");
    }
}
//返回检测结果是否OK
CHECKRESULT NetCheck::getCheckResult()
{
    return CHECKRESULT::UNKNOWN;
}

//开始检测
void NetCheck::startChecking(InnerNetCheck &checkSettings)
{
    qDebug() << "NetCheck::startChecking START  isInnerCheck:" << checkSettings.isInnerCheck;
    m_cur.setStatusCheck(CheckStatus::CHECKING);
    //修复bug122201 内外网标题显示错误
    if (checkSettings.isInnerCheck) {
        m_cur.setCurInfo(tr("Can user browse inner net?"), tr("Checking"));
        setProjectName((tr("InnerNet Check")).toStdString());
        m_cur.m_projectName = QString(tr("InnerNet Check"));
    } else {
        m_cur.setCurInfo(tr("Can user browse out net?"), tr("Checking"));
        setProjectName((tr("AccessNet Check")).toStdString());
        m_cur.m_projectName = QString(tr("AccessNet Check"));
    }
    //修复bug122201 内外网标题显示错误
    Notify(m_cur);
    qDebug() << "current status:" << m_cur.m_projectRes;
    m_settings.clear();
    m_settings = checkSettings;

    qDebug() << "NetCheck::startChecking  be start check    currentThread:" << QThread::currentThreadId();
    if (!mNetCheckThread) {
        mNetCheckThread = new NetCheckThread();
        mThread = new QThread(this);
        connect(this, &NetCheck::sigCheckIsStart, mNetCheckThread, &NetCheckThread::slotStartNetCheck);
        connect(mNetCheckThread, &NetCheckThread::sigNetCheckIsOver, this, &NetCheck::slotCheckIsOver);
        mNetCheckThread->moveToThread(mThread);
        mThread->start();
    }
    emit sigCheckIsStart(m_settings);
}
void NetCheck::slotCheckIsOver(statusStruct cur, QMap<QString, QMap<QString, bool>> resMap)
{
    qDebug() << "NetCheck::slotCheckIsOver currentThread:" << QThread::currentThreadId();

    QString projectDetail = cur.m_projectDetail;
    QString projectRes = cur.m_projectRes;

    m_cur.setCurInfo(projectDetail, projectRes);
    m_cur.setStatusCheck(cur.m_curStutus);

    if (m_settings.isInnerCheck) {
        m_cur.setDetailMode(true);
        emit detailCheckRes(resMap);
    }
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [=]() {
        Notify(m_cur);
    });
    loop.exec();
}
void NetCheck::setInit()
{
    m_cur.m_curStutus = CheckStatus::INIT;
    m_cur.m_detailMode = false;
    Notify(m_cur);
}

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

#include "hwcheck.h"
#include <QtConcurrent>
#include <QThread>
#include <QEventLoop>
HWCheck::HWCheck(QObject *parent) : QObject(parent)
{
    // set plugin type
    setPluginType(CheckType::NW_HW_DEVICE_CONF);
    m_checkTool = g_nwCheckTool;
    setCheckKey(getKeyInfoPtr()->getKey());
    m_cur.m_index = static_cast<int>(CheckType::NW_HW_DEVICE_CONF);
    m_cur.m_projectName = QString(tr("HardWare"));
    m_cur.m_projectDetail = QString(tr("Are network card OK and cable connected?"));
    setProjectName((tr("HardWare")).toStdString());
}
HWCheck::~HWCheck()
{
    //    if (m_notifier)
    //    {
    //        delete m_notifier;
    //    }
}
//返回检测结果是否OK
CHECKRESULT HWCheck::getCheckResult()
{
    qDebug() << "=====================hwcheck thread:" << QThread::currentThreadId() << "=====================";
    CHECKRESULT resType;
    if (m_checkTool->hasValidNetCard()) {
        if (m_checkTool->hasValidConnection()) {
            resType = CHECKRESULT::HAS_VALID_NETCARD_CONN;
        } else {
            resType = CHECKRESULT::HAS_VALID_NETCARD_NO_CONN;
        }

    } else {
        resType = CHECKRESULT::NO_VALID_NETCARD;
    }
    return resType;
}

//开始检测
void HWCheck::startChecking(InnerNetCheck &checkSettings)
{
    m_cur.setStatusCheck(CheckStatus::CHECKING);
    m_cur.setCurInfo(tr("Checking NetWork HardWares"), tr("Checking"));
    Notify(m_cur);

    QFuture<CHECKRESULT> resType = QtConcurrent::run(this, &HWCheck::getCheckResult);
    if (resType == CHECKRESULT::HAS_VALID_NETCARD_CONN) {
        if (m_checkTool->isWiredConnection()) {
            m_cur.setCurInfo(tr("NetWork HardWares are OK,Primary Wired."), tr("OK"));
            m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
        } else {
            m_cur.setCurInfo(tr("NetWork HardWares are OK,Primary Wireless."), tr("OK"));
            m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
        }

    } else if (resType == CHECKRESULT::HAS_VALID_NETCARD_NO_CONN) {
        m_cur.setCurInfo(tr("NetWork HardWares are OK, but no connection"), tr("ERR"));
        m_cur.setStatusCheck(CheckStatus::ERROR);
    } else if (resType == CHECKRESULT::NO_VALID_NETCARD) {
        m_cur.setCurInfo(tr("No valid net card"), tr("ERR"));
        m_cur.setStatusCheck(CheckStatus::ERROR);
    }
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [=]() {
        Notify(m_cur);
    });
    loop.exec();
}

void HWCheck::setInit()
{
    m_cur.m_curStutus = CheckStatus::INIT;
    m_cur.m_projectDetail = QString(tr("Are network card OK and cable connected?"));

    Notify(m_cur);
}

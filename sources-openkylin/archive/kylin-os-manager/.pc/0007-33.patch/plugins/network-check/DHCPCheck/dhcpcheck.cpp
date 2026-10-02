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

#include "dhcpcheck.h"
#include <QtConcurrent>
#include <QThread>
#include <QEventLoop>
DHCPCheck::DHCPCheck(QObject *parent) : QObject(parent)
{
    // set plugin type
    setPluginType(CheckType::DHCP_CONF);
    m_checkTool = NWCheckToolLib::getInstance();

    setCheckKey(getKeyInfoPtr()->getKey());
    setProjectName((tr("DHCP Config")).toStdString());
    m_cur.m_index = static_cast<int>(CheckType::DHCP_CONF);
    m_cur.m_projectName = QString(tr("DHCP Config"));
    m_cur.m_projectDetail = QString(tr("Are DHCP config right?"));
}

//返回检测结果是否OK
CHECKRESULT DHCPCheck::getCheckResult()
{
    qDebug() << "=====================dhcpcheck thread:" << QThread::currentThreadId() << "=====================";
    m_isDHCP = false;
    QString dhcpConfigCheckRes = "";
    if (m_checkTool->isIPAutoConfig()) {
        // DHCP获取，进行DHCP内容检测
        m_isDHCP = true;
        return CHECKRESULT::DHCP_ON;
    } else {
        //未启用DHCP，此项略过
        return CHECKRESULT::DHCP_OFF;
    }
}
bool DHCPCheck::checkIPFormat()
{
    return m_checkTool->isDHCPOK();
}

void DHCPCheck::startChecking(InnerNetCheck &checkSettings)
{
    m_isDHCP = false;

    m_cur.setStatusCheck(CheckStatus::CHECKING);
    m_cur.setCurInfo(tr("Checking DHCP config"), tr("Checking"));
    Notify(m_cur);

    QFuture<CHECKRESULT> resType = QtConcurrent::run(this, &DHCPCheck::getCheckResult);
    if (resType == CHECKRESULT::DHCP_ON) {
        bool isIPLegal = checkIPFormat();
        if (isIPLegal) {
            m_cur.setCurInfo(tr("DHCP RUNNING RIGHT"), tr("OK"));
            m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
            //            resType = CHECKRESULT::DHCP_OK;
        } else {
            m_cur.setCurInfo(tr("DHCP DISTRIBUTED WRONG IP"), tr("ERR"));
            m_cur.setStatusCheck(CheckStatus::ERROR);
            //            resType = CHECKRESULT::DHCP_ERR;
        }
    } else if (resType == CHECKRESULT::DHCP_OFF) {
        m_cur.setCurInfo(tr("DHCP IS OFF, NO CHECK"), tr("OK"));
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    }
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [=]() {
        Notify(m_cur);
    });
    loop.exec();
}

void DHCPCheck::setInit()
{
    m_cur.m_curStutus = CheckStatus::INIT;
    m_cur.m_projectDetail = QString(tr("Are DHCP config right?"));

    Notify(m_cur);
}

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

#include "ipcheck.h"
#include <QtConcurrent>
#include <QThread>
#include <QEventLoop>

IPCheck::IPCheck(QObject *parent) : QObject(parent)
{
    // set plugin type
    setPluginType(CheckType::NW_CON_CONF);
    m_cur.m_index = static_cast<int>(CheckType::NW_CON_CONF);
    m_checkTool = NWCheckToolLib::getInstance();

    setCheckKey(getKeyInfoPtr()->getKey());
    m_cur.m_index = static_cast<int>(CheckType::NW_CON_CONF);
    m_cur.m_projectName = QString(tr("IP Config"));
    m_cur.m_projectDetail = QString(tr("Are IP config right?"));
    setProjectName((tr("IP Config")).toStdString());
}
IPCheck::~IPCheck() {}
//返回检测结果是否OK
CHECKRESULT IPCheck::getCheckResult()
{
    qDebug() << "=====================ipcheck thread:" << QThread::currentThreadId() << "=====================";
    m_isDHCP = false;
    QString ipConfigCheckRes = "";
    if (m_checkTool->isIPAutoConfig()) {
        // DHCP则直接检测下一项，提示用户为DHCP获取IP配置方式
        m_isDHCP = true;
        return CHECKRESULT::DHCP_ON;
    } else {
        //未启用DHCP，检测是否手动配置了IP，IP与网关是否在同一个网段
        if (m_checkTool->isSameVlan()) {
            return CHECKRESULT::SAME_VLAN;
        } else {
            return CHECKRESULT::DIFF_VLAN;
        }
    }
}
//开始检测
void IPCheck::startChecking(InnerNetCheck &checkSettings)
{
    m_cur.setStatusCheck(CheckStatus::CHECKING);
    m_cur.setCurInfo(tr("Checking IP config"), tr("Checking"));
    Notify(m_cur);

    QFuture<CHECKRESULT> resType = QtConcurrent::run(this, &IPCheck::getCheckResult);
    if (resType == CHECKRESULT::DHCP_ON) {
        m_cur.setCurInfo(tr("DHCP ON"), tr("OK"));
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    } else if (resType == CHECKRESULT::DIFF_VLAN) {
        m_cur.setCurInfo(tr("IP CONFIG FALSE"), tr("ERR"));
        m_cur.setStatusCheck(CheckStatus::ERROR);
    } else if (resType == CHECKRESULT::SAME_VLAN) {
        m_cur.setCurInfo(tr("IP CONFIG RIGHT"), tr("OK"));
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    }


    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [=]() {
        Notify(m_cur);
    });
    loop.exec();
}

void IPCheck::setInit()
{
    m_cur.m_curStutus = CheckStatus::INIT;
    m_cur.m_projectDetail = QString(tr("Are IP config right?"));

    Notify(m_cur);
}

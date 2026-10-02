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

#include <cmath>
#include <QByteArray>
#include "nw_check_tool_lib.h"
#include "dnscheck.h"

DNSCheck::DNSCheck(QObject *parent) : QObject(parent)
{
    setPluginType(CheckType::DNS_CONF);
    setCheckKey(getKeyInfoPtr()->getKey());
    setProjectName((tr("DNS Config")).toStdString());

    m_cur.m_index = static_cast<int>(CheckType::DNS_CONF);
    m_cur.m_projectName = QString(tr("DNS Config"));
    m_cur.m_projectDetail = QString(tr("Are DNS config right?"));
}

DNSCheck::~DNSCheck()
{
    /* nothing to do */
}

void DNSCheck::setInit()
{
    m_cur.m_curStutus = CheckStatus::INIT;
    m_cur.m_projectDetail = QString(tr("Are DNS config right?"));

    Notify(m_cur);
}

/*
 * 开始检测
 * 检测主链接的 dns
 * 主链接从 NetworkManager 中获取，如果当前没有网络，后链接的为主链接，如果当前有网络则无线为主链接
 */
void DNSCheck::startChecking(InnerNetCheck &checkSettings)
{
    m_cur.setCurInfo(tr("Checking DNS config"), tr("Checking"));
    m_cur.setStatusCheck(CheckStatus::CHECKING);
    Notify(m_cur);

    m_dnsTask = NWCheckToolLib::getInstance()->getDNS();
    if (m_dnsTask.isEmpty()) {
        m_cur.setCurInfo(tr("NO DNS"), tr("ERR"));
        m_cur.setStatusCheck(CheckStatus::ERROR);
        Notify(m_cur);
    } else {
        // 检测到 dns 服务器的网络状态
        QString dns = m_dnsTask.takeFirst();
        pingCheck(dns);
    }
}

void DNSCheck::pingCheck(const QString &uri) {
    QProcess *process = new QProcess;
    process->setProgram("ping");
    process->setArguments(QStringList() << "-c" << "5" << "-q" << uri);
    process->setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
    connect(process, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, &DNSCheck::on_checkFinish);
    process->start();
}

void DNSCheck::on_checkFinish(int exitCode, QProcess::ExitStatus exitStatus)
{
    QObject *sender = QObject::sender();
    if (sender == nullptr)
        return;
    QProcess *process = static_cast<QProcess *>(sender);

    QByteArray outPut;
    double packetLoss {0.0};
    while (!(outPut = process->readLine()).isEmpty()) {
        if (outPut.contains("packets transmitted")
            && outPut.contains("received")
            && outPut.contains("packet loss"))
        {
            QStringList items = QString(outPut.trimmed()).split(",");
            for(auto &item : items) {
                if (item.contains("packet loss")) {
                    item.remove("packet loss");
                    item.remove("%");
                    item = item.trimmed();
                    packetLoss = item.toDouble();
                }
            }
        }
    }

    QStringList arguments = process->arguments();
    if (!arguments.isEmpty()) {
        qInfo() << "DNS check finished: " << arguments.last() << ": " << packetLoss << "% packet loss";
    }

    if (std::abs(packetLoss - 100.00) < 0.01) {
        if (!m_dnsTask.isEmpty()) {
            QString dns = m_dnsTask.takeFirst();
            pingCheck(dns);
            process->deleteLater();
            return;
        }
        m_cur.setCurInfo(tr("DNS service is working abnormally"), tr("ERR"));
        m_cur.setStatusCheck(CheckStatus::ERROR);
    } else {
        m_cur.setCurInfo(tr("DNS service is working properly"), tr("OK"));
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    }
    Notify(m_cur);

    process->deleteLater();
}

CHECKRESULT DNSCheck::getCheckResult()
{
    return CHECKRESULT::NO_DNS;
}

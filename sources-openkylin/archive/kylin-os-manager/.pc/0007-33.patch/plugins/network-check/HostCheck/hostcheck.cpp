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

#include <QtConcurrent>
#include <QThread>
#include <QEventLoop>
#include <QHostInfo>
#include <QFile>
#include <QRegExp>

#include "hostcheck.h"

HostCheck::HostCheck(QObject *parent) : QObject(parent)
{
    // set plugin type
    setPluginType(CheckType::HOST_CONF);

    setCheckKey(getKeyInfoPtr()->getKey());
    setProjectName((tr("Host File")).toStdString());
    m_cur.m_index = static_cast<int>(CheckType::HOST_CONF);
    m_cur.m_projectName = QString(tr("Host File"));
    m_cur.m_projectDetail = QString(tr("Are Host File config right?"));
}

CHECKRESULT HostCheck::getCheckResult()
{
	CHECKRESULT resType;
	m_failedInfo.clear();
	m_localHostName = QHostInfo::localHostName();

	qInfo() << "local host name is: " << m_localHostName;

	QFile hostFile("/etc/hosts");
	if (!hostFile.exists()) {
		qCritical() << "hosts file is not exists !";
		return CHECKRESULT::HOST_NO_FILE;
	}

    if (!hostFile.open(QFile::ReadOnly | QFile::Text)) {
        qCritical() << "hosts file open fail !";
        m_failedInfo.append(tr("No host file!"));
        resType = CHECKRESULT::HOST_NO_FILE;
    } else {
        QTextStream out(&hostFile);
        QString line;

        while (!out.atEnd()) {
            line = out.readLine();
            qDebug() << "HostCheck::getCheckResult:" << line;
            if (line.startsWith("#")) {
                continue;
            } else if (line.isEmpty()) {
                continue;
            } else if (((line.split(" ")).size() < 2) && ((line.split("\t")).size() < 2)) {
                qDebug() << "HostCheck::getCheckResult m_hasNoBlankLine";
                m_hasNoBlankLine = true;
                if ((m_hasNoBlankLine) && (!m_failedInfo.contains(tr("Has no sperated line.")))) {
                    m_failedInfo.append(tr("Has no sperated line."));
                }
            } else if (line.contains("127.0.0.1")) {
                QRegExp ipv4Regex(IPV4_LOCALHOST_REGEX);
                m_isLocalHostConfigRight1 &= (ipv4Regex.exactMatch(line));

                qDebug() << "HostCheck::getCheckResult  m_isLocalHostConfigRight1:" << m_isLocalHostConfigRight1;
                m_hasLocalHostConfig1 = true;
                if ((!m_isLocalHostConfigRight1) && (!m_failedInfo.contains(tr("Ipv4 localhost error.")))) {
                    m_failedInfo.append(tr("Ipv4 localhost error."));
                }
            } else if (line.contains("127.0.1.1")) {
                //                QString rex = IPV4_LOCALPCHOST_REGEX;
                //                rex.append("(");
                //                rex.append(m_localHostName);
                //                rex.append(")");
                //                QRegExp ipv4Regex(rex);
                //                m_isLocalHostConfigRight2&=(ipv4Regex.exactMatch(line));

                QStringList hostNameList;
                hostNameList.clear();
                hostNameList.append(m_localHostName);
                m_isLocalHostConfigRight2 = checkHostLineFormat(line, "127.0.1.1", hostNameList);
                qDebug() << "HostCheck::getCheckResult  m_isLocalHostConfigRight2:" << m_isLocalHostConfigRight2;
                m_hasLocalHostConfig2 = true;
                if ((!m_isLocalHostConfigRight2) && (!m_failedInfo.contains(tr("Ipv4 localPChost error.")))) {
                    m_failedInfo.append(tr("Ipv4 localPChost error."));
                }
            } else if (line.contains("::1") && (!line.contains("ff02::1"))) // ipv6
            {
                QRegExp rx(IPV6_LOCALHOST_REGEX);
                m_hasIPv6LocalHostConfig &= (rx.exactMatch(line));
                qDebug() << "HostCheck::getCheckResult  m_hasIPv6LocalHostConfig:" << m_hasIPv6LocalHostConfig;
                if ((!m_hasIPv6LocalHostConfig) && (!m_failedInfo.contains(tr("Ipv6 localhost error.")))) {
                    m_failedInfo.append(tr("Ipv6 localhost error."));
                }
            } else if (line.contains("fe00::0")) // ipv6
            {
                QRegExp rx(IPV6_LOCALNET_REGEX);
                m_hasIPv6LocalNetConfig &= (rx.exactMatch(line));
                qDebug() << "HostCheck::getCheckResult  m_hasIPv6LocalNetConfig:" << m_hasIPv6LocalNetConfig;
                if ((!m_hasIPv6LocalNetConfig) && (!m_failedInfo.contains(tr("Ipv6 localnet error.")))) {
                    m_failedInfo.append(tr("Ipv6 localnet error."));
                }
            } else if (line.contains("ff00::0")) // ipv6
            {
                QRegExp rx(IPV6_LOCALCAST_REGEX);
                m_hasIPv6LocalCastConfig &= (rx.exactMatch(line));
                qDebug() << "HostCheck::getCheckResult  m_hasIPv6LocalCastConfig:" << m_hasIPv6LocalCastConfig;
                if ((!m_hasIPv6LocalCastConfig) && (!m_failedInfo.contains(tr("Ipv6 mcastsprefix error.")))) {
                    m_failedInfo.append(tr("Ipv6 mcastsprefix error."));
                }
            } else if (line.contains("ff02::1")) // ipv6
            {
                QRegExp rx(IPV6_LOCALALLNODES_REGEX);
                m_hasIPv6LocalNodesConfig &= (rx.exactMatch(line));
                qDebug() << "HostCheck::getCheckResult  m_hasIPv6LocalNodesConfig:" << m_hasIPv6LocalNodesConfig;
                if ((!m_hasIPv6LocalNodesConfig) && (!m_failedInfo.contains(tr("Ipv6 nodes error.")))) {
                    m_failedInfo.append(tr("Ipv6 nodes error."));
                }
            } else if (line.contains("ff02::2")) // ipv6
            {
                QRegExp rx(IPV6_LOCALALLROUTERS_REGEX);
                m_hasIPv6LocalRoutersConfig &= (rx.exactMatch(line));
                qDebug() << "HostCheck::getCheckResult  m_hasIPv6LocalRoutersConfig:" << m_hasIPv6LocalRoutersConfig;
                if ((!m_hasIPv6LocalRoutersConfig) && (!m_failedInfo.contains(tr("Ipv6 routers error.")))) {
                    m_failedInfo.append(tr("Ipv6 routers error."));
                }
            } else //用户自添加内容
            {
                qDebug() << "HostCheck::getCheckResult else branch:" << line;
                QRegExp ipv4Rx(IPV4_ADDRESS_REGEX);
                QRegExp ipv6Rx(IPV6_ADDRESS_REGEX);
                bool ipv4 = ipv4Rx.exactMatch(line);
                bool ipv6 = ipv6Rx.exactMatch(line);
                m_isUserConfigRight &= (ipv4Rx.exactMatch(line) || ipv6Rx.exactMatch(line));
                qDebug() << "HostCheck::getCheckResult else branch 4:" << ipv4Rx.exactMatch(line);
                qDebug() << "HostCheck::getCheckResult else branch 6:" << ipv6Rx.exactMatch(line);
                if (!m_isUserConfigRight) {
                    qDebug() << "illegal line is " << line;
                    qDebug() << "ipv4 check is " << ipv4;
                    qDebug() << "ipv6 check is " << ipv6;
                }
                if ((!m_isUserConfigRight) && (!m_failedInfo.contains(tr("User add illegal hosts.")))) {
                    m_failedInfo.append(tr("User add illegal hosts."));
                }
            }
        }
        hostFile.close();
    }

    bool noError = true;
    noError &= !m_hasNoBlankLine;
    noError &= m_hasLocalHostConfig1;
    noError &= m_hasLocalHostConfig2;
    noError &= m_hasIPv6LocalHostConfig;
    noError &= m_hasIPv6LocalNetConfig;
    noError &= m_hasIPv6LocalCastConfig;
    noError &= m_hasIPv6LocalNodesConfig;
    noError &= m_hasIPv6LocalRoutersConfig;
    noError &= m_isUserConfigRight;
    noError &= m_isLocalHostConfigRight1;
    noError &= m_isLocalHostConfigRight2;

    if (noError) {
        resType = CHECKRESULT::HOST_OK;
        qDebug() << "HostCheck::getCheckResult all is right.";
    } else {
        resType = CHECKRESULT::HOST_ERR;
        qDebug() << "HostCheck::getCheckResult has err item!";
        checkErrorItem();
    }

    return resType;
}
void HostCheck::checkErrorItem()
{
    if (!m_hasLocalHostConfig1) {
        if (!m_failedInfo.contains(tr("Ipv4 localhost error."))) {
            m_failedInfo.append(tr("Ipv4 localhost error."));
        }
    } else if (!m_hasLocalHostConfig2) {
        if (!m_failedInfo.contains(tr("Ipv4 localPChost error."))) {
            m_failedInfo.append(tr("Ipv4 localPChost error."));
        }
    } /*else if(!m_hasIPv6LocalHostConfig){
         if (!m_failedInfo.contains(tr("Ipv6 localhost error.")))
         {
             m_failedInfo.append(tr("Ipv6 localhost error."));
         }
     }else if(!m_hasIPv6LocalNetConfig){
         if (!m_failedInfo.contains(tr("Ipv6 localnet error.")))
         {
             m_failedInfo.append(tr("Ipv6 localnet error."));
         }
     }else if(!m_hasIPv6LocalCastConfig){
         if (!m_failedInfo.contains(tr("Ipv6 mcastsprefix error.")))
         {
             m_failedInfo.append(tr("Ipv6 mcastsprefix error."));
         }
     }else if(!m_hasIPv6LocalNodesConfig){
         if (!m_failedInfo.contains(tr("Ipv6 nodes error.")))
         {
             m_failedInfo.append(tr("Ipv6 nodes error."));
         }
     }else if(!m_hasIPv6LocalRoutersConfig){
         if (!m_failedInfo.contains(tr("Ipv6 routers error.")))
         {
             m_failedInfo.append(tr("Ipv6 routers error."));
         }
     }*/
    else {
        qDebug() << "HostCheck::checkErrorItem else branch";
    }
}
bool HostCheck::checkHostLineFormat(QString needCheck, QString headReg, QStringList list)
{
    if (needCheck.isEmpty()) {
        qWarning() << "HostCheck::checkHostLineFormat needCheck isEmpty";
        return false;
    }
    QString real = needCheck.trimmed().replace("\t", " ");
    qDebug() << "HostCheck::checkHostLineFormat real:" << real;
    QStringList needList = real.split(" ");
    if (needList.size() < 2) {
        qWarning() << "HostCheck::checkHostLineFormat needList.size() < 2";
        return false;
    }
    if (needList.first() == headReg) {
        if (list.isEmpty()) {
            qWarning() << "HostCheck::checkHostLineFormat list.isEmpty()";
            return false;
        }
        for (auto child : list) {
            if (!needList.contains(child)) {
                qWarning() << "HostCheck::checkHostLineFormat list.isEmpty()";
                return false;
            }
        }
        return true;
    } else {
        qWarning() << "HostCheck::checkHostLineFormat needList.first() error!";
        return false;
    }
}

void HostCheck::startChecking(InnerNetCheck &checkSettings)
{
    m_failedInfo = "";
    m_localHostName = "";
    m_hasNoBlankLine = false;
    m_hasLocalHostConfig1 = false;
    m_hasLocalHostConfig2 = false;
    m_hasIPv6LocalHostConfig = true;
    m_hasIPv6LocalNetConfig = true;
    m_hasIPv6LocalCastConfig = true;
    m_hasIPv6LocalNodesConfig = true;
    m_hasIPv6LocalRoutersConfig = true;
    m_isUserConfigRight = true;
    m_isLocalHostConfigRight1 = true;
    m_isLocalHostConfigRight2 = true;

    m_cur.setStatusCheck(CheckStatus::CHECKING);
    m_cur.setCurInfo(tr("Checking Host Files"), tr("Checking"));
    Notify(m_cur);

    QFuture<CHECKRESULT> resType = QtConcurrent::run(this, &HostCheck::getCheckResult);
    if (resType == CHECKRESULT::HOST_OK) {
        m_cur.setCurInfo(tr("Hosts Files are OK"), tr("OK"));
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    } else if (resType == CHECKRESULT::HOST_ERR) {
        m_cur.setCurInfo(m_failedInfo, tr("ERR"));
        m_cur.setStatusCheck(CheckStatus::ERROR);
    } else if (resType == CHECKRESULT::HOST_NO_FILE) {
        m_cur.setCurInfo(tr("The local hosts file is abnormal, please repait it"), tr("ERR"));
        m_cur.setStatusCheck(CheckStatus::ERROR);
    }

    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [=]() {
        Notify(m_cur);
    });
    loop.exec();
}

void HostCheck::setInit()
{
    m_cur.m_curStutus = CheckStatus::INIT;
    m_cur.m_projectDetail = QString(tr("Are Host File config right?"));

    Notify(m_cur);
}

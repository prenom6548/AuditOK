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

#include "netcheckthread.h"
#include <QDebug>
#include <QUrl>
#include <curl/curl.h>
#include <QThread>

NetCheckThread::NetCheckThread(QObject *parent) : QObject(parent)
{
    connect(this, &NetCheckThread::sigCheckIsOver, this, &NetCheckThread::slotCheckIsOver);
}

void NetCheckThread::slotStartNetCheck(InnerNetCheck &checkSettings)
{
    qDebug() << "NetCheckThread::slotStartNetCheck threadID:" << QThread::currentThreadId();
    if (m_cmd) {
        qWarning() << "NetCheckThread::getCheckResult m_cmd is not nullptr!";
        delete m_cmd;
        m_cmd = nullptr;
    }
    //有需要复位的东西
    m_isConnect = false;
    m_isPerfect = false;
    m_isInnerIPOK = false;
    m_isInnerWebOK = false;
    m_ipConnected = 0;
    m_ipDisconnected = 0;
    m_webConnected = 0;
    m_webDisconnected = 0;
    m_ipRes.clear();
    m_webRes.clear();
    m_settings.clear();
    m_settings = checkSettings;
    qDebug() << "NetCheckThread::slotStartNetCheck m_settings.webNum:" << m_settings.webNum;
    qDebug() << "NetCheckThread::slotStartNetCheck m_settings.ipNum:" << m_settings.ipNum;
    //有需要复位的东西
    if (m_settings.isInnerCheck) {
        innerWebCheck();
        if (m_settings.ipNum == 0) {
            m_isInnerIPOK = true;
        } else {
            if (m_settings.ipNum == m_ipDisconnected) {
                m_isInnerIPOK = false;
            } else {
                m_isInnerIPOK = true;
            }
        }
        if (m_settings.webNum == 0) {
            m_isInnerWebOK = true;
        } else {
            if (m_settings.webNum == m_webDisconnected) {
                m_isInnerWebOK = false;
            } else {
                m_isInnerWebOK = true;
            }
        }

        if (m_isInnerIPOK || m_isInnerWebOK) {
            emit sigCheckIsOver(CHECKRESULT::NET_IN_CHECK_SUCC);
        } else {
            emit sigCheckIsOver(CHECKRESULT::NET_IN_CHECK_ERR);
        }
    } else {
        extraNetCheck();
    }
}
bool NetCheckThread::extraNetCheck()
{
    //首先进行外网检测
    QString cmdStr = "ping ";
    cmdStr.append(m_baiduNet);
    cmdStr.append(" -c 4 -q");
    qDebug() << "NetCheckThread::extraNetCheck " << cmdStr;
    m_cmd = new QProcess();
    connect(m_cmd, &QProcess::readyReadStandardOutput, this, &NetCheckThread::readCmdBashInfo);
    connect(m_cmd, &QProcess::readyReadStandardError, this, &NetCheckThread::slotProcessOccurError);
    m_cmd->start(cmdStr);
    qDebug() << "NetCheckThread::extraNetCheck m_cmd->waitForFinished before";
    bool isFinish = m_cmd->waitForFinished();
    qDebug() << "NetCheckThread::extraNetCheck m_cmd->waitForFinished after";
    if (!isFinish) {
        qWarning() << "NetCheckThread::extraNetCheck m_cmd isFinish:" << isFinish;
        m_cmd->kill();
    }
    disconnect(m_cmd, &QProcess::readyReadStandardOutput, this, &NetCheckThread::readCmdBashInfo);
    disconnect(m_cmd, &QProcess::readyReadStandardError, this, &NetCheckThread::slotProcessOccurError);
    delete m_cmd;
    m_cmd = nullptr;
    //    m_baiduRes.clear();
    if (m_isConnect) {
        if (m_isPerfect) {
            //            m_baiduRes.insert(m_baiduNet,true);
            emit sigCheckIsOver(CHECKRESULT::NET_OUT_CHECK_PERFECT);
            return true;
        } else {
            //            m_baiduRes.insert(m_baiduNet,true);
            emit sigCheckIsOver(CHECKRESULT::NET_OUT_CHECK_SUCC);
            return true;
        }
    } else {
        //        m_baiduRes.insert(m_baiduNet,false);
        emit sigCheckIsOver(CHECKRESULT::NET_OUT_CHECK_ERR);
        return false;
    }
}

bool NetCheckThread::pingCheck(const QString &destination)
{
    QProcess process;
    process.setProgram("ping");
    process.setArguments(QStringList() << "-c" << "5" << "-q" << destination);
    process.setProcessChannelMode(QProcess::ProcessChannelMode::MergedChannels);
    process.start();
    process.waitForFinished(-1);

    QByteArray outPut;
    bool isAnalysis {false};
    double packetLoss {0.0};
    while (!(outPut = process.readLine()).isEmpty()) {
        if (outPut.contains("packets transmitted")
            && outPut.contains("received")
            && outPut.contains("packet loss"))
        {
            isAnalysis = true;
            QStringList items = QString(outPut.trimmed()).split(",");
            for(auto &item : items) {
                if (item.contains("packet loss")) {
                    item.remove("packet loss");
                    item.remove("%");
                    item = item.trimmed();
                    packetLoss = item.toDouble();
                    break;
                }
            }
        }
    }

    if (!isAnalysis || std::abs(packetLoss - 100.00) < 0.01)
        return false;

    return true;
}

void NetCheckThread::innerWebCheck()
{
    m_ipConnected = 0;
    m_ipDisconnected = 0;
    m_webConnected = 0;
    m_webDisconnected = 0;

    if (m_settings.ipNum != 0) {
        for (QString ip : m_settings.ip) {
            QString cmdStr = "ping ";
            if (!ip.isEmpty()) {
                cmdStr.append(ip);
            } else {
                continue;
            }
            cmdStr.append(" -c 4 -q");
            qDebug() << "NetCheckThread::innerWebCheck cmdStr:" << cmdStr;
            m_cmd = new QProcess();
            connect(m_cmd, &QProcess::readyReadStandardOutput, this, &NetCheckThread::readCmdBashInfo);
            connect(m_cmd, &QProcess::readyReadStandardError, this, &NetCheckThread::slotProcessOccurError);
            m_cmd->start(cmdStr);
            qDebug() << "NetCheckThread::innerWebCheck m_cmd->waitForFinished before";
            bool isFinish = m_cmd->waitForFinished();
            qDebug() << "NetCheckThread::innerWebCheck m_cmd->waitForFinished after";
            if (!isFinish) {
                qWarning() << "NetCheckThread::innerWebCheck m_cmd isFinish:" << isFinish;
                m_cmd->kill();
            }
            disconnect(m_cmd, &QProcess::readyReadStandardOutput, this, &NetCheckThread::readCmdBashInfo);
            disconnect(m_cmd, &QProcess::readyReadStandardError, this, &NetCheckThread::slotProcessOccurError);
            delete m_cmd;
            m_cmd = nullptr;
            if (!m_isConnect) {
                ++m_ipDisconnected;
                qDebug() << "NetCheckThread::innerWebCheck:" << ip << "Disconnected";
                m_ipRes[ip] = false;
            } else {
                ++m_ipConnected;
                qDebug() << "NetCheckThread::innerWebCheck:" << ip << "Connected";
                m_ipRes[ip] = true;
            }
        }
    }
    if (m_settings.webNum != 0) {
        for (QString web : m_settings.web) {
            qDebug() << "web:" << web;
            if (!web.isEmpty()) {
                QString host;
                if (web.contains("http")) {
                    QUrl url(web);
                    host = url.host();
                } else {
                    host = web;
                }

                CURL *curl = curl_easy_init();
                CURLcode res;
                if (curl) {
                    curl_easy_setopt(curl, CURLOPT_URL, web.toStdString().c_str());
                    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
                    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 1);
                    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5);
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
                    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
                    res = curl_easy_perform(curl);
                    if (res != CURLE_OK) {
                        qDebug() << curl << "web res is err！";
                        qDebug() << "res is " << res;
                        if (pingCheck(host)) {
                            ++m_webConnected;
                            m_webRes[web] = true;
                        } else {
                            ++m_webDisconnected;
                            m_webRes[web] = false;
                        }
                    } else {
                        long contype;
                        res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &contype);
                        if (res == CURLE_OK && contype == 200) {
                            qDebug() << curl << "web res is ok！";
                            ++m_webConnected;
                            m_webRes[web] = true;
                        } else {
                            qDebug() << curl << "web res is ok,but http is err！";
                            if (pingCheck(host)) {
                                ++m_webConnected;
                                m_webRes[web] = true;
                            } else {
                                ++m_webDisconnected;
                                m_webRes[web] = false;
                            }
                        }
                    }
                    curl_easy_cleanup(curl);
                } else {
                    qCritical() << "curl start fail！";
                    return;
                }
            }
        }
    }
}

void NetCheckThread::readCmdBashInfo()
{
    qDebug() << "NetCheckThread::readCmdBashInfo threadID:" << QThread::currentThreadId();
    QByteArray cmdStdOut = m_cmd->readAll();
    qDebug() << "NetCheckThread::readCmdBashInfo cmdStdOut:" << cmdStdOut;
    if (cmdStdOut.isEmpty()) {
        qWarning() << "NetCheckThread::readCmdBashInfo cmdStdOut.isEmpty!";
        m_isConnect = false;
        m_isPerfect = false;
        return;
    }
    QStringList res = (QString::fromLocal8Bit(cmdStdOut)).split("\n");
    qDebug() << "NetCheckThread::readCmdBashInfo QStringList res:" << res;
    m_isConnect = false;
    m_isPerfect = false;
    for (auto str : res) {
        if (str.contains("received")) {
            if ((str.contains("0% packet loss") && (!str.contains("100% packet loss")))
                || str.contains("25% packet loss")) {
                m_isConnect = true;
            } else if (str.contains("100% packet loss") || str.contains("0 received")) {
                m_isConnect = false;
            }
            return;
        } else {
            continue;
        }
    }
    return;
}
void NetCheckThread::slotProcessOccurError()
{
    qWarning() << "NetCheckThread::slotProcessOccurError";
    if (m_cmd) {
        int exitCode = m_cmd->exitCode();
        QByteArray cmdErr = m_cmd->readAllStandardError();
        QString errStr = QString::fromLocal8Bit(cmdErr);
        qWarning() << "NetCheckThread::slotProcessOccurError exitCode:" << exitCode;
        qWarning() << "NetCheckThread::slotProcessOccurError errStr:" << errStr;
    }
}
void NetCheckThread::slotCheckIsOver(CHECKRESULT resType)
{
    qDebug() << "NetCheckThread::slotCheckIsOver currentThread:" << QThread::currentThreadId();
    qDebug() << "NetCheckThread::slotCheckIsOver:" << m_isInnerIPOK << m_isInnerWebOK;
    if (resType == CHECKRESULT::NET_OUT_CHECK_SUCC) {
        m_cur.setCurInfo(tr("Extranet normal"), tr("OK"));
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    } else if (resType == CHECKRESULT::NET_OUT_CHECK_PERFECT) {
        m_cur.setCurInfo(tr("Extranet normal"), tr("OK"));
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    } else if (resType == CHECKRESULT::NET_OUT_CHECK_ERR) {
        m_cur.setCurInfo(tr("Extranet abnormal"), tr("ERR"));
        m_cur.setStatusCheck(CheckStatus::ERROR);
    } /*else if(resType == CHECKRESULT::NET_ONLY_OUT_ERR){
         m_cur.setCurInfo(tr("Extranet err"), tr("ERR"));
         m_cur.setStatusCheck(CheckStatus::EXTRANET_ERR);
     }*/
    else if (resType == CHECKRESULT::NET_IN_CHECK_PERFECT) {
        m_cur.setCurInfo(tr("Intranet normal"), tr("OK"));
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    } else if (resType == CHECKRESULT::NET_IN_CHECK_SUCC) {
        if (m_isInnerIPOK && m_isInnerWebOK) {
            if (m_settings.ipNum != 0 && m_settings.webNum != 0) {
                m_cur.setCurInfo(tr("Intranet normal"), tr("OK"));
                m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
            } else if (m_settings.ipNum == 0 && m_settings.webNum != 0) {
                m_cur.setCurInfo(tr("Url can be accessed"), tr("OK"));
                m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
            } else if (m_settings.ipNum != 0 && m_settings.webNum == 0) {
                m_cur.setCurInfo(tr("IP is reachable"), tr("OK"));
                m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
            } else {
                qWarning() << "NetCheck::slotCheckIsOver ip and web sum is 0!";
            }
        } else if (m_isInnerIPOK && (!m_isInnerWebOK)) {
            if (m_settings.ipNum == 0) {
                m_cur.setCurInfo(tr("Url cannot be accessed"), tr("ERR"));
                m_cur.setStatusCheck(CheckStatus::ERROR);
            } else {
                m_cur.setCurInfo(tr("IP is reachable，url cannot be accessed"), tr("OK"));
                m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
            }
        } else if ((!m_isInnerIPOK) && m_isInnerWebOK) {
            if (m_settings.webNum == 0) {
                m_cur.setCurInfo(tr("IP is unreachable"), tr("ERR"));
                m_cur.setStatusCheck(CheckStatus::ERROR);
            } else {
                m_cur.setCurInfo(tr("IP is unreachable，url can be accessed"), tr("OK"));
                m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
            }
        } else {
            qWarning() << "NetCheck::slotCheckIsOver m_isInnerIPOK and m_isInnerWebOK all is err!";
        }
    } else if (resType == CHECKRESULT::NET_IN_CHECK_ERR) {
        m_cur.setCurInfo(tr("IP is unreachable，url cannot be accessed"), tr("ERR"));
        m_cur.setStatusCheck(CheckStatus::ERROR);
    }
    QMap<QString, QMap<QString, bool>> resultMap;
    resultMap.clear();
    resultMap.insert("ip", m_ipRes);
    resultMap.insert("web", m_webRes);
    emit sigNetCheckIsOver(m_cur, resultMap);
}

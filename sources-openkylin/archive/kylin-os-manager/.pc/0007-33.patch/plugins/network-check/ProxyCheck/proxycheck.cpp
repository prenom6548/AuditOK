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

#include "proxycheck.h"
#include <QtConcurrent>
#include <QThread>
#include <QEventLoop>
#include <QGSettings>
#include <QDebug>

#define SYSTEM_PROXY_GSETTINGS_SCHEMA "org.gnome.system.proxy"
#define SYSTEM_PROXY_HTTP_GSETTINGS_SCHEMA "org.gnome.system.proxy.http"
#define SYSTEM_PROXY_HTTPS_GSETTINGS_SCHEMA "org.gnome.system.proxy.https"

ProxyCheck::ProxyCheck(QObject *parent) : QObject(parent)
{
    // set plugin type
    setPluginType(CheckType::PROXY_CONF);
    m_checkTool = NWCheckToolLib::getInstance();

    setCheckKey(getKeyInfoPtr()->getKey());
    setProjectName((tr("NetWork Proxy")).toStdString());
    m_cur.m_index = static_cast<int>(CheckType::PROXY_CONF);
    m_cur.m_projectName = QString(tr("Proxy"));
    m_cur.m_projectDetail = QString(tr("Check whether the proxy is working?"));
}

//返回检测结果是否OK
CHECKRESULT ProxyCheck::getCheckResult()
{
    qDebug() << "=====================proxycheck thread:" << QThread::currentThreadId() << "=====================";
    CHECKRESULT resType;
    // sxs TODO: proxy mode改成枚举值
    QString mode = getSystemProxyMode();
    if (mode != "none") {
        if (mode == "manual") {
            ManualConfigCheck(resType);
        } else {
            AutoConfigURLCheck(resType);
        }
    } else {
        resType = CHECKRESULT::PROXY_OFF;
    }
    return resType;
}
//开始检测
void ProxyCheck::startChecking(InnerNetCheck &checkSettings)
{
    m_cur.setStatusCheck(CheckStatus::CHECKING);
    m_cur.setCurInfo(tr("Check whether the proxy is working?"), tr("Checking"));
    Notify(m_cur);

    QFuture<CHECKRESULT> resType = QtConcurrent::run(this, &ProxyCheck::getCheckResult);
    if (resType == CHECKRESULT::PROXY_OFF) {
        m_cur.setCurInfo(tr("proxy disable"), "OK");
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    } else if (resType == CHECKRESULT::PROXY_AUTO_VALID) {
        m_cur.setCurInfo(tr("auto proxy normal"), "OK");
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    } else if (resType == CHECKRESULT::PROXY_AUTO_INVALID) {
        m_cur.setCurInfo(tr("auto proxy abnormal"), "Warning");
        m_cur.setStatusCheck(CheckStatus::WARNING);
    } else if (resType == CHECKRESULT::PROXY_MANUAL_VALID) {
        m_cur.setCurInfo(tr("manual proxy normal"), "OK");
        m_cur.setStatusCheck(CheckStatus::EVERTHING_IS_OK);
    } else if (resType == CHECKRESULT::PROXY_MANUAL_INVALID) {
        //        m_widgetUI->setCheckRes("手动代理异常", "Warning");
        m_cur.setStatusCheck(CheckStatus::WARNING);
    }
    QEventLoop loop;
    QTimer::singleShot(1000, &loop, [=]() {
        Notify(m_cur);
    });
    loop.exec();
}

QString ProxyCheck::getSystemProxyMode()
{
    QString mode = "none";
    QVariant value = getSchemaValue(SYSTEM_PROXY_GSETTINGS_SCHEMA, "mode");

    if (!value.toString().isEmpty()) {
        mode = value.toString();
    }
    return mode;
}

void ProxyCheck::AutoConfigURLCheck(CHECKRESULT &res)
{
    QString autoConfigURL = getSchemaValue(SYSTEM_PROXY_GSETTINGS_SCHEMA, "autoconfig-url").toString();
    if (autoConfigURL.isEmpty()) {
        res = CHECKRESULT::PROXY_AUTO_INVALID;
    } else {
        res = CHECKRESULT::PROXY_AUTO_VALID;
    }
}

void ProxyCheck::ManualConfigCheck(CHECKRESULT &res)
{
    bool httpHostCheck = !getSchemaValue(SYSTEM_PROXY_HTTP_GSETTINGS_SCHEMA, "host").toString().isEmpty();
    bool httpsHostCheck = !getSchemaValue(SYSTEM_PROXY_HTTPS_GSETTINGS_SCHEMA, "host").toString().isEmpty();
    bool httpPortCheck = getSchemaValue(SYSTEM_PROXY_HTTP_GSETTINGS_SCHEMA, "port").toBool();
    bool httpsPortCheck = getSchemaValue(SYSTEM_PROXY_HTTPS_GSETTINGS_SCHEMA, "port").toBool();
    if ((httpHostCheck && httpPortCheck) || (httpsHostCheck && httpsPortCheck)) {
        res = CHECKRESULT::PROXY_MANUAL_VALID;
    } else {
        res = CHECKRESULT::PROXY_MANUAL_INVALID;
    }
}

QVariant ProxyCheck::getSchemaValue(QString schema, QString key)
{
    QVariant value;
    if (QGSettings::isSchemaInstalled(schema.toLatin1())) {
        QGSettings gsettingPoint(schema.toLatin1());
        value = gsettingPoint.get(key);
    } else {
        qWarning() << schema << " is not installed!";
    }
    return value;
}

void ProxyCheck::setInit()
{
    m_cur.m_curStutus = CheckStatus::INIT;
    Notify(m_cur);
}

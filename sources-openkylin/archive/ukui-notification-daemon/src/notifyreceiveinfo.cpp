/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#include "notifyreceiveinfo.h"

#include <QDateTime>
#include <QDebug>
#include <QXmlStreamReader>
#include <QRegExp>
#include <QRegExpValidator>
NotifyReceiveInfo::NotifyReceiveInfo(
                                     const QString &appName,
                                     const QString &id,
                                     const QString &appIcon,
                                     const QString &summary,
                                     const QString &body,
                                     const QStringList &actions,
                                     const QVariantMap hints,
                                     const QString &ctime,
                                     const QString &replacesId,
                                     const QString &timeout,
                                     QObject *parent)
    : QObject(parent)
    , m_appName(appName)
    , m_id(id)
    , m_appIcon(appIcon)
    , m_summary(summary)
    , m_body(body)
    , m_actions(actions)
    , m_hints(hints)
    , m_ctime(ctime)
    , m_replacesId(replacesId)
    , m_timeout(timeout)
{
    parseNotifyBodyUrl(m_body);
    parseNotifyActions(m_actions);
    parseNotifyHints(m_hints);
}

NotifyReceiveInfo::NotifyReceiveInfo(const NotifyReceiveInfo &notify)
    : NotifyReceiveInfo(
        notify.appName(),
        notify.id(),
        notify.appIcon(),
        notify.summary(),
        notify.body(),
        notify.actions(),
        notify.hints(),
        notify.ctime(),
        notify.replacesId(),
        notify.timeout())
{
    m_bodyText = notify.bodyText();
    m_bodyUrl = notify.bodyUrl();
    m_actions = notify.actions();
    m_defaultActions = notify.defaultActions();
    m_buttonActionsHash = notify.buttonActions();
}

NotifyReceiveInfo &NotifyReceiveInfo::operator=(const NotifyReceiveInfo &notify)
{
    NotifyReceiveInfo ent(notify);
    return ent;
}

QString NotifyReceiveInfo::appName() const
{
    return m_appName;
}

QString NotifyReceiveInfo::id() const
{
    return m_id;
}

QString NotifyReceiveInfo::appIcon() const
{
    return m_appIcon;
}

QString NotifyReceiveInfo::summary() const
{
    return m_summary;
}

QString NotifyReceiveInfo::body() const
{
    return m_body;
}

QString NotifyReceiveInfo::bodyText() const
{
    return m_bodyText;
}

QStringList NotifyReceiveInfo::actions() const
{
    return m_actions;
}

QVariantMap NotifyReceiveInfo::hints() const
{
    return m_hints;
}

QString NotifyReceiveInfo::ctime() const
{
    return m_ctime;
}

QString NotifyReceiveInfo::replacesId() const
{
    return m_replacesId;
}

QString NotifyReceiveInfo::timeout() const
{
    return m_timeout;
}

QString NotifyReceiveInfo::bodyUrl() const
{
    return m_bodyUrl;
}

QString NotifyReceiveInfo::defaultActions() const
{
    return m_defaultActions;
}

QList<ButtonActionStruct> NotifyReceiveInfo::buttonActions() const
{
    return m_buttonActionsHash;
}

/* 解析通知中的 body 字段，将body文本 和 URl 分离 */
void NotifyReceiveInfo::parseNotifyBodyUrl(QString &body)
{
    //解析消息体中的数据:freedesktop 通知协议中规定，消息体正文字段可包含 XML、HTML以及一些附加标签
    if (body.contains("href")) {
        if (body.contains("wx") && body.contains("qq.com")) { //网页微信发来的消息不太符合freedesktop通知协议,暂时分离出来
            //"<a href="https://wx2.qq.com/">wx2.qq.com</a>1"
            QXmlStreamReader xmlStream(body);
            while (!xmlStream.atEnd()) {
                if (xmlStream.tokenType() == QXmlStreamReader::StartElement) {
                    QXmlStreamAttributes attributes = xmlStream.attributes();
                    if (attributes.hasAttribute("href")) {
                        m_bodyUrl = attributes.value("href").toString();
                        break;
                    }
//                    else if (xmlStream.tokenType() == QXmlStreamReader::Characters) {
//                        m_bodyText = xmlStream.text().toString();
//                    }
                    xmlStream.readNext();
                }
                //处理微信消息字段  <a href="https://wx.qq.com/">wx.qq.com</a> 111
                QStringList strList = body.split("</a>");
                m_bodyText = strList.at(1);
                m_bodyText.remove(" ");
                m_bodyText.remove("\n");
                m_bodyText.remove("\r");
            }
        } else {
            QXmlStreamReader xmlStream(body);
            while (!xmlStream.atEnd()) {
                if (xmlStream.tokenType() == QXmlStreamReader::StartElement) {
                    QXmlStreamAttributes attributes = xmlStream.attributes();
                    if (attributes.hasAttribute("href")) {
                        m_bodyUrl = attributes.value("href").toString();
                    }
                } else if (xmlStream.tokenType() == QXmlStreamReader::Characters) {
                    m_bodyText = xmlStream.text().toString();
                }
                xmlStream.readNext();
            }
        }
        if (m_appIcon.contains("kylin-screenshot") && !m_body.contains("file://")) {
            m_bodyUrl = "file://" + m_bodyUrl;
        }
    } else {
        m_bodyText = m_body;
    }
}


/* 解析通知中的 Actions 字段 */
void NotifyReceiveInfo::parseNotifyActions(QStringList &actions)
{
    //解析默认动作
    if(actions.contains("default")) {
        const int index = actions.indexOf("default"); // For the default action
        if (index >= 0) {
            if (index % 2 == 1) {
                m_defaultActions = actions[index - 1];
                actions.removeAt(index);
                actions.removeAt(index - 1);
            }
        }
    }

    //解析按钮动作: 偶数元素是按钮动作，奇数元素是按钮名
    if(!actions.isEmpty()) {
        for (int i = 0; i != actions.size(); i=i+2) {
            ButtonAction buttonAction;
            buttonAction.buttonName = actions[i];
            buttonAction.buttonAction = actions[i+1];
            m_buttonActionsHash.append(buttonAction);
        }
    }
}

/* 解析通知中的 Hints 字段 */
void NotifyReceiveInfo::parseNotifyHints(QVariantMap &hints)
{
    //Hints字段暂时没到，先不做处理
}

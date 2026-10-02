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

#ifndef NOTIFYRECEIVEINFO_H
#define NOTIFYRECEIVEINFO_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>

typedef struct ButtonAction
{
    QString buttonName;
    QString buttonAction;
} ButtonActionStruct;

class NotifyReceiveInfo : public QObject
{
    Q_OBJECT
public:
    explicit NotifyReceiveInfo(QObject *parent = nullptr);
    NotifyReceiveInfo(
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
        QObject *parent = 0);

    NotifyReceiveInfo(const NotifyReceiveInfo &notify);
    NotifyReceiveInfo &operator=(const NotifyReceiveInfo &notify);

    QString appName() const;
    QString id() const;
    QString appIcon() const;
    QString summary() const;
    QString body() const;
    QString bodyText() const;
    QStringList actions() const;
    QVariantMap hints() const;
    QString ctime() const;
    QString replacesId() const;
    QString timeout() const;
    QString bodyUrl() const; //解析出来的url超链接
    QString defaultActions() const; //默认动作
    QList<ButtonActionStruct> buttonActions() const; //按钮动作

private:
    void parseNotifyBodyUrl(QString &body); //解析 body 字段
    void parseNotifyActions(QStringList &actions); //解析 Actions 字段
    void parseNotifyHints(QVariantMap &hints);

private:
    QString m_appName;
    QString m_id;
    QString m_appIcon;
    QString m_summary;
    QString m_body;
    QStringList m_actions;
    QString m_defaultActions;
    QList<ButtonActionStruct> m_buttonActionsHash; //<按钮动作，按钮名>
    QVariantMap m_hints;
    QString m_ctime;
    QString m_replacesId;
    QString m_timeout;
    QString m_bodyText;
    QString m_bodyUrl;
    QString m_actionStr;

};

#endif // NOTIFYRECEIVEINFO_H

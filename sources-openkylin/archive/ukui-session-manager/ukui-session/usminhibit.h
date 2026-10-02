/*****************************************************************
ukuismserver - the UKUI session management server

Copyright 2000 Matthias Ettrich <ettrich@kde.org>
Copyright 2021 KylinSoft Co., Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#ifndef USMINHIBIT_H
#define USMINHIBIT_H

#include "ukuilockinfo.h"
#include <QDBusInterface>
#include <QHash>
#include <QVector>

class inhibit{
public:
    inhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags ,quint32 cookie ,QString inhibitorName);
    ~inhibit();

    QString app_id;
    quint32 toplevel_xid;
    QString reason;
    quint32 flags;
    quint32 cookie;
    QString inhibitorName;
};

class usminhibit : public QObject
{
    Q_OBJECT
public:

    enum InhibitorFlag{
        GSM_INHIBITOR_FLAG_LOGOUT      = 1 << 0,
        GSM_INHIBITOR_FLAG_SWITCH_USER = 1 << 1,
        GSM_INHIBITOR_FLAG_SUSPEND     = 1 << 2,
        GSM_INHIBITOR_FLAG_IDLE        = 1 << 3
    };
    usminhibit();
    ~usminhibit();

    QStringList getInhibitor();
    quint32 addInhibit(QString app_id, quint32 toplevel_xid, QString reason, quint32 flags);
    uint unInhibit(quint32 cookie);
    uint generateCookie();
    bool isInhibited(quint32 flags);

public:
    QHash<quint32 , inhibit> hash;
    int inhibitor_serial;
    int inhibit_logout_num;
    int inhibit_switchuser_num;
    int inhibit_suspend_num;
    int inhibit_idle_num;
private:
    //QDBusInterface dbus;
    uint get_next_inhibitor_serial();

Q_SIGNALS:
    void inhibitAdd();
    void inhibitRemove();
    void logoutInhibitAdd();
    void logoutInhibitRemove();
};

#endif // USMINHIBIT_H

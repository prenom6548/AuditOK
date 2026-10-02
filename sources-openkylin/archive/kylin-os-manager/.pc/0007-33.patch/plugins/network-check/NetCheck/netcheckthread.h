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

#ifndef NETCHECKTHREAD_H
#define NETCHECKTHREAD_H

#include <QObject>
#include <QProcess>
#include <QMap>
#include "return_code.h"
#include "libBase.h"

class NetCheckThread : public QObject
{
    Q_OBJECT
public:
    explicit NetCheckThread(QObject *parent = nullptr);

private:
    bool pingCheck(const QString &destination);

    QProcess *m_cmd = nullptr;
    InnerNetCheck m_settings;
    bool m_isConnect = false;
    bool m_isPerfect = false;
    bool m_isInnerIPOK = false;
    bool m_isInnerWebOK = false;
    int m_ipConnected = 0;
    int m_ipDisconnected = 0;
    int m_webConnected = 0;
    int m_webDisconnected = 0;
    const QString m_baiduNet = "www.baidu.com";
    statusStruct m_cur;

    QMap<QString, bool> m_ipRes;
    QMap<QString, bool> m_webRes;

    void innerWebCheck();
    bool extraNetCheck();

signals:
    void sigCheckIsOver(CHECKRESULT resType);
    void sigNetCheckIsOver(statusStruct m_cur, QMap<QString, QMap<QString, bool>>);
private slots:
    void slotProcessOccurError();
    void readCmdBashInfo();
    void slotCheckIsOver(CHECKRESULT resType);
public slots:
    void slotStartNetCheck(InnerNetCheck &checkSettings);
};

#endif // NETCHECKTHREAD_H

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

#ifndef NETCHECK_H
#define NETCHECK_H

#include <QObject>
#include <QProcess>
#include "NetCheck_global.h"
#include "libBase.h"
#include "nw_check_tool_lib.h"
#include "netcheckthread.h"

class NetCheck_EXPORT NetCheck : public QObject, public LibBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LibBaseInterfaceIID)
    Q_INTERFACES(LibBase)
public:
    NetCheck(QObject *parent = 0);
    ~NetCheck();
    //返回检测结果是否OK
    virtual CHECKRESULT getCheckResult() override;
    virtual void setInit() override;
    void setInitialCheckType(InnerNetCheck &);
public slots:
    virtual void startChecking(InnerNetCheck &checkSettings) override;

private slots:
    void slotCheckIsOver(statusStruct cur, QMap<QString, QMap<QString, bool>> resMap);
    //    void slotProcessOccurError();
    //    void readCmdBashInfo();
signals:
    void netCheckedFinished(int, CHECKRESULT);
    void processFinished();
    void showContent();
    void detailCheckRes(QMap<QString, QMap<QString, bool>>);
    //    void sigCheckIsOver(CHECKRESULT);
    void sigCheckIsStart(InnerNetCheck &checkSettings);

private:
    //    void innerWebCheck();
    //    bool extraNetCheck();
    NWCheckToolLib *m_checkTool = nullptr;
    //    QString m_failedInfo = "";
    InnerNetCheck m_settings;
    NetCheckThread *mNetCheckThread = nullptr;
    QThread *mThread = nullptr;

    //    QProcess* m_cmd = nullptr;
    //    const QString m_baiduNet = "www.baidu.com";

    //    bool m_isConnect = false;
    //    bool m_isPerfect = false;

    //    bool m_isInnerIPOK = false;
    //    bool m_isInnerWebOK = false;

    //    int m_ipConnected = 0;
    //    int m_ipDisconnected = 0;
    //    int m_webConnected = 0;
    //    int m_webDisconnected = 0;

    //    QMap<QString,bool> m_ipRes;
    //    QMap<QString,bool> m_webRes;
    //    QMap<QString,bool> m_baiduRes;
    statusStruct m_cur;
};

#endif // HOSTCHECK_H

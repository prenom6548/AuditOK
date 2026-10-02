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

#ifndef PROXYCHECK_H
#define PROXYCHECK_H

#include <QObject>
#include "ProxyCheck_global.h"
#include "libBase.h"
#include "nw_check_tool_lib.h"
class PROXYCHECK_EXPORT ProxyCheck : public QObject, public LibBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LibBaseInterfaceIID)
    Q_INTERFACES(LibBase)
public:
    ProxyCheck(QObject *parent = 0);
    //返回检测结果是否OK
    virtual CHECKRESULT getCheckResult() override;
    virtual void setInit() override;
public slots:
    virtual void startChecking(InnerNetCheck &checkSettings) override;
signals:
    void ProxyCheckFinish(int, CHECKRESULT);

private:
    NWCheckToolLib *m_checkTool = nullptr;
    QString getSystemProxyMode();
    void AutoConfigURLCheck(CHECKRESULT &res);
    void ManualConfigCheck(CHECKRESULT &res);
    QVariant getSchemaValue(QString schema, QString key);

    statusStruct m_cur;
};

#endif // PROXYCHECK_H

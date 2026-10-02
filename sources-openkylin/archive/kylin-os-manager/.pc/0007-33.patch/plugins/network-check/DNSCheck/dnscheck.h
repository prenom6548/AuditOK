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

#ifndef DNSCheck_H
#define DNSCheck_H

#include <QObject>
#include <QProcess>
#include <QStringList>
#include "libBase.h"
#include "DNSCheck_global.h"

class DNSCHECK_EXPORT DNSCheck : public QObject, public LibBase
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID LibBaseInterfaceIID)
    Q_INTERFACES(LibBase)

public:
    DNSCheck(QObject *parent = 0);
    ~DNSCheck();

    virtual CHECKRESULT getCheckResult() override;
    virtual void setInit() override;

private:
    void pingCheck(const QString &uri);
    statusStruct m_cur;
    QStringList m_dnsTask;

public slots:
    virtual void startChecking(InnerNetCheck &checkSettings) override;
    void on_checkFinish(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif

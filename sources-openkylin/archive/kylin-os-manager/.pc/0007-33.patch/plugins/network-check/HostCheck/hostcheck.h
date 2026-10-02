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

#ifndef HOSTCHECK_H
#define HOSTCHECK_H

#include <QObject>

#include "notifier.h"
#include "libBase.h"
#include "HostCheck_global.h"
#include "nw_check_tool_lib.h"

class HostCheck_EXPORT HostCheck : public QObject, public LibBase
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID LibBaseInterfaceIID)
	Q_INTERFACES(LibBase)

public:
	HostCheck(QObject *parent = nullptr);

	virtual CHECKRESULT getCheckResult() override;
	virtual void setInit() override;

private:
	bool checkHostLineFormat(QString needCheck, QString headReg, QStringList list);
	void checkErrorItem();

	NWCheckToolLib *m_checkTool = nullptr;
	statusStruct m_cur;

	QString m_failedInfo = "";
	QString m_localHostName = "";

	bool m_hasNoBlankLine = false;
	bool m_hasLocalHostConfig1 = false;
	bool m_hasLocalHostConfig2 = false;
	bool m_hasIPv6LocalHostConfig = true;
	bool m_hasIPv6LocalNetConfig = true;
	bool m_hasIPv6LocalCastConfig = true;
	bool m_hasIPv6LocalNodesConfig = true;
	bool m_hasIPv6LocalRoutersConfig = true;
	bool m_isUserConfigRight = true;
	bool m_isLocalHostConfigRight1 = true;
	bool m_isLocalHostConfigRight2 = true;

public slots:
	virtual void startChecking(InnerNetCheck &checkSettings) override;

signals:
	void hostCheckedFinished(int, CHECKRESULT);
};

#endif

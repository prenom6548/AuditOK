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

#ifndef DHCPCheck_H
#define DHCPCheck_H

#include <QObject>

#include "libBase.h"
#include "DHCPCheck_global.h"
#include "nw_check_tool_lib.h"

class DHCPCHECK_EXPORT DHCPCheck : public QObject, public LibBase
{
	Q_OBJECT
	Q_PLUGIN_METADATA(IID LibBaseInterfaceIID)
	Q_INTERFACES(LibBase)

public:
	DHCPCheck(QObject *parent = nullptr);

	virtual CHECKRESULT getCheckResult() override;
	virtual void setInit() override;

private:
	bool checkIPFormat();

	mutable bool m_isDHCP = false;
	NWCheckToolLib *m_checkTool = nullptr;
	statusStruct m_cur;

public slots:
	virtual void startChecking(InnerNetCheck &checkSettings) override;

signals:
	void dhcpCheckedFinished(int, CHECKRESULT);
};

#endif

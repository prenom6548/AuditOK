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

#ifndef CRASH_INFO_H
#define CRASH_INFO_H

#include <unistd.h>
#include <QByteArray>

namespace crash
{

class CrashInfo
{
public:
	CrashInfo() = default;
	~CrashInfo() = default;

	QByteArray getTimestamp(void);
	void setTimestamp(QByteArray timestamp);

	QByteArray getArch(void);
	void setArch(QByteArray arch);

	QByteArray getExecutable(void);
	void setExecutable(QByteArray executable);

	QByteArray getCommandLine(void);
	void setCommandLine(QByteArray commandLine);

	QByteArray getSignal(void);
	void setSignal(QByteArray signal);

	QByteArray getPackageName(void);
	void setPackageName(QByteArray packageName);

	QByteArray getPackageVersion(void);
	void setPackageVersion(QByteArray packageVersion);

	QList<QByteArray> getBacktrace(void);
	void setBacktrace(QByteArray backtrace);

	QByteArray getPid(void);
	void setPid(QByteArray pid);

	QByteArray getUid(void);
	void setUid(QByteArray uid);

	QByteArray getGid(void);
	void setGid(QByteArray gid);

	QByteArray getUserName(void);
	void setUserName(QByteArray username);

	QByteArray getHostname(void);
	void setHostname(QByteArray hostname);

	void print(void);
	QByteArray toJson(void);

private:
	QByteArray m_timestamp;
	QByteArray m_arch;
	QByteArray m_executable;
	QByteArray m_commandLine;
	QByteArray m_signal;
	QByteArray m_packageName;
	QByteArray m_packageVersion;
	QList<QByteArray> m_backtrace;

	QByteArray m_pid;
	QByteArray m_uid;
	QByteArray m_gid;
	QByteArray m_username;
	QByteArray m_hostname;
};

}

#endif

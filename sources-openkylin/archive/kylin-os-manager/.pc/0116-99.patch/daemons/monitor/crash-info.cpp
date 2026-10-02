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

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include "crash-info.h"

namespace crash
{

QByteArray CrashInfo::getTimestamp(void)
{
	return m_timestamp;
}

void CrashInfo::setTimestamp(QByteArray timestamp)
{
	m_timestamp = timestamp;
}

QByteArray CrashInfo::getArch(void)
{
	return m_arch;
}

void CrashInfo::setArch(QByteArray arch)
{
	m_arch = arch;
}

QByteArray CrashInfo::getExecutable(void)
{
	return m_executable;
}

void CrashInfo::setExecutable(QByteArray executable)
{
	m_executable = executable;
}

QByteArray CrashInfo::getCommandLine(void)
{
	return m_commandLine;
}

void CrashInfo::setCommandLine(QByteArray commandLine)
{
	m_commandLine = commandLine;
}

QByteArray CrashInfo::getSignal(void)
{
	return m_signal;
}

void CrashInfo::setSignal(QByteArray signal)
{
	m_signal = signal;
}

QByteArray CrashInfo::getPackageName(void)
{
	return m_packageName;
}

void CrashInfo::setPackageName(QByteArray packageName)
{
	m_packageName = packageName;
}

QByteArray CrashInfo::getPackageVersion(void)
{
	return m_packageVersion;
}
void CrashInfo::setPackageVersion(QByteArray packageVersion)
{
	m_packageVersion = packageVersion;
}

QList<QByteArray> CrashInfo::getBacktrace(void)
{
	return m_backtrace;
}

void CrashInfo::setBacktrace(QByteArray backtrace)
{
	m_backtrace << backtrace;
}

QByteArray CrashInfo::getPid(void)
{
	return m_pid;
}

void CrashInfo::setPid(QByteArray pid)
{
	m_pid = pid;
}

QByteArray CrashInfo::getUid(void)
{
	return m_uid;
}

void CrashInfo::setUid(QByteArray uid)
{
	m_uid = uid;
}

QByteArray CrashInfo::getGid(void)
{
	return m_gid;
}

void CrashInfo::setGid(QByteArray gid)
{
	m_gid = gid;
}

QByteArray CrashInfo::getUserName(void)
{
	return m_username;
}

void CrashInfo::setUserName(QByteArray username)
{
	m_username = username;
}

QByteArray CrashInfo::getHostname(void)
{
	return m_hostname;
}

void CrashInfo::setHostname(QByteArray hostname)
{
	m_hostname = hostname;
}

QByteArray CrashInfo::toJson(void)
{
	QJsonObject object;
	object.insert("timestamp", m_timestamp.constData());
	object.insert("arch", m_arch.constData());
	object.insert("executable", m_executable.constData());
	object.insert("commandLine", m_commandLine.constData());
	object.insert("signal", m_signal.constData());
	object.insert("packageName", m_packageName.constData());
	object.insert("packageVersion", m_packageVersion.constData());

	QJsonArray stackFrame;
	int size = m_backtrace.size();
	for (int i = 0; i < size; i++) {
		QByteArray item = m_backtrace.at(i);
		if (item.startsWith("Stack trace"))
			stackFrame << "";
		stackFrame << item.constData();
	}
	object.insert("backtrace", stackFrame);

	object.insert("pid", m_pid.constData());
	object.insert("uid", m_uid.constData());
	object.insert("gid", m_gid.constData());
	object.insert("username", m_username.constData());
	object.insert("hostname", m_hostname.constData());

	QJsonDocument doc;
	doc.setObject(object);

	return doc.toJson(QJsonDocument::Compact);
}

void CrashInfo::print(void)
{
	qDebug() << "********************";
	qDebug() << "|" << "key" << "\t\t" << "|" << "\t" << "value" << "|";
	qDebug() << "|" << "timestamp" << "\t\t" << "|" << "\t" << m_timestamp << "|";
	qDebug() << "|" << "arch" << "\t\t" << "|" << "\t" << m_arch << "|";
	qDebug() << "|" << "executable" << "\t\t" << "|" << "\t" << m_executable << "|";
	qDebug() << "|" << "command line" << "\t\t" << "|" << "\t" << m_commandLine << "|";
	qDebug() << "|" << "signal" << "\t\t" << "|" << "\t" << m_signal << "|";
	qDebug() << "|" << "package name" << "\t\t" << "|" << "\t" << m_packageName << "|";
	qDebug() << "|" << "package version" << "\t\t" << "|" << "\t" << m_packageVersion << "|";
	qDebug() << "|" << "backtrace" << "\t\t" << "|" << "\t" << m_backtrace << "|";
	qDebug() << "|" << "pid" << "\t\t" << "|" << "\t" << m_pid << "|";
	qDebug() << "|" << "uid" << "\t\t" << "|" << "\t" << m_uid << "|";
	qDebug() << "|" << "gid" << "\t\t" << "|" << "\t" << m_gid << "|";
	qDebug() << "|" << "username" << "\t\t" << "|" << "\t" << m_username << "|";
	qDebug() << "|" << "hostname" << "\t\t" << "|" << "\t" << m_hostname << "|";
	qDebug() << "********************";
}

}
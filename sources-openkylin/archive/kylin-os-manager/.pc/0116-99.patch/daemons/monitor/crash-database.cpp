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

#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QString>
#include <QVariant>
#include <QDebug>

#include "crash-database.h"

namespace crash
{

static const QString CONN_NAME = "komd-crash-database-conn";
static const QString DATABASE_PATH = "/usr/share/kylin-os-manager/komd-crash/komd-crash.db";
static const QString TABLE_NAME = "komd_crash_info";

CrashDatabase::CrashDatabase()
{
	if (QSqlDatabase::contains(CONN_NAME)) {
		m_conn = QSqlDatabase::database(CONN_NAME);
	} else {
		m_conn = QSqlDatabase::addDatabase("QSQLITE", CONN_NAME);
		m_conn.setDatabaseName(DATABASE_PATH);
	}

	if (!m_conn.open())
		qCritical() << "open database conn fail !";
}

CrashDatabase::~CrashDatabase()
{
	m_conn.close();
}

bool CrashDatabase::saveCrashInfo(CrashInfo &crashInfo)
{
	if (!m_conn.isValid())
		return false;

	if (!isTableExist(TABLE_NAME))
		if (!createTable())
			return false;

	QSqlQuery sqlQuery(m_conn);
	QString sql = QString("INSERT INTO %1 VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)").arg(TABLE_NAME);
	sqlQuery.prepare(sql);
	sqlQuery.addBindValue(crashInfo.getTimestamp());
	sqlQuery.addBindValue(crashInfo.getArch());
	sqlQuery.addBindValue(crashInfo.getExecutable());
	sqlQuery.addBindValue(crashInfo.getCommandLine());
	sqlQuery.addBindValue(crashInfo.getSignal());
	sqlQuery.addBindValue(crashInfo.getPackageName());
	sqlQuery.addBindValue(crashInfo.getPackageVersion());

	QByteArray serialize;
	QList<QByteArray> stackFrame = crashInfo.getBacktrace();
	int size = stackFrame.size();
	for (int i = 0; i < size; i++) {
		QByteArray item = stackFrame.at(i);
		if (item.startsWith("Stack trace"))
			serialize.append('\n');
		serialize.append(item).append('\n');
	}
	sqlQuery.addBindValue(serialize);

	sqlQuery.addBindValue(crashInfo.getPid());
	sqlQuery.addBindValue(crashInfo.getUid());
	sqlQuery.addBindValue(crashInfo.getGid());
	sqlQuery.addBindValue(crashInfo.getUserName());
	sqlQuery.addBindValue(crashInfo.getHostname());

	if (!sqlQuery.exec()) {
		qCritical() << "save crash info to database fail !" << sqlQuery.lastError();
		return false;
	}

	return true;
}

bool CrashDatabase::isTableExist(const QString &tableName)
{
	if (!m_conn.isValid())
		return false;

	if (m_conn.tables().contains(tableName))
		return true;

	return false;
}

bool CrashDatabase::createTable()
{
	if (!m_conn.isValid())
		return false;

	QSqlQuery sqlQuery(m_conn);

	QString sql = QString("CREATE TABLE %1 (timestamp TEXT,			\
					arch TEXT,				\
					executable TEXT,			\
					commandLine TEXT,			\
					signal TEXT,				\
					packageName TEXT,			\
					packageVersion TEXT,			\
					backtrace TEXT,				\
					pid TEXT,				\
					uid TEXT,				\
					gid TEXT,				\
					username TEXT,				\
					hostname)").arg(TABLE_NAME);
	sqlQuery.prepare(sql);

	if (!sqlQuery.exec()) {
		qCritical() << "create komd-crash database tabel fail !" << sqlQuery.lastError();
		return false;
	}

	return true;
}

}

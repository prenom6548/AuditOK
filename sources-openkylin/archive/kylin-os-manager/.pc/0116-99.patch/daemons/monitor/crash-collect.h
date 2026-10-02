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

#ifndef CRASH_COLLECT_H
#define CRASH_COLLECT_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusPendingCallWatcher>

#include "crash-monitor.h"
#include "crash-info.h"

namespace crash
{

class CrashCollect : public QObject
{
	Q_OBJECT

public:
	CrashCollect();
	~CrashCollect();

Q_SIGNALS:
	void collected(QString path);

public Q_SLOTS:
	void on_run();

private Q_SLOTS:
	void collectInfo(pid_t pid);
	void uploadDataByControlLogPending(QDBusPendingCallWatcher *self);

private:
	void collectInfoByCoredumpctl(pid_t pid, CrashInfo &info);
	QString getMachineArch(void);
	QString getPackageName(QString file);
	QString getPackageVersion(QString packageName);

	bool uploadData(QByteArray data);
	bool uploadDataByBuriedPoint(QByteArray &data);
	bool uploadDataByControlLog(QByteArray &data);
	QString removeBrackets(QString src);
	QString getCurrentTime(void);
	QString getHostName(void);
	QString getHostIp(void);

	CrashMonitor *m_monitor;
	QDBusInterface *m_interface;
};

}

#endif
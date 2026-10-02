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

#ifndef CRASH_MONITOR_H
#define CRASH_MONITOR_H

#include <unistd.h>

#include <QFileSystemWatcher>
#include <QString>
#include <QList>

namespace crash
{

class CrashMonitor : public QObject
{
	Q_OBJECT

public:
	CrashMonitor(QObject *parent = nullptr);
	~CrashMonitor();

	void setMonitor(bool state);

Q_SIGNALS:
	void crashed(pid_t pid);

private Q_SLOTS:
	void getCrashed(void);

private:
	QList<QString> getNewFile(void);
	QList<QString> listFiles(void);

	QFileSystemWatcher *m_fileSystemMonitor;
	QList<QString> m_fileList;
};

}

#endif
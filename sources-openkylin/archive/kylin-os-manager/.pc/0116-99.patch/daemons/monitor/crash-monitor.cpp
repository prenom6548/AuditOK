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

#include <QDebug>
#include <QDir>

#include "crash-monitor.h"

namespace crash
{

static QString monitorPath = "/var/lib/systemd/coredump";

CrashMonitor::CrashMonitor(QObject *parent)
	: QObject(parent)
	, m_fileSystemMonitor(new QFileSystemWatcher(this))
{
	qRegisterMetaType<pid_t>();
	m_fileSystemMonitor->addPath(monitorPath);
	setMonitor(true);
}

CrashMonitor::~CrashMonitor()
{
	/* nothing to do */
}

void CrashMonitor::setMonitor(bool state)
{
	if (state) {
		m_fileList = listFiles();
		connect(m_fileSystemMonitor, &QFileSystemWatcher::directoryChanged, this, &CrashMonitor::getCrashed);
	} else {
		m_fileList.clear();
		disconnect(m_fileSystemMonitor, &QFileSystemWatcher::directoryChanged, this, &CrashMonitor::getCrashed);
	}
}

QList<QString> CrashMonitor::listFiles(void)
{
	QDir dir(monitorPath);
	return dir.entryList();
}

QList<QString> CrashMonitor::getNewFile(void)
{
	QDir dir(monitorPath);
	QList<QString> currFileList = dir.entryList();

	for (int i = 0; i < m_fileList.size(); i++) {
		if (currFileList.contains(m_fileList.at(i))) {
			currFileList.removeAll(m_fileList.at(i));
		}
	}

	return currFileList;
}

void CrashMonitor::getCrashed(void)
{
	/*
	* ToDo : Qt 监控到目录下文件变化 , 但此时 coredump 并没有将 core 信息记录到日志中 ,
	* 导致执行脚本时查询不到对应的崩溃信息
	*/
	sleep(3);

	QList<QString> newFileList = getNewFile();

	/* 刷新文件列表 */
	m_fileList = listFiles();

	for (int i = 0; i < newFileList.size(); i++) {
		QList<QString> itemList = newFileList.at(i).split('.');
		if (itemList.size() == 7) {
			pid_t pid = itemList.at(4).toUInt();
			if (pid != 0) {
				qInfo() << "crash detected, pid is: " << pid;
				Q_EMIT crashed(pid);
			}
		}
	}
}

}
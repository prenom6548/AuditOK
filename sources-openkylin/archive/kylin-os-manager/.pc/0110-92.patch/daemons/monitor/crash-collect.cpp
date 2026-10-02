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

#include <time.h>

#include <QSysInfo>
#include <QProcess>
#include <QStringList>
#include <QDebug>
#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QGSettings>
#include <QNetworkInterface>
#include <QHostInfo>

#include <libkydiagnostics.h>

#include "crash-collect.h"
#include "crash-database.h"

namespace crash
{

CrashCollect::CrashCollect()
	: m_monitor(new CrashMonitor(this))
	, m_interface(nullptr)
{
	// nothing to do
}

CrashCollect::~CrashCollect()
{
	// nothing to do
}

void CrashCollect::on_run()
{
	connect(m_monitor, &CrashMonitor::crashed, this, &CrashCollect::collectInfo);
}

void CrashCollect::collectInfo(pid_t pid)
{
	qInfo() << "start collecting crash information, pid is: " << pid;

	CrashInfo crashInfo;
	CrashDatabase crashDatabase;

	collectInfoByCoredumpctl(pid, crashInfo);

	crashInfo.setArch(getMachineArch().toLocal8Bit());

	QString packageName = getPackageName(crashInfo.getExecutable());
	crashInfo.setPackageName(packageName.toLocal8Bit());
	crashInfo.setPackageVersion(getPackageVersion(packageName).toLocal8Bit());

	if (!crashDatabase.saveCrashInfo(crashInfo))
		qCritical() << "save crash info fail !";

	if (!uploadData(crashInfo.toJson()))
		qCritical() << "upload crash fail !";
}

void CrashCollect::collectInfoByCoredumpctl(pid_t pid, CrashInfo &info)
{
	const QString command = QString("coredumpctl info %1").arg(pid);

	QProcess process;
	process.start(command);
	process.waitForFinished(-1);

	QByteArray data = process.readAllStandardOutput().trimmed();
	QList<QByteArray> lines = data.split('\n');
	for (int i = 0; i < lines.size(); i++) {
		QString line = lines.at(i).trimmed();

		if (line.contains("Timestamp:")) {
			QString timestamp = removeBrackets(line.remove("Timestamp:").trimmed()).trimmed();
			info.setTimestamp(timestamp.toLocal8Bit());
			continue;
		}
		if (line.contains("Executable:")) {
			QString executable = line.remove("Executable:").trimmed();
			info.setExecutable(executable.toLocal8Bit());
			continue;
		}
		if (line.contains("Command Line:")) {
			QString commandLine = line.remove("Command Line:").trimmed();
			info.setCommandLine(commandLine.toLocal8Bit());
			continue;
		}
		if (line.contains("Signal:")) {
			QString signal = removeBrackets(line.remove("Signal:").trimmed()).trimmed();
			info.setSignal(signal.toLocal8Bit());
			continue;
		}
		if (line.contains("PID:")) {
			QString pid = removeBrackets(line.remove("PID:").trimmed()).trimmed();
			info.setPid(pid.toLocal8Bit());
			continue;
		}
		if (line.contains("UID:") && !line.contains("Owner UID:")) {
			QString uid = line.remove("UID:").trimmed();
			info.setUid(uid.toLocal8Bit());
			continue;
		}
		if (line.contains("GID:")) {
			QString gid = line.remove("GID:").trimmed();
			info.setGid(gid.toLocal8Bit());
			continue;
		}
		if (line.contains("Hostname:")) {
			QString hostname = line.remove("Hostname:").trimmed();
			info.setHostname(hostname.toLocal8Bit());
			continue;
		}
		if (line.contains("Message:")) {
			QString message = line.remove("Message:").trimmed();
			info.setBacktrace(message.toLocal8Bit());
			continue;
		}
		if (line.startsWith("Stack trace")) {
			QString stack = line.trimmed();
			info.setBacktrace(stack.toLocal8Bit());
			continue;
		}
		if (line.startsWith("#")) {
			QString frame = line.trimmed();
			info.setBacktrace(frame.toLocal8Bit());
			continue;
		}
	}
}

QString CrashCollect::getMachineArch(void)
{
	return QSysInfo::currentCpuArchitecture();
}

QString CrashCollect::getPackageName(QString file)
{
	// 因为程序的二进制名是不可控的，为防止注入漏洞，此处过滤 ' 和 " 符号
	if (file.contains('\'') || file.contains('"')) {
		return "";
	}

	QString command = QString("dpkg -S \"%1\"").arg(file);

	QProcess process;
	process.start(command);
	process.waitForFinished(-1);

	QByteArray data = process.readAllStandardOutput().trimmed();
	if (data.isEmpty())
		return "";

	return data.split(':').at(0);
}

QString CrashCollect::getPackageVersion(QString packageName)
{
	QString command = QString("dpkg -l %1").arg(packageName);

	QProcess process;
	process.start(command);
	process.waitForFinished(-1);

	QByteArray data = process.readAllStandardOutput().trimmed();
	if (data.isEmpty())
		return "";

	QList<QByteArray> lines = data.split('\n');
	QByteArray line = lines.last();
	QList<QByteArray> items = line.split(' ');
	items.removeAll(QByteArray());
	if (items.size() < 3)
		return "";

	return items.at(2);
}

bool CrashCollect::uploadData(QByteArray data)
{
	if (!QGSettings::isSchemaInstalled("komd.crash.upload")) {
		qCritical() << "upload gsettings don't have install !";
		return false;
	}

	bool isBuriedPoint = false;
	bool isControlLog = false;
	QGSettings upload("komd.crash.upload");
	if (upload.keys().contains("buried-point") || upload.keys().contains("buriedPoint"))
		isBuriedPoint = upload.get("buried-point").toBool();

	if (upload.keys().contains("control-log") || upload.keys().contains("controlLog"))
		isControlLog = upload.get("control-log").toBool();

	if (isBuriedPoint)
		if (!uploadDataByBuriedPoint(data))
			return false;

	if (isControlLog)
		if (!uploadDataByControlLog(data))
			return false;

	return true;
}

bool CrashCollect::uploadDataByBuriedPoint(QByteArray &data)
{
	static char appName[] = "komd-crash";
	static char messageType[] = "CrashType";

	KBuriedPoint pt[1];
	pt[0].key = "CrashInfo";
	pt[0].value = data.toBase64().constData();

	if (kdk_buried_point(appName, messageType, pt, 1)) {
		qCritical() << "kdk buried point fail !";
		return false;
	} else {
		qInfo() << "upload data by buried point success !";
	}

	return true;
}

bool CrashCollect::uploadDataByControlLog(QByteArray &data)
{
	if (m_interface == nullptr) {
		m_interface = new QDBusInterface("org.log.sys_transmit",
						"/org/log/sys_transmit",
						"org.log.transmit",
						QDBusConnection::systemBus());
		if (m_interface == nullptr || !m_interface->isValid()) {
			m_interface = nullptr;
			qCritical() << "create dbus interface error !";
			return false;
		}
	}

	QJsonObject object;
	object.insert("time", getCurrentTime());
	object.insert("hostname", getHostName());
	object.insert("ip", getHostIp());
	object.insert("name", "komd-crash");
	object.insert("lv", "debug");
	object.insert("message", data.toBase64().constData());

	QJsonDocument doc;
	doc.setObject(object);
	QByteArray log = doc.toJson(QJsonDocument::Compact);

	QDBusPendingCall pendCall = m_interface->asyncCall("log_transmit", QString::fromUtf8(log));
	QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(pendCall, this);
	connect(watcher, &QDBusPendingCallWatcher::finished, this, &CrashCollect::uploadDataByControlLogPending);

	return true;
}

void CrashCollect::uploadDataByControlLogPending(QDBusPendingCallWatcher *self)
{
	QDBusPendingReply<QString> reply = *self;
	if (reply.isError()) {
		qCritical() << "upload data to control log error !";
	} else {
		QString message = reply.value();
		if (message != "log sent")
			qCritical() << "upload data by control log error: " << message;
		else
			qInfo() << "upload data by control log success !";
	}

	self->deleteLater();
}

QString CrashCollect::removeBrackets(QString src)
{
	qsizetype leftBrackets = src.indexOf('(');
	if (leftBrackets == -1)
		return src;

	qsizetype rightBrackets = src.indexOf(')');
	if (rightBrackets != -1)
		return src.remove(leftBrackets, rightBrackets - leftBrackets + 1);
	else
		return src.remove(leftBrackets, src.size() - leftBrackets);
}

QString CrashCollect::getCurrentTime(void)
{
	time_t time = ::time(NULL);
	struct tm buf;
	if (::localtime_r(&time, &buf) == NULL)
		qCritical() << "get current time error !";

	char str[20];
	::strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", &buf);

	return QString(str);
}

QString CrashCollect::getHostName(void)
{
	QString hostName = QHostInfo::localHostName();
	return hostName.isEmpty() ? "root" : hostName;
}

QString CrashCollect::getHostIp(void)
{
	QString currentAddress;
	QList<QHostAddress> addrList = QNetworkInterface::allAddresses();
	for (int i = 0; i < addrList.size(); i++) {
		QHostAddress item = addrList.at(i);
		if ((item != QHostAddress::LocalHost) && (item.toIPv4Address())) {
			currentAddress = item.toString();
			break;
		}
	}

	return currentAddress.isEmpty() ? QHostAddress(QHostAddress::LocalHost).toString() : currentAddress;
}

}

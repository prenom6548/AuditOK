#include <QFile>
#include <QSettings>
#include <QDebug>
#include <QTextCodec>
#include <QDir>

#include "kom-global.h"
#include "kom-configure.h"

namespace kom
{

#define CONFIG_PLUGIN_SYSTEM_PATH "/etc/kylin-os-manager/kylin-os-manager-plugin.ini"
#define CONFIG_PLUGIN_USER_PATH ".kylin-os-manager/kylin-os-manager-plugin.ini"

class Configure::Impl
{
public:
	QString getUserConfigFile(const QString &name)
	{
		return QString("%1/%2").arg(getenv("HOME")).arg(name);
	}

	QVariant value(const QString &group, const QString &key, const QVariant &defaultValue = QVariant())
	{
		QVariant value;

		QString path = getUserConfigFile(CONFIG_PLUGIN_USER_PATH);
		QFile userConfigFile(path);
		if (userConfigFile.exists()) {
			QSettings settings(path, QSettings::IniFormat);
			settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
			settings.beginGroup(group);
			value = settings.value(key);
			settings.endGroup();
		}

		if (!value.isNull())
			return value;

		QFile systemConfigFile(CONFIG_PLUGIN_SYSTEM_PATH);
		if (systemConfigFile.exists()) {
			QSettings settings(CONFIG_PLUGIN_SYSTEM_PATH, QSettings::IniFormat);
			settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
			settings.beginGroup(group);
			value = settings.value(key);
			settings.endGroup();
		}

		if (!value.isNull())
			return value;
		else
			return defaultValue;
	}

	void setValue(const QString &group, const QString &key, const QVariant &value)
	{
		QString path = getUserConfigFile(CONFIG_PLUGIN_USER_PATH);
		QDir().mkpath(QFileInfo(path).absolutePath());
		QFile userConfigFile(path);
		if (!userConfigFile.exists()) {
			if (!userConfigFile.open(QIODevice::ReadWrite)) {
				qCritical() << KOM_ERR << "create user config file fail !";
				return;
			} else {
				userConfigFile.close();
			}
		}

		QSettings settings(path, QSettings::IniFormat);
		settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
		settings.beginGroup(group);
		settings.setValue(key, value);
		settings.endGroup();
	}
};

Configure::Configure()
{
	m_impl = std::make_unique<Impl>();
}

Configure::~Configure()
{
	/* nothing to do */
}

QVariant Configure::value(const QString &group, const QString &key, const QVariant &defaultValue)
{
	return m_impl->value(group, key, defaultValue);
}

void Configure::setValue(const QString &group, const QString &key, const QVariant &value)
{
	m_impl->setValue(group, key, value);
}

}

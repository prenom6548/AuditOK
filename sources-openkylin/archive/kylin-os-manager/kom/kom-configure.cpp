#include <QFile>
#include <QSettings>
#include <QDebug>
#include <QTextCodec>
#include <QDir>

#include "kom-global.h"
#include "kom-configure.h"

#define CONFIG_PLUGIN_SYSTEM_PATH "/etc/kylin-os-manager/kylin-os-manager-plugin.ini"
#define CONFIG_PLUGIN_ROCK_PATH "/opt/system/conf/kylin/kylin-os-manager/kylin-os-manager-plugin.ini"
#define CONFIG_PLUGIN_USER_PATH ".kylin-os-manager/kylin-os-manager-plugin.ini"

namespace kom {

class Configure::Impl {
public:
	QString getUserConfigFile(const QString &name) {
		return QString("%1/%2").arg(getenv("HOME")).arg(name);
	}

    QVariant getValueFromIni(const QString &path, const QString &group, const QString &key) {
        QVariant value;
        if (!QFileInfo::exists(path)) {
            return value;
        }

        QSettings s(path, QSettings::IniFormat);
        s.setIniCodec(QTextCodec::codecForName("UTF-8"));
        s.beginGroup(group);
        value = s.value(key);
        s.endGroup();

        return value;
    }

	QVariant value(const QString &group, const QString &key, const QVariant &defaultValue = QVariant()) {
		QVariant value;

        // 获取用户级别的配置
		QString homeConfigFile = getUserConfigFile(CONFIG_PLUGIN_USER_PATH);
        value = getValueFromIni(homeConfigFile, group, key);
        if (!value.isNull()) {
            return value;
        }

        // 获取磐石系统路径下的配置
        value = getValueFromIni(CONFIG_PLUGIN_ROCK_PATH, group, key);
        if (!value.isNull()) {
            return value;
        }

        // 获取系统级别的配置
        value = getValueFromIni(CONFIG_PLUGIN_SYSTEM_PATH, group, key);
        if (!value.isNull()) {
            return value;
        }

        return defaultValue;
	}

	void setValue(const QString &group, const QString &key, const QVariant &value) {
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

Configure::Configure() {
	m_impl = std::make_unique<Impl>();
}

Configure::~Configure() {}

QVariant Configure::value(const QString &group, const QString &key, const QVariant &defaultValue) {
	return m_impl->value(group, key, defaultValue);
}

void Configure::setValue(const QString &group, const QString &key, const QVariant &value) {
	m_impl->setValue(group, key, value);
}

}

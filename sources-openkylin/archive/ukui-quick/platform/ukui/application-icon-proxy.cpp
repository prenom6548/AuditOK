/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
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
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#include "application-icon-proxy.h"

#include <mutex>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

namespace UkuiQuick {

class ApplicationIconConfig : public QObject
{
    Q_OBJECT
public:
    static ApplicationIconConfig* instance();
    QString getIcon(const QString &appId) const;

Q_SIGNALS:
    void changed();

private:
    explicit ApplicationIconConfig(QObject *parent = nullptr);
    QJsonObject m_map;
    QString m_prefix;
};

static ApplicationIconConfig *g_instance = nullptr;
static std::once_flag onceFlag;

ApplicationIconConfig* ApplicationIconConfig::instance()
{
    std::call_once(onceFlag, [ & ] {
        g_instance = new ApplicationIconConfig();
    });
    return g_instance;
}

QString ApplicationIconConfig::getIcon(const QString &appId) const
{
    if (appId.isEmpty()) {
        return {};
    }
    if (m_map.contains(appId)) {
        return QDir::cleanPath(m_prefix + "/" + m_map.value(appId).toString());
    }
    return {};
}

ApplicationIconConfig::ApplicationIconConfig(QObject* parent) : QObject(parent)
{
    QString path = QStringLiteral("/usr/share/config/themeconfig/iconsoverride/iconsoverride.json");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "ApplicationIconConfig: Failed to open config file:" << path;
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError{};
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (doc.isNull()) {
        qWarning() << "ApplicationIconConfig: Failed to parse JSON：" << parseError.errorString() << path;
    }

    if (!doc.isObject()) {
        qWarning() << "ApplicationIconConfig: Config file format error." << path;
        return;
    }

    QJsonObject obj = doc.object();
    m_map = obj.value("rules").toObject();
    m_prefix = obj.value("prefix").toString();
    //TODO 监听配置文件变化
}

class ApplicationIconProxyPrivate
{
public:
    QString appId;
    QString icon;
};
ApplicationIconProxy::ApplicationIconProxy(QObject* parent) : QObject(parent), d(new ApplicationIconProxyPrivate)
{
}

ApplicationIconProxy::ApplicationIconProxy(const QString &appId, QObject* parent) : QObject(parent), d(new ApplicationIconProxyPrivate)
{
    d->appId = appId;
    d->icon = ApplicationIconConfig::instance()->getIcon(appId);
}

ApplicationIconProxy::~ApplicationIconProxy()
{
    if (d) {
        delete d;
        d = nullptr;
    }
}

QString ApplicationIconProxy::appId() const
{
    return d->appId;
}

void ApplicationIconProxy::setAppId(const QString &appId)
{
    if (d->appId != appId) {
        d->appId = appId;
        Q_EMIT appIdChanged();
        d->icon = ApplicationIconConfig::instance()->getIcon(appId);
        Q_EMIT iconChanged();
    }
}

QString ApplicationIconProxy::icon() const
{
    return d->icon;
}
}
#include "application-icon-proxy.moc"
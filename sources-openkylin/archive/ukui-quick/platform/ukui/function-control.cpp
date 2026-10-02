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
#include "function-control.h"

#include <QHash>
#include <QDBusInterface>
#include <QDBusReply>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

#include "dbus-connector.h"

static UkuiQuick::FunctionControl *g_instance = nullptr;

namespace UkuiQuick {
class FunctionControlPrivate : public QObject
{
    Q_OBJECT
public:
    FunctionControlPrivate(FunctionControl *q, QObject *parent = nullptr);
    void loadConfig(const QString &path, const QString &name);
    void refreshConfig(const QString &name);

public Q_SLOTS:
    void onConfigChanged(const QString &userName, const QStringList &appConfigList);
    void onSecurityExit();

public:
    QHash<QString, uint> m_configRefs;
    QHash<QString, QJsonObject> m_configs;
    QDBusInterface *m_interface = nullptr;
    FunctionControl *q = nullptr;
};

FunctionControlPrivate::FunctionControlPrivate(FunctionControl *q, QObject* parent) : QObject(parent), q(q)
{
}

void FunctionControlPrivate::onConfigChanged(const QString &userName, const QStringList &appConfigList)
{
    qDebug() << "FunctionControlPrivate::onConfigChanged" << userName << appConfigList << m_configRefs;
    if (QString(qgetenv("USER")) != userName && !userName.isEmpty()) {
        return;
    }
    for (auto &config : appConfigList) {
        if (m_configRefs.contains(config)) {
            refreshConfig(config);
            Q_EMIT g_instance->functionControlChanged(config);
        }
    }
}

void FunctionControlPrivate::onSecurityExit()
{
    m_configs.clear();
    Q_EMIT q->functionControlExit();
}

void FunctionControlPrivate::loadConfig(const QString &path, const QString &name)
{
    if (!QFile::exists(path)) {
        qWarning() << "Function control config file does not exist:" << path << ":" << name;
        m_configs.remove(name);
        return;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open function control config file:" << path << ":" << name;
        return;
    }
    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError{};
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (doc.isNull()) {
        qWarning() << "FunctionControl: Failed to parse JSON：" << parseError.errorString() << path;
    }

    if (!doc.isObject()) {
        qWarning() << "FunctionControl: Configuration file format error." << path;
        return;
    }

    QJsonObject obj = doc.object();
    m_configs.insert(name, obj);
}

void FunctionControlPrivate::refreshConfig(const QString &name)
{
    if (m_interface) {
         QDBusReply<QString> reply = m_interface->call("GetConfigPath", QString(qgetenv("USER")), name);
        if (!reply.isValid()) {
            qWarning() << "FunctionControl: Call GetConfigPath error" << reply.error();
        }
        else {
            loadConfig(reply.value(), name);
            Q_EMIT q->functionControlChanged(name);
        }
    }
}


FunctionControl* FunctionControl::instance()
{
    if (!g_instance) {
        g_instance = new FunctionControl();
    }
    return g_instance;
}

FunctionControl::~FunctionControl()
{
    g_instance = nullptr;
    disconnect(this);
    if (d) {
        d->disconnect(d);
        delete d;
        d = nullptr;
    }
}

void FunctionControl::addWatch(const QString &name)
{
    if (d->m_configRefs.contains(name)) {
        d->m_configRefs.insert(name, d->m_configRefs.value(name) + 1);
    } else {
        d->refreshConfig(name);
        d->m_configRefs.insert(name, 1);
    }
}

void FunctionControl::removeWatch(const QString &name)
{
    if (d->m_configRefs.contains(name)) {
        int value = d->m_configRefs.value(name) - 1;
        if (value < 1) {
            d->m_configRefs.remove(name);
            d->m_configs.remove(name);
        } else {
            d->m_configRefs.insert(name, value);
        }
    }
}

void FunctionControl::removeAllWatches(const QString &name)
{
    d->m_configRefs.remove(name);
    d->m_configs.remove(name);
}

QJsonObject FunctionControl::functionControlObject(const QString &name)
{
    return d->m_configs.value(name);
}

FunctionControl::FunctionControl(QObject* parent): QObject(parent), d(new FunctionControlPrivate(this))
{
    d->m_interface = new QDBusInterface(QStringLiteral("com.kylin.ukui.SettingsDaemon"),
                                        QStringLiteral("/securityConfig"),
                                        QStringLiteral("com.kylin.ukui.SettingsDaemon.interface"),
                                        QDBusConnection::systemBus(), d);
    if (!d->m_interface->isValid()) {
        qWarning() << "FunctionControl::Failed to connect to com.kylin.ukui.SettingsDaemon";
        return;
    }
    connect(d->m_interface, SIGNAL(ConfigChanged(QString, QStringList)), d,
            SLOT(onConfigChanged(QString, QStringList)));
    connect(d->m_interface, SIGNAL(SecurityExit()), d, SLOT(onSecurityExit()));
    for (auto config: d->m_configRefs.keys()) {
        d->refreshConfig(config);
    }
}
}

#include "function-control.moc"

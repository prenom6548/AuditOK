/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "upowerlinepowerdbus.h"
#include <QFile>
#include <QTextStream>

UpowerLinePowerDBus::UpowerLinePowerDBus(int linePowerId, const QString &linePowerDBusPath, QObject *parent)
    : QObject(parent), m_linePowerId(linePowerId)
{
    qDebug() << "linePowerDBusPath :" << linePowerDBusPath;
    m_DBusInterface = new QDBusInterface("org.freedesktop.UPower",
                                         linePowerDBusPath,
                                         "org.freedesktop.DBus.Properties",
                                         QDBusConnection::systemBus(),
                                         this);

    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                         linePowerDBusPath,
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this,
                                         SLOT(dealLinePowerPropertiesMessage(QDBusMessage)));

    QDBusReply<QVariant> reply = m_DBusInterface->call("Get", "org.freedesktop.UPower.Device", "Online");
    if (reply.isValid()) {
        m_acOnlineState = reply.value().toBool();
    } else {
        m_acOnlineState = false;
    }

    QDBusReply<QVariant> pathReply = m_DBusInterface->call("Get", "org.freedesktop.UPower.Device", "NativePath");
    if (pathReply.isValid()) {
        m_nativePath = pathReply.value().toString();
        qDebug() << "NativePath = " << m_nativePath;
    } else {
        qWarning() << "get native path error！";
    }
}

UpowerLinePowerDBus::~UpowerLinePowerDBus() {}

void UpowerLinePowerDBus::dealLinePowerPropertiesMessage(QDBusMessage msg)
{
    const QDBusArgument &dbusArgs = msg.arguments().at(1).value<QDBusArgument>();
    QMap<QString, QVariant> map;
    dbusArgs >> map;

    qDebug() << "line power id:" << m_linePowerId;
    if (map.contains("Online")) {
        bool acOnlineState = map.value(QString("Online")).toBool();
        if (m_acOnlineState != acOnlineState) {
            m_acOnlineState = acOnlineState;
            Q_EMIT acOnlineStateChanged(m_linePowerId, m_acOnlineState);
        }
    }
}

int UpowerLinePowerDBus::getAcOnlineState()
{
    return m_acOnlineState;
}

bool UpowerLinePowerDBus::getOnLineStateFromSysFile()
{
    QFile file("/sys/class/power_supply/" + m_nativePath + "/online");

    // 尝试以只读和文本模式打开文件
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开文件：" << file.errorString();
        return false;
    }

    QTextStream in(&file);
    QString content = in.readAll().trimmed(); // 读取全部内容并去除首尾空白字符
    file.close();

    qDebug() << QString ("/sys/class/power_supply/" + m_nativePath + "/online is ") << content;
    if (content == "1") {
        return true;
    }

    return false;
}

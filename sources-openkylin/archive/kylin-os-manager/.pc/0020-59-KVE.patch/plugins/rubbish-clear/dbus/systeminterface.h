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

#ifndef SYSTEMINTERFACE_H
#define SYSTEMINTERFACE_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

// NOTICE:add by manual
#include "../dbus/customdata.h"
#include "../dbus/customdatalist.h"
namespace KylinRubbishClear
{

/*
 * Proxy class for interface com.kylin.assistant.qsystemdbus
 */
class SystemInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    static inline const char *staticInterfaceName()
    {
        return "com.kylin.assistant.qsystemdbus";
    }

public:
    SystemInterface(const QString &service, const QString &path, const QDBusConnection &connection,
                    QObject *parent = 0);

    ~SystemInterface();

    Q_PROPERTY(CustomDataList Customs READ customs)
    inline CustomDataList customs() const
    {
        return qvariant_cast<CustomDataList>(property("Customs"));
    }

public Q_SLOTS: // METHODS
    inline QDBusPendingReply<QString> demoInfo()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("demoInfo"), argumentList);
    }

    inline QDBusPendingReply<> exitService()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("exitService"), argumentList);
    }

    inline QDBusPendingReply<CustomData> getCustomData()
    {
        QList<QVariant> argumentList;
        return asyncCallWithArgumentList(QStringLiteral("getCustomData"), argumentList);
    }

    inline QDBusPendingReply<> sendCustomData(CustomData message)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(message);
        return asyncCallWithArgumentList(QStringLiteral("sendCustomData"), argumentList);
    }

    inline QDBusPendingReply<bool> userIsActive(const QString &user, bool active)
    {
        QList<QVariant> argumentList;
        argumentList << QVariant::fromValue(user) << QVariant::fromValue(active);
        return asyncCallWithArgumentList(QStringLiteral("userIsActive"), argumentList);
    }

Q_SIGNALS: // SIGNALS
    void alertCustomData(CustomData message);
    void reportAlert(int ret, const QString &description);
};

namespace com
{
namespace kylin
{
namespace assistant
{
typedef ::SystemInterface qsystemdbus;
}
} // namespace kylin
} // namespace com
} // namespace KylinRubbishClear

#endif

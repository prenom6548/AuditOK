/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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
 */

#ifndef AUTOTEST_UTILS_FAKE_DBUS_FAKES_H
#define AUTOTEST_UTILS_FAKE_DBUS_FAKES_H

#include <QObject>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QMap>
#include <QMetaType>
#include <QPoint>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <type_traits>

namespace TestDbusFake {

struct ReplyData
{
    QVariant value;
    bool valid {false};
};

inline QString makeKey(const QString &service, const QString &path, const QString &iface, const QString &name)
{
    return service + QLatin1Char('\n') + path + QLatin1Char('\n') + iface + QLatin1Char('\n') + name;
}

inline bool serviceRegistered {false};
inline bool registerObjectResult {true};
inline int registerServiceReply {0};
inline bool registerServiceReplyValid {true};
inline QMap<QString, uint> servicePidByName;
inline QMap<QString, bool> interfaceValidity;
inline QMap<QString, ReplyData> callReplies;
inline QMap<QString, ReplyData> propertyReplies;
inline QString lastConnectedService;
inline QString lastDisconnectedService;
inline QString lastSentSignalPath;
inline QString lastSentSignalName;
inline QString lastAsyncCallMethod;
inline QString lastAsyncCallArgument;
inline QString lastErrorMessage {QStringLiteral("fake-dbus-error")};
inline bool watcherDestroyed {false};
inline QString lastWatcherService;
inline bool lastWatcherWatchForOwnerChange {false};

inline void reset()
{
    serviceRegistered = false;
    registerObjectResult = true;
    registerServiceReply = 0;
    registerServiceReplyValid = true;
    servicePidByName.clear();
    interfaceValidity.clear();
    callReplies.clear();
    propertyReplies.clear();
    lastConnectedService.clear();
    lastDisconnectedService.clear();
    lastSentSignalPath.clear();
    lastSentSignalName.clear();
    lastAsyncCallMethod.clear();
    lastAsyncCallArgument.clear();
    lastErrorMessage = QStringLiteral("fake-dbus-error");
    watcherDestroyed = false;
    lastWatcherService.clear();
    lastWatcherWatchForOwnerChange = false;
}

inline void setServiceRegistered(bool value) { serviceRegistered = value; }
inline void setRegisterObjectResult(bool value) { registerObjectResult = value; }
inline void setRegisterServiceReply(int value, bool valid = true)
{
    registerServiceReply = value;
    registerServiceReplyValid = valid;
}
inline void setServicePid(const QString &service, uint pid)
{
    servicePidByName.insert(service, pid);
}
inline void setInterfaceValid(const QString &service, const QString &path, const QString &iface, bool valid)
{
    interfaceValidity.insert(makeKey(service, path, iface, QStringLiteral("__valid__")), valid);
}
inline void setCallReply(const QString &service, const QString &path, const QString &iface, const QString &method,
                         const QVariant &value, bool valid = true)
{
    callReplies.insert(makeKey(service, path, iface, method), ReplyData{value, valid});
}
inline void setPropertyReply(const QString &service, const QString &path, const QString &iface, const QString &name,
                             const QVariant &value, bool valid = true)
{
    propertyReplies.insert(makeKey(service, path, iface, name), ReplyData{value, valid});
}
inline void setLastErrorMessage(const QString &message)
{
    lastErrorMessage = message;
}

inline bool interfaceIsValid(const QString &service, const QString &path, const QString &iface)
{
    return interfaceValidity.value(makeKey(service, path, iface, QStringLiteral("__valid__")), false);
}

inline ReplyData callReplyFor(const QString &service, const QString &path, const QString &iface, const QString &method)
{
    return callReplies.value(makeKey(service, path, iface, method), ReplyData{});
}

inline ReplyData propertyReplyFor(const QString &service, const QString &path, const QString &iface, const QString &name)
{
    return propertyReplies.value(makeKey(service, path, iface, name), ReplyData{});
}

template<typename T>
class QDBusReply
{
public:
    QDBusReply() = default;
    explicit QDBusReply(const T &value)
        : m_value(value)
        , m_valid(true)
    {
    }

    template<typename U = T, typename = std::enable_if_t<!std::is_same_v<U, QVariant>>>
    QDBusReply(const QVariant &value)
    {
        setFromVariant(value);
    }

    QDBusReply(const QDBusMessage &message)
    {
        setFromMessage(message);
    }

    QDBusReply &operator=(const QVariant &value)
    {
        setFromVariant(value);
        return *this;
    }

    QDBusReply &operator=(const QDBusMessage &message)
    {
        setFromMessage(message);
        return *this;
    }

    bool isValid() const { return m_valid; }
    T value() const { return m_value; }
    operator T() const { return m_value; }

private:
    void setFromVariant(const QVariant &value)
    {
        m_valid = value.isValid();
        if (!m_valid) {
            m_value = T {};
            return;
        }

        if constexpr (std::is_enum_v<T>) {
            m_value = static_cast<T>(value.toInt());
        } else if constexpr (std::is_same_v<T, uint>) {
            m_value = value.toUInt();
        } else if constexpr (std::is_same_v<T, int>) {
            m_value = value.toInt();
        } else if constexpr (std::is_same_v<T, bool>) {
            m_value = value.toBool();
        } else if constexpr (std::is_same_v<T, QVariant>) {
            m_value = unwrapDbusVariant(value);
        } else {
            m_value = value.value<T>();
        }
    }

    void setFromMessage(const QDBusMessage &message)
    {
        m_valid = message.type() == QDBusMessage::ReplyMessage && !message.arguments().isEmpty();
        if (!m_valid) {
            m_value = T {};
            return;
        }

        setFromVariant(message.arguments().constFirst());
    }

    static QVariant unwrapDbusVariant(const QVariant &value)
    {
        if (value.canConvert<QDBusVariant>()) {
            return value.value<QDBusVariant>().variant();
        }
        return value;
    }

    T m_value {};
    bool m_valid {false};
};

class QDBusConnectionInterface
{
public:
    enum RegisterServiceReply {
        ServiceRegistered,
        ServiceNotRegistered
    };
    enum ConnectionCapability {
        ReplaceExistingService = 0,
        DontAllowReplacement = 1
    };

    bool isServiceRegistered(const QString &service) const
    {
        return TestDbusFake::serviceRegistered && service == QStringLiteral("org.ukui.menu");
    }

    QVariant registerService(const QString &, int, int) const
    {
        if (!registerServiceReplyValid) {
            return QVariant();
        }
        return QVariant::fromValue(registerServiceReply);
    }

    QDBusReply<uint> servicePid(const QString &service) const
    {
        return QDBusReply<uint>(QVariant::fromValue(servicePidByName.value(service, 0u)));
    }
};

class QDBusConnection
{
public:
    static QDBusConnection sessionBus()
    {
        return QDBusConnection(true);
    }

    static QDBusConnection systemBus()
    {
        return QDBusConnection(false);
    }

    QDBusConnectionInterface *interface() const
    {
        return &s_interface;
    }

    bool registerObject(const QString &, QObject *) const
    {
        return registerObjectResult;
    }

    bool connect(const QString &service, const QString &, const QString &, const QString &, QObject *, const char *) const
    {
        lastConnectedService = service;
        return true;
    }

    bool disconnect(const QString &service, const QString &, const QString &, const QString &, QObject *, const char *) const
    {
        lastDisconnectedService = service;
        return true;
    }

    bool send(const QDBusMessage &message) const
    {
        lastSentSignalPath = message.path();
        lastSentSignalName = message.member();
        return true;
    }

    QDBusError lastError() const
    {
        return QDBusError(QDBusError::Failed, lastErrorMessage);
    }

private:
    explicit QDBusConnection(bool) {}

    inline static QDBusConnectionInterface s_interface {};
};

class QDBusInterface : public QObject
{
public:
    QDBusInterface(const QString &service, const QString &path, const QString &iface, const QDBusConnection &, QObject *parent = nullptr)
        : QObject(parent)
        , m_service(service)
        , m_path(path)
        , m_iface(iface)
        , m_valid(interfaceIsValid(service, path, iface))
    {
    }

    bool isValid() const
    {
        return m_valid;
    }

    QVariant call(const QString &method) const
    {
        return callReplyFor(m_service, m_path, m_iface, method).value;
    }

    QVariant call(const QString &method, const QString &) const
    {
        return callReplyFor(m_service, m_path, m_iface, method).value;
    }

    QVariant call(const QString &method, const QString &, const QString &) const
    {
        return callReplyFor(m_service, m_path, m_iface, method).value;
    }

    QVariant property(const QString &name) const
    {
        return propertyReplyFor(m_service, m_path, m_iface, name).value;
    }

    void asyncCall(const QString &method, const QString &argument = QString()) const
    {
        lastAsyncCallMethod = method;
        lastAsyncCallArgument = argument;
    }

private:
    QString m_service;
    QString m_path;
    QString m_iface;
    bool m_valid {false};
};

} // namespace TestDbusFake

template<typename T>
using QDBusReply = TestDbusFake::QDBusReply<T>;
using QDBusConnectionInterface = TestDbusFake::QDBusConnectionInterface;
using QDBusConnection = TestDbusFake::QDBusConnection;
using QDBusInterface = TestDbusFake::QDBusInterface;

#ifndef AUTOTEST_UTILS_FAKE_DBUS_REPLY_METATYPE
#define AUTOTEST_UTILS_FAKE_DBUS_REPLY_METATYPE
Q_DECLARE_METATYPE(QDBusReply<int>)
Q_DECLARE_METATYPE(QDBusReply<uint>)
Q_DECLARE_METATYPE(QDBusReply<bool>)
#endif

#endif // AUTOTEST_UTILS_FAKE_DBUS_FAKES_H

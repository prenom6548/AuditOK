#ifndef QBUSLOGIN1ABSTRACTINTERFACE_H
#define QBUSLOGIN1ABSTRACTINTERFACE_H

#include <QObject>
#include <QMetaType>
#include <QDBusReply>
#include <QDBusMessage>
#include <QDBusMetaType>
#include <QDBusInterface>
#include <QDBusPendingReply>
#include <QDBusAbstractInterface>
struct SessionStruct{
    QString userId;
    QDBusObjectPath userPath;
};

typedef QList<SessionStruct> SessionStructList;
Q_DECLARE_METATYPE(SessionStruct)
Q_DECLARE_METATYPE(SessionStructList)

inline QDBusArgument &operator << (QDBusArgument &argument, const SessionStruct &sessionStruct)
{
    argument.beginStructure();
    argument << sessionStruct.userId;
    argument << sessionStruct.userPath;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator >> (const QDBusArgument &argument, SessionStruct &sessionStruct) {
    argument.beginStructure();
    argument >> sessionStruct.userId;
    argument >> sessionStruct.userPath;
    argument.endStructure();
    return argument;
}

inline QDBusArgument &operator << (QDBusArgument &argument, const QList<QString> &sessionStruct)
{
    argument.beginStructure();
    argument << sessionStruct;
    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator >> (const QDBusArgument &argument, QList<QString> &sessionStruct) {
    argument.beginStructure();
    argument >> sessionStruct;
    argument.endStructure();
    return argument;
}


class DBusLogin1Interface : public QDBusAbstractInterface
{
    Q_OBJECT

public:
    DBusLogin1Interface(const QString &service, const QString &path, const char *interface,
                        const QDBusConnection &connection, QObject *parent = nullptr):

        QDBusAbstractInterface(service,path,interface,connection,parent)
    {
        qRegisterMetaType<SessionStruct>("SessionStruct");
        qRegisterMetaType<SessionStructList>("SessionStructList");
        qDBusRegisterMetaType<SessionStruct>();
        qDBusRegisterMetaType<SessionStructList>();
    }
    virtual ~DBusLogin1Interface() {}

    Q_PROPERTY(SessionStructList sessions READ readSessions)
    SessionStructList readSessions() const
    {
        return qvariant_cast<SessionStructList>(property("Sessions"));
    }

    Q_PROPERTY(QString name READ readName)
    QString readName() const
    {
        return qvariant_cast<QString>(property("Name"));
    }
private:
    SessionStructList sessions;
    QString name;
Q_SIGNALS:
    void PropertiesChanged(const QVariantMap &properties);
};

#endif // QBUSLOGIN1ABSTRACTINTERFACE_H

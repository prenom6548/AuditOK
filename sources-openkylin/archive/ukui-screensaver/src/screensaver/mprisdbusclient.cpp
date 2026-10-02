
#include "mprisdbusclient.h"

#define MPRIS_DBUS_INTERFACE "org.mpris.MediaPlayer2.Player"

MPRISDBusClient::MPRISDBusClient(
    const QString &service, const QString &path, const QDBusConnection &connection, QObject *parent)
    : QDBusAbstractInterface(service, path, MPRIS_DBUS_INTERFACE, connection, parent)
{
    QDBusConnection::sessionBus().connect(
        this->service(),
        this->path(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        "sa{sv}as",
        this,
        SLOT(onPropertyChanged(QDBusMessage)));
}

MPRISDBusClient::~MPRISDBusClient()
{
    QDBusConnection::sessionBus().disconnect(
        service(),
        path(),
        "org.freedesktop.DBus.Properties",
        "PropertiesChanged",
        "sa{sv}as",
        this,
        SLOT(onPropertyChanged(QDBusMessage)));
}

void MPRISDBusClient::onPropertyChanged(const QDBusMessage &msg)
{
    QList<QVariant> arguments = msg.arguments();
    if (3 != arguments.count())
        return;
    QString interfaceName = msg.arguments().at(0).toString();
    if (interfaceName != MPRIS_DBUS_INTERFACE)
        return;
    QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
    for (const QString &prop : changedProps.keys()) {
        const QMetaObject *self = metaObject();
        for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
            QMetaProperty p = self->property(i);
            QGenericArgument value(QMetaType::typeName(p.type()), const_cast<void *>(changedProps[prop].constData()));
            if (p.name() == prop) {
                Q_EMIT p.notifySignal().invoke(this, value);
            }
        }
    }
}

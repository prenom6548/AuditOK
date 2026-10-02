#include "interactiondbusinterface.h"
#include <QDBusMessage>
#include <QDebug>
#include <QList>
#include <QString>

InteractionDBusInterface::InteractionDBusInterface(const QString &service, const QDBusConnection &connection, QObject *parent):
    QDBusAbstractInterface(service, getInterfacePath(), getInterfaceName(), connection, parent)
{

}

QString InteractionDBusInterface::getPinCode(int right)
{
    QDBusMessage message = call("Confirmed", right);
    if(message.type() == QDBusMessage::ErrorMessage){
        qDebug() << "调用接口失败!";
        return QString("getPinFailed");
    }
    QList<QVariant> args = message.arguments();
    qDebug() << args.at(0).toInt();
    qDebug() << args.at(1).toString();
    return args.at(1).toString();
}

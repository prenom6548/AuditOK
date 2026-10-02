#include "system_notification.h"
#include <QApplication>
#include <QUrl>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QDebug>


SystemNotification::SystemNotification(QObject *parent) : QObject(parent)
{
    m_interface = new QDBusInterface(QStringLiteral("org.freedesktop.Notifications"),
                                     QStringLiteral("/org/freedesktop/Notifications"),
                                     QStringLiteral("org.freedesktop.Notifications"),
                                     QDBusConnection::sessionBus(),
                                     this);
}

void SystemNotification::sendMessage(const QString &text)
{
    sendMessage(text, tr("Printer Info"), 5000, QStringList());
}

void SystemNotification::sendMessage(const QString &text, const QString &title, int timeout)
{
    sendMessage(text, title, 5000, QStringList());
}

void SystemNotification::sendMessage(const QString &text, const QString &title, int timeout, const QStringList &actions)
{
    QList<QVariant> args;
    // QVariantMap hintsMap;
    QMap<QString, QVariant> hintsMap;
    hintsMap.insert("x-ukui-popup-timeout", timeout);

    args << tr("Printer")                 // appname
         << static_cast<unsigned int>(0) // id
         << "kylin-printer"              // icon
         << title                        // summary
         << text                         // body
         << actions                      // actions
         << hintsMap                     // hints
         << (timeout == -1 ? 0 : -1);    // timeout
    auto s = m_interface->callWithArgumentList(QDBus::AutoDetect, QStringLiteral("Notify"), args);
    qDebug() << s;
}

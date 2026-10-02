#ifndef UKUI_SEARCH_AUTOTEST_DBUS_CONNECTOR_H
#define UKUI_SEARCH_AUTOTEST_DBUS_CONNECTOR_H

#include <QObject>
#include <QDBusConnection>
#include <QDBusInterface>
#include <chrono>
#include <thread>

namespace UkuiQuick {

class DbusConnector : public QObject
{
    Q_OBJECT
public:
    DbusConnector(const QString &service,
                  const QString &path,
                  const QString &interface,
                  const QDBusConnection &connection,
                  QObject *parent = nullptr)
        : QObject(parent),
          m_service(service),
          m_path(path),
          m_interface(interface),
          m_connection(connection)
    {
    }

    void start()
    {
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            auto *dbusInterface = new QDBusInterface(QString(), m_path, m_interface, m_connection);
            Q_EMIT ready(dbusInterface);
        }).detach();
    }

Q_SIGNALS:
    void ready(QDBusInterface *interface);

private:
    QString m_service;
    QString m_path;
    QString m_interface;
    QDBusConnection m_connection{QString()};
};

}

#endif

#include "startup_interface.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDateTime>
#include <QFile>
#include <QDebug>

namespace startup {

Startup::Startup(QObject *parent)
    : QObject(parent) {

}

void Startup::Start() {
    if (!calledFromDBus()) {
        return;
    }

    uint pid{0};
    const QDBusMessage msg = message();
    const QString senderAddr = msg.service();
    if (QDBusConnectionInterface *iface = connection().interface()) {
        QDBusReply<uint> reply = iface->servicePid(senderAddr);
        if (reply.isValid()) {
            pid = reply.value();
        } else {
            qWarning() << "sender pid failed: " << reply.error().name() << reply.error().message();
        }
    }

    QFile f(QString("/proc/%1/cmdline").arg(pid));
    if (f.open(QIODevice::ReadOnly)) {
        QByteArray data = f.readAll();
        QList<QByteArray> parts = data.split('\0');
        if (!parts.isEmpty() && !parts.first().isEmpty()) {
            qInfo() << "DBus called by service = " << senderAddr
                    << ", pid = " << pid
                    << ", process = " << QString::fromUtf8(parts.first())
                    << ", time = " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        } else {
            qWarning() << "Sender process name failed";
        }
    }
}

}
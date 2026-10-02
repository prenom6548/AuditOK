#include "tray-app.h"
#include <QDBusConnection>
#include "trayappstatusnotifieritemadaptor.h"
#include <QIcon>

TrayApp::TrayApp(QObject *parent) : QObject(parent)
{
    QImage image;
    image.load("://res/icon/application-x-desktop.png");
    DbusImageVector vector;
    vector.append(DbusImageStruct(image));
    m_attentionIconPixmap = vector;

    QImage image1;
    image1.load("://res/icon/computer.png");
    DbusImageVector vector1;
    vector1.append(DbusImageStruct(image1));
    m_overlayIconPixmap = vector1;
}

void TrayApp::Activate(int x, int y)
{

}

void TrayApp::ContextMenu(int x, int y)
{

}

void TrayApp::ProvideXdgActivationToken(const QString &token)
{

}

void TrayApp::Scroll(int delta, const QString &orientation)
{

}

void TrayApp::SecondaryActivate(int x, int y)
{

}

void TrayApp::init(qint64 pid)
{
    QString serviceName = QStringLiteral("org.kde.StatusNotifierItem") + QStringLiteral("-") + QString::number(pid) + QStringLiteral("-1");

    new StatusNotifierItemAdaptor(this);
    QDBusConnection connection = QDBusConnection::sessionBus();
    auto registration = connection.interface()->registerService(serviceName);

    if(m_statusNotifierWatcher) {
        delete m_statusNotifierWatcher;
        m_statusNotifierWatcher = nullptr;
    }

    m_statusNotifierWatcher = new org::kde::StatusNotifierWatcher(QStringLiteral("org.kde.StatusNotifierWatcher"),
                                                                  QStringLiteral("/StatusNotifierWatcher"),
                                                                  QDBusConnection::sessionBus(),
                                                                  this);

    if (m_statusNotifierWatcher->isValid()) {
        m_statusNotifierWatcher->call(QDBus::NoBlock, QStringLiteral("RegisterStatusNotifierItem"), serviceName);
    }

    if (registration.value() != QDBusConnectionInterface::ServiceRegistered) {
        qWarning() << "Failed to register StatusNotifierItem service on DBus!";
        return;
    }
    if(!connection.registerObject(QStringLiteral("/StatusNotifierItem"), this)) {
        qWarning() << "Failed to register StatusNotifierItem DBus object!" << connection.lastError().message();
        return;
    }
    return;
}

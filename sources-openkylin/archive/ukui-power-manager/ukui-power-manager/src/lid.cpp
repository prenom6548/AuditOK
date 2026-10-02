#include "lid.h"
#include "error.h"

Lid::Lid(QObject *parent) : QObject(parent)
{
    qDebug() << "Lid init start";
    inhibitSystemdLogin();
    initLidConnection();
    m_lidIsClosed = readLidState();
}

Lid::~Lid() {}

int Lid::inhibitSystemdLogin()
{
    int ret(0);
    QDBusInterface manager(
        QStringLiteral(LOGIN1_SERVICE),
        QStringLiteral(LOGIN1_PATH),
        QStringLiteral(LOGIN1_MANAGER),
        QDBusConnection::systemBus());
    QDBusReply<QDBusUnixFileDescriptor> reply = manager.call(
        QStringLiteral("Inhibit"),
        QStringLiteral("handle-lid-switch"),
        QStringLiteral("powermanagment"),
        QStringLiteral("ukui-powermanagement blocked LidSwitch!"),
        QStringLiteral("block"));
    if (reply.isValid()) {
        logindLock = reply.value();
        qDebug() << "Inhibit got:" << logindLock.fileDescriptor();
    } else {
        ret = INIT_LID_ERROR;
        qCritical() << "error code: " << ret << ", msg: " << reply.error();
    }
    return ret;
}

int Lid::initLidConnection()
{
    int ret(0);
    bool result = QDBusConnection::systemBus().connect(
        LOGIN1_SERVICE, LOGIN1_PATH, LOGIN1_MANAGER, "PrepareForSleep", this, SLOT(dealWakeUpSignal(bool)));
    if (!result) {
        ret = LID_CREATE_LOGIN1_CONNECTION_ERROR;
        qCritical() << "error code: " << ret;
    }
    result = QDBusConnection::sessionBus().connect(
        UKUI_UPOWER_SERVICE,
        UKUI_UPOWER_PATH,
        UKUI_UPOWER_INTERFACE,
        "LidState",
        this,
        SLOT(dealLidWatcherMssage(bool)));
    if (!result) {
        ret = LID_CREATE_UPOWER_CONNECTION_ERROR;
        qCritical() << "error code: " << ret;
    }
    return ret;
}

bool Lid::readLidState()
{
    QDBusInterface iface(UPOWER_SERVICE, UPOWER_PATH, UPOWER_INTERFACE, QDBusConnection::systemBus());
    return iface.property("LidIsClosed").toBool();
}

void Lid::dealLidWatcherMssage(bool isClosed)
{
    qDebug() << "lid watcher, lid is closed:" << isClosed;
    m_lidIsClosed = isClosed;
    emit lidChanged(m_lidIsClosed);
}

void Lid::dealWakeUpSignal(bool isWakeup)
{
    qDebug() << "wake up , system wake up state:" << isWakeup;
    if (isWakeup) {
        return;
    }
    if (m_lidIsClosed) {
        qWarning() << "Sleep is awakened unexpectedly!";
        lidChanged(m_lidIsClosed);
    }
}

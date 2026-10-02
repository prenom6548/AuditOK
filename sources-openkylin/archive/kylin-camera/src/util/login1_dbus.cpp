#include <QDebug>

#include "login1_dbus.h"
#include <dbus.hpp>

static const QString LOGIND_DEST = "org.freedesktop.login1";
static const QString LOGIND_PATH = "/org/freedesktop/login1";
static const QString LOGIND_IFACE = "org.freedesktop.login1.Manager";
static const QString login1SessionInterface = QStringLiteral("org.freedesktop.login1.Session");

static const QString SCREENSAVER_DEST = "org.ukui.ScreenSaver";
static const QString SCREENSAVER_IFACE = "org.ukui.ScreenSaver";

static const QString LOGOUT_DEST = "org.gnome.SessionManager";
static const QString LOGOUT_IFACE = "org.gnome.SessionManager";



static const QString INHIBIT_WHO = "Kylin Camera";
static const QString INHIBIT_WHY = "Stop Recording when system is shutdown or sleep";
static const QString INHIBIT_MODE = "block";



login1dbusManager::login1dbusManager() :
    m_inhibitShutdownFD(QDBusUnixFileDescriptor()),
    m_inhibitSleepFD(QDBusUnixFileDescriptor())
{    
    auto sysBus = QDBusConnection::systemBus();
    sysBus.connect(LOGIND_DEST, LOGIND_PATH,LOGIND_IFACE,
                    QString("PrepareForShutdown"), this, SLOT(login1PrepareForShutdown(bool)));
    sysBus.connect(LOGIND_DEST, LOGIND_PATH,LOGIND_IFACE,
                    QString("PrepareForSleep"), this, SLOT(login1PrepareForSleep(bool)));

    auto sesBus = QDBusConnection::sessionBus();
    sesBus.connect(SCREENSAVER_DEST,QString("/"),QString(SCREENSAVER_IFACE),
                   QString("lock"),this,SLOT(screenSaverLocked()));
    sesBus.connect(SCREENSAVER_DEST,QString("/"),QString(SCREENSAVER_IFACE),
                   QString("unlock"),this,SLOT(screenSaverUnlocked()));
    auto sesLogoutBus = QDBusConnection::sessionBus();
    sesLogoutBus.connect(LOGOUT_DEST,QString("/org/gnome/SessionManager"),QString(LOGOUT_IFACE),
                   QString("PrepareForSwitchuser"),this,SLOT(logoutSig()));

}

login1dbusManager::~login1dbusManager()
{

}

//Call this method when start record ...
bool login1dbusManager::inhibit(INHIBIT_TYPE type)
{
    qDebug() << "inhibit";
    QString what;
    if(type == INHIBIT_SLEEP){
        what = "sleep";
    }else if(type == INHIBIT_SHUTDOWN){
        what = "shutdown";
    }else{
        return false;
    }
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.login1",
                                                          "/org/freedesktop/login1",
                                                          "org.freedesktop.login1.Manager",
                                                          QStringLiteral("Inhibit"));
    message.setArguments(QVariantList({what,
                                       "Kylin Camera",
                                       INHIBIT_WHY,
                                       QStringLiteral("delay")}));
    QDBusPendingReply<QDBusUnixFileDescriptor> reply = QDBusConnection::systemBus().call(message);
    qDebug() << "xx" << reply.value().isValid();
    if(type == INHIBIT_SLEEP){
        reply.value().swap(m_inhibitSleepFD);
    }else if(type == INHIBIT_SHUTDOWN){
        reply.value().swap(m_inhibitShutdownFD);
    }else{
        return false;
    }

    return reply.value().isValid();
}

//Call this method when stop recording ...
void login1dbusManager::unInhibit()
{
    qDebug() << "uninhibit";
    if(m_inhibitSleepFD.isValid() == true)
    {
        m_inhibitSleepFD = QDBusUnixFileDescriptor();
    }
    if(m_inhibitShutdownFD.isValid() == true)
    {
        m_inhibitShutdownFD = QDBusUnixFileDescriptor();
    }
}


void login1dbusManager::login1PrepareForSleep(bool in)
{
    m_sleepFlag = in;
    if(in){
        if(m_logOutflag || m_screenLockFlag){
            return;
        }else{
            Q_EMIT locking(true);
            m_needUnlock = true;
            qDebug()<< "login1PrepareForSleep on signal has emited!";
        }
    }else{
        if(m_screenLockFlag){
            return;
        }else{
            if(m_needUnlock){
                Q_EMIT unlocking();
                m_needUnlock = false;
            }
        }
        m_logOutflag = false;
    }
}
void login1dbusManager::login1PrepareForShutdown(bool in)
{
    qDebug()<< "login1PrepareForShutdown signal has emited!";
    Q_EMIT shuttingDown(in);
}
void login1dbusManager::logoutSig()
{
    m_logOutflag = true;
    if(m_sleepFlag || m_screenLockFlag){
        return;
    }else{
        Q_EMIT locking(true);
        m_needUnlock = true;
        qDebug()<< "logout signal has emited!";
    }
}
void login1dbusManager::screenSaverLocked()
{
    m_screenLockFlag = true;
    if(m_logOutflag || m_sleepFlag){
        return;
    }else{
        Q_EMIT locking(true);
        m_needUnlock = true;
        qDebug()<< "screenSaverLocked signal has emited!";
    }
}
void login1dbusManager::screenSaverUnlocked()
{
    m_screenLockFlag = false;
    if(m_sleepFlag){
        return;
    }else{
        if(m_needUnlock){
            Q_EMIT unlocking();
            m_needUnlock = false;
        }
    }
    m_logOutflag = false;
}

#ifndef __LOGIN1_DBUS_H__
#define __LOGIN1_DBUS_H__

#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QCoreApplication>
#include <QDBusInterface>
#include <QTime>
#include <QDBusUnixFileDescriptor>

class login1dbusManager: public QObject
{
    Q_OBJECT
public:
    enum INHIBIT_TYPE
    {
        INHIBIT_SLEEP = 0,
        INHIBIT_SHUTDOWN = 1
    };
    login1dbusManager();
    ~login1dbusManager();
public:
    bool inhibit(INHIBIT_TYPE type);
    void unInhibit();
Q_SIGNALS:
    void shuttingDown(bool);
    //void locking(bool in);
    void locking(bool flag);
    void unlocking();
    void changeUserLocking(bool);
private Q_SLOTS:
    void login1PrepareForSleep(bool);
    void login1PrepareForShutdown(bool);
    void screenSaverLocked();
    void screenSaverUnlocked();
    void logoutSig();
private:
    QDBusUnixFileDescriptor m_inhibitShutdownFD;
    QDBusUnixFileDescriptor m_inhibitSleepFD;
    bool m_logOutflag = false;
    bool m_sleepFlag = false;
    bool m_screenLockFlag = false;
    bool m_needUnlock = false;
};
#endif

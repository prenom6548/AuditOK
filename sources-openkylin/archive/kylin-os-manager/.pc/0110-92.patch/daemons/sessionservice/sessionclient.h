#ifndef SESSIONCLIENT_H
#define SESSIONCLIENT_H

#include <QObject>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusReply>
#include <QProcess>
#include <QString>
#include "driverinstall/aptinstaller.h"

class SessionClient : public QObject {
Q_OBJECT
public:
    SessionClient(QObject *parent = nullptr);

public slots:

    void onNewDeviceConnectSlot(QString devString);

private:
    QString m_driverInfo;
    QString m_devType;
    QString m_devModel;
    QString m_driverSize;
    AptInstaller *m_installer = nullptr;

    QString oninstall = "";
    void initHotplugClient();

private slots:

    void actionInvokeSlot(quint32 id, QString actionkey);

    void showSuccessMsg();

    void showFailMsg();
};

#endif // SESSIONCLIENT_H

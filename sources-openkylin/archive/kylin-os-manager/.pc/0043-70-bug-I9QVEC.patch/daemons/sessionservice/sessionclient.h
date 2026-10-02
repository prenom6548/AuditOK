#ifndef SESSIONCLIENT_H
#define SESSIONCLIENT_H

#include <QObject>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusPendingReply>
#include <QProcess>

class SessionClient : public QObject {
Q_OBJECT
public:
    SessionClient(QObject *parent = nullptr);

public slots:

    void onNewDeviceConnectSlot(QString devString);


private:
    void initHotplugClient();

private slots:

    void actionInvokeSlot(quint32 id, QString actionkey);

};

#endif //KYLIN_OS_MANAGER_SESSIONCLIENT_H
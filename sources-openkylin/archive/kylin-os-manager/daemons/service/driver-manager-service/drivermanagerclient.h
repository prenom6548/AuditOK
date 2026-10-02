#ifndef DRIVERMANAGERCLIENT_H
#define DRIVERMANAGERCLIENT_H

#include <QObject>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusPendingReply>
#include <QProcess>

class DrivermanagerClient : public QObject {
Q_OBJECT
public:
    DrivermanagerClient(QObject *parent = nullptr);

private:
    void initDrivermanagerClient();

public slots:
    void updateDevInfo(QString devString);
};


#endif //DRIVERMANAGERCLIENT_H

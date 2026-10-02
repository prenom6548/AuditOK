#ifndef CONNECTIVITYMANAGER_H
#define CONNECTIVITYMANAGER_H

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QtDBus/QtDBus>

void outLog(QString str);

class ConnectivityManager
    : public QObject
    , protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.connectivity.manager")
public:
    ConnectivityManager();
    ~ConnectivityManager();

private:
    bool checkEnviron(int pid);
    bool isAllowedCaller();
    void startCommand(QString common);
    void killProcess(const char *cmd);

Q_SIGNALS:
    void sigMountError(bool, QString);
    void sigInitDiscServer(bool);

public Q_SLOTS:
    int mountFtp(QString urlStr, QString userName, QString pwd, QString path);
    int umountFtp(QString path);
    int startDiscServer();
    int stopDiscServer();
    int killServerPortProcess();
    int killConnectivity();

private:
    bool m_isMount = true;
    QProcess *m_discServer = nullptr;
    bool m_isDiscServerInit = false;
    QTimer *m_timer = nullptr;
};

#endif  // !CONNECTIVITYMANAGER_H
#ifndef CONNECTIVITYMANAGER_H
#define CONNECTIVITYMANAGER_H

#include <QObject>
#include <QProcess>
#include <QTimer>

void outLog(QString str);

class ConnectivityManager : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.kylin.connectivity.manager")
public:
    ConnectivityManager();
    ~ConnectivityManager();

private:
    void startCommand(QString common);
    void killProcess(const char *cmd);

Q_SIGNALS:
    void sigMountError(bool, QString);
    void sigInitDiscServer(bool);

public Q_SLOTS:
    void mountFtp(QString urlStr, QString userName, QString pwd, QString path);
    void umountFtp(QString path);
    void startDiscServer(QString cmd);
    void stopDiscServer();
    void killServerPortProcess();
    void killConnectivity();

private:
    bool m_isMount = true;
    QProcess *m_discServer = nullptr;
    bool m_isDiscServerInit = false;
    QTimer *m_timer = nullptr;
};

#endif  // !CONNECTIVITYMANAGER_H
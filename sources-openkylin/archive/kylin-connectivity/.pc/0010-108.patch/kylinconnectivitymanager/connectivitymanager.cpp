#include "connectivitymanager.h"
#include <QString>
#include <QUrl>
#include <QDebug>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <QFile>
#include <QThread>
#include <QDir>
#include <QCoreApplication>
#include <sys/syslog.h>

const QString DSOFTBUS_UUID_PATH = "/data/data";
const char *IPC_SERVER_SOCKET_ADDR = "/tmp/ipc.socket.server";
const char *IPC_CLIENT_SOCKET_ADDR = "/tmp/ipc.socket.client";
const char *SERVER_UNIX_SOCKET_ADDR = "/tmp/kylin.deviceDiscover.socket";

void outLog(QString str)
{
    str.prepend(QString("ConnectivityManager: "));
    syslog(LOG_INFO, "%s\n", str.toStdString().c_str());
}

ConnectivityManager::ConnectivityManager()
{
}

ConnectivityManager::~ConnectivityManager() = default;

void ConnectivityManager::mountFtp(QString urlStr, QString userName, QString pwd, QString path)
{
    QDir dir;
    if (!dir.exists(path)) {
        dir.mkdir(path);
    }
    umountFtp(path);
    m_isMount = true;
    QUrl url(urlStr);
    int uid = getuid();
    QString common = "curlftpfs -o rw,allow_other,uid=" + QString::number(uid) + " -o nonempty " + url.scheme() +
                     "://" + userName + ":" + pwd + "@" + url.host() + ":" + QString::number(url.port()) + " " + path;
    startCommand(common);
}

void ConnectivityManager::umountFtp(QString path)
{
    m_isMount = false;
    QString common = "umount -f " + path;
    startCommand(common);
}

void ConnectivityManager::startCommand(QString common)
{
    outLog(common);
    QProcess *process = new QProcess(this);
    process->start(common);
    connect(process, &QProcess::readyReadStandardError, this, [=]() {
        QString errOutput = process->readAllStandardError();
        outLog(errOutput);
        if (!errOutput.isEmpty()) {
            Q_EMIT sigMountError(m_isMount, errOutput);
        }
        m_isMount = true;
        process->deleteLater();
    });
}

void ConnectivityManager::killProcess(const char *cmd)
{
    outLog(QString("start killProcess") + QString(cmd));
    FILE *fp = popen(cmd, "r");
    if (fp == NULL) {
        outLog("command fail !");
        return;
    }

    char buf[1024];
    memset(buf, '\0', sizeof(buf));
    while (!feof(fp)) {
        fgets(buf, sizeof(buf), fp);
        QString item(buf);
        memset(buf, '\0', sizeof(buf));

        outLog(item);

        if (item.isEmpty()) {
            continue;
        }

        if (item.contains("grep")) {
            continue;
        }

        if (item.contains("kylin-connectivity-manager")) {
            continue;
        }

        if (item.contains("kylin-connectivity.log")) {
            continue;
        }

        QStringList itemList = item.trimmed().split(' ');
        itemList.removeAll("");
        QString info = itemList.value(1);
        bool ok = true;
        info.toLongLong(&ok);
        if (ok) {
            QString common = QString("kill -9 ") + info;
            QProcess::startDetached(common);
        }
    }

    pclose(fp);
}

void ConnectivityManager::startDiscServer(QString cmd)
{
    outLog("disc softbus init");
    m_discServer = new QProcess(this);
    QDir dir;
    if (dir.exists(DSOFTBUS_UUID_PATH)) {
        outLog("DSoftbus is useful");
    } else {
        dir.mkdir(DSOFTBUS_UUID_PATH);
    }

    connect(m_discServer, &QProcess::started, this, [=]() { Q_EMIT sigInitDiscServer(true); });
    connect(m_discServer, &QProcess::readyReadStandardError, this, [=]() {
        QString errOutput = m_discServer->readAllStandardError();
        outLog(QString("softbus server is not exist, softbus init failed.") + errOutput);
        Q_EMIT sigInitDiscServer(false);
    });
    m_discServer->start(cmd);
}

void ConnectivityManager::stopDiscServer()
{
    outLog("stop disc server");
    if (m_discServer) {
        if (m_discServer->state() == QProcess::Running) {
            m_discServer->close();
        }
        delete m_discServer;
        m_discServer = nullptr;
    }
}

void ConnectivityManager::killServerPortProcess()
{
    killProcess("lsof -i:27189");
    killProcess("ps -aux | grep connectivity_softbus");
    remove(IPC_SERVER_SOCKET_ADDR);
    remove(IPC_CLIENT_SOCKET_ADDR);
    remove(SERVER_UNIX_SOCKET_ADDR);
}

void ConnectivityManager::killConnectivity()
{
    outLog("Connecicvity quit!");
    QCoreApplication::quit();
    // outLog("start killConnecicvity");
    // if (m_timer == nullptr) {
    //     m_timer = new QTimer(this);
    // }
    // m_timer->start(3000);
    // connect(m_timer, &QTimer::timeout, this, [=]() {
    //     outLog("killConnecicvity timeout");
    //     m_timer->stop();
    //     m_timer->deleteLater();
    //     m_timer = nullptr;
    //     killProcess("ps -aux | grep kylin-connectivity");
    //     QCoreApplication::quit();
    // });
}

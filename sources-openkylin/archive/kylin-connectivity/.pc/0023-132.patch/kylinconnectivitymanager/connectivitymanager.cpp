#include "connectivitymanager.h"
#include <QString>
#include <QUrl>
#include <QDir>
#include <QFileInfo>
#include <QFile>
#include <QThread>
#include <QTextStream>
#include <QCoreApplication>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syslog.h>
#include <errno.h>

const QString DISC_SERVER_PATH = "connectivity_softbus_server";
const QString DSOFTBUS_UUID_PATH = "/data/data";
const char *IPC_SERVER_SOCKET_ADDR = "/tmp/ipc.socket.server";
const char *IPC_CLIENT_SOCKET_ADDR = "/tmp/ipc.socket.client";
const char *SERVER_UNIX_SOCKET_ADDR = "/tmp/kylin.deviceDiscover.socket";
static const QStringList programWhiteList = {"/usr/sbin/ksc-defender", "/usr/bin/kylin-connectivity"};

void outLog(QString str)
{
    str.prepend(QString("ConnectivityManager: "));
    syslog(LOG_INFO, "%s\n", str.toStdString().c_str());
}

ConnectivityManager::ConnectivityManager()
{
}

ConnectivityManager::~ConnectivityManager() = default;

bool ConnectivityManager::checkEnviron(int pid)
{
    QString filePath = QString("/proc/%1/environ").arg(pid);
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        outLog(QString("file.open failed %s").arg(filePath));
        return false;
    }
    QTextStream in(&file);
    QString data = in.readAll();
    file.close();
    data.replace('\0', '\n');
    QStringList environVars = data.split('\n');
    for (const QString &var : environVars) {
        if (!var.isEmpty()) {
            int pos = var.indexOf('=');
            if (pos != -1) {
                QString key = var.left(pos);
                if (key == "LD_PRELOAD" || key == "LD_LIBRARY_PATH" || key == "LD_AUDIT") {
                    return false;
                }
            }
        }
    }
    return true;
}

bool ConnectivityManager::isAllowedCaller()
{
    QString clientService = message().service();
    QDBusConnectionInterface *intf = connection().interface();
    unsigned int pid = intf->servicePid(clientService).value();
    /*check caller environ*/
    if (!checkEnviron(pid)) {
        sendErrorReply(QDBusError::ErrorType::Failed, QString("[%2] dbus method control,env forbidden").arg((pid)));
        return false;
    }
    /*check caller white list*/
    QFileInfo file(QString("/proc/%1/exe").arg(pid));
    QString execPath = "";
    if (file.exists()) {
        execPath = file.canonicalFilePath();
        for (const QString &whitePath : programWhiteList) {
            if (execPath == whitePath) {
                return true;
            }
        }
    }
    sendErrorReply(QDBusError::ErrorType::Failed,
                   QString("[%2] dbus method control,operation not permitted").arg((execPath)));
    return false;
}

int ConnectivityManager::mountFtp(QString urlStr, QString userName, QString pwd, QString path)
{
    if (!isAllowedCaller()) {
        return -EPERM;
    }
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

int ConnectivityManager::umountFtp(QString path)
{
    if (!isAllowedCaller()) {
        return -EPERM;
    }
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

int ConnectivityManager::startDiscServer()
{
    if (!isAllowedCaller()) {
        return -EPERM;
    }
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
    m_discServer->start(DISC_SERVER_PATH);
}

int ConnectivityManager::stopDiscServer()
{
    if (!isAllowedCaller()) {
        return -EPERM;
    }
    outLog("stop disc server");
    if (m_discServer) {
        if (m_discServer->state() == QProcess::Running) {
            m_discServer->close();
        }
        delete m_discServer;
        m_discServer = nullptr;
    }
}

int ConnectivityManager::killServerPortProcess()
{
    if (!isAllowedCaller()) {
        return -EPERM;
    }
    killProcess("lsof -i:27189");
    killProcess("ps -aux | grep connectivity_softbus");
    remove(IPC_SERVER_SOCKET_ADDR);
    remove(IPC_CLIENT_SOCKET_ADDR);
    remove(SERVER_UNIX_SOCKET_ADDR);
}

int ConnectivityManager::killConnectivity()
{
    if (!isAllowedCaller()) {
        return -EPERM;
    }
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

// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause


#include "qtlocalpeer.h"
#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>
#include <QStandardPaths>

#if defined(Q_OS_WIN)
#include <QLibrary>
#include <qt_windows.h>
typedef BOOL(WINAPI*PProcessIdToSessionId)(DWORD,DWORD*);
static PProcessIdToSessionId pProcessIdToSessionId = 0;
#endif
#if defined(Q_OS_UNIX)
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#endif

namespace QtLP_Private {
#include "qtlockedfile.cpp"
#if defined(Q_OS_WIN)
#include "qtlockedfile_win.cpp"
#else
#include "qtlockedfile_unix.cpp"
#endif
}

const char* QtLocalPeer::ack = "ack";

#if defined(Q_OS_UNIX)
static bool isSecureRuntimeDir(const QString &path)
{
    if (path.isEmpty())
        return false;

    const QFileInfo info(path);
    const QFile::Permissions required = QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner;
    const QFile::Permissions forbidden = QFile::ReadGroup | QFile::WriteGroup | QFile::ExeGroup
            | QFile::ReadOther | QFile::WriteOther | QFile::ExeOther;
    const QFile::Permissions permissions = info.permissions();

    return info.isDir()
            && info.ownerId() == ::getuid()
            && ((permissions & required) == required)
            && !(permissions & forbidden)
            && ::access(QFile::encodeName(path).constData(), W_OK | X_OK) == 0;
}

static bool ensurePrivateRuntimeDir(const QString &path)
{
    QDir dir(path);
    if (!dir.exists() && !dir.mkpath(QLatin1String(".")))
        return false;

    if (::chmod(QFile::encodeName(path).constData(), S_IRWXU) != 0)
        return isSecureRuntimeDir(path);

    return isSecureRuntimeDir(path);
}

static QString homeDirectory()
{
    struct passwd *pw = ::getpwuid(::getuid());
    if (pw && pw->pw_dir && pw->pw_dir[0] != '\0')
        return QString::fromLocal8Bit(pw->pw_dir);

    return QDir::homePath();
}

static bool ensureLockFileExists(const QString &lockFileName)
{
    int flags = O_CREAT | O_RDWR;
#ifdef O_NOFOLLOW
    flags |= O_NOFOLLOW;
#endif

    const QByteArray lockFileBytes = QFile::encodeName(lockFileName);
    const int fd = ::open(lockFileBytes.constData(), flags, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        qWarning("QtLocalPeer: Failed to create/open lock file: %s", ::strerror(errno));
        return false;
    }

    struct stat st;
    if (::fstat(fd, &st) != 0 || !S_ISREG(st.st_mode) || st.st_uid != ::getuid()) {
        qWarning("QtLocalPeer: Unsafe lock file");
        ::close(fd);
        return false;
    }

    if (::fchmod(fd, S_IRUSR | S_IWUSR) != 0)
        qWarning("QtLocalPeer: Failed to restrict lock file permissions: %s", ::strerror(errno));

    ::close(fd);
    return true;
}

static bool writeLockRecord(QtLP_Private::QtLockedFile &lockFile)
{
    const QByteArray content = QByteArray::number(::getpid()) + ":"
            + QByteArray::number(::getuid()) + "\n";

    if (!lockFile.resize(0))
        return false;
    if (lockFile.write(content) != content.size())
        return false;

    return lockFile.flush();
}
#endif

static QString secureRuntimeDir()
{
#if defined(Q_OS_UNIX)
    const QString expectedRuntimePath = QLatin1String("/run/user/") + QString::number(::getuid());
    if (isSecureRuntimeDir(expectedRuntimePath))
        return expectedRuntimePath;

    const QString runtimePath = QStandardPaths::writableLocation(QStandardPaths::RuntimeLocation);
    if (isSecureRuntimeDir(runtimePath))
        return runtimePath;

    const QString homePath = homeDirectory();
    if (homePath.isEmpty())
        return QString();

    const QString fallbackDir = homePath + QLatin1String("/.cache/kylin-runtime");
    return ensurePrivateRuntimeDir(fallbackDir) ? fallbackDir : QString();
#else
    const QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    return tempPath.isEmpty() ? QDir::homePath() : tempPath;
#endif
}

QtLocalPeer::QtLocalPeer(QObject* parent, const QString &appId)
    : QObject(parent), id(appId)
{
    QString prefix = id;
    if (id.isEmpty()) {
        id = QCoreApplication::applicationFilePath();
#if defined(Q_OS_WIN)
        id = id.toLower();
#endif
        prefix = id.section(QLatin1Char('/'), -1);
    }
    prefix.remove(QRegularExpression("[^a-zA-Z]"));
    prefix.truncate(6);

    QByteArray idc = id.toUtf8();
    quint16 idNum = qChecksum(idc.constData(), idc.size());
    socketName = QLatin1String("qtsingleapp-") + prefix
                 + QLatin1Char('-') + QString::number(idNum, 16);

#if defined(Q_OS_WIN)
    if (!pProcessIdToSessionId) {
        QLibrary lib("kernel32");
        pProcessIdToSessionId = (PProcessIdToSessionId)lib.resolve("ProcessIdToSessionId");
    }
    if (pProcessIdToSessionId) {
        DWORD sessionId = 0;
        pProcessIdToSessionId(GetCurrentProcessId(), &sessionId);
        socketName += QLatin1Char('-') + QString::number(sessionId, 16);
    }
#else
    socketName += QLatin1Char('-') + QString::number(::getuid(), 16);
#endif

    server = new QLocalServer(this);
#if defined(Q_OS_UNIX)
    server->setSocketOptions(QLocalServer::UserAccessOption);
#endif
    const QString lockFileDir = secureRuntimeDir();
    if (lockFileDir.isEmpty()) {
        qWarning("QtLocalPeer: No secure runtime directory available");
        return;
    }

    const QString lockFileName = lockFileDir + QLatin1Char('/') + socketName
                                 + QLatin1String("-lockfile");
#if defined(Q_OS_UNIX)
    // Keep the local-server endpoint and lock file in the same runtime directory.
    socketName = lockFileDir + QLatin1Char('/') + socketName;
#endif

#if defined(Q_OS_UNIX)
    if (!ensureLockFileExists(lockFileName))
        return;

    lockFile.setFileName(lockFileName);
    if (!lockFile.open(QIODevice::ReadWrite)) {
        qWarning("QtLocalPeer: Failed to open lock file: %s",
                 qPrintable(lockFile.errorString()));
        return;
    }

    // Hold the write lock immediately so another process cannot become primary
    // between construction and the first isClient() call.
    if (lockFile.lock(QtLP_Private::QtLockedFile::WriteLock, false)
            && !writeLockRecord(lockFile))
        qWarning("QtLocalPeer: Failed to write lock file");
#else
    lockFile.setFileName(lockFileName);
    lockFile.open(QIODevice::ReadWrite);
#endif
}



bool QtLocalPeer::isClient()
{
    if (!lockFile.isOpen()) {
        qWarning("QtLocalPeer: single-instance lock is unavailable");
        return false;
    }

    if (lockFile.isLocked() && server->isListening())
        return false;

    if (!lockFile.isLocked()
            && !lockFile.lock(QtLP_Private::QtLockedFile::WriteLock, false)) {
        return true;
    }

#if defined(Q_OS_UNIX)
    if (!writeLockRecord(lockFile))
        qWarning("QtLocalPeer: Failed to write lock file");
#endif

    bool res = server->listen(socketName);
#if defined(Q_OS_UNIX) && (QT_VERSION >= QT_VERSION_CHECK(4,5,0))
    // ### Workaround
    if (!res && server->serverError() == QAbstractSocket::AddressInUseError) {
        QFile::remove(socketName);
        res = server->listen(socketName);
    }
#endif
    if (!res)
        qWarning("QtSingleCoreApplication: listen on local socket failed, %s", qPrintable(server->errorString()));
    else
        QObject::connect(server, SIGNAL(newConnection()), this, SLOT(receiveConnection()), Qt::UniqueConnection);
    return false;
}


bool QtLocalPeer::sendMessage(const QString &message, int timeout)
{
    if (!isClient())
        return false;

    QLocalSocket socket;
    bool connOk = false;
    for(int i = 0; i < 2; i++) {
        // Try twice, in case the other instance is just starting up
        socket.connectToServer(socketName);
        connOk = socket.waitForConnected(timeout/2);
        if (connOk || i)
            break;
        int ms = 250;
#if defined(Q_OS_WIN)
        Sleep(DWORD(ms));
#else
        struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
        nanosleep(&ts, NULL);
#endif
    }
    if (!connOk)
        return false;

    QByteArray uMsg(message.toUtf8());
    QDataStream ds(&socket);
    ds.writeBytes(uMsg.constData(), uMsg.size());
    bool res = socket.waitForBytesWritten(timeout);
    if (res) {
        res &= socket.waitForReadyRead(timeout);   // wait for ack
        if (res)
            res &= (socket.read(qstrlen(ack)) == ack);
    }
    return res;
}


void QtLocalPeer::receiveConnection()
{
    QLocalSocket* socket = server->nextPendingConnection();
    if (!socket)
        return;

    while (true) {
        if (socket->state() == QLocalSocket::UnconnectedState) {
            qWarning("QtLocalPeer: Peer disconnected");
            delete socket;
            return;
        }
        if (socket->bytesAvailable() >= qint64(sizeof(quint32)))
            break;
        socket->waitForReadyRead();
    }

    QDataStream ds(socket);
    QByteArray uMsg;
    quint32 remaining;
    ds >> remaining;
    uMsg.resize(remaining);
    int got = 0;
    char* uMsgBuf = uMsg.data();
    do {
        got = ds.readRawData(uMsgBuf, remaining);
        remaining -= got;
        uMsgBuf += got;
    } while (remaining && got >= 0 && socket->waitForReadyRead(2000));
    if (got < 0) {
        qWarning("QtLocalPeer: Message reception failed %s", socket->errorString().toLatin1().constData());
        delete socket;
        return;
    }
    QString message(QString::fromUtf8(uMsg));
    socket->write(ack, qstrlen(ack));
    socket->waitForBytesWritten(1000);
    socket->waitForDisconnected(1000); // make sure client reads ack
    delete socket;
    emit messageReceived(message); //### (might take a long time to return)
}

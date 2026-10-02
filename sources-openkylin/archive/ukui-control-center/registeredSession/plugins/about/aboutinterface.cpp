/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "aboutinterface.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QProcess>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>

#define STYLE_NAME_KEY "style-name"
#define CONTAIN_STYLE_NAME_KEY "styleName"
#define UKUI_DEFAULT "ukui-default"
#define UKUI_DARK "ukui-dark"

QStringList AboutInterface::ipList;
QMutex AboutInterface::ipListMutex;

AboutInterface::AboutInterface():UKUI::ServiceObject(nullptr)
{
    mStyleGsettings = UniversalInterface::self()->ukuiStyleGsettings();
    mNtphostNameList.append(QString("0.cn.pool.ntp.org"));
    mNtphostNameList.append(QString("1.cn.pool.ntp.org"));
    mNtphostNameList.append(QString("2.cn.pool.ntp.org"));
    mNtphostNameList.append(QString("3.cn.pool.ntp.org"));
    mNtphostNameList.append(QString("cn.pool.ntp.org"));
    mNtphostNameList.append(QString("0.tw.pool.ntp.org"));
    mNtphostNameList.append(QString("1.tw.pool.ntp.org"));
    mNtphostNameList.append(QString("2.tw.pool.ntp.org"));
    mNtphostNameList.append(QString("3.tw.pool.ntp.org"));
    mNtphostNameList.append(QString("tw.pool.ntp.org"));
    mNtphostNameList.append(QString("pool.ntp.org"));
    mNtphostNameList.append(QString("time.windows.com"));
    mNtphostNameList.append(QString("time.nist.gov"));
    mNtphostNameList.append(QString("time-nw.nist.gov"));
    mNtphostNameList.append(QString("asia.pool.ntp.org"));
    mNtphostNameList.append(QString("europe.pool.ntp.org"));
    mNtphostNameList.append(QString("oceania.pool.ntp.org"));
    mNtphostNameList.append(QString("north-america.pool.ntp.org"));
    mNtphostNameList.append(QString("south-america.pool.ntp.org"));
    mNtphostNameList.append(QString("africa.pool.ntp.org"));
    mNtphostNameList.append(QString("ca.pool.ntp.org"));
    mNtphostNameList.append(QString("uk.pool.ntp.org"));
    mNtphostNameList.append(QString("us.pool.ntp.org"));
    mNtphostNameList.append(QString("au.pool.ntp.org"));

    getSystemVersionList();
    getBuildList();
    getDesktopEnvStr();
    getTotalMemoryStr();
    getBlockInfoStr();

    if (mStyleGsettings) {
        connect(mStyleGsettings, &QGSettings::changed, [=](const QString &key) {
           emit changed(key);
        });
    }
}

QStringList AboutInterface::getSystemVersion()
{
    return mSystemVersion;
}

void AboutInterface::getSystemVersionList()
{
    QString versionPath = "/etc/os-release";
    QStringList osRes = readFile(versionPath);
    QString versionID;
    QString version;

    for (QString str : osRes) {
        if (str.contains("VERSION_ID=")) {
            QRegExp rx("VERSION_ID=\"(.*)\"$");
            int pos = rx.indexIn(str);
            if (pos > -1) {
                versionID = rx.cap(1);
            }
        }

        if (!QLocale::system().name().compare("zh_CN", Qt::CaseInsensitive)) {
            if (str.contains("VERSION=")) {
                QRegExp rx("VERSION=\"(.*)\"$");
                int pos = rx.indexIn(str);
                if (pos > -1) {
                    version = rx.cap(1);
                }
            }
        } else if (!QLocale::system().name().compare("en_US", Qt::CaseInsensitive)) {
            if (str.contains("VERSION_US=")) {
                QRegExp rx("VERSION_US=\"(.*)\"$");
                int pos = rx.indexIn(str);
                if (pos > -1) {
                    version = rx.cap(1);
                }
            }
        }
    }
    version = kdk_system_get_version(0);
    qDebug() << Q_FUNC_INFO << "kdk_system_get_version" << version;
    mSystemVersion.clear();
    mSystemVersion.append(versionID);
    mSystemVersion.append(version);
}

void AboutInterface::setThemeMode(QString styleName)
{
    if (mStyleGsettings->keys().contains("styleName")) {
        mStyleGsettings->set(STYLE_NAME_KEY, styleName);
    } else {
        qCritical() << "mStyleGsettings not contains the key: " << STYLE_NAME_KEY;
    }
}

QString AboutInterface::getThemeMode()
{
    if (mStyleGsettings->keys().contains("styleName")) {
        return mStyleGsettings->get(STYLE_NAME_KEY).toString();
    } else {
        qCritical() << "mStyleGsettings not contains the key: " << STYLE_NAME_KEY;
    }
    return QString();
}

void AboutInterface::getBuildList() {
    mBuild.clear();
    version_t ver= kdk_system_get_version_detaile();
    mBuild.append(ver.os_version);
    mBuild.append(ver.update_version);
}

QStringList AboutInterface::getBuild()
{
    mBuild.clear();
    version_t ver= kdk_system_get_version_detaile();
    mBuild.append(ver.os_version);
    mBuild.append(ver.update_version);
    qDebug() << Q_FUNC_INFO << "os_version" << ver.os_version << "update_version" << ver.update_version;
    return mBuild;
}

void AboutInterface::getDesktopEnvStr()
{
    QString dEnv;
    foreach (dEnv, QProcess::systemEnvironment()) {
        if (dEnv.startsWith("XDG_CURRENT_DESKTOP"))
            break;
    }
    mDesktopEnv.clear();
    if (!dEnv.isEmpty()) {
        QString desktop = dEnv.section("=", -1, -1);
        if (desktop.contains("UKUI", Qt::CaseInsensitive)) {
            mDesktopEnv = QString("UKUI");
            return;
        } else {
            mDesktopEnv = desktop;
            return;
        }
    }
    mDesktopEnv = QString("UKUI");
}

QString AboutInterface::getDesktopEnv()
{
    return mDesktopEnv;
}

void AboutInterface::getTotalMemoryStr() {
    m_availableMem.clear();
    const QString fileName = "/proc/meminfo";
    QFile meninfoFile(fileName);
    if (!meninfoFile.exists()) {
        qDebug() << "/proc/meminfo file not exist";
    }
    if (!meninfoFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "open /proc/meminfo fail";
    }

    QTextStream in(&meninfoFile);
    QString line = in.readLine();
    float memtotal = 0;
    int count = 0;

    while (!line.isNull()) {
        if (line.contains("MemTotal")) {
            line.replace(QRegExp("[\\s]+"), " ");

            QStringList lineList = line.split(" ");
            QString mem = lineList.at(1);
            memtotal = mem.toFloat();
            count++;
            if (count == 2) {
               break;
            } else {
                line = in.readLine();
            }
        } else {
            line = in.readLine();
        }
    }
    float memAvailableFloat = memtotal / 1024 / 1024; // bug#520069 可用内存改为除去硬件保留的内存，而非实时可用内存
    memtotal = ceil(memtotal / 1024 / 1024);
    mTotalMem = QString("%1%2").arg(QString::number(memtotal)).arg("GB");
    m_availableMem = QString::number(memAvailableFloat, 'f', 1);
    qDebug() << Q_FUNC_INFO << "mTotalMem" << mTotalMem << "availableMem" << m_availableMem << "memAvailableFloat" << memAvailableFloat;
    meninfoFile.close();
}

QString AboutInterface::getAvailableMemory() {
    qDebug() << Q_FUNC_INFO << "getAvailableMemory" << m_availableMem;
    return m_availableMem;
}

QString AboutInterface::getTotalMemory() {
    qDebug() << Q_FUNC_INFO << "getTotalMemory" << mTotalMem + m_availableMem;
    return mTotalMem;
}

void AboutInterface::getBlockInfoStr()
{
    QProcess process;
    process.start("lsblk");
    process.waitForFinished();
    mBlockInfo = process.readAllStandardOutput();
}

QString AboutInterface::getBlockInfo()
{
    return mBlockInfo;
}

QString AboutInterface::getDiskInfo()
{
    QProcess process;
    process.start("df -Tl");
    process.waitForFinished();
    return process.readAllStandardOutput();
}

QString AboutInterface::getUpgradeDate()
{
    QSqlDatabase db;
    if(QSqlDatabase::contains("qt_sql_default_connection"))
        db = QSqlDatabase::database("qt_sql_default_connection");
    else
        db  = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/var/cache/kylin-system-updater/kylin-system-updater.db");

    if (!db.open()) {
        perror("kylin-system-updater.db open error");
        return QString();
    }

    //载入数据库数据
    QString date;
    QSqlQuery query(db);
    query.exec("SELECT * FROM updateinfos where appname=\"kylin-update-desktop-system\"");

    while(query.next()){
        QSqlRecord rec = query.record();
        int snamecol = rec.indexOf("appname");
        QString value = query.value(snamecol).toString();
        if (value == "kylin-update-desktop-system") {
            snamecol = rec.indexOf("date");
            date = query.value(snamecol).toString().split(" ").at(0);
            qDebug()<<date;
            db.close();
            return date;
        }

    }
    db.close();
    return QString();
}

QString AboutInterface::getNtpTime()
{
    int count = 0;
    for (QString host : mNtphostNameList) {
        {
            QMutexLocker locker(&ipListMutex);
            ipList.clear();
        }
        QByteArray hostBytes = host.toLatin1();
        ntpGethostbyname(hostBytes.data());
        QStringList resolvedIps;
        {
            QMutexLocker locker(&ipListMutex);
            resolvedIps = ipList;
        }
        for (QString ip : resolvedIps) {
            ++count;
            qDebug() << Q_FUNC_INFO << host << "---->" << ip << ":" << count;
            if (count >= 5) {
                return "";
            }
            QByteArray ipBytes = ip.toLatin1();
            char *result = ntpdate(ipBytes.data());
            if (result != NULL) {
                return QString(result);
            }
        }
    }
    return QString();
}

bool AboutInterface::openActivation()
{
    QString cmd = "kylin-activation";
    QProcess process(this);
    process.startDetached(cmd);
    return true;
}

QStringList AboutInterface::readFile(QString filepath)
{
    QStringList fileCont;
    QFile file(filepath);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "ReadFile() failed to open" << filepath;
            return QStringList();
        }
        QTextStream textStream(&file);
        while (!textStream.atEnd()) {
            QString line = textStream.readLine();
            line.remove('\n');
            fileCont<<line;
        }
        file.close();
        return fileCont;
    } else {
        qWarning() << filepath << " not found"<<endl;
        return QStringList();
    }
}

int AboutInterface::ntpGethostbyname(char *servname)
{
    pthread_t t1 = 0;
    int ret = 0;

    // 在堆上保存参数，避免与子线程生命周期竞态；join 前主线程持有有效数据
    struct dnsInfo *dnsinfo = new dnsInfo;
    dnsinfo->servname = servname ? strdup(servname) : nullptr;
    dnsinfo->pid = pthread_self();

    ret = pthread_create(&t1, NULL, threadGetAddrInfo, (void *)dnsinfo);
    if (ret != 0) {
        qDebug() << "pthread_create error";
        free(dnsinfo->servname);
        delete dnsinfo;
        return -1;
    }

    // 等待解析线程结束，保证 ipList 写入完成后再被主线程读取
    pthread_join(t1, nullptr);
    return 0;
}

char *AboutInterface::ntpdate(char *hostname)
{
    int portno = 123;     //NTP is port 123
    int maxlen = 1024;        //check our buffers
    int i;          // misc var i
    unsigned char msg[48]={(0 << 6) | (3 << 3) | (3 << 0), 0, 4, ((-6) & 0xFF), 0, 0, 0, 0, 0};    // the packet we send
    unsigned long  buf[maxlen]; // the buffer we get back
    struct protoent *proto;
    struct sockaddr_in server_addr;
    int s;  // socket
    long tmit;   // the time -- This is a time_t sort of

    proto = getprotobyname("udp");
    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == s) {
        perror("socket");
        return NULL;
    }

    memset( &server_addr, 0, sizeof( server_addr ));
    server_addr.sin_family=AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(hostname);

    server_addr.sin_port=htons(portno);

    i=sendto(s,msg,sizeof(msg),0,(struct sockaddr *)&server_addr,sizeof(server_addr));
    if (-1 == i) {
        perror("sendto");
        return NULL;
    }

    // 设置超时
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 280000;//微秒
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO_NEW, &timeout, sizeof(timeout)) == -1) {
        perror("setsockopt failed:");
        return NULL;
    }


    struct sockaddr saddr;
    socklen_t saddr_l = sizeof (saddr);
    i=recvfrom(s,buf,48,0,&saddr,&saddr_l);
    if (-1 == i) {
        perror("recvfr");
        return NULL;
    }

    tmit=ntohl((time_t)buf[4]);    // get transmit time

    tmit -= 2208988800U;

    return ctime(&tmit);
}

void *AboutInterface::threadGetAddrInfo(void *arg)
{
    int ret = 0;
    struct dnsInfo *dnsinfo = (struct dnsInfo *)arg;
    struct addrinfo *rptr = NULL;
    struct addrinfo *iptr = NULL;
    char iphost[256] = { 0 };

    char *servname = dnsinfo ? dnsinfo->servname : nullptr;
    pthread_t pid_master = dnsinfo ? dnsinfo->pid : 0;

    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    ret = getaddrinfo(servname, nullptr, &hints, &rptr);

    if (ret != 0) {
        free(servname);
        delete dnsinfo;
        return nullptr;
    }

    ret = pthread_kill(pid_master, 0); //判断主线程是否退出
    if (0 == ret) {
        QMutexLocker locker(&ipListMutex);
        for (iptr = rptr; NULL != iptr; iptr = iptr->ai_next) {
            if (AF_INET != iptr->ai_family)
                continue;
            memset(iphost, 0, 256);
            if (NULL == inet_ntop(AF_INET, &(((struct sockaddr_in *)(iptr->ai_addr))->sin_addr), iphost, 256)) {
                continue;
            }
            ipList.append(QString::fromLatin1(iphost));
        }
    }

    freeaddrinfo(rptr);
    free(servname);
    delete dnsinfo;
    return nullptr;
}

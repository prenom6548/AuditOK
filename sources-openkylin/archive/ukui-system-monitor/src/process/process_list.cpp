/*
 * Copyright (C) 2021 KylinSoft Co., Ltd.
 *
 * Authors:
 *  Yang Min yangmin@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "process_list.h"

#include <unistd.h>
#include <systemd/sd-login.h>
#include <set>
#include <list>
#include <glib.h>
#include <glib/gi18n.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/resource.h> 
#include <systemd/sd-login.h>

#include <QDebug>
#include <QFileInfo>
#include <kysdk/kysdk-base/kyutils.h>

#include "../sysresource/commoninfo.h"
#include "../util.h"
#include "../desktopfileinfo.h"

namespace sysmonitor {
namespace process {

// 新增：批量读取缓冲区（线程本地，避免重复分配）
thread_local static char g_readBuf[4096];

// wayland 过滤词条
const QStringList g_appFilterList = {"peony-qt-desktop", "ukui-panel", "ukui-menu","ukui-tablet-desktop", "ukui-sidebar"};

ProcessPriority getProcessPriorityStub(int prio)
{
    if (prio == 0) {
        return NormalPriority;
    } else if (prio == VeryHighPriority || prio == HighPriority || prio == LowPriority || prio == VeryLowPriority) {
        return ProcessPriority(prio);
    } else if (prio <= VeryLowPriorityMin && prio >= VeryHighPriorityMax) {
        return CustomPriority;
    } else {
        return InvalidPriority;
    }
}

Process::Process()
    : d(new ProcessData())
{

}
Process::Process(pid_t pid)
    : d(new ProcessData())
{
    d->pid = pid;
}
Process::Process(const Process &other)
    : d(other.d)
{
}
Process &Process::operator=(const Process &rhs)
{
    if (this == &rhs)
        return *this;

    d = rhs.d;
    return *this;
}

Process::~Process()
{
}

bool Process::isValid() const
{
    return d && d->isValid();
}

void Process::setValid(bool isValid)
{
    if (d) {
        d->valid = isValid;
    }
}

pid_t Process::pid() const
{
    return d->pid;
}

QString Process::getProcState() const
{
    return d->m_processState;
}

void Process::setProcState(QString mode)
{
    if (d) {
        d->m_processState = mode;
    }
}

double Process::getCpuPercent() const
{
    return d->cpu;
}

void Process::setCpuPercent(double cpuPercent)
{
    if (d) {
        d->cpu = cpuPercent;
    }
}

qulonglong Process::getMemeryUsed() const
{
    return d->m_memory;
}

void Process::setMemeryUsed(qulonglong lMemUsed)
{
    if (d) {
        d->m_memory = lMemUsed;
    }
}

int Process::getNice() const
{
    return d->m_nice;
}

void Process::setNice(int lNice)
{
    if (d) {
        d->m_nice = lNice;
    }
}

QString Process::getFlowNetDesc() const
{
    return d->m_flownet;
}

void Process::setFlowNetDesc(QString strFlowNet)
{
    if (d) {
        d->m_flownet = strFlowNet;
    }
}

QString Process::getDiskIODesc() const
{
    return d->m_diskio;
}

void Process::setDiskIODesc(QString strDiskIO)
{
    if (d) {
        d->m_diskio = strDiskIO;
    }
}

qint64 Process::getFlowNet() const
{
    return d->m_numFlowNet;
}

void Process::setFlowNet(qint64 llFlowNet)
{
    if (d) {
        llFlowNet = llFlowNet < 0 ? 0 : llFlowNet;
        d->m_numFlowNet = llFlowNet;
    }
}

qint64 Process::getDiskIO() const
{
    return d->m_numDiskIo;
}

void Process::setDiskIO(qint64 llDiskIO)
{
    if (d) {
        llDiskIO = llDiskIO < 0 ? 0 : llDiskIO;
        d->m_numDiskIo = llDiskIO;
    }
}

QString Process::getIconPath() const
{
    return d->strIconPath;
}

void Process::setIconPath(QString iconPath)
{
    if (d) {
        d->strIconPath = iconPath;
    }
}

QString Process::getProcName() const
{
    return d->processName;
}

void Process::setProcName(QString strProcName)
{
    if (d) {
        d->processName = strProcName;
    }
}

QString Process::getDisplayName() const
{
    return d->displayName;
}

void Process::setDisplayName(QString strDisplayName)
{
    if (d) {
        d->displayName = strDisplayName;
    }
}

QString Process::getProcPath() const
{
    return d->path;
}

void Process::setProcPath(QString strProcPath)
{
    if (d) {
        d->path = strProcPath;
    }
}

QString Process::getCmd() const
{
    return d->m_cmd;
}

void Process::setCmd(QString cmd)
{
    if (d) {
        d->m_cmd = cmd;
    }
}

int Process::getUid() const
{
    return d->m_uid;
}

void Process::setUid(int uid)
{
    if (d) {
        d->m_uid = uid;
    }
}

unsigned long long Process::getSTime() const
{
    return d->m_stime;
}

void Process::setSTime(unsigned long long time)
{
    if (d) {
        d->m_stime = time;
    }
}

unsigned long long Process::getUTime() const
{
    return d->m_utime;
}

void Process::setUTime(unsigned long long time)
{
    if (d) {
        d->m_utime = time;
    }
}

QString Process::getProcUser() const
{
    return d->user;
}

void Process::setProcUser(QString strProcUser)
{
    if (d) {
        d->user = strProcUser;
    }
}

QString Process::getProcStatus() const
{
    return d->m_status;
}

void Process::setProcStatus(QString strProcStatus)
{
    if (d) {
        d->m_status = strProcStatus;
    }
}

QString Process::getProcSession() const
{
    return d->m_session;
}

void Process::setProcSession(QString strProcSession)
{
    if (d) {
        d->m_session = strProcSession;
    }
}

QString Process::getProcCpuDurationTime() const
{
    return d->cpu_duration_time;
}

void Process::setProcCpuDurationTime(QString strProcCpuDurationTime)
{
    if (d) {
        d->cpu_duration_time = strProcCpuDurationTime;
    }
}

unsigned int Process::getProcType() const
{
    return d->m_type;
}

void Process::addProcType(unsigned int uType)
{
    if (d) {
        d->m_type |= uType;
    }
}

void Process::clearProcType()
{
    if (d) {
        d->m_type = NoFilter;
    }
}

QString Process::getProcSeat() const
{
    return d->m_seat;
}

void Process::setProcSeat(QString strSeat)
{
    if (d) {
        d->m_seat = strSeat;
    }
}

unsigned long Process::getProcStartTime() const
{
    return d->m_start_time;
}

void Process::setProcStartTime(unsigned long luStartTime)
{
    if (d) {
        d->m_start_time = luStartTime;
    }
}

qulonglong Process::getProcCpuTime() const
{
    return d->m_cpu_time;
}

void Process::setProcCpuTime(qulonglong lluCpuTime)
{
    if (d) {
        d->m_cpu_time = lluCpuTime;
    }
}

QString Process::getStatus() const
{
    return d->m_ustatus;
}

void Process::setStatus(QString uStatus)
{
    if (d) {
        d->m_ustatus = uStatus;
    }
}

unsigned Process::getFrequency() const
{
    return d->m_frequency;
}

void Process::setFrequency(unsigned uFreq)
{
    if (d) {
        d->m_frequency = uFreq;
    }
}

QString Process::getProcTooltip() const
{
    return d->m_tooltip;
}

void Process::setProcTooltip(QString strTooltip)
{
    if (d) {
        d->m_tooltip = strTooltip;
    }
}
QString Process::getProcArgments() const
{
    return d->m_arguments;
}

void Process::setProcArgments(QString strArgments)
{
    if (d) {
        d->m_arguments = strArgments;
    }
}

QDateTime Process::getPreDiskIoTime() const
{
    return d->m_preDiskIoTime;
}

void Process::setPreDiskIoTime(QDateTime dt)
{
    if (d) {
        d->m_preDiskIoTime = dt;
    }
}

qint64 Process::getPreDiskIoCount() const
{
    return d->m_preDiskIoCount;
}

void Process::setPreDiskIoCount(qint64 nCount)
{
    if (d) {
        nCount = nCount < 0 ? 0 : nCount;
        d->m_preDiskIoCount = nCount;
    }
}

qint64 Process::getMergePreDiskIoCount() const
{
    return d->m_mergePreDiskIoCount;
}

void Process::setMergePreDiskIoCount(qint64 nCount)
{
    if (d) {
        nCount = nCount < 0 ? 0 : nCount;
        d->m_mergePreDiskIoCount = nCount;
    }
}

QDateTime Process::getPreFlownetTime() const
{
    return d->m_preFlownetTime;
}

void Process::setPreFlownetTime(QDateTime dt)
{
    if (d) {
        d->m_preFlownetTime = dt;
    }
}

qint64 Process::getPreFlownetCount() const
{
    return d->m_preFlownetCount;
}

void Process::setPreFlownetCount(qint64 nCount)
{
    if (d) {
        nCount = nCount < 0 ? 0 : nCount;
        d->m_preFlownetCount = nCount;
    }
}

unsigned Process::getProcThreads() const
{
    return d->m_uThreads;
}

void Process::setProcThreads(unsigned uThreads)
{
    if (d) {
        uThreads = uThreads == 0 ? 1 : uThreads;
        d->m_uThreads = uThreads;
    }
}

QString Process::calcDiskIoPerSec(qint64 nNewCount, bool isMerge)
{
    qint64 bandwith;
    if (!isMerge) {
        bandwith = nNewCount - getPreDiskIoCount();
    } else {
        bandwith = nNewCount - getMergePreDiskIoCount();
    }
    QDateTime now = QDateTime::currentDateTime();
    quint64 ms_lapse = d->m_preDiskIoTime.msecsTo(now);
    QString speedPerSec = "";

    if (ms_lapse < 1000)
        ms_lapse = 1000; // prevent division by 0 ;-)
    qint64 speed = bandwith * 1000 / ms_lapse;

    char data[128] = {0};
    snprintf(data, sizeof(data), "%lld", speed);
    char result_data[128] = {0};
    int m_speed = 0;

    if (speed == 0 || speed < 0) {
        speedPerSec = "0 KB/s";
    } else if (speed < 1900) {
        speedPerSec.setNum(speed);
        speedPerSec.append(" B/s");
    } else if (speed < 1900000) {
        if (kdkVolumeBaseCharacterConvert(data, KDK_KILOBYTE, result_data) == KDK_NOERR) {
            sscanf(result_data, "%d", &m_speed);
            speedPerSec.setNum(m_speed);
        } else {
            speedPerSec.setNum(speed/1024);
        }
        speedPerSec.append(" KB/s");
    } else if (speed < 1900000000) {
        if (kdkVolumeBaseCharacterConvert(data, KDK_MEGABYTE, result_data) == KDK_NOERR) {
            sscanf(result_data, "%d", &m_speed);
            speedPerSec.setNum(m_speed);
        } else {
            speedPerSec.setNum(speed/(1024*1024));
        }
        speedPerSec.append(" MB/s");
    } else {
        if (kdkVolumeBaseCharacterConvert(data, KDK_GIGABYTE, result_data) == KDK_NOERR) {
            sscanf(result_data, "%d", &m_speed);
            speedPerSec.setNum(m_speed);
        } else {
            speedPerSec.setNum(speed/(1024*1024*1024));
        }
        speedPerSec.append(" GB/s");
    }

    d->m_preDiskIoTime.swap(now);
    if (!isMerge) {
        setPreDiskIoCount(nNewCount);
    } else {
        setMergePreDiskIoCount(nNewCount);
    }

    return speedPerSec;
}

QString Process::calcFlownetPerSec(double lfValue, bool isSpeed)
{
    if (!isSpeed) {
        qint64 bandwith = lfValue - getPreFlownetCount();
        QDateTime now = QDateTime::currentDateTime();
        quint64 ms_lapse = d->m_preFlownetTime.msecsTo(now);

        if (ms_lapse < 1000)
            ms_lapse = 1000; // prevent division by 0 ;-)
        qint64 speed = bandwith * 1000 / ms_lapse;
        QString speedPerSec = "";

        char data[128] = {0};
        snprintf(data, sizeof(data), "%lld",speed);
        char result_data[128] = {0};
        int m_speed = 0;

        if (speed == 0 || speed < 0) {
            speedPerSec = "0 KB/s"; 
        } else if (speed < 1900) {
            speedPerSec.setNum(speed);
            speedPerSec.append(" B/s");
        } else if (speed < 1900000) {
            if (kdkVolumeBaseCharacterConvert(data, KDK_KILOBYTE, result_data) == KDK_NOERR) {
                sscanf(result_data, "%d", &m_speed);
                speedPerSec.setNum(m_speed);
            } else {
                speedPerSec.setNum(speed/1024);
            }
            speedPerSec.append(" KB/s");
        } else if (speed < 1900000000) {
            if (kdkVolumeBaseCharacterConvert(data, KDK_MEGABYTE, result_data) == KDK_NOERR) {
                sscanf(result_data, "%d", &m_speed);
                speedPerSec.setNum(m_speed);
            } else {
                speedPerSec.setNum(speed/(1024*1024));
            }
            speedPerSec.append(" MB/s"); 
        } else {
            if (kdkVolumeBaseCharacterConvert(data, KDK_GIGABYTE, result_data) == KDK_NOERR) {
                sscanf(result_data, "%d", &m_speed);
                speedPerSec.setNum(m_speed);
            } else {
                speedPerSec.setNum(speed/(1024*1024*1024));
            }
            speedPerSec.append(" GB/s");
        }

        d->m_preFlownetTime.swap(now);
        setPreFlownetCount(lfValue);
        return speedPerSec;
    } else {
        QString speedPerSec = "";
        char data[128] = {0};
        snprintf(data,sizeof(data),"%fK",lfValue);
        char result_data[128] = {0};
        double m_speed = 0;

        if (lfValue < 0)
            lfValue = 0;
        if (lfValue == 0 || lfValue < 0) {
            speedPerSec = "0 KB/s"; 
        } else if (lfValue < 1024) {
            speedPerSec.setNum(lfValue, 'f', 1);
            speedPerSec.append(" KB/s"); 
        } else if (lfValue < 1024 * 1024) {
            if (kdkVolumeBaseCharacterConvert(data, KDK_MEGABYTE, result_data) == KDK_NOERR) {
                sscanf(result_data, "%lf", &m_speed);
                speedPerSec.setNum(m_speed, 'f', 1);
            } else {
                speedPerSec.setNum(lfValue/1024, 'f', 1);
            }
            speedPerSec.append(" MB/s"); 
        } else {
            if (kdkVolumeBaseCharacterConvert(data, KDK_GIGABYTE, result_data) == KDK_NOERR) {
                sscanf(result_data, "%lf", &m_speed);
                speedPerSec.setNum(m_speed, 'f', 1);
            } else {
                speedPerSec.setNum(lfValue/(1024*1024), 'f', 1);
            }
            speedPerSec.append(" GB/s");
        }
        return speedPerSec;
    }
}

void Process::updateProcUser(unsigned uUid)
{
    if (!d || d->m_uid == uUid)
        return;
    d->m_uid = uUid;
    // getuser
    struct passwd* pwd;
    pwd = getpwuid(uUid);

    if (pwd && pwd->pw_name)
        setProcUser(pwd->pw_name);
    else {
        QString username = QString::number(uUid);
        setProcUser(username);
    }
}

ProcessList::ProcessList(QObject* parent)
    : QObject(parent), m_set {}
{
    procNetThread = new ProcessNetwork(this);
    procNetThread->start(QThread::LowPriority);

    m_isExistEfficiencyDbus =  checkEfficiencyMode();
    m_pProcessManagerDbus = new QDBusInterface("com.kylin.ProcessManagerDaemon",
                                               "/com/kylin/ProcessManagerDaemon/EfficiencyMode",
                                               "com.kylin.ProcessManagerDaemon.EfficiencyMode",
                                               QDBusConnection::systemBus(),
                                               this);
    if (m_isExistEfficiencyDbus) {
        QDBusReply<QDBusArgument> reply = m_pProcessManagerDbus->call("ListProcessModes");
        if (reply.isValid()) {
            // 解包并使用
            const QDBusArgument arg = reply.value();
            arg.beginArray();
            while (!arg.atEnd()) {
                arg.beginMapEntry();
                int key;
                QString value;
                arg >> key >> value;
                arg.endMapEntry();
                m_processStateMap.insert(key, value);
            }
            arg.endArray();
        } else {
            qDebug() << "ListProcessModes Error:" << reply.error().message();
        }
    }

    connect(procNetThread, SIGNAL(procDetected(const QString &, double , double , int , unsigned int , const QString&)),
             this, SLOT(refreshLine(const QString &, double , double , int, unsigned int , const QString&)));

    // 获取shell命令列表
    [ = ] {
        FILE *fp;
        fp = fopen("/etc/shells", "r");
        if (fp)
        {
            char buf[128] {};
            char *s;
            while ((s = fgets(buf, 128, fp))) {
                if (s[0] == '/') {
                    auto sh = QLatin1String(basename(s));
                    if (sh.endsWith('\n'))
                        sh.chop(1);
                    if (!m_shellList.contains(sh)) {
                        m_shellList << sh;
                    }
                }
            }
        }
    }();
    // 获取script命令列表
    m_scriptingList << "/usr/bin/python";
    m_scriptingList << "/usr/bin/perl";
    m_scriptingList << "/usr/bin/php";
    m_scriptingList << "/usr/bin/python3";
    // 获取环境paths
    auto binPaths = qgetenv("PATH");
    auto pathList = binPaths.split(':');
    if (pathList.size() > 0) {
        for (auto p : pathList) {
            m_envPathList << p;
        }
    } else {
        // use default path
        m_envPathList << "/usr/bin";
    }

    m_procWndInfos = new ProcessWndInfo(this);
    // get default icon from theme
    m_defaultIcon = QIcon::fromTheme("application-x-executable");
}

ProcessList::~ProcessList()
{
    if(procNetThread) {
        procNetThread->stop();
        procNetThread->wait();
    }
}

void ProcessList::connectNetStateRefresh()
{
    connect(procNetThread, SIGNAL(procDetected(const QString &, double , double , int , unsigned int , const QString&)),
             this, SLOT(refreshLine(const QString &, double , double , int, unsigned int , const QString&)));
}

void ProcessList::disconnectNetStateRefresh()
{
    disconnect(procNetThread, SIGNAL(procDetected(const QString &, double , double , int , unsigned int , const QString&)),
             this, SLOT(refreshLine(const QString &, double , double , int, unsigned int , const QString&)));
}

bool ProcessList::containsById(pid_t pid)
{
    bool bContainsObj = false;
    bContainsObj = m_set.contains(pid);
    return bContainsObj;
}

void ProcessList::setScanFilter(int filterIndex)
{
    m_filterType = filterIndex;
}

void ProcessList::onClearAllProcess()
{
    m_set.clear();
}

void ProcessList::refresh()
{
    scanProcess();
}

void ProcessList::mergeSubProcNetIO(pid_t ppid, qreal &mergeBps)
{
    auto it = m_pidPtoCMapping.find(ppid);
    while (it != m_pidPtoCMapping.end() && it.key() == ppid) {
        mergeSubProcNetIO(it.value(), mergeBps);
        ++it;
    }
    const Process &proc = m_allSet[ppid];
//    qDebug() << "ppid = " << ppid << " Bps of ppid = " << proc.getFlowNet();
    mergeBps += proc.getFlowNet();
}

void ProcessList::mergeSubProcCpu(pid_t ppid, qreal &cpu)
{
    auto it = m_pidPtoCMapping.find(ppid);
    while (it != m_pidPtoCMapping.end() && it.key() == ppid) {
        mergeSubProcCpu(it.value(), cpu);
        ++it;
    }

    const Process &proc = m_allSet[ppid];
//    qDebug() << "ppid = " << ppid << "cpu of ppid = " << proc.getCpuPercent();
    cpu += proc.getCpuPercent();
}

void ProcessList::mergeSubProcDiskIO(pid_t ppid, qreal &diskIO, qreal &mergePreCount)
{
    auto it = m_pidPtoCMapping.find(ppid);
    while (it != m_pidPtoCMapping.end() && it.key() == ppid) {
        mergeSubProcDiskIO(it.value(), diskIO, mergePreCount);
        ++it;
    }

    const Process &proc = m_allSet[ppid];
//    qDebug() << "ppid = " << ppid << "disk of ppid = " << proc.getDiskIO() << proc.getPreDiskIoCount();
    diskIO += proc.getDiskIO() + proc.getPreDiskIoCount();
    mergePreCount += proc.getPreDiskIoCount();
}

void ProcessList::mergeSubProcMemery(pid_t ppid, qreal &memery)
{
    auto it = m_pidPtoCMapping.find(ppid);
    while (it != m_pidPtoCMapping.end() && it.key() == ppid) {
        mergeSubProcMemery(it.value(), memery);
        ++it;
    }

    const Process &proc = m_allSet[ppid];
//    qDebug() << "ppid = " << ppid << "memery of ppid = " << proc.getMemeryUsed();
    memery += proc.getMemeryUsed();
}

// 新增：批量读取所有进程数据（核心优化）
QHash<pid_t, ProcRawStat> ProcessList::batchReadAllProcData()
{
    QHash<pid_t, ProcRawStat> result;

    // 先读取CPU频率（只读一次，应用到所有进程）
    unsigned int cpuFreq = 0;
    readCpuInfo(cpuFreq);

    DIR *dir = opendir("/proc");
    if (!dir) return result;

    struct dirent *entry;
    char path[64];

    while ((entry = readdir(dir)) != nullptr) {
        // 快速过滤：只处理数字目录
        if (entry->d_type != DT_DIR) {
            continue;
        }
        char *endptr;
        pid_t pid = strtol(entry->d_name, &endptr, 10);
        if (*endptr != '\0') {
            continue;
        }

        snprintf(path, sizeof(path), "/proc/%d", pid);
        int dirfd = open(path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
        if (dirfd < 0) {
            continue;
        }

        ProcRawStat stat;
        stat.pid = pid;
        stat.cpuFreq = cpuFreq;

        // 使用 openat 批量读取，减少路径解析
        readProcStat(dirfd, stat);      // stat: CPU, state, ppid, nice, threads
        readProcStatus(dirfd, stat);    // status: uid
        readProcIo(dirfd, stat);        // io: read_bytes, write_bytes
        readProcCmdline(dirfd, stat);   // cmdline
        readProcStatm(dirfd, stat);     // statm: rss

        QStringList args = stat.cmdline.split(" ");
        for (int i = 0; i < qMin(2, args.size()); ++i) {
            // 提取文件名（去掉路径）
            QString basename = args[i].mid(args[i].lastIndexOf('/') + 1);
            // 检查是否以 cmd 开头（处理 prctl 修改过 cmd 的情况）
            if (basename.startsWith(stat.name)) {
                stat.name = basename;
                break;
            }
        }

        close(dirfd);
        result.insert(pid, stat);
    }

    closedir(dir);
    return result;
}

// 读取CPU频率（与readProcStat等函数风格一致）
bool ProcessList::readCpuInfo(unsigned int &cpuFreq)
{
    int fd = open("/proc/cpuinfo", O_RDONLY | O_CLOEXEC);
    if (fd < 0) return false;

    ssize_t n = read(fd, g_readBuf, sizeof(g_readBuf) - 1);
    close(fd);
    if (n <= 0) return false;
    g_readBuf[n] = '\0';

    const char *p = strstr(g_readBuf, "cpu MHz");
    if (!p) return false;

    p = strchr(p, ':');
    if (!p) return false;

    cpuFreq = static_cast<unsigned int>(strtod(p + 1, nullptr));
    return cpuFreq > 0;
}

void ProcessList::readProcStat(int dirfd, ProcRawStat &stat)
{
    int fd = openat(dirfd, "stat", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return;
    }

    ssize_t n = read(fd, g_readBuf, sizeof(g_readBuf) - 1);
    close(fd);
    if (n <= 0) {
        return;
    }
    g_readBuf[n] = '\0';

    // 找到最后一个 ')'，comm 在其中
    char *last_paren = strrchr(g_readBuf, ')');
    if (!last_paren) {
        return;
    }

    // 提取 comm
    char *first_paren = strchr(g_readBuf, '(');
    if (!first_paren || first_paren > last_paren) {
        return;
    }

    size_t comm_len = last_paren - first_paren - 1;
    if (comm_len > 255) {
        comm_len = 255;
    }

    char comm[256];
    strncpy(comm, first_paren + 1, comm_len);
    comm[comm_len] = '\0';
    stat.name = QString::fromLocal8Bit(comm);

    // 从 ')' 后面开始分割字段
    // 格式: ") state ppid pgrp session tty_nr tpgid flags minflt cminflt majflt cmajflt utime stime ..."
    char *field_str = last_paren + 2;  // 跳过 ") "

    // 使用 strtok 分割，得到所有字段数组
    // 字段 0=state, 1=ppid, 2=pgrp, 3=session, 4=tty_nr, 5=tpgid,
    //       6=flags, 7=minflt, 8=cminflt, 9=majflt, 10=cmajflt, 11=utime, 12=stime, ...
    char *fields[30];
    int field_count = 0;

    char *token = strtok(field_str, " ");
    while (token && field_count < 30) {
        fields[field_count++] = token;
        token = strtok(nullptr, " ");
    }

    if (field_count < 13) {
        qDebug() << "Not enough fields:" << field_count << "for pid" << stat.pid;
        return;
    }

    // 直接按索引取值（最可靠）
    stat.state = fields[0][0];
    stat.ppid = atoi(fields[1]);
    // fields[2]=pgrp, [3]=session, [4]=tty_nr, [5]=tpgid, [6]=flags
    // fields[7]=minflt, [8]=cminflt, [9]=majflt, [10]=cmajflt
    stat.utime = strtoull(fields[11], nullptr, 10);   // 第14个字段，索引11
    stat.stime = strtoull(fields[12], nullptr, 10);   // 第15个字段，索引12

    if (field_count > 16) {
        stat.nice = atoi(fields[16]);                  // 第19个字段，索引16
    }
    if (field_count > 17) {
        stat.threads = (unsigned int)atoi(fields[17]); // 第20个字段，索引17
    }
    if (field_count > 19) {
        stat.startTime = strtoul(fields[19], nullptr, 10); // 第22个字段，索引19
    }

    // 调试：打印关键字段验证
    // qDebug() << "PID" << stat.pid << "utime:" << stat.utime << "stime:" << stat.stime
    //          << "fields:" << field_count;
}

void ProcessList::readProcStatus(int dirfd, ProcRawStat &stat)
{
    int fd = openat(dirfd, "status", O_RDONLY | O_CLOEXEC);
    if (fd < 0) return;

    ssize_t n = read(fd, g_readBuf, sizeof(g_readBuf) - 1);
    close(fd);
    if (n <= 0) {
        return;
    }
    g_readBuf[n] = '\0';

    // 只找 Uid: 行
    const char *p = g_readBuf;
    while (*p) {
        if (strncmp(p, "Uid:\t", 5) == 0) {
            p += 5;
            stat.uid = 0;
            while (*p >= '0' && *p <= '9') {
                stat.uid = stat.uid * 10 + (*p - '0');
                p++;
            }
            return;
        }
        while (*p && *p != '\n') {
            p++;
        }
        if (*p == '\n') {
            p++;
        }
    }
}

void ProcessList::readProcIo(int dirfd, ProcRawStat &stat)
{
    int fd = openat(dirfd, "io", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return;
    }

    ssize_t n = read(fd, g_readBuf, sizeof(g_readBuf) - 1);
    close(fd);
    if (n <= 0) {
        return;
    }
    g_readBuf[n] = '\0';

    const char *p = g_readBuf;
    while (*p) {
        if (strncmp(p, "read_bytes:", 11) == 0) {
            p += 11;
            while (*p == ' ' || *p == '\t') {
                p++;
            }
            stat.ioRead = strtoull(p, nullptr, 10);
        } else if (strncmp(p, "write_bytes:", 12) == 0) {
            p += 12;
            while (*p == ' ' || *p == '\t') {
                p++;
            }
            stat.ioWrite = strtoull(p, nullptr, 10);
        }
        while (*p && *p != '\n') {
            p++;
        }
        if (*p == '\n') {
            p++;
        }
    }
}

void ProcessList::readProcCmdline(int dirfd, ProcRawStat &stat)
{
    int fd = openat(dirfd, "cmdline", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return;
    }

    ssize_t n = read(fd, g_readBuf, sizeof(g_readBuf) - 1);
    close(fd);
    if (n <= 0) {
        return;
    }

    // 将 \0 替换为空格，构造命令行字符串
    for (ssize_t i = 0; i < n - 1; i++) {
        if (g_readBuf[i] == '\0') g_readBuf[i] = ' ';
    }
    g_readBuf[n] = '\0';

    stat.cmdline = QString::fromLocal8Bit(g_readBuf);
}

void ProcessList::readProcStatm(int dirfd, ProcRawStat &stat)
{
    int fd = openat(dirfd, "statm", O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        return;
    }

    ssize_t n = read(fd, g_readBuf, sizeof(g_readBuf) - 1);
    close(fd);
    if (n <= 0) {
        return;
    }
    g_readBuf[n] = '\0';

    // 格式: size resident shared text lib data dt
    // resident 是第2个字段（索引1）
    long resident = 0;
    sscanf(g_readBuf, "%*ld %ld", &resident);
    stat.rss = resident * sysconf(_SC_PAGESIZE);
}

// 从原始数据填充 Process 对象
void ProcessList::fillProcessFromRaw(Process &proc, const ProcRawStat &raw)
{
    proc.setProcName(raw.name);
    proc.setCmd(raw.name);  // 兼容原有逻辑
    proc.setStatus(QString(raw.state));
    proc.setNice(raw.nice);
    proc.setSTime(raw.stime);
    proc.setUTime(raw.utime);
    proc.setProcStartTime(raw.startTime);
    proc.setProcThreads(raw.threads);
    proc.setUid(raw.uid);
    proc.setFrequency(raw.cpuFreq);

    // 获取用户名
    struct passwd* pwd = getpwuid(raw.uid);
    proc.setProcUser(pwd && pwd->pw_name ? QString(pwd->pw_name) : QString::number(raw.uid));

    // 构建 tooltip
    QString tooltip = raw.cmdline.isEmpty() ? raw.name : raw.cmdline;
    proc.setProcTooltip(tooltip.toHtmlEscaped());
    proc.setProcArgments(tooltip);

    // systemd 信息（可选，保持原有逻辑）
    // 注意：sd_pid_get_unit 等调用也有开销，可考虑缓存
//    proc.get_process_systemd_info();
}

// 优化的 scanProcess
void ProcessList::scanProcess()
{
    if (m_isScanStoped) {
        return;
    }

    uid_t curUID = getuid();
    unsigned long long lluCurTick = QDateTime::currentMSecsSinceEpoch();
    bool bQuickUpdate = m_lluLastUpdateTick > 0 && (lluCurTick - m_lluLastUpdateTick) < 2500;
    if (!bQuickUpdate) {
        m_lluLastUpdateTick = lluCurTick;
    }

    // ========== 1. 批量读取所有进程数据（单次遍历 /proc） ==========
    QHash<pid_t, ProcRawStat> rawData = batchReadAllProcData();

    // 构建有效 PID 集合和父子关系
    QSet<pid_t> validPids;
    validPids.reserve(rawData.size());
    m_procCtoPMap.clear();
    m_pidPtoCMapping.clear();

    for (auto it = rawData.begin(); it != rawData.end(); ++it) {
        pid_t pid = it.key();
        const ProcRawStat &stat = it.value();
        validPids.insert(pid);
        m_procCtoPMap.insert(pid, stat.ppid);
        m_pidPtoCMapping.insert(stat.ppid, pid);
    }

    // ========== 2. 更新窗口信息 ==========
    m_procWndInfos->updateWindowInfos();
    m_listGuiPids = m_procWndInfos->getWindowPids();

    // ========== 3. 快速清理无效进程（O(1) 查找） ==========
    QList<pid_t> appList;
    QMutableMapIterator<pid_t, Process> itSet(m_set);
    while (itSet.hasNext()) {
        itSet.next();
        pid_t pid = itSet.key();
        if (!validPids.contains(pid)) {
            itSet.remove();
            m_allSet.remove(pid);
            m_mapCpuTimes.remove(pid);
            continue;
        }
        if (m_filterType == PROCESSTYPE_APPLICATION && m_listGuiPids.contains(pid)) {
            appList.append(pid);
        }
    }

    // ========== 4. 获取 CPU 总量 ==========
    CpuUsageData cpu = cpuInfo::getTotalCpuUsageData();
    unsigned long long lluCpuTotal = cpu.total();
    this->cpu_total_time = MAX(lluCpuTotal - this->cpu_total_time_last, 1);
    this->cpu_total_time_last = lluCpuTotal;

    // ========== 5. 单次遍历处理所有进程 ==========
    int allProcCount = 0, appProcCount = 0, userProcCount = 0;

    for (auto it = rawData.begin(); it != rawData.end(); ++it) {
        pid_t pidCur = it.key();
        const ProcRawStat &raw = it.value();

        // 跳过僵尸进程
        if (raw.state == 'Z') {
            continue;
        }

        allProcCount++;

        // 统计
        if (m_listGuiPids.contains(pidCur) && !g_appFilterList.contains(raw.name)) {
            appProcCount++;
        }
        if (raw.uid == curUID) {
            userProcCount++;
        }

        // 创建 Process 对象
        Process proc(pidCur);
        proc.setValid(true);

        // 从原始数据填充（避免重复读取 /proc）
        fillProcessFromRaw(proc, raw);

        // 获取旧数据
        Process oldProc = m_allSet.value(pidCur);

        // 处理图标和显示名（带缓存）
        if (!oldProc.isValid() || raw.name != oldProc.getProcName()) {
            // 新进程或名称变更，查询桌面信息
            QString icon = DesktopFileInfo::instance()->getIconByExec(raw.name);
            if (icon.isEmpty()) {
                icon = DesktopFileInfo::instance()->getAndroidAppIconByCmd(raw.cmdline);
            }
            proc.setIconPath(icon);

            QString title = DesktopFileInfo::instance()->getNameByExec(raw.name);
            if (title.isEmpty()) {
                title = DesktopFileInfo::instance()->getAndroidAppNameByCmd(raw.cmdline);
            }
            if (title.isEmpty()) {
                title = getWndNameByPid(pidCur);
            }
            proc.setDisplayName(title.isEmpty() ? raw.name : title);          
        } else {
            proc.setIconPath(oldProc.getIconPath());
            proc.setDisplayName(oldProc.getDisplayName());
        }

        if (!oldProc.isValid()) {
            if (m_isExistEfficiencyDbus) {
                if (m_set.isEmpty()) {
                    if (m_processStateMap.contains(proc.pid())) {
                        proc.setProcState(m_processStateMap.value(proc.pid()));
                    }
                } else {
                    QDBusReply<QString> mode = m_pProcessManagerDbus->call("GetProcessMode", proc.pid());
                    proc.setProcState(mode);
                }
            }
        } else {
            proc.setProcState(oldProc.getProcState());
        }

        // 计算磁盘 IO
        unsigned long long diskTotal = raw.ioRead + raw.ioWrite;
        if (!oldProc.isValid()) {
            proc.setPreDiskIoTime(QDateTime::currentDateTime());
            proc.setPreDiskIoCount(diskTotal);
            proc.setDiskIO(0);
        } else {
            proc.setPreDiskIoTime(oldProc.getPreDiskIoTime());
            proc.setPreDiskIoCount(oldProc.getPreDiskIoCount());
            proc.setDiskIO(diskTotal - oldProc.getPreDiskIoCount());
        }
        proc.setDiskIODesc(proc.calcDiskIoPerSec(diskTotal));

        // 设置状态
        proc.setProcStatus(formatProcessState(QString(raw.state)));

        // 计算 CPU
        unsigned long long procCpuTime = raw.utime + raw.stime;
        if (m_mapCpuTimes.contains(pidCur)) {
            unsigned long long oldTime = m_mapCpuTimes[pidCur];
            if (bQuickUpdate && oldProc.isValid()) {
                proc.setCpuPercent(oldProc.getCpuPercent());
            } else {
                unsigned long long diff = procCpuTime - oldTime;
                if (diff > 0) {
                    proc.setStatus("R");
                }
                double percent = (double)diff * 100.0 / this->cpu_total_time;
                proc.setCpuPercent(MIN(percent, 100.0));
            }
        }
        proc.setProcCpuTime(procCpuTime);
        m_mapCpuTimes[pidCur] = procCpuTime;

        // 网络数据（保持原有逻辑）
        if (!oldProc.isValid()) {
            proc.setPreFlownetTime(QDateTime::currentDateTime());
            proc.setPreFlownetCount(0);
            proc.setFlowNet(0);
            proc.setFlowNetDesc(proc.calcFlownetPerSec(0));
        } else {
            proc.setPreFlownetTime(oldProc.getPreFlownetTime());
            proc.setPreFlownetCount(oldProc.getPreFlownetCount());
            proc.setFlowNet(oldProc.getFlowNet());
            proc.setFlowNetDesc(oldProc.getFlowNetDesc());
        }

        // 内存
        proc.setMemeryUsed(raw.rss);

        // 应用过滤
        bool shouldShow = true;
        if (m_filterType == PROCESSTYPE_APPLICATION) {
            shouldShow = m_listGuiPids.contains(pidCur) && !g_appFilterList.contains(raw.name);
        } else if (m_filterType == PROCESSTYPE_USER) {
            shouldShow = (raw.uid == curUID);
        }

        if (shouldShow) {
            m_set[pidCur] = proc;
        } else if (m_set.contains(pidCur)) {
            m_set.remove(pidCur);
        }

        m_allSet[pidCur] = proc;
    }

    Q_EMIT procCountUpdated(appProcCount, userProcCount, allProcCount);

    // ========== 6. 合并应用程序子进程数据 ==========
    for (const pid_t &pid : appList) {
        if (!m_set.contains(pid)) continue;

        qreal mergeBps = 0;
        mergeSubProcNetIO(pid, mergeBps);
        m_set[pid].setFlowNet(mergeBps);
        m_set[pid].setFlowNetDesc(m_set[pid].calcFlownetPerSec(mergeBps, true));

        qreal ptotalCpu = 0;
        mergeSubProcCpu(pid, ptotalCpu);
        m_set[pid].setCpuPercent(ptotalCpu);

        qreal mergeDiskIO = 0, mergePreCount = 0;
        mergeSubProcDiskIO(pid, mergeDiskIO, mergePreCount);
        m_set[pid].setMergePreDiskIoCount(mergePreCount);
        m_set[pid].setDiskIO(mergeDiskIO - mergePreCount);
        m_set[pid].setDiskIODesc(m_set[pid].calcDiskIoPerSec(mergeDiskIO, true));

        qreal mergeMemery = 0;
        mergeSubProcMemery(pid, mergeMemery);
        m_set[pid].setMemeryUsed(mergeMemery);
    }
}

Process ProcessList::getProcessById(pid_t pid)
{
    Process procInfo(pid);
    if (m_set.contains(pid)) {
        procInfo = m_set[pid];
    }
    return procInfo;
}

Process ProcessList::getProcessByAllSetId(pid_t pid)
{
    Process procInfo(pid);
    if (m_allSet.contains(pid)) {
        procInfo = m_allSet[pid];
    }
    return procInfo;
}

QList<pid_t> ProcessList::getPIDList()
{
    QList<pid_t> pidList{};
    pidList.clear();
    int size = m_set.size();
    QMap<pid_t, Process>::key_iterator iterBegin = m_set.keyBegin();
    for (; iterBegin != m_set.keyEnd(); ++iterBegin) {
        pid_t tmpKey = *iterBegin;
        pidList.append(tmpKey);
        if (size != m_set.size()) {
            break;
        }
    }
    return pidList;
}

void ProcessList::removeProcess(pid_t pid)
{
    m_set.remove(pid);
}

void ProcessList::updateProcessState(pid_t pid, QString& state)
{
    if (m_set.contains(pid))
        m_set[pid].setProcStatus(state);
}

void ProcessList::updateProcessEfficiencyState(pid_t pid, QString state)
{
    if (m_set.contains(pid)) {
        m_set[pid].setProcState(state);
    }

    if (m_allSet.contains(pid)) {
        m_allSet[pid].setProcState(state);
    }
}

void ProcessList::updateProcessPriority(pid_t pid, qulonglong priority)
{
    if (m_set.contains(pid))
        m_set[pid].setNice(priority);
}

void ProcessList::stopScanProcess()
{
    m_isScanStoped = true;
}

void ProcessList::startScanProcess()
{
    m_isScanStoped = false;
}

void ProcessList::refreshLine(const QString &procname, double rcv, double sent, int pid, unsigned int uid, const QString &devname)
{
    Q_UNUSED(procname);
    Q_UNUSED(uid);
    Q_UNUSED(devname);
    // record process flownet
    #if 0
    qint64 tmptotalFlowNetPerSec = rcv + sent;
    if (m_set.contains(pid) && m_set[pid].isValid()) {
        if (tmptotalFlowNetPerSec < m_set[pid].getPreFlownetCount()) {
            m_set[pid].setPreFlownetCount(tmptotalFlowNetPerSec);
            m_set[pid].setFlowNet(tmptotalFlowNetPerSec);
        } else {
            m_set[pid].setFlowNet(tmptotalFlowNetPerSec-m_set[pid].getPreFlownetCount());
        }
        QString addFlownetPerSec = m_set[pid].calcFlownetPerSec(tmptotalFlowNetPerSec);
        m_set[pid].setFlowNetDesc(addFlownetPerSec);
    }
    #else
    double tmptotalFlowNetPerSec = rcv + sent;
    if (m_set.contains(pid) && m_set[pid].isValid()) {
        m_set[pid].setPreFlownetCount(tmptotalFlowNetPerSec);
        m_set[pid].setFlowNet(tmptotalFlowNetPerSec);
        QString addFlownetPerSec = m_set[pid].calcFlownetPerSec(tmptotalFlowNetPerSec, true);
        m_set[pid].setFlowNetDesc(addFlownetPerSec);
    }
    if (m_allSet.contains(pid)) {
        m_allSet[pid].setPreFlownetCount(tmptotalFlowNetPerSec);
        m_allSet[pid].setFlowNet(tmptotalFlowNetPerSec);
        QString addFlownetPerSec = m_allSet[pid].calcFlownetPerSec(tmptotalFlowNetPerSec, true);
        m_allSet[pid].setFlowNetDesc(addFlownetPerSec);
        qDebug() << "refresh pid flownet = " << m_allSet[pid].getFlowNet()  << "pid = " << pid;
    }
//    qDebug() << "m_allSet.contains(pid) = " << m_allSet.contains(pid)  << "pid = " << pid << "tmptotalFlowNet = " << tmptotalFlowNetPerSec;
    #endif
}

bool ProcessList::isShellCmd(QString strCmd)
{
    if (strCmd.isEmpty())
        return false;
    return m_shellList.contains(strCmd);
}

QIcon ProcessList::getWndIconByPid(pid_t pid)
{
    QIcon wndIcon = m_procWndInfos->getWndIconByPid(pid);
    if (!wndIcon.isNull()) {
        return wndIcon;
    }
    if (m_procCtoPMap.contains(pid)) {
        return getWndIconByPid(m_procCtoPMap[pid]);
    }
    return wndIcon;
}

QString ProcessList::getWndNameByPid(pid_t pid)
{
    return m_procWndInfos->getWndNameByPid(pid);
}

QPixmap ProcessList::getProcIcon(pid_t pid, QString& strIcon, int nSize)
{
    Process proc = getProcessById(pid);
    QString strIconPath = proc.getIconPath();
    QPixmap iconPixmap;
    QIcon icon;
    if (!strIconPath.isEmpty()) {
        if (strIconPath.contains("/")) {
            QFileInfo fileInfo(strIconPath);
            if (fileInfo.exists()) {
                icon = QIcon(strIconPath);
            }
        } else {
            icon = QIcon::fromTheme(strIconPath);
        }
    }
//    if (icon.isNull() && !proc.getProcName().isEmpty()) {
//        QStringList procNameList = proc.getProcName().split(" ");
//        procNameList = procNameList[0].split("/");
//        qInfo()<<"IconName:"<<procNameList[0];
//        icon = QIcon::fromTheme(procNameList[0]);
//    }
    if (icon.isNull()) {
        if (isShellCmd(proc.getProcName())) {
            strIconPath = proc.getProcName();
            icon = QIcon::fromTheme("terminal");
        }
    }
    if (icon.isNull()) {
        QIcon iconWnd = getWndIconByPid(pid);
        if (!iconWnd.isNull()) {
            strIconPath = proc.getProcName();
            icon = iconWnd;
        }
    }
    if (icon.isNull()) {
        icon = m_defaultIcon;
        strIconPath = "application-x-executable";
    }
    strIcon = strIconPath;
    iconPixmap = icon.pixmap(nSize, nSize);
    return iconPixmap;
}

void ProcessList::forceQuickUpdate()
{
    m_lluLastUpdateTick = 0;
}

} // namespace process
} // namespace sysmonitor

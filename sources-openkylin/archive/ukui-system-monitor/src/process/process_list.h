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

#ifndef __PROCESS_LIST_H__
#define __PROCESS_LIST_H__

#include "process_data.h"
#include "process_network.h"
#include "processwndinfo.h"

#include <glib.h>
#include <QMap>
#include <QList>
#include <QIcon>
#include <QPixmap>
#include <QReadWriteLock>
#include <QDBusInterface>
#include <QDBusReply>
#include <dirent.h>


namespace sysmonitor {
namespace process {

enum FilterType { NoFilter,
                  FilterActiced = 1,
                  FilterCurrentUser = 2
                };

enum {
    PROCESSTYPE_ACTIVE,
    PROCESSTYPE_USER,
    PROCESSTYPE_ALL,
    PROCESSTYPE_APPLICATION
};

enum ProcessPriority {
    InvalidPriority = INT_MAX,
    VeryHighPriority = -20, // default veryhigh priority
    HighPriority = -10, // default high priority
    NormalPriority = 0, // normal priority
    LowPriority = 10, // default low priority
    VeryLowPriority = 19, // default verylow priority
    CustomPriority
};

struct ProcRawStat {
    pid_t pid = 0;
    pid_t ppid = 0;
    uid_t uid = 0;
    char state = 0;
    QString name;
    QString cmdline;
    unsigned long long utime = 0;
    unsigned long long stime = 0;
    unsigned long long ioRead = 0;
    unsigned long long ioWrite = 0;
    unsigned long long rss = 0;  // bytes
    int nice = 0;
    unsigned int threads = 0;
    unsigned long startTime = 0;
    unsigned int cpuFreq = 0;
};


static const int VeryHighPriorityMax = -20;
static const int VeryHighPriorityMin = -11;
static const int HighPriorityMax = -10;
static const int HighPriorityMin = -1;
static const int LowPriorityMax = 1;
static const int LowPriorityMin = 10;
static const int VeryLowPriorityMax = 11;
static const int VeryLowPriorityMin = 19;

ProcessPriority getProcessPriorityStub(int prio);

class Process
{
public:
    explicit Process();
    explicit Process(pid_t pid);
    Process(const Process &other);
    Process &operator=(const Process &rhs);
    ~Process();

    bool isValid() const;
    void setValid(bool isValid);

    pid_t pid() const;

    QString getCmd() const;
    void setCmd(QString cmd);

    int getUid() const;
    void setUid(int uid);
    unsigned long long getSTime() const;
    void setSTime(unsigned long long time);
    unsigned long long getUTime() const;
    void setUTime(unsigned long long time);
    QString getProcState() const;
    void setProcState(QString mode);

    double getCpuPercent() const;
    void setCpuPercent(double cpuPercent);

    qulonglong getMemeryUsed() const;
    void setMemeryUsed(qulonglong lMemUsed);

    int getNice() const;
    void setNice(int lNice);

    QString getFlowNetDesc() const;
    void setFlowNetDesc(QString strFlowNet);

    QString getDiskIODesc() const;
    void setDiskIODesc(QString strDiskIO);

    qint64 getFlowNet() const;
    void setFlowNet(qint64 llFlowNet);

    qint64 getDiskIO() const;
    void setDiskIO(qint64 llDiskIO);

    QString getIconPath() const;
    void setIconPath(QString iconPath);

    QString getProcName() const;
    void setProcName(QString strProcName);

    QString getDisplayName() const;
    void setDisplayName(QString strDisplayName);

    QString getProcPath() const;
    void setProcPath(QString strProcPath);

    QString getProcUser() const;
    void setProcUser(QString strProcUser);

    QString getProcStatus() const;
    void setProcStatus(QString strProcStatus);

    QString getProcSession() const;
    void setProcSession(QString strProcSession);

    QString getProcCpuDurationTime() const;
    void setProcCpuDurationTime(QString strProcCpuDurationTime);

    unsigned int getProcType() const;
    void addProcType(unsigned int uType);
    void clearProcType();

    QString getProcUnit() const;
    void setProcUnit(QString strUnit);

    QString getProcSeat() const;
    void setProcSeat(QString strSeat);

    unsigned long getProcStartTime() const;
    void setProcStartTime(unsigned long luStartTime);

    qulonglong getProcCpuTime() const;
    void setProcCpuTime(qulonglong lluCpuTime);

    QString getStatus() const;
    void setStatus(QString uStatus);

    unsigned getFrequency() const;
    void setFrequency(unsigned uFreq);

    QString getProcTooltip() const;
    void setProcTooltip(QString strTooltip);

    QString getProcArgments() const;
    void setProcArgments(QString strArgments);

    QDateTime getPreDiskIoTime() const;
    void setPreDiskIoTime(QDateTime dt);

    qint64 getPreDiskIoCount() const;
    void setPreDiskIoCount(qint64 nCount);

    qint64 getMergePreDiskIoCount() const;
    void setMergePreDiskIoCount(qint64 nCount);

    QDateTime getPreFlownetTime() const;
    void setPreFlownetTime(QDateTime dt);

    qint64 getPreFlownetCount() const;
    void setPreFlownetCount(qint64 nCount);

    unsigned getProcThreads() const;
    void setProcThreads(unsigned uThreads);

    void updateProcUser(unsigned uUid);

    QString calcDiskIoPerSec(qint64 nNewCount, bool isMerge = false);
    QString calcFlownetPerSec(double lfValue, bool isSpeed = false);

private:
    QSharedDataPointer<ProcessData> d;
};

class ProcessList : public QObject
{
    Q_OBJECT
public:
    explicit ProcessList(QObject* parent = nullptr);
    virtual ~ProcessList();

    Process getProcessById(pid_t pid);
    Process getProcessByAllSetId(pid_t pid);
    bool containsById(pid_t pid);
    QList<pid_t> getPIDList();
    void removeProcess(pid_t pid);
    void updateProcessState(pid_t pid, QString& state);
    void updateProcessEfficiencyState(pid_t pid, QString state);
    void updateProcessPriority(pid_t pid, qulonglong priority);
    void refresh();
    void setScanFilter(int filterIndex);
    void connectNetStateRefresh();
    void disconnectNetStateRefresh();
    void stopScanProcess();
    void startScanProcess();
    bool isShellCmd(QString strCmd);
    QIcon getWndIconByPid(pid_t pid);
    QString getWndNameByPid(pid_t pid);
    QPixmap getProcIcon(pid_t pid, QString& strIcon, int nSize);
    void forceQuickUpdate();

public slots:
    void refreshLine(const QString& procname, double rcv, double sent, int pid, unsigned int uid, const QString& devname);
    void onClearAllProcess();

Q_SIGNALS:
    void processListUpdated();
    void processEnded(pid_t pid);
    void processPaused(pid_t pid, char state);
    void processResumed(pid_t pid, char state);
    void processKilled(pid_t pid);
    void processPriorityChanged(pid_t pid, int priority);
    void priorityPromoteResultReady(const int &ec);
    void processControlResultReady(const int &ec);
    void filterTypeChanged(FilterType filter);
    void procCountUpdated(int appCount, int myCount, int AllCount);

private:
    void mergeSubProcNetIO(pid_t ppid, qreal &mergeBps);
    void mergeSubProcCpu(pid_t ppid, qreal &cpu);
    void mergeSubProcDiskIO(pid_t ppid, qreal &diskIO, qreal &mergePreCount);
    void mergeSubProcMemery(pid_t ppid, qreal &memery);

    void scanProcess();

    // 批量数据读取
    QHash<pid_t, ProcRawStat> batchReadAllProcData();
    bool readCpuInfo(unsigned int &cpuFreq);
    void readProcStat(int dirfd, ProcRawStat &stat);
    void readProcStatus(int dirfd, ProcRawStat &stat);
    void readProcIo(int dirfd, ProcRawStat &stat);
    void readProcCmdline(int dirfd, ProcRawStat &stat);
    void readProcStatm(int dirfd, ProcRawStat &stat);
    void fillProcessFromRaw(Process &proc, const ProcRawStat &raw);

private:
    QMap<pid_t, Process> m_set;
    QMap<pid_t, Process> m_allSet;
    QMap<pid_t, quint64> m_mapCpuTimes;
    ProcessNetwork *procNetThread = nullptr;

    guint64 cpu_total_time = 0;
    guint64 cpu_total_time_last = 0;
    QMap<int,QString> pidMap;

    long long int disk_io_bytes_total;
    int m_filterType = PROCESSTYPE_ALL;
    bool m_isScanStoped = false;
    QList<QString> m_shellList; // shell命令列表
    QList<QString> m_scriptingList; // script命令列表
    QList<QByteArray> m_envPathList; // path环境列表
    QMap<pid_t,pid_t> m_procCtoPMap; // 子进程对应父进程列表
    QMultiMap<pid_t, pid_t> m_pidPtoCMapping {}; // parent to child pid mapping
    QList<pid_t> m_listGuiPids; //带UI的进程id列表
    ProcessWndInfo *m_procWndInfos = nullptr;
    QIcon m_defaultIcon;
    unsigned long long m_lluLastUpdateTick = 0;
    QDBusInterface *m_pProcessManagerDbus = nullptr;
    bool m_isExistEfficiencyDbus = false;
    QMap<int, QString> m_processStateMap;
    bool m_isExistRunEnvDbus = false;
};

} // namespace process
} // namespace sysmonitor

#endif // __PROCESS_LIST_H__

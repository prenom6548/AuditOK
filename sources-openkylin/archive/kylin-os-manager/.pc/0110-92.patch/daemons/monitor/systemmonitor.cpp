#include <unistd.h>
#include <fcntl.h>
#include <glibtop.h>
#include <glibtop/mem.h>
#include <glibtop/swap.h>
#include <glibtop/proctime.h>
#include <glibtop/proclist.h>
#include <glibtop/procmem.h>
#include <glibtop/procargs.h>
#include <glibtop/procuid.h>
#include <glibtop/procstate.h>
#include <glibtop/sysinfo.h>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QTimer>
#include "systemmonitor.h"

#ifndef DEBUG_MODE
QString OUTPUT_FILE_PATH = "/var/log/kylin-os-manager/systemMonitor/";
#else
QString OUTPUT_FILE_PATH = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#endif

using namespace systemmonitor;

SystemMonitor *SystemMonitor::m_systemMonitor = nullptr;
SystemMonitor *SystemMonitor::getInstance()
{
    if (m_systemMonitor == nullptr) {
        m_systemMonitor = new SystemMonitor();
    }
    return m_systemMonitor;
}

QString SystemMonitor::getConf() const
{
    QString str;
    str += QString(CONFIG_DAEMON_SYSTEM_MONITOR_TIMER) + ":" + QString::number(*m_timer);
    str += "\n" + QString(CONFIG_DAEMON_SYSTEM_MONITOR_CPU) + ":" + QString::number(*m_cpuThreshold);
    str += "\n" + QString(CONFIG_DAEMON_SYSTEM_MONITOR_MEMORY) + ":" + QString::number(*m_memoryThreshold);
    return str;
}

void SystemMonitor::setTimer(int t)
{
    if (t < 1) {
        return;
    }
    *m_timer = t;
    m_setting->setValue(CONFIG_DAEMON_SYSTEM_MONITOR, CONFIG_DAEMON_SYSTEM_MONITOR_TIMER, t);
}

void SystemMonitor::setCpuThreshold(int c)
{
    if (c < 1) {
        return;
    }
    *m_cpuThreshold = c;
    m_setting->setValue(CONFIG_DAEMON_SYSTEM_MONITOR, CONFIG_DAEMON_SYSTEM_MONITOR_CPU, c);
}

void SystemMonitor::setMemoryThreshold(int m)
{
    if (m < 1) {
        return;
    }
    *m_memoryThreshold = m;
    m_setting->setValue(CONFIG_DAEMON_SYSTEM_MONITOR, CONFIG_DAEMON_SYSTEM_MONITOR_MEMORY, m);
}

void SystemMonitor::outputChange(bool b)
{
    m_setting->setValue(CONFIG_DAEMON_SYSTEM_MONITOR, CONFIG_DAEMON_SYSTEM_MONITOR_OUTPUT, b);
}

SystemMonitor::SystemMonitor()
{

    m_setting = new kom::Configure();
    m_timer = new int(0);
    m_cpuThreshold = new int(0);
    m_memoryThreshold = new int(0);
    *m_timer = m_setting->value(CONFIG_DAEMON_SYSTEM_MONITOR, CONFIG_DAEMON_SYSTEM_MONITOR_TIMER).toInt();
    if (*m_timer < 1) {
        *m_timer = 120;
    }
    *m_cpuThreshold = m_setting->value(CONFIG_DAEMON_SYSTEM_MONITOR, CONFIG_DAEMON_SYSTEM_MONITOR_CPU).toInt();
    if (*m_cpuThreshold < 1) {
        *m_cpuThreshold = 80;
    }
    *m_memoryThreshold = m_setting->value(CONFIG_DAEMON_SYSTEM_MONITOR, CONFIG_DAEMON_SYSTEM_MONITOR_MEMORY).toInt();
    if (*m_memoryThreshold < 1) {
        *m_memoryThreshold = 80;
    }
    m_output = m_setting->value(CONFIG_DAEMON_SYSTEM_MONITOR, CONFIG_DAEMON_SYSTEM_MONITOR_OUTPUT).toBool();
    Conf c(m_timer, m_cpuThreshold, m_memoryThreshold, m_output);
    m_monitor = new Monitor(c);
    connect(m_monitor, &Monitor::outputChange, this, &SystemMonitor::outputChange);
    m_monitor->start();
}

Monitor::Monitor(Conf c)
{
    if (!QDir().mkpath(OUTPUT_FILE_PATH)) {
        qDebug() << "creat path error:" << OUTPUT_FILE_PATH;
    }
    glibtop_init();
    m_timer = c.timer;
    m_cpuThreshold = c.cpuThreshold;
    m_memoryThreshold = c.memoryThreshold;
    m_output = c.output;
    m_cpunum = getCpuCount();
}

void Monitor::holdOn()
{
    sleep(1);
}

bool Monitor::PassDetection()
{
    //系统内存占用情况
    glibtop_mem memTotal;
    glibtop_get_mem(&memTotal);
    m_memProportion = double(memTotal.used) / double(memTotal.total) * 100.0;
    //系统cpu占用情况
    glibtop_cpu cpuTotalLast;
    glibtop_get_cpu(&cpuTotalLast);
    holdOn();
    glibtop_get_cpu(&m_cpuTotal);
    double cpuProportion = (1.0 - double(m_cpuTotal.idle - cpuTotalLast.idle) / double(m_cpuTotal.total - cpuTotalLast.total)) * 100;
    qDebug() << "men:" << m_memProportion << " cpu:" << cpuProportion;
    //未达到阈值则等待下一次循环
    if (m_memProportion < *m_memoryThreshold && cpuProportion < *m_cpuThreshold) {
        return true;
    }
    qDebug() << "检测到异常！";
    return false;
}

void Monitor::getProcInfo(MonitorInfoList *monitorInfoList, QMap<pid_t, quint64> *mapCpuTimes, pid_t pidCur, quint64 totalDifference)
{
    MonitorInfo info;
    info.mypid = pidCur;
    //获取进程状态
    glibtop_proc_state gpstate;
    glibtop_get_proc_state(&gpstate, pidCur);
    info.processor = gpstate.processor;
    info.state = gpstate.state;
    info.cmd = gpstate.cmd;
    //            //获取进程参数
    //            glibtop_proc_args arg;
    //            char **args = glibtop_get_proc_argv(&arg, pidCur, 0);
    //            if (args) {
    //                if (arg.size > 0) {
    //                    info.cmd = args[0];
    //                }
    //                g_strfreev(args);
    //                args = NULL;
    //            }
    //获取进程信息
    glibtop_proc_uid gpuid;
    glibtop_get_proc_uid(&gpuid, pidCur);
    info.ppid = gpuid.ppid;
    info.gid = gpuid.gid;
    info.nice = gpuid.nice;
    //获取进程内存占用
    glibtop_proc_mem gpmem;
    glibtop_get_proc_mem(&gpmem, pidCur);
    info.memory = gpmem.resident;
    //获取进程时间
    glibtop_proc_time proctime;
    glibtop_get_proc_time(&proctime, pidCur);
    info.start_time = proctime.start_time;
    //获取进程cpu占用
    quint64 lluProcessTime = proctime.stime + proctime.utime;
    quint64 lastProcessTime = mapCpuTimes->value(pidCur);
    quint64 difference = lluProcessTime - lastProcessTime;
    double cpu_scale = 100 * m_cpunum;
    double sPcpu = (double)difference * cpu_scale / double(totalDifference);
    sPcpu = MIN(sPcpu, cpu_scale);
    info.cpu = sPcpu;
    //放入队列
    monitorInfoList->append(info);
}

void Monitor::run()
{
    //系统内存占用情况
    glibtop_mem memTotal;
    glibtop_get_mem(&memTotal);
    m_memTotal = QString::number(memTotal.total / 1024 / 1024) + "M";

    QTimer timer;
    timer.setInterval(0);
    connect(&timer, &QTimer::timeout, [&]() {
        timer.setInterval(*m_timer * 1000);

        if (PassDetection()) {
            return;
        }

        QMap<pid_t, quint64> mapCpuTimes;
        //获取pid列表
        glibtop_proclist proclist;
        pid_t *pid_list = glibtop_get_proclist(&proclist, 0, 0);
        std::sort(pid_list, pid_list + proclist.number);
        //获取cpu占用基础值（对比用）
        for (quint64 i = 0; i < proclist.number; ++i) {
            pid_t pidCur = pid_list[i];
            glibtop_proc_time proctime;
            glibtop_get_proc_time(&proctime, pidCur);
            quint64 lluProcessTime = proctime.stime + proctime.utime;
            mapCpuTimes[pidCur] = lluProcessTime;
        }
        holdOn();
        //系统cpu占用情况
        glibtop_cpu cpuTotalLast = m_cpuTotal;
        glibtop_get_cpu(&m_cpuTotal);
        quint64 totalDifference = MAX(m_cpuTotal.total - cpuTotalLast.total, 1);
        double cpuProportion = (1.0 - double(m_cpuTotal.idle - cpuTotalLast.idle) / totalDifference) * 100;
        //获取各进程信息
        MonitorInfoList monitorInfoList;
        for (quint64 i = 0; i < proclist.number; ++i) {
            pid_t pidCur = pid_list[i];
            getProcInfo(&monitorInfoList, &mapCpuTimes, pidCur, totalDifference);
        }
        g_free(pid_list);
        saveToFile(cpuProportion, m_memProportion, monitorInfoList);
    });
    timer.start();

    exec();
}

int Monitor::getCpuCount()
{
    const glibtop_sysinfo *info = glibtop_get_sysinfo();
    int ncpu = info->ncpu;
    if (ncpu < 1) {
        qDebug() << "获取cpu个数失败！";
        ncpu = 1;
    }
    return ncpu;
}

void Monitor::saveToFile(int cpuInt, int memInt, const MonitorInfoList &list)
{
    QString fileName;
    QString fileName1 = QString(OUTPUT_FILE_PATH) + "log.1";
    QString fileName2 = QString(OUTPUT_FILE_PATH) + "log.2";
    if (m_output) {
        fileName = fileName2;
    } else {
        fileName = fileName1;
    }
    QIODevice::OpenModeFlag flag = QIODevice::Append;
    if (QFileInfo(fileName).size() > 1024 * 1024) {
        m_output = !m_output;
        if (m_output) {
            fileName = fileName2;
        } else {
            fileName = fileName1;
        }
        flag = QIODevice::Truncate;
        emit outputChange(m_output);
    };
    QString nowTime = QDateTime::currentDateTime().toString("yyyy-MM-dd.hh:mm:ss");
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | flag)) {
        qDebug() << "打开文件失败:" << fileName << nowTime;
        return;
    }
    QByteArray interlayer = " | ";
    QByteArray arr;
    qDebug() << "写入文件:" << fileName << nowTime;
    if (list.isEmpty()) {
        arr.append(nowTime + interlayer + "cpu:" + QByteArray::number(cpuInt) + "%  mem:" + QByteArray::number(memInt) + "%\n");
        file.write(arr);
        file.close();
        return;
    }
    arr = "===================" + nowTime.toLocal8Bit() + "===================\n";

    arr.append("cpuNum:" + QString::number(int(m_cpunum)) + "  cpu:" + QByteArray::number(cpuInt) + "%     memTotal:" + m_memTotal
               + "  mem:" + QByteArray::number(memInt) + "%\n");
    arr.append("_________________________________________________________\n");
    arr.append(format("PID") + interlayer + format("PPID") + interlayer + format("CPU") + interlayer + format("RES") + interlayer + format("CMD")
               + "\n");
    for (const MonitorInfo &info : list) {
        unsigned cpuIntTmp = info.cpu;
        unsigned memIntTmp = info.memory / 1024 / 1024;
        if (cpuIntTmp == 0 && memIntTmp == 0) {
            continue;
        }
        arr.append(format(QByteArray::number(info.mypid)));
        arr.append(interlayer + format(QByteArray::number(info.ppid)));
        arr.append(interlayer + format(QByteArray::number(cpuIntTmp) + "%"));
        arr.append(interlayer + format(QByteArray::number(memIntTmp) + "M"));
        arr.append(interlayer + info.cmd);
        arr.append("\n");
    }
    arr.append("_________________________________________________________\n");
    file.write(arr);
    file.close();
}

QString Monitor::format(const QString &str, int f)
{
    QString tmp = str;
    while (tmp.length() < f) {
        tmp += " ";
    }
    return tmp;
}

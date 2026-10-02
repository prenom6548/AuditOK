#ifndef SYSTEMMONITOR_H
#define SYSTEMMONITOR_H

#include <QObject>
#include <QThread>
#include <QDBusConnection>
#include <glibtop/cpu.h>
#include "kom-configure.h"
namespace systemmonitor
{

struct Conf
{
    Conf(int *t, int *c, int *m, bool o) : timer(t), cpuThreshold(c), memoryThreshold(m), output(o) {}
    int *timer;
    int *cpuThreshold;
    int *memoryThreshold;
    bool output;
};

class MonitorConf
{
protected:
    int *m_timer = nullptr;
    int *m_cpuThreshold = nullptr;
    int *m_memoryThreshold = nullptr;
    bool m_output;
};

struct MonitorInfo
{
    QByteArray cmd;
    pid_t mypid;
    double cpu;
    quint64 memory;
    unsigned state;     // process state
    pid_t ppid;         // parent process
    pid_t gid;          // process group id
    quint64 start_time; // start time since system boot in clock ticks
    quint64 processor;  // cpu number
    quint64 nthreads;   // number of threads
    int nice;           // process nice
};
typedef QList<MonitorInfo> MonitorInfoList;


class Monitor : public QThread, public MonitorConf
{
    Q_OBJECT
signals:
    void outputChange(bool);

public:
    Monitor(Conf c);
    void run();

private:
    int getCpuCount();
    void holdOn();
    void saveToFile(int cpuInt, int memInt, const MonitorInfoList &list = MonitorInfoList());
    QString format(const QString &str, int f = 7);
    bool PassDetection();
    void getProcInfo(MonitorInfoList *monitorInfoList, QMap<pid_t, quint64> *mapCpuTimes, pid_t pidCur, quint64 totalDifference);
    glibtop_cpu m_cpuTotal;
    double m_cpunum;
    double m_memProportion;
    QString m_memTotal;
};

class SystemMonitor : public QObject, public MonitorConf
{
    Q_OBJECT
public:
    static SystemMonitor *getInstance();

public slots:
    QString getConf() const;
    void setTimer(int t);
    void setCpuThreshold(int c);
    void setMemoryThreshold(int m);
private slots:
    void outputChange(bool b);

private:
    SystemMonitor();
    static SystemMonitor *m_systemMonitor;
    Monitor *m_monitor = nullptr;
    kom::Configure *m_setting = nullptr;
};
} // namespace systemmonitor
#endif // SYSTEMMONITOR_H

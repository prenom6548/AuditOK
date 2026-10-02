#ifndef KYLIN_OS_MANAGER_DAEMONS_SERVICE_PERFORMANCE_MONITOR_H
#define KYLIN_OS_MANAGER_DAEMONS_SERVICE_PERFORMANCE_MONITOR_H

#include <QObject>
#include <QTimer>
#include "defines.h"

/**
 * @note 信号
 *       MemoryExceedsThreshold - 内存超出阈值
 */
class PerformanceMonitor: public QObject {
    Q_OBJECT
public:
    PerformanceMonitor(const PerformanceMonitorConf &performance_monitor_conf);
    ~PerformanceMonitor();

    void Start();

private Q_SLOTS:
    void on_MemoryThresholdMonitor();

private:
    QTimer timer_;
    const PerformanceMonitorConf &conf_;

};

#endif

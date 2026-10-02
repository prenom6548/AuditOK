#ifndef KYLIN_OS_MANAGER_DAEMONS_SERVICE_DEFINES_H
#define KYLIN_OS_MANAGER_DAEMONS_SERVICE_DEFINES_H

#include <QString>

struct PerformanceMonitorConf {
    PerformanceMonitorConf(): enable(false), threshold_percentage_mode(true), threshold(100), frequency(1800), prompt_content(""){}

    bool enable;                       // 是否启用
    bool threshold_percentage_mode;    // 阈值是否是百分百模式
    int threshold;                     // 阈值
    int frequency;                     // 监控频率，单位秒
    QString prompt_content;            // 提示内容
};

struct SystemServiceConf {
    PerformanceMonitorConf performance_monitor_conf;
};

#endif

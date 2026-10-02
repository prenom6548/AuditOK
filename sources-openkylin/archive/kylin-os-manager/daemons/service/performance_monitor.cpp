#include <QFile>
#include <QDebug>
#include <QDBusMessage>
#include <QDBusConnection>
#include "performance_monitor.h"

static const QString DBUS_PATH = "/com/KylinOsManager/PerformanceMonitor";
static const QString DBUS_INTERFACE = "com.KylinOsManager.PerformanceMonitor";

PerformanceMonitor::PerformanceMonitor(const PerformanceMonitorConf &performance_monitor_conf)
    : conf_(performance_monitor_conf)
{

}

PerformanceMonitor::~PerformanceMonitor() {

}

void PerformanceMonitor::Start() {
    if (conf_.enable) {
        timer_.setInterval(conf_.frequency * 1000);
        connect(&timer_, &QTimer::timeout, this, &PerformanceMonitor::on_MemoryThresholdMonitor, Qt::ConnectionType::UniqueConnection);
        timer_.start();
        qInfo() << "[Performance monitor]: Start memory threshold monitoring, monitoring frequency " << conf_.frequency << " seconds";
    }
}

void PerformanceMonitor::on_MemoryThresholdMonitor() {
    // MemTotal - MemFree - Buffers - Cached - SReclaimable
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "[Performance monitor][Memory threshold monitor]: Failed to retrieve memory information";
        return;
    }
    auto data = file.readAll();
    file.close();

    auto extract_value = [](QString line) -> int {
        auto elementes = line.split(':');
        if (elementes.size() < 2) {
            return -1;
        }
        auto e = elementes.at(1);
        auto value = e.remove("kB").trimmed();

        bool ok{false};
        auto num = value.toInt(&ok);
        if (ok) {
            return num;
        }

        return -1;
    };

    int kb_mem_total{-1};
    int kb_mem_free{-1};
    int kb_buffers{-1};
    int kb_cached{-1};
    int kb_sreclaimable{-1};
    auto lines = QString::fromLocal8Bit(data).split('\n');
    for (const auto &line: lines) {
        if (line.startsWith("MemTotal:")) {
            auto num = extract_value(line);
            if (num >= 0) {
                kb_mem_total = num;
            } else {
                qCritical() << "[Performance monitor][Memory threshold monitor]: Failed to retrieve the total memory size, " << line;
            }
        } else if (line.startsWith("MemFree:")) {
            auto num = extract_value(line);
            if (num >= 0) {
                kb_mem_free = num;
            } else {
                qCritical() << "[Performance monitor][Memory threshold monitor]: Failed to retrieve the free memory size, " << line;
            }
        } else if (line.startsWith("Buffers:")) {
            auto num = extract_value(line);
            if (num >= 0) {
                kb_buffers = num;
            } else {
                qCritical() << "[Performance monitor][Memory threshold monitor]: Failed to retrieve the buffers size, " << line;
            }
        } else if (line.startsWith("Cached:")) {
            auto num = extract_value(line);
            if (num >= 0) {
                kb_cached = num;
            } else {
                qCritical() << "[Performance monitor][Memory threshold monitor]: Failed to retrieve the cached size, " << line;
            }
        } else if (line.startsWith("SReclaimable:")) {
            auto num = extract_value(line);
            if (num >= 0) {
                kb_sreclaimable = num;
            } else {
                qCritical() << "[Performance monitor][Memory threshold monitor]: Failed to retrieve the sreclaimable size, " << line;
            }
        }
    }

    // kb_mem_total 需要作为计算内存占比的分母，所以不可为 0
    if (kb_mem_total <= 0 || kb_mem_free == -1 || kb_buffers == -1 || kb_cached == -1 || kb_sreclaimable == -1) {
        return;
    }

    auto kb_used_memory = kb_mem_total - kb_mem_free - kb_buffers - kb_cached - kb_sreclaimable;

    qInfo() << "[Performance monitor][Memory threshold monitor]: MemTotal: " << kb_mem_total
            << " MemUsed: " << kb_used_memory
            << " MemFree: " << kb_mem_free
            << " Buffers: " << kb_buffers
            << " Cached: " << kb_cached
            << " SReclaimable: " << kb_sreclaimable;

    // 判断是否超出阈值
    bool exceed_threshold{false};
    if (conf_.threshold_percentage_mode) {
        // 百分比模式
        int percentage = static_cast<double>(kb_used_memory) / kb_mem_total * 100;
        if (percentage > conf_.threshold) {
            exceed_threshold = true;
        }
    } else {
        if (kb_used_memory > conf_.threshold) {
            exceed_threshold = true;
        }
    }

    if (exceed_threshold) {
        auto message = QDBusMessage::createSignal(DBUS_PATH, DBUS_INTERFACE, "MemoryExceedsThreshold");
        QDBusConnection::systemBus().send(message);
    }
}

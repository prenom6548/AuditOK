#include <QCoreApplication>
#include <QThread>
#include <QDebug>
#include <QSettings>
#include <libkylog.h>
#include "kom-utils.h"
#include "dbus.h"
#include "disk_monitor.h"
#include "defines.h"

#define SECURITY_LIB "/usr/lib/kysdk/kysdk-security/libkyprocesssec.so"

typedef int (*KdkProcessEnableNetworkingFunc)(const char *filepath);

SystemServiceConf g_system_service_conf;

static void logOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QByteArray localMsg = msg.toLocal8Bit();
    const char *message = localMsg.constData();
    const char *file = context.file ? context.file : "";
    const char *func = context.function ? context.function : "";

    switch (type) {
        case QtDebugMsg:
            klog_debug("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        case QtInfoMsg:
            klog_info("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        case QtWarningMsg:
            klog_warning("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        case QtCriticalMsg:
            klog_err("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        case QtFatalMsg:
            klog_emerg("%s (%s:%u,%s)\n", message, file, context.line, func);
            break;
        default:
            break;
    }
}

void ParseSystemServiceConf() {
    QSettings settings("/etc/kylin-os-manager/kylin-os-manager-plugin.ini", QSettings::Format::IniFormat);

    settings.beginGroup("SystemService");
    // 是否开启内存阈值监控
    auto memory_threshold_monitor_enable = settings.value("MemoryThresholdMonitor_Enable");
    if (!memory_threshold_monitor_enable.isNull()) {
        auto enable = memory_threshold_monitor_enable.toString();
        if (enable == "true") {
            g_system_service_conf.performance_monitor_conf.enable = true;
        } else {
            g_system_service_conf.performance_monitor_conf.enable = false;
        }
    }
    // 内存阈值监控 - 阈值
    auto memory_threshold_monitor_threshold = settings.value("MemoryThresholdMonitor_Threshold");
    if (!memory_threshold_monitor_threshold.isNull()) {
        auto threshold = memory_threshold_monitor_threshold.toString();
        if (threshold.contains('%')) {
            // 百分比模式
            threshold.remove('%');
            bool ok{false};
            auto num = threshold.toInt(&ok);
            if (ok) {
                g_system_service_conf.performance_monitor_conf.threshold_percentage_mode = true;
                g_system_service_conf.performance_monitor_conf.threshold = num;
            } else {
                qCritical() << "Parse system service conf memory threshold monitor threshold percentage convert int fail";
            }
        } else {
            // 数值模式
            bool ok{false};
            auto num = threshold.toInt(&ok);
            if (ok) {
                g_system_service_conf.performance_monitor_conf.threshold_percentage_mode = false;
                g_system_service_conf.performance_monitor_conf.threshold = num;
            } else {
                qCritical() << "Parse system service conf memory threshold monitor threshold convert int fail";
            }
        }
    }
    // 内存阈值监控 - 频率
    auto memory_threshold_monitor_frequency = settings.value("MemoryThresholdMonitor_Frequency");
    if (!memory_threshold_monitor_frequency.isNull()) {
        auto frequency = memory_threshold_monitor_frequency.toString();
        bool ok{false};
        auto num = frequency.toInt(&ok);
        if (ok) {
            g_system_service_conf.performance_monitor_conf.frequency = num;
        } else {
            qCritical() << "Parse system service conf memory threshold monitor frequency convert int fail, use default frequency 1800 seconds";
            g_system_service_conf.performance_monitor_conf.frequency = 1800;
        }
    }
    // 内存阈值监控 - 提示内容
    auto memory_threshold_monitor_prompt_content = settings.value("MemoryThresholdMonitor_PromptContent");
    if (!memory_threshold_monitor_prompt_content.isNull()) {
        auto prompt_content = memory_threshold_monitor_prompt_content.toString();
        g_system_service_conf.performance_monitor_conf.prompt_content = prompt_content;
    }
    settings.endGroup();
}

int main(int argc, char *argv[]) {
    qInstallMessageHandler(logOutput);

    QCoreApplication app(argc, argv);

    // 添加安全中心网络控制白名单
    kom::KomUtils::setNetworkWhiteListFromPackage(kom::KomUtils::Service);

    ParseSystemServiceConf();

    dbus::DbusManager dbusManager;

    // 创建线程，启动磁盘监控
    monitor::DiskMonitor disk_monitor;
    QThread disk_monitor_thread;
    disk_monitor.moveToThread(&disk_monitor_thread);
    QObject::connect(&disk_monitor_thread, &QThread::started, &disk_monitor, &monitor::DiskMonitor::on_Run);
    disk_monitor_thread.start();

    qInfo() << "Service started ...";

    return app.exec();
}

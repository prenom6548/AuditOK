#ifndef KYLIN_OS_MANAGER_DAEMONS_SERVICE_JUNK_CLEAN_INTERFACE_H
#define KYLIN_OS_MANAGER_DAEMONS_SERVICE_JUNK_CLEAN_INTERFACE_H

#include <mutex>
#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QList>
#include <QMap>
#include <QDir>

#define JUNK_CLEAN_DBUS_NAME "com.kylin-os-manager"
#define JUNK_CLEAN_DBUS_PATH "/com/KylinOsManager/JunkClean"
#define JUNK_CLEAN_DBUS_INTERFACE "com.KylinOsManager.JunkClean"

namespace junk_clean {

class JunkCleanAdaptor : public QObject {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.KylinOsManager.JunkClean")
public:
    JunkCleanAdaptor(QObject *obj);

Q_SIGNALS:
    void ScanForJunk(QString cleaner_mark, quint64 junk_mark, QString path, quint64 size);
    void ScanFinish(QString cleaner_mark, quint64 total_size);
    void CleanForJunk(QString cleaner_mark, quint64 junk_mark);
    void CleanFinish(QString cleaner_mark);

public Q_SLOTS:
    // 用于磁盘监控功能，记录大小异常的日志文件是否已经被清理过
    void RecordCleanedFiles(QStringList cleaned_files);

    // 安装包: installation_package_cleaner
    void ScanInstallationPackage();
    void CleanInstallationPackage(QList<quint64> junk_marks);

    // 安装包卸载残留: residual_cleaner
    void ScanUninstallResidual();
    void CleanUninstallResidual(QList<quint64> junk_marks);

    // 释放系统内存: system_memory_cleaner
    void CleanSystemMemory();

    // boot 分区: boot_partition_cleaner
    void ScanBootPartition();
    void CleanBootPartition(QList<quint64> junk_marks);

    // 系统日志
    void ScanLog();
    void CleanLog(QList<quint64> junk_marks);

private:
    void EmitScanForJunkSignal(QString cleaner_mark, quint64 junk_mark, QString path, quint64 size) const;
    void EmitScanFinishSignal(QString cleaner_mark, quint64 total_size) const;
    void EmitCleanForJunk(QString cleaner_mark, quint64 junk_mark) const;
    void EmitCleanFinish(QString cleaner_mark) const;
    void ScanSystemLog(const QDir &dir);

    std::mutex installation_package_mutex_;
    QMap<quint64, QString> installation_package_junk_;

    std::mutex uninstall_residual_mutex_;
    QMap<quint64, QString> uninstall_residual_junk_;

    std::mutex boot_partition_mutex_;
    QMap<quint64, QString> boot_partition_junk_;

    std::mutex log_mutex_;
    quint64 log_junk_mark_;
    quint64 log_junk_total_size_;
    QMap<quint64, QString> log_junk_;
};

}

Q_DECLARE_METATYPE(QList<quint64>)

#endif
